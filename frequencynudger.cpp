#include "frequencynudger.h"
#include "mainwindow.h"
#include <QDebug>

FrequencyNudger::FrequencyNudger(QObject *parent)
    : QObject{parent}
{

        qDebug() << "FrequencyNudger constructor called";
        fn_timer = new QTimer(this);
        // connect(sender, &Sender::valueChanged, receiver, &Receiver::updateValue);
        connect(fn_timer, &QTimer::timeout, this, &FrequencyNudger::nudge_frequency);

        /* Connect this FreqPoller object to the hamlib connector slot */
        MainWindow* pMainWindow = static_cast<MainWindow *>(parent);

        connect(this, &FrequencyNudger::nudge_again, pMainWindow->getHamlibPointer(), &HamlibConnector::autoupdate_frequency);

        fn_timer->start(75);

}

// Slot - timer connected to this one
void FrequencyNudger::nudge_frequency() {
        static int i = 0;
        qDebug() << "poll_rig_frequency() called " << i++;
        emit nudge_timeout();
        emit nudge_again();
}
