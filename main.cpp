#include <stdio.h>
#include <cassert>

#include <gtkmm.h>
#include <gtk/gtk.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#include "mainwindow.h"

static pa_context* context = NULL;
static pa_mainloop_api* api = NULL;
static int n_outstanding = 0;
static bool retry = false;
static int reconnect_timeout = 1;

void on_ready(pa_context*);
void show_error(const char *txt) {
    char buf[256];

    snprintf(buf, sizeof(buf), "%s: %s", txt, pa_strerror(pa_context_errno(context)));

    Gtk::MessageDialog dialog(buf, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
    dialog.run();

    Gtk::Main::quit();
}

static void dec_outstanding(MainWindow *w) {
    if (n_outstanding <= 0)
        return;

    if (--n_outstanding <= 0) {
        //w->get_window()->set_cursor();
        //w->setConnectionState(true);
    }
}

void context_state_callback(pa_context *c, void *userdata) {
    printf("%s\n", __FUNCTION__);

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_UNCONNECTED: {
        printf("PA_CONTEXT_UNCONNECTED\n");
        break;
    }
    case PA_CONTEXT_CONNECTING: {
        printf("PA_CONTEXT_CONNECTING\n");
        break;
    }
    case PA_CONTEXT_AUTHORIZING: {
        printf("PA_CONTEXT_AUTHORIZING\n");
        break;
    }
    case PA_CONTEXT_SETTING_NAME:  {
        printf("Here\n");
        break;
    }

    case PA_CONTEXT_READY: {
        printf("PA_CONTEXT_READY\n");
        on_ready(c);
        break;
    }
    case PA_CONTEXT_FAILED: {
        printf("PA_CONTEXT_FAILED\n");
        break;
    }
    case PA_CONTEXT_TERMINATED: {
        printf("PA_CONTEXT_TERMINATED\n");
        break;
    }

    }
}

gboolean connect_to_pulse(gpointer userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);
    if (context) {
        fprintf(stderr, "context != NULL\n");
        return false;
    }
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "PulseAudio Volume Control");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.PulseAudio.pavucontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "1.0");

    context = pa_context_new_with_proplist(api, NULL, proplist);
    assert(context);

    pa_proplist_free(proplist);

    pa_context_set_state_callback(context, context_state_callback, w);

    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        if (pa_context_errno(context) == PA_ERR_INVALID) {
            char msg[] = "Connection to PulseAudio failed. Automatic retry in 5s\n\n"
                "In this case this is likely because PULSE_SERVER in the Environment/X11 Root Window Properties\n"
                "or default-server in client.conf is misconfigured.\n"
                "This situation can also arrise when PulseAudio crashed and left stale details in the X11 Root Window.\n"
                "If this is the case, then PulseAudio should autospawn again, or if this is not configured you should\n"
                "run start-pulseaudio-x11 manually.";
            printf ("%s\n", msg);
            reconnect_timeout = 5;
        }
        else {
            if(!retry) {
                reconnect_timeout = -1;
                Gtk::Main::quit();
            } else {
                g_debug("Connection failed, attempting reconnect");
                reconnect_timeout = 5;
                g_timeout_add_seconds(reconnect_timeout, connect_to_pulse, w);
            }

        }
    }

    return false;
}

void card_cb(pa_context *, const pa_card_info *i, int eol, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(("Card callback failure"));
        return;
    }

    if (eol > 0) {
        dec_outstanding(w);
        return;
    }

    w->updateCard(*i);
}

void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(("Sink callback failure"));
        return;
    }

    if (eol > 0) {
        dec_outstanding(w);
        return;
    }

#if HAVE_EXT_DEVICE_RESTORE_API
    if (w->updateSink(*i))
        ext_device_restore_subscribe_cb(c, PA_DEVICE_TYPE_SINK, i->index, w);
#else
    w->updateSink(*i);
#endif
}

void source_cb(pa_context *, const pa_source_info *i, int eol, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(("Source callback failure"));
        return;
    }

    if (eol > 0) {
        dec_outstanding(w);
        return;
    }

    w->updateSource(*i);
}

void sink_input_cb(pa_context *, const pa_sink_input_info *i, int eol, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(("Sink input callback failure"));
        return;
    }

    if (eol > 0) {
        dec_outstanding(w);
        return;
    }

    w->updateSinkInput(*i);
}

void source_output_cb(pa_context *, const pa_source_output_info *i, int eol, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(("Source output callback failure"));
        return;
    }

    if (eol > 0)  {

        // if (n_outstanding > 0) {
        //     /* At this point all notebook pages have been populated, so
        //      * let's open one that isn't empty */
        //     if (default_tab != -1) {
        //         if (default_tab < 1 || default_tab > w->notebook->get_n_pages()) {
        //             if (w->sinkInputWidgets.size() > 0)
        //                 w->notebook->set_current_page(0);
        //             else if (w->sourceOutputWidgets.size() > 0)
        //                 w->notebook->set_current_page(1);
        //             else if (w->sourceWidgets.size() > 0 && w->sinkWidgets.size() == 0)
        //                 w->notebook->set_current_page(3);
        //             else
        //                 w->notebook->set_current_page(2);
        //         } else {
        //             w->notebook->set_current_page(default_tab - 1);
        //         }
        //         default_tab = -1;
        //     }
        // }

        dec_outstanding(w);
        return;
    }

    w->updateSourceOutput(*i);
}

