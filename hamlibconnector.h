#ifndef HAMLIBCONNECTOR_H
#define HAMLIBCONNECTOR_H

#include <QObject>
#include "hamlib/rig.h"
#include <iostream>
#include <iomanip>
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define MAXCONFLEN 1024
#define TOKEN_FRONTEND(t) ((t)|(1<<30))

class HamlibConnector : public QObject
{
    Q_OBJECT
public:
    explicit HamlibConnector(QObject *parent = nullptr);

private:
    RIG *my_rig;        /* handle to rig (instance) */
//    const char *rig_file = "localhost"; /* Change this for real network useage */
        const char *rig_file = "imac-wifi";
    rig_model_t my_model;
    int retcode;
    int verbose = 0;
    vfo_t vfo_a;
    freq_t freq_a;
    QString frequency;
    Ui::MainWindow *ui_pointer;

private:

public:
    QString &getFrequency();
    void store_ui_pointer(Ui::MainWindow *p);

#if 0
    static HamlibConnector& getInstance() {
            static HamlibConnector instance;
            return instance;
    }
#endif

public slots:
    void autoupdate_frequency();

};

#endif // HAMLIBCONNECTOR_H
