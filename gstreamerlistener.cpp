#include "gstreamerlistener.h"
#include <QDebug>

GstreamerListener::GstreamerListener()
{

}

void GstreamerListener::run() {

    /* Initialize GStreamer */
    gst_init (&argc, &argv);  // Ugh - this is 'C' afterall

    /* Build the pipeline */
    // pipeline = gst_parse_launch("udpsrc port=5000 ! \"application/x-rtp,media=(string)audio, "
    //                            "clock-rate=(int)48000, width=16, height=16, encoding-name=(string)L16, "
    //                            "encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, payload=(int)96\" "
    //                            "! rtpL16depay ! audioconvert ! osxaudiosink sync=false", gst_error);

    pipeline = gst_parse_launch("udpsrc port=5000 ! application/x-rtp,media=(string)audio, "
                                "clock-rate=(int)48000, width=16, height=16, encoding-name=(string)L16, "
                                "encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, payload=(int)96 "
                                "! rtpL16depay ! audioconvert ! osxaudiosink sync=false", &gst_error);

    qDebug() << "gst_parse_launcher() returned" << gst_error;

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    msg =
        gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* See next tutorial for proper error message handling/parsing */
    if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
      g_error ("An error occurred! Re-run with the GST_DEBUG=*:WARN environment "
          "variable set for more details.");
    }

    /* Free resources */
    gst_message_unref (msg);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
}
