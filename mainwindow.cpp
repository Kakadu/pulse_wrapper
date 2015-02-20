#include "mainwindow.h"

CheckStateHandler::CheckStateHandler()
    : MessagesHandler()
{

}
bool CheckStateHandler::updateSink(const pa_sink_info &info) {
    mylog("%s idx=%d %s", __FUNCTION__, info.index, info.name);
    return true;
}
void CheckStateHandler::updateSource(const pa_source_info &info) {
    mylog("%s %s", __FUNCTION__, info.name);
}
void CheckStateHandler::updateSinkInput(const pa_sink_input_info &info) {
    mylog("%s idx=%d %s", __FUNCTION__, info.index, info.name);
    mylog("driver=%s", info.driver);
}
void CheckStateHandler::updateSourceOutput(const pa_source_output_info &info) {
    mylog("%s %s", __FUNCTION__, info.name);
}
void CheckStateHandler::updateClient(const pa_client_info &info) {
    mylog("%s idx=%d %s", __FUNCTION__, info.index, info.name);
}
void CheckStateHandler::updateServer(const pa_server_info &info) {
    mylog("%s default_sink_name: %s", __FUNCTION__, info.default_sink_name);
}
void CheckStateHandler::updateVolumeMeter(uint32_t source_index, uint32_t sink_input_index, double v) {

}
//void updateRole(const pa_ext_stream_restore_info &info);

void CheckStateHandler::updateCard(const pa_card_info& i) {

}

CURRENT_STATUS  CheckStateHandler::currentStatus() {
    return NO_STREAM;
}
