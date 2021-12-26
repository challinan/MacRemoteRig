#include "hamlibconnector.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QApplication>

HamlibConnector::HamlibConnector(QObject *parent)
    : QObject{parent}
{
    verbose = 1;
    my_model = 1035;
    my_rig = rig_init(RIG_MAKE_MODEL(RIG_YAESU, 35));
    if (!my_rig) {
        qDebug() << "Unknown rig num " << my_model << "or initialization error\n";
        QApplication::exit();
    }

    retcode = set_conf(my_rig, conf_parms);
    if (retcode != RIG_OK) {
        qDebug() << "Config parameter error" << rigerror(retcode) << "\n";
        QApplication::exit();
    }
    strncpy(my_rig->state.rigport.pathname, rig_file, 511);

    retcode = rig_open(my_rig);
    if (retcode != RIG_OK) {
        qDebug() << "rig_open: error = " << rigerror(retcode) << rig_file << strerror(errno) << "\n";
        QApplication::exit();
    }
}

QString &HamlibConnector::getFrequency(void) {
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
    qDebug() << "Current freq (FVO_A) is " << freq_a;

    frequency = QString::number(freq_a, 'f', 3);
    frequency.truncate(frequency.lastIndexOf(QChar('.')));
    qDebug() << frequency;
    return frequency;
}

void HamlibConnector::autoupdate_frequency() {
    ui_pointer->FreqDisplay->display(getFrequency());
}

void HamlibConnector::store_ui_pointer(Ui::MainWindow *p) {
    ui_pointer = p;
}


