#include "hamlibconnector.h"
#include "ui_mainwindow.h"
#include "config_object.h"
#include <QDebug>
#include <QApplication>

HamlibConnector::HamlibConnector(QObject *parent)
    : QObject{parent}
{
    verbose = RIG_DEBUG_NONE;
    // verbose = RIG_DEBUG_TRACE;

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

    retcode = rig_get_vfo(my_rig, &current_vfo_a);
    strength = -54;     // Initialize S-Meter to effectively zero
}

freq_t &HamlibConnector::mrr_getFrequency(vfo_t vfo) {

    retcode = rig_get_freq(my_rig, vfo, &current_freq_a);
    if (retcode != RIG_OK) {
        qDebug() << "rig_get_freq: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }

    qDebug() << "Current freq (on currently selected VFO) is " << current_freq_a;
    return current_freq_a;
}

void HamlibConnector::autoupdate_frequency() {
    freq_t f = mrr_getFrequency(current_vfo_a);
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
    rig_get_level(my_rig, current_vfo_a, RIG_LEVEL_STRENGTH, &s);
    return (s.i);
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
    return rig_set_freq(my_rig, current_vfo_a, f);
}

vfo_t HamlibConnector::mrr_getVFO() {
    return current_vfo_a;
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
