#include "hamlibconnector.h"
#include "ui_mainwindow.h"
#include "config_object.h"
#include <QDebug>
#include <QApplication>

HamlibConnector::HamlibConnector(QObject *parent)
    : QObject{parent}
{
    verbose = 0;
    my_model = 1035;
    rig_set_debug(RIG_DEBUG_NONE);
    my_rig = rig_init(RIG_MAKE_MODEL(RIG_YAESU, 35));
    if (!my_rig) {
        qDebug() << "Unknown rig num " << my_model << "or initialization error\n";
        QApplication::exit();
    }
    qDebug() << "Rig init called, my_rig = " << my_rig;

    strncpy(my_rig->state.rigport.pathname, rig_file, 511);
    qDebug() << "rig_file = " << my_rig->state.rigport.pathname;


    retcode = rig_open(my_rig);
    if (retcode != RIG_OK) {
        qDebug() << "rig_open: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }
    strength = -54;
}

freq_t &HamlibConnector::getFrequency(void) {
    retcode = rig_get_vfo(my_rig, &vfo_a);
    if (retcode != RIG_OK) {
        qDebug() << "rig_get_vfo: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }
    qDebug() << "Current VFO is " << vfo_a;

    rig_get_freq(my_rig, vfo_a, &freq_a);
    if (retcode != RIG_OK) {
        qDebug() << "rig_get_freq: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }

    qDebug() << "Current freq (VFO_A) is " << freq_a;
    // frequency = freq_a / 1.00e6;

    // qDebug() << "HamlibConnector::getFrequency(): frequency = " << frequency;
    // return frequency;
    return freq_a;
}

void HamlibConnector::autoupdate_frequency() {
    ui_pointer->freqDisplay->display(getFrequency());
}

void HamlibConnector::store_ui_pointer(Ui::MainWindow *p) {
    ui_pointer = p;
}

void HamlibConnector::autoupdate_smeter() {
    qDebug("HamlibConnector::autoupdate_smeter() called");
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
    rig_get_level(my_rig, vfo_a, RIG_LEVEL_STRENGTH, &s);
    return (s.i);
}

QString HamlibConnector::get_display_frequency(freq_t f) {
    // Hamlib hands us a double value equal to the frequency.  We want to display in MHz
    freq_t f_MHz = f / 1000;
    QString ftmp = QString("%1").arg(f_MHz, 0, 'f', 5);
    qsizetype dot_index = ftmp.indexOf(QChar('.'));
    QString fd = ftmp.left(dot_index+3);    // Print the decimal point + 2 digits after
    return fd;
}
