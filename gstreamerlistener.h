#ifndef GSTREAMERLISTENER_H
#define GSTREAMERLISTENER_H

#include <gst/gst.h>
#include <QObject>
#include <QThread>

class GstreamerListener : public QThread
{
public:
    explicit GstreamerListener();
    virtual void run();

private:
    GstElement *pipeline;
    GError *gst_error = NULL;
    GstBus *bus;
    // GstMessage *msg;
    int argc = 0;
    char **argv;
    static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data);

signals:

};

#endif // GSTREAMERLISTENER_H
