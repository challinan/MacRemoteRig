#include "tune_dialog.h"
#include "mainwindow.h"

TuneDialog::TuneDialog(QObject *parent)
    : QObject{parent}
{
    tp = nullptr;
    tuning = 0;

    pMainWindow = static_cast<MainWindow *>(parent);
    pHamLib = static_cast<MainWindow *>(pMainWindow)->getHamlibPointer();
    dp = new QDialog(static_cast<QWidget *>(parent));
    dp->setModal(true);
    dp->setMinimumSize(QSize(300, 100));
    pLabel = new QLabel("Click TUNE Button to Tune - Watch SWR", dp);
    pLabel->setGeometry(QRect(5,5,300,25));
    swrLabel = new QLabel("SWR: ", dp);
    swrLabel->setGeometry((QRect(25, 40, 200, 25)));
    pbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dp);

    pbox->setGeometry(50, 80, 150, 60);
    QPushButton *tb = pbox->button(QDialogButtonBox::Ok);
    tb->setText("TUNE");
    connect(pbox, &QDialogButtonBox::clicked, this, &TuneDialog::pb_ok_clicked);
    connect(pbox, &QDialogButtonBox::accepted, this, &TuneDialog::accept_clicked);

    // Connect signals
    connect(this, &TuneDialog::setTune_sig, pHamLib, &HamlibConnector::mrrSetTune);
    dp->exec();
}

void TuneDialog::pb_ok_clicked(QAbstractButton *button) {
    QString button_text = button->text();
    qDebug() << "TuneDialog::pb_ok_clicked(): entered";
    if ( button_text == "Cancel" ) {
        if ( tuning ) {
            emit setTune_sig(0);
            tuning = 0;
        }
        if ( tp ) {
            tp->terminate();
            tp->wait();
            delete tp;
            tp = nullptr;
        }
        dp->close();
     }
    return;
}

TuneDialog::~TuneDialog() {
    delete pbox; delete pLabel; delete dp;
}

void TuneDialog::accept_clicked() {
    qDebug() << "TuneDialog::accept_clicked(): entered";
    tp = new TuneThread(dp);
    connect(tp, &TuneThread::setTune_sig, pHamLib, &HamlibConnector::mrrSetTune);
    connect(tp, &TuneThread::finished, this,  &TuneDialog::tuneFinished);
    connect(tp, &TuneThread::updateSWR, this, &TuneDialog::updateSWR);
    MainWindow *pmw = static_cast<MainWindow *>(pMainWindow);
    tp->pHamLib = pmw->getHamlibPointer();
    tuning = 1;
    tp->start();
}

void TuneDialog::tuneFinished() {
    qDebug() << "TuneDialog::tuneFinished(): cleaning up";
    emit setTune_sig(0);
    tp->terminate();
    tp->wait();
    delete tp;
    tp = nullptr;
}

void TuneDialog::updateSWR(float s) {
    QString str = "SWR: ";
    QString str2;
    str2.setNum(s);

    swrLabel->setText(str + str2);
}
/*
 ****************** Tune Thread **************************
 */
TuneThread::TuneThread()
{

}

TuneThread::TuneThread(QDialog *p)
{
    qDebug() << "TuneThread::TuneThread() constructor entered";
    pDialog = p;
}


void TuneThread::run() {

    qDebug() << "TuneThread running";
    emit setTune_sig(1);
    for ( int i=0; i<1; i++ ) {
        qDebug() << "TuneThread::run(): calling read_rig_swr()";
        float swr = pHamLib->read_rig_swr();
        emit updateSWR(swr);
        // pDialog->swrLabel->setText("SWR: ");

        // pDialog->
        qDebug() << "Transmit SWR = " << swr;
        QThread::msleep(2000);
    }
}
