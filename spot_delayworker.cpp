#include "spot_delayworker.h"

SpotDelayWorker::SpotDelayWorker(QObject *parent)
    : QThread{parent}
{

}

void SpotDelayWorker::run() {
    qDebug() << "SpotDelayWorker::run(): entered";
    QThread::msleep(3000);
    emit spotDelayExpired();
}
