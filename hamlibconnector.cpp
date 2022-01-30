#include "hamlibconnector.h"
#include "ui_mainwindow.h"
#include "config_object.h"
#include <QDebug>
#include <QApplication>

HamlibConnector::HamlibConnector(QObject *parent)
    : QObject{parent}
{
    //  verbose = RIG_DEBUG_NONE;
    verbose = RIG_DEBUG_TRACE;

#if 0
    // Figure out how we're configured - ie what rig and device
    QString rig_model_str = get_value_from_key("Rig Model");
    qDebug() << "HamlibConnector::HamlibConnector(): Rig Model configured as" << rig_model_str;
#endif

    my_model = RIG_MODEL_K3;
    rig_set_debug(verbose);
    // my_rig = rig_init(RIG_MAKE_MODEL(RIG_YAESU, 35));
    my_rig = rig_init(my_model);
    if (!my_rig) {
        qDebug() << "Unknown rig num " << my_model << "or initialization error\n";
        QApplication::exit();
    }
    qDebug() << "Rig init called, my_rig = " << my_rig;

    strncpy(my_rig->state.rigport.pathname, rig_file, 511);
    qDebug() << "rig_file = " << my_rig->state.rigport.pathname;


    retcode = rig_open(my_rig);
    if (retcode != RIG_OK) {
        qDebug() << "HamlibConnector::HamlibConnector(): rig_open: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        // Can't quit here - main loop isn't yet running.  All we can do is report the failure.
    }

    retcode = rig_get_vfo(my_rig, &current_vfo);
    qDebug() << "HamlibConnector::HamlibConnector(): rig_get_vfo returned" << current_vfo;
    strength = -54;     // Initialize S-Meter to effectively zero

    lockout_spot = false;
    spotDelayWorker_p = nullptr;
    current_vfo = RIG_VFO_A;
    current_mode = RIG_MODE_NONE;
    current_pbwidth = 0;
    modeStr_p = nullptr;
    current_freq_a = 0e0;
    current_freq_b = 0e0;

    // Initialize rig mode, etc
    get_rig_mode_and_bw();
    mrr_getRigFrequency(RIG_VFO_A);
    mrr_getRigFrequency(RIG_VFO_B);
}

freq_t HamlibConnector::mrr_getRigFrequency(vfo_t vfo) {

    freq_t freq;
    retcode = rig_get_freq(my_rig, vfo, &freq);
    if (retcode != RIG_OK) {
        qDebug() << "rig_get_freq: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }
    if ( vfo == RIG_VFO_A ) {
        current_freq_a = freq;
        return freq;
    }
        else {
        current_freq_b = freq;
        return freq;
    }

    qDebug() << "Current freq (on currently selected VFO) is " << current_freq_a;
    return -1e0;
}

void HamlibConnector::autoupdate_frequency() {
    freq_t f = mrr_getRigFrequency(current_vfo);
    QString str_tmp = HamlibConnector::get_display_frequency(f);
    ui_pointer->freqDisplay->display(str_tmp);
}

void HamlibConnector::store_ui_pointer(Ui::MainWindow *p) {
    ui_pointer = p;
}

void HamlibConnector::autoupdate_smeter() {
    // qDebug("HamlibConnector::autoupdate_smeter() called");
    int s = read_rig_strength();
    int t = get_SMeter_progbar_value(s);
    ui_pointer->smeterProgressBar->setValue(t);
}

int HamlibConnector::get_SMeter_progbar_value(int x) {

    // Get value appropriate for our S-Meter progress bar, from the raw S-Meter data from the rig
    for (int i = 0; i < sMeter_cal.size() - 1; ++i) {
        if ( x >= sMeter_cal.at(i) && x < sMeter_cal.at(i+1) ) {
            return (strength = i);
        }
    }
    return 0;  // Default - shouldn't get here
}

int HamlibConnector::read_rig_strength() {
    value_t s;
    rig_get_level(my_rig, current_vfo, RIG_LEVEL_STRENGTH, &s);
    return (s.i);
}

float HamlibConnector::read_rig_swr() {
    value_t s;
    int rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_SWR, &s);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::read_rig_swr(): rig_get_level failed" << rigerror(rc);
        return -1e0f;
    }
    qDebug() << "HamlibConnector::read_rig_swr(): returned" << s.f;
    return (s.f);
}

QString HamlibConnector::get_display_frequency(freq_t f) {
    // Hamlib hands us a double value equal to the frequency in Hz.  We want to display in MHz
    // qDebug() << "HamlibConnector::get_display_frequency() entered: (freq_t f) = " << f;
    freq_t f_MHz = f / 1000000;
    // qDebug() << "HamlibConnector::get_display_frequency() entered: f_MHZ = " << f_MHz;
    QString ftmp = QString("%1").arg(f_MHz, 0, 'f', 5);
    qsizetype dot_index = ftmp.indexOf(QChar('.'));
    if ( dot_index == -1 ) {
        qDebug() << "HamlibConnector::get_display_frequency(): No decimal point found in frequency string" << ftmp;
        QApplication::quit();
    }

    // qDebug() << "String ftmp now: " << ftmp << "Dot Index = " << dot_index;
    QString fd = ftmp.left(dot_index+6);    // Print the decimal point + 5 digits after (dot_index is zero based, left() arg is 1 based.
    return fd;
}

int HamlibConnector::set_rig_freq(freq_t f) {
    return rig_set_freq(my_rig, current_vfo, f);
}

vfo_t HamlibConnector::mrr_getVFO() {
    return current_vfo;
}

