#ifndef __CHAT_SESSION_H__
#define __CHAT_SESSION_H__

#include "src/http/ws_session.h"

namespace chat {

struct OnlineUserInfo
{
    int32_t     userid;
    std::string username;
    std::string nickname;
    std::string password;
    int32_t     clienttype;     // client type, 0-unknown, 1-pc, 2-android/ios
    int32_t     status;         // 0-offline 1-online 2-busy 3-left 4-Invisibility
};

class ChatSession : public tao::http::WSSession {
public:
    using ptr = std::shared_ptr<ChatSession>;

private:
    int32_t           m_id;                 //session id
    OnlineUserInfo    m_userinfo;           //user info
    int32_t           m_seq;                //cur Session
    bool              m_isLogin;            //Cur Session is logged in
    time_t            m_lastPackageTime;    //last time of sending message
    //TimerId           m_checkOnlineTimerId; //
};

}

#endif