#ifndef HAMLIBCONNECTOR_H
#define HAMLIBCONNECTOR_H

#include <QObject>
#include <QThread>
#include <iostream>
#include <iomanip>
#include "hamlib/rig.h"
#include "config_object.h"
#include "spot_delayworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ConfigObject;

#define MAXCONFLEN 1024
#define TOKEN_FRONTEND(t) ((t)|(1<<30))
#define BANDWIDTH_STEP 10

typedef struct { int raw; QString s; } s_meter_cal_t;

typedef struct {
    int filter_number;
    const char *filter_bandwidth;
} xtal_filter_values_t;

const static xtal_filter_values_t xtal_filter_values[5] = { {1, "13KHz"}, {2, "2.7KHz"}, {3, "1.8KHz"}, {4, "500Hz"}, {5, "250Hz"} };

// K3 Response to "IF" command
struct rig_if_response {
    char fa[11];
    unsigned char pad1[5];
    char rit_offset_sign;
    char rit_offset[4];
    char rit_on;
    char xit_on;
    unsigned char pad2[3];
    char tx_active;
    char rig_mode;
    char rx_vfo;
    char scan_on;
    char split_on;
    unsigned char pad3;   // RSP on K3, this bit is always 0 on K3
    char k3_rsp;
    unsigned char pad4[2];
};

class HamlibConnector : public QObject
{
    Q_OBJECT
public:
    explicit HamlibConnector(QObject *parent = nullptr);
    freq_t mrr_getRigFrequency(vfo_t vfo);
    void store_ui_pointer(Ui::MainWindow *p);
    int get_SMeter_progbar_value(int x);
    int read_rig_strength();
    static QString get_display_frequency(freq_t f);
    int mrrSetRigFreqA(freq_t f);
    int mrrSetRigFreqB(freq_t f);
    vfo_t mrr_getVFO(void);
    freq_t mrrGetCachedFreqA();
    freq_t mrrGetCachedFreqB();
    void mrrSetCachedFreqA(freq_t f);
    void mrrSetCachedFreqB(freq_t f);
    int get_retcode(void);
    mode_t mrrGetMode();
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
    int mrrGetIcConfig(unsigned char *p);
    void mrr_set_tx_test();
    void mrr_set_band(int band);
    int mrr_get_band();
    void mrr_a_2_b();
    int mrrGetMonLevel();
    int mrrSetMonLevel(int level);
    void mrrSetXFIL();
    int mrrGetXFILValue();
    void powerOFF();
    const char *getRigError(int err_number);
    const char *getXFILString(int number);
    int mrrRigSetSplitVfo(bool split_on);
    void mrrGetRigIF_XCVR_Info();
    void parseRigIF_Response(unsigned char *r);
    void setMainWindowPtr(Ui::MainWindow *p);

public slots:
    int bwidthChangeRequest(int up_or_down);
    void mrrSetTune(bool on);
    float read_rig_swr();
    void mrrSetRx();
    int txCW_Char(char c);

private:
    RIG *my_rig;        /* handle to rig (instance) */
    ConfigObject *config_obj_p;
    //  const char *rig_file = "localhost"; /* Change this for real network useage */
    const char *rig_file = "mac-mini";
    rig_model_t my_model;
    int retcode;
    rig_debug_level_e verbose = RIG_DEBUG_NONE;
    vfo_t current_vfo;
    freq_t cached_freq_a;  // This is a type:double
    freq_t cached_freq_b;
    float frequency;
    Ui::MainWindow *ui_pointer;
    int strength;
    const s_meter_cal_t sMeter_cal[15] = { {-54, "1"}, {-48, "2"}, {-42, "3"}, {-30, "4"}, {-24, "5"}, {-18, "6"}, {-12, "7"}, {-6, "8"}, {0, "9"}, {10, "+10"}, {20, "+20"}, {30, "+30"}, {40, "+40"}, {50, "+50"}, {60, "+60"} };
    bool lockout_spot;
    SpotDelayWorker *spotDelayWorker_p;
    rmode_t current_mode;
    pbwidth_t current_pbwidth;
    const char *modeStr_p;
    int cw_speed;
    bool init_succeeded;
    int xfil_bandwidth;
    bool split_enabled;
    struct rig_if_response if_resp;

private:
    static int listTokensCallback(const struct confparams *cp, rig_ptr_t rp);
    static const char *get_rig_conf_type(enum rig_conf_e type);

public slots:
    void autoupdate_frequency();
    void autoupdate_smeter();
    void cleanupSpotDelay();
    int get_rig_mode_and_bw();

signals:
    void updateWidthSlider(pbwidth_t w);
    void pauseTxSig(bool pause);
    void updateXFIL_sig();
    void spotDone();

};

#endif // HAMLIBCONNECTOR_H
