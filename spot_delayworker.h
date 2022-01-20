#ifndef SPOTDELAYWORKER_H
#define SPOTDELAYWORKER_H

#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QObject>

class SpotDelayWorker : public QThread
{
    Q_OBJECT
public:
    explicit SpotDelayWorker(QObject *parent = nullptr);
    virtual void run();

signals:
    void spotDelayExpired();
};

#endif // SPOTDELAYWORKER_H
