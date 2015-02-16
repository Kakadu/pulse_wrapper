#include <stdio.h>
#include <cassert>

#include <gtkmm.h>
#include <gtk/gtk.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
// #include <pulse/ext-stream-restore.h>
// #include <pulse/ext-device-manager.h>

static pa_context* context = NULL;
static pa_mainloop_api* api = NULL;

void context_state_callback(pa_context *c, void *userdata) {
    printf("%s\n", __FUNCTION__);
}

bool connect_to_pulse() {
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

    pa_context_set_state_callback(context, context_state_callback, NULL);

    printf("A\n");
    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        if (pa_context_errno(context) == PA_ERR_INVALID) {
            char msg[] = "Connection to PulseAudio failed. Automatic retry in 5s\n\n"
                "In this case this is likely because PULSE_SERVER in the Environment/X11 Root Window Properties\n"
                "or default-server in client.conf is misconfigured.\n"
                "This situation can also arrise when PulseAudio crashed and left stale details in the X11 Root Window.\n"
                "If this is the case, then PulseAudio should autospawn again, or if this is not configured you should\n"
                "run start-pulseaudio-x11 manually.";
            printf ("%s\n", msg);
            exit(1);
        }
        else {
            return true;
        }
    }

    return false;
}

int main(int argc, char** argv) {
    Glib::OptionContext options;
    Gtk::Main kit(argc, argv, options);

    //ca_context_set_driver(ca_gtk_context_get(), "pulse");

    pa_glib_mainloop *m = pa_glib_mainloop_new(g_main_context_default());
    g_assert(m);
    api = pa_glib_mainloop_get_api(m);
    g_assert(api);

    context = NULL;
    if (!connect_to_pulse()) {
        printf("can't connect");
        return 1;
    }
    printf("connected\n");
    return 0;
}