freq_t HamlibConnector::mrr_getCurrentFreq_A() {
    return current_freq_a;
}

void HamlibConnector::mrr_setCurrentFreq_A(freq_t f) {
    current_freq_a = f;
}

int HamlibConnector::get_retcode(void) {
    return retcode;
}

void HamlibConnector::setSpot(void) {
    if ( lockout_spot ) { return; }

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_SPOT, 1);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::setSpot() failed: rc = " << rigerror(rc);
    }
    lockout_spot = true;
    // Start a thread with a 2 second timer.  Update freq when done.
    spotDelayWorker_p = new SpotDelayWorker();
    connect(spotDelayWorker_p, &SpotDelayWorker::spotDelayExpired, this, &HamlibConnector::cleanupSpotDelay);
    spotDelayWorker_p->start();  // start() unlike run() detaches and returns immediately
}

void HamlibConnector::cleanupSpotDelay() {
    qDebug() << "HamlibConnector::cleanupSpotDelay(): killing worker thread, updating freq";
    delete spotDelayWorker_p;
    spotDelayWorker_p = nullptr;
    autoupdate_frequency();
    lockout_spot = false;
}

void HamlibConnector::setSwapAB() {

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_ABSWAP, 1);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::setSpot() failed: rc = " << rigerror(rc);
    }
}

mode_t HamlibConnector::mrr_get_mode() {

    return current_mode;
}

const char *HamlibConnector::mrr_getModeString(mode_t mode) {

    return rig_strrmode(mode);
}

int HamlibConnector::mrr_set_mode(mode_t mode) {

    pbwidth_t w = rig_passband_normal(my_rig, mode);
    qDebug() << "HamlibConnector::mrr_set_mode(): normal passband reported as" << w;
    // int rc = rig_set_mode(my_rig, current_vfo, mode, RIG_PASSBAND_NOCHANGE);
    int rc = rig_set_mode(my_rig, current_vfo, mode, w);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_set_mode() failed: rc = " << rigerror(rc);
    }
    current_mode = mode;

    // Update bandwidth on change of mode
    get_rig_mode_and_bw();
    return rc;
}

pbwidth_t HamlibConnector::mrr_get_width() {

    return current_pbwidth;
}

int HamlibConnector::bwidth_change_request(int bw){

    int rc;

    // qDebug() << "HamlibConnector::bwidth_change_request() with signal:" << "bw:" << bw;
    rc = rig_set_mode(my_rig, current_vfo, current_mode, bw);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::bwidth_change_request(): rig_set_mode failed" << rigerror(rc);
        return rc;
    }
    emit updateWidthSlider(bw);
    current_pbwidth = bw;
    return rc;
}

int HamlibConnector::get_rig_mode_and_bw() {

    rmode_t mode;
    pbwidth_t bw;

    int rc = rig_get_mode(my_rig, current_vfo, &mode, &bw);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::get_rig_mode(): rig_get_mode failed" << rigerror(rc);
    }
    current_mode = mode;
    current_pbwidth = bw;
    return rc;
}

void HamlibConnector::mrrSetTune(int on) {

    if ( on ) {
        int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_TUNE, 1);
        if ( rc != RIG_OK ) {
            qDebug() << "HamlibConnector::mrrSetTune() failed: rc = " << rigerror(rc);
        }
    }
    else {
        mrrSetRx();
    }
}

void HamlibConnector::mrrSetRx() {

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_RX, 1);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrrSetRx() failed: rc = " << rigerror(rc);
    }
}

int HamlibConnector::txCW_Char(char c) {

    char c_tmp = c;
    int rc;

    qDebug() << "HamlibConnector::txCW_Char(): Entered with " << c;;
    rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_CWTX, c_tmp);
    return rc;
}

int HamlibConnector::mrr_set_level(setting_t level, value_t val) {

    int rc = rig_set_level(my_rig, current_vfo, level, val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_set_level() failed: rc = " << rigerror(rc);
    }
    return rc;
}

void HamlibConnector::abortTX() {
    qDebug() << "HamlibConnector::abortTX(): entered";
    mrrSetRx();
}

int HamlibConnector::getCwSpeed() {
    int rc;
    value_t val;
    qDebug() << " ";
    qDebug() << "********************************************************************";
    rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_CWSPEED, &val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::getCwSpeed(): failed" << rigerror(rc);
        return -1;
    }
    qDebug() << "HamlibConnector::getCwSpeed(): returned" << val.i;
    cw_speed = val.i;
    return val.i;
}

int HamlibConnector::bumpCwSpeed(bool up) {

    value_t val;
    // Rig limits: 8 - 50wpm
    if ( up == true ) {
        cw_speed = (cw_speed >= 50) ? 50 : ++cw_speed;
        val.i =  cw_speed;
    } else {
        cw_speed = (cw_speed <= 8) ? 8 : --cw_speed;
        val.i = cw_speed;
    }

    qDebug() << "HamlibConnector::bumpCwSpeed(): new value is" << cw_speed;

    int rc = rig_set_level(my_rig, current_vfo, RIG_LEVEL_CWSPEED, val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::bumpCwSpeed(): failed" << rigerror(rc);
        return -1;
    }
    return cw_speed;
}

void HamlibConnector::setPauseTx(bool checked) {
    emit pauseTxSig(checked);
    qDebug() << "HamlibConnector::setPauseTx(): paused =" << checked;
}

void HamlibConnector::mrr_get_ic_config(char *p) {

    value_t val;
    val.s = p;  // Storage in mainwindow.cpp
    int rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_ICONSTATUS, &val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_get_ic_config(): failed" << rigerror(rc);
    }
}
