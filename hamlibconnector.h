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

public:
    // QString &getFrequency();
    freq_t &mrr_getFrequency(vfo_t vfo);
    void store_ui_pointer(Ui::MainWindow *p);
    int get_SMeter_progbar_value(int x);
    int read_rig_strength();
    static QString get_display_frequency(freq_t f);
    int set_rig_freq(freq_t f);
    vfo_t mrr_getVFO(void);
    freq_t mrr_getCurrentFreq_A();
    void mrr_setCurrentFreq_A(freq_t f);

private:
    RIG *my_rig;        /* handle to rig (instance) */
//    const char *rig_file = "localhost"; /* Change this for real network useage */
        const char *rig_file = "imac-wifi";
    rig_model_t my_model;
    int retcode;
    rig_debug_level_e verbose = RIG_DEBUG_NONE;
    vfo_t current_vfo_a;
    freq_t current_freq_a;  // This is a type:double
    // QString frequency;
    float frequency;
    Ui::MainWindow *ui_pointer;
    int strength;
    const QList<int> sMeter_cal = {-54, -48, -42, -30, -24, -18, -12, -6, 0, 10, 20, 30, 40, 50, 60};

private:

#if 0
    static HamlibConnector& getInstance() {
            static HamlibConnector instance;
            return instance;
    }
#endif

public slots:
    void autoupdate_frequency();
    void autoupdate_smeter();

};

#endif // HAMLIBCONNECTOR_H
