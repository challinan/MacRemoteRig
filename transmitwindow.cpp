#include "transmitwindow.h"

#define ENABLE_TX_THREAD

QSemaphore highlightTextSem(1);
QSemaphore bufferSem(1);

CBuffer::CBuffer() {
    head_index = 0;
    tail_index = 0;
    count = 0;
    // For debug only
    for (int i=0; i< (int) sizeof(cbuff); i++) cbuff[i] = '\0';
}

bool CBuffer::put(char *c) {

    // Check for buffer full
    if ( count == sizeof(cbuff) ) {
        // Buffer full
        return false;
    }

    // We write to "tail" and read from "head"
    count++;
    cbuff[tail_index++] = *c;
    tail_index %= sizeof(cbuff);
    return true;
}

char CBuffer::get() {
    if ( count == 0 ) {
        // Buffer empty
        return -1;
    }
    count--;

    // We write to "tail" and read from "head"
    char c = cbuff[head_index++];
    head_index %= sizeof(cbuff);
    return c;
}

void CBuffer::clear() {
    head_index = 0;
    tail_index = 0;
    count = 0;
}

bool CBuffer::isEmpty() {
    return (head_index == tail_index);
}

bool CBuffer::deleteLast() {
    if ( isEmpty() )
        return false;

    qDebug() << "CBuffer::deleteLast(): head_index:" << head_index << " | Head char" << cbuff[head_index] << "| Tail index" << tail_index << "tail char" << cbuff[tail_index-1];
    if ( tail_index == 0 ) {
        tail_index = sizeof(cbuff) - 1;
    } else {
        tail_index--;
    }
    count--;
    return true;
}

TransmitWindow::TransmitWindow(QMainWindow *parent, HamlibConnector *phamlib)
        : QTextEdit(parent)
{

    hamlib_p = phamlib;
    setGeometry(QRect(325, 260, 600, 100));
    show();
    qDebug() << "TransmitWindow::TransmitWindow(): this object:" << this;
    setPlaceholderText("Transmit here");

    // Create a cursor object at the beginning of our text window
    cursor = textCursor();
    setTextCursor(cursor);
    qDebug() << "TransmitWindow::TransmitWindow(): cursor now at" << cursor.position();

    // Create a format object for "normal" text and capture it's properties
    norm.setFontStrikeOut(false);
    norm.setForeground(QBrush(Qt::blue));
    norm.setFontPointSize(16);
    setCurrentCharFormat(norm);

    // Create a Format Object to highlight characters that have been transmitted already
    f.setFontStrikeOut(true);
    f.setForeground(QBrush(Qt::red));
    f.setFontPointSize(16);

    tpos.block = 1;
    tpos.position = 0;
    tx_position = 0;
    last_size = 0;
    is_transmitting = false;
    key_count = 0;
    // For debug only
    debug_count = 0;

    // Initialize circular buffer
    ccbuf.clear();

    // Start a thread to handle transmit characters asynchronously
#ifdef ENABLE_TX_THREAD
    tx_thread_p = new CWTX_Thread(this);
    tx_thread_p->cbuf_p = &ccbuf;
    tx_thread_p->start();
#endif

    // Connect signals
    connect(this, &QTextEdit::textChanged, this, &TransmitWindow::processTextChanged);
    // connect(this, &QTextEdit::blockCountChanged, this, &TransmitWindow::updateBlockCount);
    connect(this, &QTextEdit::cursorPositionChanged, this, &TransmitWindow::CursorPositionChangedSlot);
#ifndef SKIP_RIG_INIT
    qDebug() << "TransmitWindow::TransmitWindow(): ******************* hamlib_p =" << hamlib_p;
    connect(hamlib_p, &HamlibConnector::pauseTxSig, tx_thread_p, &CWTX_Thread::pauseTx);
#endif
#ifdef ENABLE_TX_THREAD
    connect(this, &TransmitWindow::startTx, tx_thread_p, &CWTX_Thread::startStopTX);
    connect(tx_thread_p, &CWTX_Thread::deQueueChar, this, &TransmitWindow::markCharAsSent);
    connect(tx_thread_p, &CWTX_Thread::txChar, this, &TransmitWindow::txCharTransmitComplete);
#endif

}

TransmitWindow::~TransmitWindow() {
#ifdef ENABLE_TX_THREAD
    tx_thread_p->terminate();
    tx_thread_p->wait();
    delete tx_thread_p;
#endif
}

