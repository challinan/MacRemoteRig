#include "hamlibconnector.h"
#include "ui_mainwindow.h"
#include "config_object.h"
#include <QDebug>
#include <QApplication>

HamlibConnector::HamlibConnector(QObject *parent)
    : QObject{parent}
{
    // For debug only

    verbose = RIG_DEBUG_NONE;
    // verbose = RIG_DEBUG_TRACE;

    lockout_spot = false;
    spotDelayWorker_p = nullptr;
    current_vfo = RIG_VFO_A;
    current_mode = RIG_MODE_NONE;
    current_pbwidth = 800;      // Good default for CW
    modeStr_p = nullptr;
    cached_freq_a = 0e0;
    cached_freq_b = 0e0;
    init_succeeded = false;
    split_enabled = false;

#if 0
    // Figure out how we're configured - ie what rig and device
    QString rig_model_str = get_value_from_key("Rig Model");
    qDebug() << "HamlibConnector::HamlibConnector(): Rig Model configured as" << rig_model_str;
#endif

    my_model = RIG_MODEL_K3;
    rig_set_debug(verbose);
    my_rig = rig_init(my_model);
    if (!my_rig) {
        qDebug() << "Unknown rig num " << my_model << "or initialization error\n";
        QApplication::exit(16);
    }

    strncpy(my_rig->state.rigport.pathname, rig_file, 511);
    // qDebug() << "rig_file = " << my_rig->state.rigport.pathname;

    // rig_open will take 75 seconds if the server end is not listening
    retcode = rig_open(my_rig);
    if (retcode != RIG_OK) {
        qDebug() << "HamlibConnector::HamlibConnector(): rig_open: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        // Can't quit here - main loop isn't yet running.  All we can do is report the failure.
        goto bailout;
    }

    retcode = rig_get_vfo(my_rig, &current_vfo);
    qDebug() << "HamlibConnector::HamlibConnector(): rig_get_vfo returned" << current_vfo;
    strength = -54;     // Initialize S-Meter to effectively zero

    // Initialize rig mode, etc
    get_rig_mode_and_bw();
    mrr_getRigFrequency(RIG_VFO_A);
    mrr_getRigFrequency(RIG_VFO_B);
    init_succeeded = true;

bailout:
    return;
}

freq_t HamlibConnector::mrr_getRigFrequency(vfo_t vfo) {

    freq_t freq;
    retcode = rig_get_freq(my_rig, vfo, &freq);
    if (retcode != RIG_OK) {
        qDebug() << "rig_get_freq: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }
    if ( vfo == RIG_VFO_A ) {
        cached_freq_a = freq;
        return freq;
    }
        else {
        cached_freq_b = freq;
        return freq;
    }

    qDebug() << "Current freq (on currently selected VFO) is " << cached_freq_a;
    return 0;
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
    // Create a moving average
    static int moving_avg_length = 5;
    static QList<int> s_readings;
    int avg = 0, i;

    // Read the rig's S meter value
    int s = read_rig_strength();
    s_readings.append(s);

    if ( s_readings.size() > moving_avg_length ) {
        // Can't really do the moving average with less than enough readings
        avg = 0;
        for ( i=0; i<5; i++) {
            avg += s_readings.at(i);
        }
        s_readings.removeFirst();
        avg = avg / 5;
    }

    int t = get_SMeter_progbar_value(avg);
    ui_pointer->smeterProgressBar->setValue(t);

    // Get the text equivalent of the s-meter value
    ui_pointer->smeterLabel->setText(sMeter_cal[t].s);
}

int HamlibConnector::get_SMeter_progbar_value(int x) {

    // Get value appropriate for our S-Meter progress bar, from the raw S-Meter data from the rig
    for (int i = 0; i < (int) sizeof(sMeter_cal) / (int) (sizeof(sMeter_cal[0]) - 1); ++i) {
        if ( x >= sMeter_cal[i].raw && x < sMeter_cal[i+1].raw ) {
            // qDebug() << "HamlibConnector::get_SMeter_progbar_value(): sMeter_cal[].raw" << sMeter_cal[i].raw;
            strength = i;
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

int HamlibConnector::mrrSetRigFreqA(freq_t f) {

    return rig_set_freq(my_rig, RIG_VFO_A, f);
}

int HamlibConnector::mrrSetRigFreqB(freq_t f) {

    return rig_set_freq(my_rig, RIG_VFO_B, f);
}

vfo_t HamlibConnector::mrr_getVFO() {
    return current_vfo;
}

freq_t HamlibConnector::mrrGetCachedFreqA() {
    return cached_freq_a;
}

void HamlibConnector::mrrSetCachedFreqA(freq_t f) {
    cached_freq_a = f;
}

freq_t HamlibConnector::mrrGetCachedFreqB() {
    return cached_freq_b;
}

void HamlibConnector::mrrSetCachedFreqB(freq_t f) {
    cached_freq_b = f;
}

int HamlibConnector::get_retcode(void) {
    return retcode;
}

void HamlibConnector::setSpot(void) {

    if ( lockout_spot ) return;

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
    emit spotDone();
}

void HamlibConnector::setSwapAB() {

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_ABSWAP, 1);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::setSpot() failed: rc = " << rigerror(rc);
    }
}

mode_t HamlibConnector::mrrGetMode() {

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

int HamlibConnector::bwidthChangeRequest(int bw){

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

void HamlibConnector::mrrSetTune(bool on) {

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

    // qDebug() << "HamlibConnector::txCW_Char(): Entered with " << c;
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
    rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_KEYSPD, &val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::getCwSpeed(): failed" << rigerror(rc);
        return -1;
    }
    // qDebug() << "HamlibConnector::getCwSpeed(): returned" << val.i;
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

    int rc = rig_set_level(my_rig, current_vfo, RIG_LEVEL_KEYSPD, val);
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

void HamlibConnector::mrrGetIcConfig(unsigned char *p) {

#pragma unused(p)
    value_t val;

    int rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_ICONSTATUS, &val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_get_ic_config(): failed" << rigerror(rc);
    }
    qDebug() << "HamlibConnector::mrrGetIcConfig(): exiting";
    // strncpy( (char *)p, val.s, 5);
}

void HamlibConnector::mrr_set_tx_test() {

    // This is a toggle function - needs no "value"
    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_TXTEST, 0);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_set_tx_test(): failed" << rigerror(rc);
    }
}

void HamlibConnector::mrr_set_band(int band) {

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_BANDNUM, band);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_set_band(): failed" << rigerror(rc);
    }
}

