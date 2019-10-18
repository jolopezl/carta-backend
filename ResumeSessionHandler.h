#ifndef CARTA_BACKEND__RESUMESESSIONHANDLER_H_
#define CARTA_BACKEND__RESUMESESSIONHANDLER_H_

#include <fmt/format.h>

#include "EventHeader.h"
#include "OnMessageTask.h"
#include "Session.h"
#include "Util.h"

class ResumeSessionHandler {
public:
    explicit ResumeSessionHandler(Session* session, CARTA::ResumeSession message, uint32_t request_id);

private:
    Session* _session;
    uint32_t _request_id;
    CARTA::ResumeSession _message;

    void Execute();

    void Command(CARTA::CloseFile message);
    void Command(CARTA::OpenFile message);
    void Command(CARTA::SetImageChannels message);
    void Command(CARTA::SetRegion message);
};

#endif // CARTA_BACKEND__RESUMESESSIONHANDLER_H_