void TransmitWindow::keyReleaseEvent(QKeyEvent *event) {

    CBuffer &bf = ccbuf;

    if ( event->key() == Qt::Key_Backslash ) {
        return;   // backslash key
    }

    debug_count++;
    highlightTextSem.acquire();
    moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    setCurrentCharFormat(norm);
    highlightTextSem.release();

    // Suppoprt for the delete key
    if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace ) {
        qDebug() << "TransmitWindow::keyReleaseEvent(): Backspace: text window size:" << toPlainText().size() << "tx_position" << tx_position;
        if ( toPlainText().size() > tx_position ) {
            bufferSem.acquire();
            bool rc = bf.deleteLast();
            bufferSem.release();
            if ( rc == false ) {
                qDebug() << "TransmitWindow::keyReleaseEvent(): could not remove end of buffer on backspace";
                QApplication::exit(8);
            }
            int i = toPlainText().size();
            if ( i > 0 ) {
                setText(toPlainText().left(i));
                moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            }
            else
                qDebug() << "TransmitWindow::keyReleaseEvent(): can't backspace";
        }
    }

    QTextEdit::keyReleaseEvent(event);
}

void TransmitWindow::keyPressEvent(QKeyEvent *event) {
    const static char valid_keys[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',\
                                      'V','W','X','Y','Z','1','2','3','4','5','6','7','8','9','0',';','?','.',' ','=','+','%','*','<','>',','};

    static int key_loop = 0;
    int i;
    char c;
    int key = event->key();
    bool b;

    CBuffer &bf = ccbuf;

    if ( key == Qt::Key_Escape) {
        clear();
        tpos.position = 0;      // clear() sets block back to its initial value of 1
        tpos.block = 1;
        last_size = 0;
        tx_position = 0;
        key_count = 0;
        debug_count = 0;
        bufferSem.acquire();
        bf.clear();      // Clear our circular buffer
        bufferSem.release();
        key_loop = 0;
        emit startTx(false);

        // Set Rig to RX immediately here
    }

    if ( key == Qt::Key_Slash ) {
        qDebug() << "SLASH" << "key_loop" << key_loop << "position" << cursor.position() << "anchor" << cursor.anchor();
        cursor.setPosition(4, QTextCursor::MoveAnchor);
        qDebug() << "Anchor now at" << cursor.anchor();
        b = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 5);
        qDebug() << "Position now at" << cursor.position();
        if ( b == false ) {
            qDebug() << "KeyEvent(): cursor.movePosition() failed1:" << "cursor anchor" << cursor.anchor();
            return;
        }
        cursor.setCharFormat(f);
        key_loop++;
        return;
    }

    if ( key == Qt::Key_BracketLeft ) {
        qDebug() << "[[[[[" << "key_loop" << key_loop << "position" << cursor.position() << "anchor" << cursor.anchor();
        cursor.setPosition(11, QTextCursor::MoveAnchor);
        qDebug() << "Anchor now at" << cursor.anchor();
        b = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 15);
        qDebug() << "Position now at" << cursor.position();
        if ( b == false ) {
            qDebug() << "KeyEvent(): cursor.movePosition() failed2:" << "cursor anchor" << cursor.anchor();
            return;
        }
        cursor.setCharFormat(f);
        key_loop++;
        return;
    }

    if ( key == Qt::Key_BracketRight ) {
        qDebug() << "]]]]]" << "key_loop" << key_loop << "position" << cursor.position() << "anchor" << cursor.anchor();
        cursor.setPosition(43, QTextCursor::MoveAnchor);
        qDebug() << "Anchor now at" << cursor.anchor();
        b = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        qDebug() << "Position now at" << cursor.position();
        if ( b == false ) {
            qDebug() << "KeyEvent(): cursor.movePosition() failed3:" << "cursor anchor" << cursor.anchor();
            return;
        }
        cursor.setCharFormat(f);
        return;
    }

    if ( key == Qt::Key_Backslash ) {
        qDebug() << R"(\\\\\)" << "text window size" << toPlainText().size() << "debug count" << debug_count << "tx_position" << tx_position;
        return;
    }

    // Allow the Delete key
    if ( key == Qt::Key_Delete || key == Qt::Key_Backspace ) {
        goto eventDone;
    }

    if ( key == Qt::Key_Return || key == Qt::Key_Shift ) {
        goto eventDone;
    }

    c = (char) key;
    for (i=0; i< (int)sizeof(valid_keys); i++) {
        if ( c == valid_keys[i]) {
            break;
        }
    }

    if (i == sizeof(valid_keys) ) return;       // No valid key found

    bufferSem.acquire();
    b = bf.put(&c);
    bufferSem.release();
    // Is the buffer full?
    if ( b == false ) {
        QApplication::beep();
        // Ignore this character - buffer is full
        return;
    }
    emit startTx(true);

    // Reset the text formatting
    // At this point, the character reported by this event hasn't hit the text window yet.
    // So size() will return zero on the first character.  Duh!
    // So we'll let the KeyReleased() event do the unhighlighting
    key_count++;

