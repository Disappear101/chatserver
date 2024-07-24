#include "chat_servlet.h"
#include "chat_message.h"
#include "user.h"
#include "src/util.h"

namespace chat {

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

tao::RWMutex m_mutex;
//name -> <session, OnlineUserInfo>
std::map<std::string,  std::pair<tao::http::WSSession::ptr, OnlineUserInfo::ptr>> m_sessions;

bool session_exists(const std::string& id) {
    TAO_LOG_INFO(g_logger) << "session_exists id=" << id;
    tao::RWMutex::ReadLock lock(m_mutex);
    auto it = m_sessions.find(id);
    return it != m_sessions.end();
}

void session_add(const std::string& id, tao::http::WSSession::ptr session) {
    TAO_LOG_INFO(g_logger) << "session_add id=" << id;
    tao::RWMutex::WriteLock lock(m_mutex);
    m_sessions[id] = std::make_pair(session, std::make_shared<OnlineUserInfo>());
}

void session_del(const std::string& id) {
    TAO_LOG_INFO(g_logger) << "session_add del=" << id;
    tao::RWMutex::WriteLock lock(m_mutex);
    m_sessions.erase(id);
}

 tao::http::WSSession::ptr get_session_by_userId_clientType(int32_t usrId, int clientType) {
    tao::RWMutex::ReadLock lock(m_mutex);
    for (const auto& iter : m_sessions)
    {
        if (iter.second.second->userid == usrId && iter.second.second->clienttype == clientType)
        {
            return iter.second.first;
        }
    }
    return nullptr;
 }

std::list<tao::http::WSSession::ptr> get_sessions_by_userId(int32_t userId) {
    std::list<tao::http::WSSession::ptr>sessions;
    tao::RWMutex::ReadLock lock(m_mutex);
    for (const auto & iter : m_sessions) {
        if (iter.second.second->userid == userId) {
            sessions.push_back(iter.second.first);
        }
    }
    return sessions;
 }

int32_t SendMessage(tao::http::WSSession::ptr session
                    , ChatMessage::ptr msg) {
    TAO_LOG_INFO(g_logger) << msg->toString() << " - " << session;
    return session->sendMessage(msg->toString()) > 0 ? 0: 1;
}

void session_notify_all(ChatMessage::ptr msg, tao::http::WSSession::ptr session = nullptr) {
    tao::RWMutex::ReadLock lock(m_mutex);
    auto sessions = m_sessions;
    lock.unlock();

    for(auto& i : sessions) {
        if(i.second.first == session) {
            continue;
        }
        SendMessage(i.second.first, msg);
    }
}

void session_notify(ChatMessage::ptr msg, std::list<tao::http::WSSession::ptr>&to) {
    for(auto& i : to) {
        SendMessage(i, msg);
    }
}


ChatServlet::ChatServlet()
    :tao::http::WSServlet("chat_servlet"){

}

int32_t ChatServlet::onConnect(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session)
{
    TAO_LOG_INFO(g_logger) << "onConnect " << session;
    auto id = header->getHeader("$id");
    session_add(id, session);
    return 0;
}

int32_t ChatServlet::onClose(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session)
{
    auto id = header->getHeader("$id");
    TAO_LOG_INFO(g_logger) << "onClose " << session << " id=" << id;
    if(!id.empty()) {
        session_del(id);
        ChatMessage::ptr nty = std::make_shared<ChatMessage>();
        nty->set("type", "user_leave");
        nty->set("time", tao::Time2Str(time(0)));
        nty->set("name", id);
        //session_notify(nty);
    }
    return 0;
}

int32_t ChatServlet::handle(tao::http::HttpRequest::ptr header, tao::http::WSFrameMessage::ptr msg, tao::http::WSSession::ptr session)
{
    TAO_LOG_INFO(g_logger) << "handle " << session
            << " opcode=" << msg->getOpcode()
            << " data=" << msg->getData();

    auto data = ChatMessage::Create(msg->getData());

    auto id = header->getHeader("$id");
    //if wrong data format, erase session
    if (!data) {
        if (!id.empty()) {
            tao::RWMutex::WriteLock lock(m_mutex);
            m_sessions.erase(id);
        }
    }

    auto type = data->get("type");

    if (type =="login") {
        return onLoginResponse(header, session, data);
    } 
    else if (type == "register") {
        return onRegisterResponse(header, session, data);
    }
    else if (type == "get_friend_list") {

    }
    else if (type == "find_user") {

    }
    else if (type == "operate_friend") {

    }
    else if (type == "change_user_status") {

    }
    else if (type == "modify_password") {

    }
    else if (type == "create_group") {

    }
    else if (type == "get_groupmembers") {

    }
    else if (type == "chat_signle") {

    }
    else if (type == "chat_group") {

    }
    else if (type == "kicj_user") {

    }
    else if (type == "remote_desktop") {

    }
    else if (type == "update_teaminfo") {

    }
    else if (type == "modify_friendmarkname") {
    }
    else if (type == "move_friend_to_otherteam") {

    }
    else {
        ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
        rsp->set("result", "400");
        rsp->set("msg", "type is not found");
        return SendMessage(session, rsp);
    }
    return 0;
}

int32_t ChatServlet::onLoginResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{
    //{"username": "13917043329", "password": "123", "clienttype": 1, "status": 1}
    std::string id = header->getHeader("$id");

    tao::RWMutex::ReadLock lock(m_mutex);
    auto userOline = m_sessions[id].second;
    m_mutex.unlock();

    std::string username = data->get("username");
    std::string password = data->get("password");
    int clientType = std::atoi(data->get("clienttype").c_str());

    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
    User cachedUser;
    cachedUser.userId = 0;
    UserMgr::GetInstance()->getUserInfoByUsername(username, cachedUser);
    if (cachedUser.userId == 0) {
        rsp->set("result", "400");
        rsp->set("msg", "not registered");
    } else {
        if (cachedUser.passWord != password) {
            rsp->set("result", "400");
            rsp->set("msg", "incorrect password");
        } else {
            auto online_session = get_session_by_userId_clientType(cachedUser.userId, clientType);
            //if session already exist(user is online in the same device)
            if (online_session) {
                rsp->set("type", "kick_user");
                rsp->set("msg", "login in other device");
                SendMessage(online_session, rsp);
                TAO_LOG_INFO(g_logger) << "Response to client, userId: " << cachedUser.userId
                                << "cmd = kick_user";
            }

            userOline->userid = cachedUser.userId;
            userOline->username= cachedUser.userName;
            userOline->nickname = cachedUser.nickName;
            userOline->password = cachedUser.passWord;
            userOline->clienttype = clientType;
            userOline->status = std::atoi(data->get("status").c_str());

            rsp->set("result", "400");
            rsp->set("msg", "ok");
            rsp->set("userid", std::to_string(userOline->userid));
            rsp->set("username", userOline->username);
            rsp->set("nickname", userOline->nickname);
            rsp->set("facetype", std::to_string(cachedUser.faceType));
            rsp->set("customAvatar", cachedUser.customAvatar);
            rsp->set("gender", std::to_string(cachedUser.gender));
            rsp->set("birthday", std::to_string(cachedUser.birthday));
            rsp->set("signature", cachedUser.signature);
            rsp->set("address", cachedUser.address);
            rsp->set("phonenumber", cachedUser.phoneNumber);
            rsp->set("mail", cachedUser.mail);
        }
    }

    //send response
    SendMessage(session, rsp);

    TAO_LOG_INFO(g_logger) << "Response to client: " << id
                            << " cmd = login, data = " << rsp->toString();
    
    userOline->m_isLogin = true;

    //notify online message to friends
    std::list<User> friends;
    UserMgr::GetInstance()->getFriendInfoByUserId(userOline->userid, friends);
    rsp->clear();
    for (const auto& iter : friends)
    {
        //one user id refers to multiple session(different platform)
        auto sessions = get_sessions_by_userId(iter.userId);
        rsp->set("type", "user_status_change");
        rsp->set("onlinestatus", "1");
        rsp->set("clientType", std::to_string(clientType));
        session_notify(rsp, sessions);
        TAO_LOG_INFO(g_logger) << "send to friend: " << iter.userId
                                << " cmd = user_status_change, data = " << rsp->toString();
    }  
    return 0;
}

int32_t ChatServlet::onRegisterResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{

    return 0;
}
}