void client_cb(pa_context *, const pa_client_info *i, int eol, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(("Client callback failure"));
        return;
    }

    if (eol > 0) {
        dec_outstanding(w);
        return;
    }

    w->updateClient(*i);
}

void server_info_cb(pa_context *, const pa_server_info *i, void *userdata) {
    MainWindow *w = static_cast<MainWindow*>(userdata);

    if (!i) {
        show_error("Server info callback failure");
        return;
    }

    w->updateServer(*i);
    dec_outstanding(w);
}

void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    void *w = userdata;

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                //w->removeSink(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_info_by_index(c, index, sink_cb, w))) {
                    show_error(("pa_context_get_sink_info_by_index() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                // w->removeSource(index); //
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_source_info_by_index(c, index, source_cb, w))) {
                    show_error(("pa_context_get_source_info_by_index() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                // w->removeSinkInput(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_input_info(c, index, sink_input_cb, w))) {
                    show_error(("pa_context_get_sink_input_info() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                // w->removeSourceOutput(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_source_output_info(c, index, source_output_cb, w))) {
                    show_error(("pa_context_get_sink_input_info() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_CLIENT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                // w->removeClient(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_client_info(c, index, client_cb, w))) {
                    show_error(("pa_context_get_client_info() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SERVER: {
                pa_operation *o;
                if (!(o = pa_context_get_server_info(c, server_info_cb, w))) {
                    show_error(("pa_context_get_server_info() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_CARD:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                // w->removeCard(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_card_info_by_index(c, index, card_cb, w))) {
                    show_error(("pa_context_get_card_info_by_index() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

    }
}

void on_ready(pa_context *c) {
    printf("Ready\n");

    pa_operation *o;

    reconnect_timeout = 1;

    /* Create event widget immediately so it's first in the list */
    //w->createEventRoleWidget();

    pa_context_set_subscribe_callback(c, subscribe_cb, NULL);

    if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                   (PA_SUBSCRIPTION_MASK_SINK|
                                    PA_SUBSCRIPTION_MASK_SOURCE|
                                    PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                    PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                    PA_SUBSCRIPTION_MASK_CLIENT|
                                    PA_SUBSCRIPTION_MASK_SERVER|
                                    PA_SUBSCRIPTION_MASK_CARD), NULL, NULL))) {
        show_error(("pa_context_subscribe() failed"));
        return;
    }
    pa_operation_unref(o);
    void *w = NULL;
    /* Keep track of the outstanding callbacks for UI tweaks */
    n_outstanding = 0;

    if (!(o = pa_context_get_server_info(c, server_info_cb, w))) {
        show_error(("pa_context_get_server_info() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

    if (!(o = pa_context_get_client_info_list(c, client_cb, w))) {
        show_error(("pa_context_client_info_list() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

    if (!(o = pa_context_get_card_info_list(c, card_cb, w))) {
        show_error(("pa_context_get_card_info_list() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

    if (!(o = pa_context_get_sink_info_list(c, sink_cb, w))) {
        show_error(("pa_context_get_sink_info_list() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

    if (!(o = pa_context_get_source_info_list(c, source_cb, w))) {
        show_error(("pa_context_get_source_info_list() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

    if (!(o = pa_context_get_sink_input_info_list(c, sink_input_cb, w))) {
        show_error(("pa_context_get_sink_input_info_list() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

    if (!(o = pa_context_get_source_output_info_list(c, source_output_cb, w))) {
        show_error(("pa_context_get_source_output_info_list() failed"));
        return;
    }
    pa_operation_unref(o);
    n_outstanding++;

}

int main(int argc, char** argv) {
    Glib::OptionContext options;
    Gtk::Main kit(argc, argv, options);

    MainWindow *w = new MainWindow();
    //ca_context_set_driver(ca_gtk_context_get(), "pulse");

    pa_glib_mainloop *m = pa_glib_mainloop_new(g_main_context_default());
    g_assert(m);
    api = pa_glib_mainloop_get_api(m);
    g_assert(api);

    context = NULL;
    connect_to_pulse(w);
    if (reconnect_timeout >= 0) {
        printf("Do something\n");
        Gtk::Main::run();
    }

    if (reconnect_timeout < 0)
        show_error("Fatal Error: Unable to connect to PulseAudio");

    if (context)
        pa_context_unref(context);
    pa_glib_mainloop_free(m);

    delete w;

    return 0;
}
