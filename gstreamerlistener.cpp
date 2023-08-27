#include "gstreamerlistener.h"
#include <QDebug>

GstreamerListener::GstreamerListener()
{

}

void GstreamerListener::run() {

    guint bus_watch_id;
    GMainLoop *loop;
    // GstElement *source;

    /* Initialize GStreamer */
    gst_init (&argc, &argv);  // Ugh - this is 'C' afterall
    loop = g_main_loop_new (NULL, FALSE);

    // source = gst_element_factory_make("udpsrc", nullptr);

    g_set_print_handler (NULL);
    /*
     * Build the pipeline
     * From notes:  This works as of Jan 16/22 with this gst-launch command on imac:
     * gst-launch-1.0 -v udpsrc port=5000 ! "application/x-rtp,media=(string)audio, clock-rate=(int)44100, width=16, height=16,
     *        encoding-name=(string)L16, encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, payload=(int)96" !
     *        rtpL16depay ! audioconvert ! osxaudiosink sync=false
     *
     *  From March 11 - RTSP _and_ rtpjitterbuffer
     *  gst-launch-1.0 rtspsrc location=rtsp://mac-mini:8554/test ! rtpjitterbuffer mode=synced ! rtpL16depay ! audioconvert ! osxaudiosink sync=true
     */

#ifdef WIN32
    pipeline = gst_parse_launch("udpsrc port=5000 ! application/x-rtp,media=(string)audio, clock-rate=(int)44100, width=16, height=16,"
                                " encoding-name=(string)L16, encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, payload=(int)96 "
                                " ! rtpjitterbuffer mode=synced ! rtpL16depay ! audioconvert ! directsoundsink sync=true", &gst_error);
#elif __APPLE__
    // pipeline = gst_parse_launch("rtspsrc location=rtsp://mac-mini:8554/test latency=0 buffer-mode=auto ! rtpjitterbuffer mode=synced \
    //                              ! rtpL16depay ! audioconvert ! osxaudiosink device=67", &gst_error);

    // Removing the jitter buffer cut latency by a noticeable amount
    pipeline = gst_parse_launch("rtspsrc location=rtsp://mac-mini:8554/test latency=0 buffer-mode=auto \
                                 ! rtpL16depay ! audioconvert ! osxaudiosink device=67", &gst_error);
#elif
#elif
#error "Platform undefined in gstreamerlistener.cpp"
#endif

    qDebug() << "gst_parse_launcher() returned" << gst_error;
    qDebug() << "GStreamerListener pipeline: " << pipeline;

    // Add a message handler
    bus = gst_pipeline_get_bus ( GST_PIPELINE_CAST (pipeline));
    bus_watch_id = gst_bus_add_watch (bus, &GstreamerListener::bus_callback, loop);
    gst_object_unref (bus);

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_message ("g_main_loop Running...\n");
    g_main_loop_run (loop);

    /* Out of the main loop, clean up nicely */
    g_message ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_message ("Deleting pipeline\n");
    gst_object_unref (GST_PIPELINE_CAST (pipeline));
    g_source_remove (bus_watch_id);
    g_main_loop_unref (loop);

#if 0
    /* Free resources */
    gst_message_unref (msg);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
#endif
}

