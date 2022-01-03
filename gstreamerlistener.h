#ifndef GSTREAMERLISTENER_H
#define GSTREAMERLISTENER_H

#include <gst/gst.h>
#include <QObject>

class GstreamerListener : public QObject
{
    Q_OBJECT
public:
    explicit GstreamerListener(QObject *parent = nullptr);

private:
    GstElement *pipeline;
    GError ** gst_error;
    GstBus *bus;
    GstMessage *msg;
    int argc = 0;
    char **argv;

signals:

};

#endif // GSTREAMERLISTENER_H