int HamlibConnector::mrr_get_band() {

    int band, rc;
    rc = rig_get_func(my_rig, current_vfo, RIG_FUNC_BANDNUM, &band);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_get_band(): failed" << rigerror(rc);
    }
    return band;
}

void HamlibConnector::mrr_a_2_b() {

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_VFOA2B, 0);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrr_a_2_b(): failed" << rigerror(rc);
    }
}

int HamlibConnector::mrrGetMonLevel() {

    value_t val;
    int rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_MONITOR_GAIN, &val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrrGetMonLevel(): failed" << rigerror(rc);
    }
    qDebug() << "HamlibConnector::mrrGetMonLevel(): monitor level:" << val.f;
    return (int) (val.f * 60.0f);
}

int HamlibConnector::mrrSetMonLevel(int level) {

    value_t val;
    val.f = (float) (level / 60.0f);

    int rc = rig_set_level(my_rig, current_vfo, RIG_LEVEL_MONITOR_GAIN, val);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrrSetMonLevel(): failed" << rigerror(rc);
    }
    return rc;
}

int HamlibConnector::mrrGetXFILValue() {

    value_t status;
    int rc = rig_get_level(my_rig, current_vfo, RIG_LEVEL_XFILV, &status);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrrGetXFIL(): failed" << rigerror(rc);
    }
    xfil_bandwidth = status.i;
    return status.i;
}

void HamlibConnector::mrrSetXFIL() {

    int rc = rig_set_func(my_rig, current_vfo, RIG_FUNC_XFIL, 0);
    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrrSetXFIL(): failed" << rigerror(rc);
    }
    emit updateXFIL_sig();
}

void HamlibConnector::powerOFF() {

    rig_set_powerstat(my_rig, RIG_POWER_OFF);
}

const char *HamlibConnector::getRigError(int err_number) {

    return rigerror(err_number);
}

const char *HamlibConnector::getXFILString(int number) {

    if ( number < 1 || number > 5 ) {
        qDebug() << "HamlibConnector::getXFILString()): Invalid filter number" << number;
    }
    // Array is zero based, XFIL starts at 1
    return xtal_filter_values[number - 1].filter_bandwidth;
}

/*
extern HAMLIB_EXPORT(int)
rig_token_foreach HAMLIB_PARAMS((RIG *rig,
                                 int (*cfunc)(const struct confparams *,
                                              rig_ptr_t),
                                 rig_ptr_t data));

rig.h:2802:1: note: candidate function not viable:
no known conversion from 'int (HamlibConnector::*)(const struct confparams *, void *)'
to                       'int (*)(const struct confparams *, void *)' for 2nd argument
*/

int HamlibConnector::listTokensCallback(const struct confparams *cp, rig_ptr_t rp) {

    qDebug() << "HamlibConnector::listTokensCallback(): confparms:";
    qDebug() << "    token:" << cp->token;
    qDebug() << "    name:" << cp->name;
    qDebug() << "    label:" << cp->label;
    qDebug() << "    type:" << get_rig_conf_type(cp->type);
    if ( cp->type == RIG_CONF_STRING )
        qDebug() << "    data:" << *cp->u.c.combostr;
    qDebug() << "";
    return 1;
}

const char *HamlibConnector::get_rig_conf_type(enum rig_conf_e type)
{
    switch (type)
    {
    case RIG_CONF_STRING:
        return "STRING";

    case RIG_CONF_COMBO:
        return "COMBO";

    case RIG_CONF_NUMERIC:
        return "NUMERIC";

    case RIG_CONF_CHECKBUTTON:
        return "CHECKBUTTON";

    case RIG_CONF_BUTTON:
        return "BUTTON";

    case RIG_CONF_BINARY:
        return "BINARY";
    }

    return "UNKNOWN";
}

int HamlibConnector::mrrRigSetSplitVfo(bool split_on) {

    int rc;

    if ( split_on ) {
        rc = rig_set_split_vfo	(my_rig, RIG_VFO_A, RIG_SPLIT_ON, RIG_VFO_B);
    }
    else {
        rc = rig_set_split_vfo	(my_rig, RIG_VFO_A, RIG_SPLIT_OFF, RIG_VFO_B);
    }

    if ( rc != RIG_OK ) {
        qDebug() << "HamlibConnector::mrrRigSetSplitVfo(): failed" << rigerror(rc);
    }
    return rc;
}
