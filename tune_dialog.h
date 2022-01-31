#ifndef TUNEDIALOG_H
#define TUNEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QApplication>
#include <QThread>
#include <QDebug>
#include "mainwindow.h"
#include "hamlibconnector.h"

class TuneThread;
class MainWindow;

class TuneDialog : public QObject
{
    Q_OBJECT
public:
    explicit TuneDialog(QObject *parent = nullptr);
    ~TuneDialog();

private slots:
     void pb_ok_clicked(QAbstractButton *button);

private:
         QDialog *dp;
         QLabel *pLabel;
         QLabel *swrLabel;
         QDialogButtonBox *pbox;
         TuneThread *tp;
         QPushButton *close_pb;
         QPushButton *tune_pb;
         MainWindow *pMainWindow;
         int tuning;

public:
        HamlibConnector *pHamLib;

private slots:
    void accept_clicked();
    void tuneFinished();
    void updateSWR(float s);

signals:
    void setTune_sig(bool on);

};


class TuneThread : public QThread
{
    Q_OBJECT
public:
    explicit TuneThread();
    TuneThread(QDialog *p);
    virtual void run();

private:
    QDialog *pDialog;

public:
    HamlibConnector *pHamLib;

signals:
    void reportSWR_sig();
    void setTune_sig(int on);
    void updateSWR(float s);
};

#endif // TUNEDIALOG_H
