#include <glib.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#define mylog(...)       g_log("", G_LOG_LEVEL_MESSAGE, __VA_ARGS__)

enum CURRENT_STATUS {
    NO_STREAM = 0,
    HAS_STREAM = 1,
    NEED_RESET = 2
};
class MessagesHandler {
public:
    virtual bool updateSink(const pa_sink_info &info) = 0;
    virtual void updateSource(const pa_source_info &info) = 0;
    virtual void updateSinkInput(const pa_sink_input_info &info) = 0;
    virtual void updateSourceOutput(const pa_source_output_info &info) = 0;
    virtual void updateClient(const pa_client_info &info) = 0;
    virtual void updateServer(const pa_server_info &info) = 0;
    virtual void updateVolumeMeter(uint32_t source_index, uint32_t sink_input_index, double v) = 0;
    virtual void updateCard(const pa_card_info& i) = 0;
    virtual CURRENT_STATUS currentStatus() = 0;
};

class CheckStateHandler : public MessagesHandler {
public:
    CheckStateHandler();
    bool updateSink(const pa_sink_info &info);
    void updateSource(const pa_source_info &info);
    void updateSinkInput(const pa_sink_input_info &info);
    void updateSourceOutput(const pa_source_output_info &info);
    void updateClient(const pa_client_info &info);
    void updateServer(const pa_server_info &info);
    void updateVolumeMeter(uint32_t source_index, uint32_t sink_input_index, double v);
    void updateCard(const pa_card_info& i);
    CURRENT_STATUS currentStatus();
};

class MainWindow {
private:
    MessagesHandler* m_curHandler = nullptr;
public:
    MainWindow() {
        //mylog("Creating MainWindow");
        m_curHandler = new CheckStateHandler();
    }
    MessagesHandler* handler() const {
        g_assert(m_curHandler != nullptr);
        return m_curHandler;
    }
};
