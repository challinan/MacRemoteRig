#ifndef FREQUENCYPOLLER_H
#define FREQUENCYPOLLER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "hamlibconnector.h"

class FrequencyPoller : public QObject
{
    Q_OBJECT
public:
    explicit FrequencyPoller(QObject *parent = nullptr);

public slots:
    void poll_rig_frequency();

signals:
    void freq_ready();
    void smeter_ready();

private:
    QTimer *poll_timer;
};

#endif // FREQUENCYPOLLER_H
