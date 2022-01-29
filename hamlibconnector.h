#ifndef HAMLIBCONNECTOR_H
#define HAMLIBCONNECTOR_H

#include <QObject>
#include <QThread>
#include "hamlib/rig.h"
#include <iostream>
#include <iomanip>
#include "ui_mainwindow.h"
#include "spot_delayworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define MAXCONFLEN 1024
#define TOKEN_FRONTEND(t) ((t)|(1<<30))
#define BANDWIDTH_STEP 20

class HamlibConnector : public QObject
{
    Q_OBJECT
public:
    explicit HamlibConnector(QObject *parent = nullptr);

public:
    freq_t mrr_getRigFrequency(vfo_t vfo);
    void store_ui_pointer(Ui::MainWindow *p);
    int get_SMeter_progbar_value(int x);
    int read_rig_strength();
    static QString get_display_frequency(freq_t f);
    int set_rig_freq(freq_t f);
    vfo_t mrr_getVFO(void);
    freq_t mrr_getCurrentFreq_A();
    void mrr_setCurrentFreq_A(freq_t f);
    int get_retcode(void);
    mode_t mrr_get_mode();
    int mrr_set_mode(mode_t mode);
    int mrr_set_level(setting_t level, value_t v);
    pbwidth_t mrr_get_width();
    void setSpot();
    void setSwapAB();
    const char *mrr_getModeString(mode_t mode);
    void abortTX();
    int getCwSpeed();
    int bumpCwSpeed(bool up);
    void setPauseTx(bool checked);

public slots:
    int bwidth_change_request(int up_or_down);
    void mrrSetTune(int on);
    float read_rig_swr();
    void mrrSetRx();
    void txCW_Char(char c);

private:
    RIG *my_rig;        /* handle to rig (instance) */
    //  const char *rig_file = "localhost"; /* Change this for real network useage */
    const char *rig_file = "imac-wifi";
    rig_model_t my_model;
    int retcode;
    rig_debug_level_e verbose = RIG_DEBUG_NONE;
    vfo_t current_vfo;
    freq_t current_freq_a;  // This is a type:double
    freq_t current_freq_b;
    float frequency;
    Ui::MainWindow *ui_pointer;
    int strength;
    const QList<int> sMeter_cal = {-54, -48, -42, -30, -24, -18, -12, -6, 0, 10, 20, 30, 40, 50, 60};
    bool lockout_spot;
    SpotDelayWorker *spotDelayWorker_p;
    rmode_t current_mode;
    pbwidth_t current_pbwidth;
    const char *modeStr_p;
    int cw_speed;

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
    void cleanupSpotDelay();
    int get_rig_mode_and_bw();

signals:
    void updateWidthSlider(pbwidth_t w);
    void pauseTxSig(bool pause);

private slots:

};

#endif // HAMLIBCONNECTOR_H
