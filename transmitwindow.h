#ifndef TRANSMITWINDOW_H
#define TRANSMITWINDOW_H

#include <QObject>
#include <QApplication>
#include <QDebug>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QString>
#include <QThread>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QSemaphore>
#include <QDateTime>
#include "mainwindow.h"
#include "hamlibconnector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CWTX_Thread;

class CBuffer {
public:
    CBuffer();
    bool put(char *c);
    char get();
    void clear();
    bool isEmpty();
    bool deleteLast();

private:
    char cbuff[128];
    // We write to "tail" and read from "head"
    int head_index;
    int tail_index;
    int count;
};

class TransmitWindow : public QTextEdit
{
    friend class CWTX_Thread;
    Q_OBJECT
public:
    explicit TransmitWindow(QMainWindow *parent = nullptr, HamlibConnector *p = nullptr);
    ~TransmitWindow();
    void abortTxNow();
    int getCwSpeed();

private:
    void txReset();

public slots:
    void processTextChanged();
    void updateBlockCount(int count);
    void markCharAsSent(char c);
    void txCharTransmitComplete(char c);
    void CursorPositionChangedSlot();
    void setHamlibPointer(HamlibConnector *p);
    void updateRigCwSpeedSlot(int speed);

protected:
     void keyPressEvent(QKeyEvent *event) override;
     void mousePressEvent(QMouseEvent *event) override;
     void keyReleaseEvent(QKeyEvent *event) override;

private:
     // QString textEditBuffer;
     int tx_position;
     char key_count;
     int last_size;
     bool is_transmitting;
     QTextCursor cursor;
     QTextCharFormat f;
     QTextCharFormat norm;
     struct {
         int position;  // Transmit character position
         int block;  // Transmit block
     } tpos;
     CWTX_Thread *tx_thread_p;
     CBuffer ccbuf;
     HamlibConnector *hamlib_p;
     int cw_speed;
     // For debug only
     int key_release_count;
     int key_down_count;

signals:
     void startTx(bool start);

};

// *********   QThread Class *****************

class CWTX_Thread : public QThread
{
    Q_OBJECT;

public:
    CWTX_Thread(TransmitWindow *p);
    void run() override;
    CBuffer *cbuf_p;
    // For debug only
    int deQueue_count;

private:
    int calculate_delay(char c);

private:
    bool transmitNow;
    bool paused;
    TransmitWindow *txwinObj_p;
    int dit_timing_factor;

public slots:
    void startStopTX(bool start);
    void pauseTx(bool pause);

signals:
    void deQueueChar(char c);
    void txChar(char c);

};

#endif // TRANSMITWINDOW_H
