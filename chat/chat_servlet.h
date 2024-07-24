#ifndef __CHAT_SERVLET_H__
#define __CHAT_SERVLET_H__

#include "src/http/ws_servlet.h"
#include "src/mutex.h"
#include "chat_message.h"
#include <stdint.h>

namespace chat {

struct OnlineUserInfo
{
    using ptr = std::shared_ptr<OnlineUserInfo>;

    int32_t     userid;
    std::string username;
    std::string nickname;
    std::string password;
    int32_t     clienttype;     // client type, 0-unknown, 1-pc, 2-android/ios
    int32_t     status;         // 0-offline 1-online 2-busy 3-left 4-Invisibility
    bool        m_isLogin;
};

class ChatServlet : public tao::http::WSServlet {
public:
    using ptr = std::shared_ptr<ChatServlet>;

    ChatServlet();

    virtual int32_t onConnect(tao::http::HttpRequest::ptr header
                              ,tao::http::WSSession::ptr session) override;
    virtual int32_t onClose(tao::http::HttpRequest::ptr header
                             ,tao::http::WSSession::ptr session) override;
    virtual int32_t handle(tao::http::HttpRequest::ptr header
                           ,tao::http::WSFrameMessage::ptr msg
                           ,tao::http::WSSession::ptr session) override;

private:
    int32_t onLoginResponse(tao::http::HttpRequest::ptr header
                            ,tao::http::WSSession::ptr session
                            ,ChatMessage::ptr data);

    int32_t onRegisterResponse(tao::http::HttpRequest::ptr header
                            ,tao::http::WSSession::ptr session
                            ,ChatMessage::ptr data);

    
};


}

#endif