eventDone:
    QTextEdit::keyPressEvent((event));
}

void TransmitWindow::processTextChanged() {

    int s = toPlainText().size();
    // qDebug() << "TransmitWindow::processTextChanged(): size =" << s << "last_size" << last_size;
    if ( s > last_size ) {
        last_size = s;
        emit startTx(true);
    }
}

void TransmitWindow::updateBlockCount(int count) {
    tpos.block = count;
    qDebug() << "TransmitWindow::updateBlockCount(): new block count" << count;
}

void TransmitWindow::markCharAsSent(char c) {
#ifdef ENABLE_TX_THREAD

    (void)c;    // Suppress unused warning
    int size = toPlainText().size();
    // Did we get a clear() event (ESC)?
    if ( size == 0 ) return;

    tx_position++;
    highlightTextSem.acquire();

    // Higlight the character as it is processed from the transmit window
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    bool b = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, tx_position);
    if ( b == false ) {
        qDebug() << "TransmitWindow::markCharAsSent(): cursor.movePosition() failed5:" << "cursor anchor" << cursor.anchor();
        highlightTextSem.release();
        return;
    }
    cursor.setCharFormat(f);
    highlightTextSem.release();
    qDebug() << "TransmitWindow::mackCharAsSent(): debug count =" << debug_count << "tx_position =" << tx_position;
#endif
}

void TransmitWindow::mousePressEvent(QMouseEvent *event) {
    // Disable mouse click events in the transmit window
    // This essentially discards the mouse click event
    (void)event; // Supress warning about unused variable 'event'
    qDebug() << "CLICK" << "position" << cursor.position() << "anchor" << cursor.anchor();
    QTextEdit::mousePressEvent(event);
}

void TransmitWindow::txCharTransmitComplete(char c) {

    // Send character to rig
    hamlib_p->txCW_Char(c);
}

void TransmitWindow::CursorPositionChangedSlot() {
    // int size = toPlainText().length();
    // qDebug() << "TransmitWindow::cursorPositionChangedSlot(): position:" << cursor.position();
}

void TransmitWindow::setHamlibPointer(HamlibConnector *p) {
    hamlib_p = p;
}

// ***********************************************************************
// *********************   QThread Class *********************************
// ***********************************************************************

CWTX_Thread::CWTX_Thread(TransmitWindow *p) {
    transmitNow = false;
    txwinObj_p = p;
    paused = false;
}

void CWTX_Thread::run() {

    char c;
    CBuffer &b = *cbuf_p;

    while ( true ) {
        if ( transmitNow && !paused ) {
            // qDebug() << "CWTX_Thread::run(): ***transmitNow*** signal arrived";
            bufferSem.acquire();
            c = b.get();
            bufferSem.release();
            if ( c == -1 ) {
                // Buffer is empty
                transmitNow = false;
                qDebug() << "WTX_Thread::run(): buffer empty";
                continue;  // Is this what we want to do here?
            }
            // qDebug() << QDateTime::currentMSecsSinceEpoch() << "CWTX_Thread::run(): deQueueing char" << c;
            emit deQueueChar(c);
#ifdef SKIP_RIG_INIT
            QThread::msleep(50);
#endif
#ifndef SKIP_RIG_INIT
            emit txChar(c);
#endif
        }
        QThread::msleep(10);
    }
}

void CWTX_Thread::startStopTX(bool start) {
    // qDebug() << QDateTime::currentMSecsSinceEpoch() << "CWTX_Thread::startStopTX() signal received:" << start;
    if ( start == true )
        transmitNow = true;
    else
        transmitNow = false;
}

void CWTX_Thread::pauseTx(bool pause) {

    qDebug() << "CWTX_Thread::pauseTx(): pause =" << pause;
    paused = pause;
}