gboolean GstreamerListener::bus_callback(GstBus *bus, GstMessage *msg, gpointer data)
{
    (void) bus;
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
        g_message ("End of stream\n");
        g_main_loop_quit (loop);
        break;

    case GST_MESSAGE_STATE_CHANGED:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STATE_CHANGED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_UNKNOWN:
        g_message("%llu: GST Message: %d - GST_MESSAGE_UNKNOWN from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_WARNING:
        g_message("%llu: GST Message: %d - GST_MESSAGE_WARNING from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_INFO:
        g_message("%llu: GST Message: %d - GST_MESSAGE_INFO from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_TAG:
        g_message("%llu: GST Message: %d - GST_MESSAGE_TAG from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_BUFFERING:
        g_message("%llu: GST Message: %d - GST_MESSAGE_BUFFERING from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STATE_DIRTY:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STATE_DIRTY from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STEP_DONE:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STEP_DONE from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_CLOCK_PROVIDE:
        g_message("%llu: GST Message: %d - GST_MESSAGE_CLOCK_PROVIDE from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_CLOCK_LOST:
        g_message("%llu: GST Message: %d - GST_MESSAGE_CLOCK_LOST from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_NEW_CLOCK:
        g_message("%llu: GST Message: %d - GST_MESSAGE_NEW_CLOCK from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STRUCTURE_CHANGE:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STRUCTURE_CHANGE from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STREAM_STATUS:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STREAM_STATUS from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_APPLICATION:
        g_message("%llu: GST Message: %d - GST_MESSAGE_APPLICATION from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_ELEMENT:
        g_message("%llu: GST Message: %d - GST_MESSAGE_ELEMENT from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_SEGMENT_START:
        g_message("%llu: GST Message: %d - GST_MESSAGE_SEGMENT_START from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_SEGMENT_DONE:
        g_message("%llu: GST Message: %d - GST_MESSAGE_SEGMENT_DONE from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_DURATION_CHANGED:
        g_message("%llu: GST Message: %d - GST_MESSAGE_DURATION_CHANGED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_LATENCY:
        g_message("%llu: GST Message: %d - GST_MESSAGE_LATENCY from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_ASYNC_START:
        g_message("%llu: GST Message: %d - GST_MESSAGE_ASYNC_START from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_ASYNC_DONE:
        g_message("%llu: GST Message: %d - GST_MESSAGE_ASYNC_DONE from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_REQUEST_STATE:
        g_message("%llu: GST Message: %d - GST_MESSAGE_REQUEST_STATE from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STEP_START:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STEP_START from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_QOS:
        g_message("%llu: GST Message: %d - GST_MESSAGE_QOS from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_PROGRESS:
        g_message("%llu: GST Message: %d - GST_MESSAGE_PROGRESS from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_TOC:
        g_message("%llu: GST Message: %d - GST_MESSAGE_TOC from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_RESET_TIME:
        g_message("%llu: GST Message: %d - GST_MESSAGE_RESET_TIME from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STREAM_START:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STREAM_START from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_NEED_CONTEXT:
        g_message("%llu: GST Message: %d - GST_MESSAGE_NEED_CONTEXT from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_HAVE_CONTEXT:
        g_message("%llu: GST Message: %d - GST_MESSAGE_HAVE_CONTEXT from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_EXTENDED:
        g_message("%llu: GST Message: %d - GST_MESSAGE_EXTENDED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_DEVICE_ADDED:
        g_message("%llu: GST Message: %d - GST_MESSAGE_DEVICE_ADDED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_DEVICE_REMOVED:
        g_message("%llu: GST Message: %d - GST_MESSAGE_DEVICE_REMOVED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_PROPERTY_NOTIFY:
        g_message("%llu: GST Message: %d - GST_MESSAGE_PROPERTY_NOTIFY from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STREAM_COLLECTION:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STREAM_COLLECTION from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_STREAMS_SELECTED:
        g_message("%llu: GST Message: %d - GST_MESSAGE_STREAMS_SELECTED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_REDIRECT:
        g_message("%llu: GST Message: %d - GST_MESSAGE_REDIRECT from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_DEVICE_CHANGED:
        g_print("%llu: GST Message: %d - GST_MESSAGE_DEVICE_CHANGED from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_INSTANT_RATE_REQUEST:
        g_print("%llu: GST Message: %d - GST_MESSAGE_INSTANT_RATE_REQUEST from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_ANY:
        g_print("%llu: GST Message: %d - GST_MESSAGE_ANY from Obj: %s", GST_MESSAGE_TIMESTAMP(msg), GST_MESSAGE_SEQNUM(msg), GST_MESSAGE_SRC_NAME(msg));
        break;

    case GST_MESSAGE_ERROR: {
        gchar  *debug;
        GError *error;

        gst_message_parse_error (msg, &error, &debug);
        g_free (debug);

        g_printerr ("GStreamer GST_MESSAGE_ERROR ********8: %s\n", error->message);
        g_error_free (error);

        // g_main_loop_quit (loop);
        break;
    }
    default:
        g_print("GstreamerListener::bus_callback(): message not handled - %d\n", GST_MESSAGE_TYPE (msg));
        break;
    }

    return TRUE;
}
