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

typedef struct {
    char letter;
    int duration;   // duration in dits
} morse_table_t;

// Based upon a 50 dot duration standard word such as PARIS, the time for one dit
//   duration or one unit can be computed by the formula:
//   T = 1200 / Speed(wpm) where T is the dit duration.
//         11   3  5  3  7  3  3   3  5   7  = 50
//   PARIS  .__.    ._    ._.    ..    ...

const static morse_table_t valid_keys_timing[] = {
{'A', 8},     // ._  1 dit, 1 intercharacter, 3 dah, 3 after char (inter char spacing)
{'B', 12},    // _...
{'C', 14},    //_._.
{'D', 10},    // _..
{'E', 4},     // .
{'F', 12},    // .._.
{'G', 12},    // __.
{'H', 10},    // ....
{'I', 6},     // ..
{'J', 16},    // .___
{'K', 12},    // _._
{'L', 12},    // ._..
{'M', 10},    // __
{'N', 8},     // _.
{'O', 14},    // ___
{'P', 14},    // .__.
{'Q', 16},    // __._
{'R', 10},    // ._.
{'S', 9},     // ...
{'T', 6},     // _
{'U', 10},    // .._
{'V', 12},    // ..._
{'W', 12},    // .__
{'X', 14},    // .__.
{'Y', 16},    // _.__
{'Z', 14},    // __..
{'1', 20},    // .____
{'2', 18},    // ..___
{'3', 16},    // ...__
{'4', 14},    // ...._
{'5', 12},    // .....
{'6', 14},    // _....
{'7', 16},    // __...
{'8', 18},    // ___..
{'9', 20},    // ____.
{'0', 22},    // _____
{'?', 18},    // ..__..
{'.', 20},    // ._._._
{' ', 7},     // space is 7 elements
{'=', 16},    // _..._  Prosign BT
{'+', 16},    // ._._.  Prosign AR
{'%', 14},    // ._...  Prosign AS
{'*', 18},    // ..._._  Prosign SK
{',', 22},    // __..__
{'/', 16}     // _.._.
};


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
    void mrr_get_ic_config(char *p);
    void mrr_set_tx_test();
    void mrr_set_band(int band);
    int mrr_get_band();
    void mrr_a_2_b();

public slots:
    int bwidth_change_request(int up_or_down);
    void mrrSetTune(bool on);
    float read_rig_swr();
    void mrrSetRx();
    int txCW_Char(char c);

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

};

#endif // HAMLIBCONNECTOR_H
