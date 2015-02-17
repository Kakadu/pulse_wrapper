#include <glib.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#define mylog(...)       g_log("", G_LOG_LEVEL_MESSAGE, __VA_ARGS__)

class MainWindow {

public:
    bool updateSink(const pa_sink_info &info) {
        mylog("%s idx=%d %s", __FUNCTION__, info.index, info.name);
        return true;
    }
    void updateSource(const pa_source_info &info) {
        mylog("%s %s", __FUNCTION__, info.name);

    }
    void updateSinkInput(const pa_sink_input_info &info) {
        mylog("%s idx=%d %s", __FUNCTION__, info.index, info.name);
        mylog("driver=%s", info.driver);
    }
    void updateSourceOutput(const pa_source_output_info &info) {
        mylog("%s %s", __FUNCTION__, info.name);
    }
    void updateClient(const pa_client_info &info) {
        mylog("%s idx=%d %s", __FUNCTION__, info.index, info.name);
    }
    void updateServer(const pa_server_info &info) {
        mylog("%s default_sink_name: %s", __FUNCTION__, info.default_sink_name);
    }
    void updateVolumeMeter(uint32_t source_index, uint32_t sink_input_index, double v);
    //void updateRole(const pa_ext_stream_restore_info &info);

    void updateCard(const pa_card_info& i) {

    }
};
