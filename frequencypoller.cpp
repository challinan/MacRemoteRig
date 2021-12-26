#include "frequencypoller.h"
#include "mainwindow.h"

class HamlibConnector;

FrequencyPoller::FrequencyPoller(QObject *parent)
    : QObject{parent}
{
    qDebug() << "FrequencyPoller constructor called";
    poll_timer = new QTimer(this);
    /* connect(sender, &Sender::valueChanged, receiver, &Receiver::updateValue);  */

    connect(poll_timer, &QTimer::timeout, this, QOverload<>::of(&FrequencyPoller::poll_rig_frequency) );
    // connect(this, &FrequencyPoller::freq_ready, this, &FrequencyPoller::poll_rig_frequency );
    /* Connect this FreqPoller object to the hamlib connector slot */
    MainWindow* pMainWindow = static_cast<MainWindow *>(parent);
    connect(this, &FrequencyPoller::freq_ready, pMainWindow->getHamlibPointer(), &HamlibConnector::autoupdate_frequency);
    poll_timer->start(1000);
}

void FrequencyPoller::poll_rig_frequency() {
    static int i = 0;
    qDebug() << "poll_rig_frequency() called " << i++;
    emit freq_ready();
}

