#ifndef FREQUENCYNUDGER_H
#define FREQUENCYNUDGER_H

#include <QObject>
#include <QTimer>

class FrequencyNudger : public QObject
{
    Q_OBJECT
public:
    explicit FrequencyNudger(QObject *parent = nullptr);

private:
    QTimer *fn_timer;

public slots:
    void nudge_frequency();

signals:
    void nudge_timeout();
    void nudge_again();

};

#endif // FREQUENCYNUDGER_H
