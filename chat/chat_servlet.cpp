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

std::atomic<int> session_id{0};

int32_t ChatServlet::onConnect(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session)
{
    TAO_LOG_INFO(g_logger) << "onConnect " << session;
    ++session_id;
    header->setHeader("$id", std::to_string(session_id));
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
        return onGetFriendListResponse(header, session, data);
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
                rsp->set("msg_type", "kick_user");
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

    rsp->set("msg_type", "login");
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
        rsp->set("result", "400");
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
    User usr;
    usr.userName = data->get("username");
    usr.nickName = data->get("nickname");
    usr.passWord = data->get("password");

    User cachedUser;
    cachedUser.userId = 0;
    UserMgr::GetInstance()->getUserInfoByUsername(usr.userName, cachedUser);

    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
    rsp->set("msg_type", "register");
    if (cachedUser.userId != 0) {
        rsp->set("result", "400");
        rsp->set("msg", "this username has been registered already");
    } else {
        if (!UserMgr::GetInstance()->addUser(usr)) {
            rsp->set("result", "400");
            rsp->set("msg", "register failed");
        } else {
            rsp->set("result", "400");
            rsp->set("msg", "ok");
        }
    }
    return SendMessage(session, rsp);
}
int32_t ChatServlet::onGetFriendListResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{
    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
    rsp->set("msg_type", "getfriendlist");

    std::string id = header->getHeader("$id");
    tao::RWMutex::ReadLock lock(m_mutex);
    auto userOline = m_sessions[id].second;
    m_mutex.unlock();

    User cur;
    UserMgr::GetInstance()->getUserInfoByUserId(userOline->userid, cur);

    if (cur.friends.empty()) {
        rsp->set("nfriends", "0");
        rsp->set("friends", "[]");
        return SendMessage(session, rsp);
    }

    std::stringstream friendsinfo;
    rsp->set("nfriends", std::to_string(cur.friends.size()));
    for (auto& friendinfo : cur.friends) {
        User u;
        UserMgr::GetInstance()->getUserInfoByUserId(friendinfo.friendid, u);
        ChatMessage::ptr tmp = std::make_shared<ChatMessage>();
        tmp->set("userid", std::to_string(userOline->userid));
        tmp->set("username", userOline->username);
        tmp->set("nickname", userOline->nickname);
        tmp->set("facetype", std::to_string(u.faceType));
        tmp->set("customAvatar", u.customAvatar);
        tmp->set("gender", std::to_string(u.gender));
        tmp->set("birthday", std::to_string(u.birthday));
        tmp->set("signature", u.signature);
        tmp->set("address", u.address);
        tmp->set("phonenumber", u.phoneNumber);
        tmp->set("mail", u.mail);
        friendsinfo << tmp->toString();
    }
    rsp->set("friends", friendsinfo.str());

    TAO_LOG_INFO(g_logger) << "Found friends" << rsp->toString();

    return SendMessage(session, rsp);
}

int32_t ChatServlet::onFindUserResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{
    //{ "type": 1, "username" : "zhang" }
    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();

    rsp->set("msg_type", "finduser");

    std::string username = data->get("username");
    User u;
    if (!UserMgr::GetInstance()->getUserInfoByUsername(username, u)) {
        rsp->set("msg", "ok");
        rsp->set("userinfo", "[]");
    } else {
        rsp->set("msg", "ok");
        ChatMessage::ptr tmp = std::make_shared<ChatMessage>();
        tmp->set("userid", std::to_string(u.userId));
        tmp->set("username", u.userName);
        tmp->set("nickname", u.nickName);
        tmp->set("facetype", std::to_string(u.faceType));
        tmp->set("customAvatar", u.customAvatar);
        tmp->set("gender", std::to_string(u.gender));
        tmp->set("birthday", std::to_string(u.birthday));
        tmp->set("signature", u.signature);
        
        rsp->set("userinfo", tmp->toString());

        TAO_LOG_INFO(g_logger) << "found user: " << rsp->toString();
    }
    return SendMessage(session, rsp);
}

int32_t ChatServlet::onChangeUserStatusResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{
    //{"type": 1, "onlinestatus" : 1}
    std::string status = data->get("onlinestatus");
    std::string id = header->getHeader("$id");
    tao::RWMutex::ReadLock lock(m_mutex);
    auto userOline = m_sessions[id].second;
    m_mutex.unlock();

    if (userOline->status == stoi(status)) {
        return 0;
    }
    userOline->status = stoi(status);

    std::list<User>friends;
    UserMgr::GetInstance()->getFriendInfoByUserId(userOline->userid, friends);
    for (const auto& iter : friends) {
        //multiple terminal for the same id
        std::list<tao::http::WSSession::ptr> sessions = get_sessions_by_userId(iter.userId);
        ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
        rsp->set("msg_type", "userstatuschange");
        rsp->set("result", "400");
        rsp->set("onlinestatus", status);
        session_notify(rsp, sessions);

    }
    
    return 0;
}

int32_t ChatServlet::onOperateFriendResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{
    //"operation": 1-add friend request, 2-receive add reqeust, 3-response add friend, 4-exit group/del friend, 5-response del friend
    std::string id = header->getHeader("$id");
    tao::RWMutex::ReadLock lock(m_mutex);
    auto userOline = m_sessions[id].second;
    m_mutex.unlock();

    std::string operation = data->get("operation");
    int32_t targetUserid = stoi(data->get("userid"));
    if (targetUserid > GROUPID_BOUBDARY) {
        //exit group
        if (operation == "4") {
            deleteFriend(session, targetUserid, userOline->userid);
            return 0;
        }
    }

    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
    if (operation == "4") {
        deleteFriend(session, targetUserid, userOline->userid);
        return 0;
    } else if (operation == "1") {
        if (UserMgr::GetInstance()->isFriend(userOline->userid, targetUserid)) {
            TAO_LOG_INFO(g_logger) << "Friendship already, unbale to add friend, friend id: " << targetUserid
                                    << " userid: " << targetUserid
                                    << " client: " << session->getSocket()->getLocalAddress();
        }
        rsp->set("userid", std::to_string(userOline->userid));
        rsp->set("type", "2");
        rsp->set("username", userOline->username);
    } else if (operation == "3") {
        std::string accept = data->get("accept");
        if (accept == "1") {
            if (!UserMgr::GetInstance()->makeFriendRelationshipInDB(targetUserid, userOline->userid)) {
                TAO_LOG_ERROR(g_logger) << "makeFriendRelationshipInDB error: " << rsp->toString()
                                        << " userid: " << userOline->userid;
                return 0;
            }
            if (!UserMgr::GetInstance()->updateUserRelationshipInMemory(userOline->userid, targetUserid, FRIEND_OPERATION_ADD)) {
                TAO_LOG_ERROR(g_logger) << "updateUserRelationshipInMemory error: " << rsp->toString()
                                        << " userid: " << userOline->userid;
                return 0;
            }
        }
        rsp->set("userid", std::to_string(userOline->userid));
        rsp->set("type", "3");
        rsp->set("username", userOline->username);
        rsp->set("accept", accept);

        User targetUser;
        if (!UserMgr::GetInstance()->getUserInfoByUserId(targetUserid, targetUser)) {
            TAO_LOG_ERROR(g_logger) << "getUserInfoByUserId error, targetuserid: " << targetUserid
                                        << " userid: " << userOline->userid
                                        << " data: " << data->toString();
            return 0;
        }
        //send notify of successful adding friend to src user
        ChatMessage::ptr msg = std::make_shared<ChatMessage>();
        msg->set("msg_type", "operatefriend");
        msg->set("userid", std::to_string(targetUser.userId));
        msg->set("type", "3");
        msg->set("username", targetUser.userName);
        msg->set("accept", accept);
        return SendMessage(session, msg);
    }

    rsp->set("msg_type", "operatefriend");
    //send notify of successfuly adding friend to dest users
    std::list<tao::http::WSSession::ptr> sessions = get_sessions_by_userId(targetUserid);
    //dest user is not online
    if (sessions.empty()) {
        TAO_LOG_INFO(g_logger) << "userid: " << targetUserid
                                << ", is not online, cache notify msg, msg: " << rsp->toString();
        //TODO: add notify to the msg cache
        return 0;
    }
    session_notify(rsp, sessions);

    TAO_LOG_INFO(g_logger) << "response to client: userid-" << targetUserid
                            << ", cmd=msg_type_addfriend, data: " << data->toString();

    return 0;
}

int32_t ChatServlet::deleteFriend(tao::http::WSSession::ptr session
                            ,int32_t friendid
                            ,int32_t userid)
{

    if (UserMgr::GetInstance()->releaseFriendRelationshipInDBAndMemory(friendid, userid)) {
        TAO_LOG_ERROR(g_logger) << "Delete friend - releaseFriendRelationshipInDBAndMemory error, friendid: " << friendid 
                                << ", userid: " << userid
                                <<", client: " << session->getSocket()->getLocalAddress();
    }
    User u;
    if (!UserMgr::GetInstance()->getUserInfoByUserId(friendid, u)) {
        TAO_LOG_ERROR(g_logger) << "Delete friend - getUserInfoByUserId error, friendid: " << friendid 
                                << ", userid: " << userid
                                <<", client: " << session->getSocket()->getLocalAddress();
    }
    if (!UserMgr::GetInstance()->updateUserRelationshipInMemory(userid, friendid, FRIEND_OPERATION_DELETE)) {
        TAO_LOG_ERROR(g_logger) << "Delete friend - getUserInfoByUserId error, friendid: " << friendid 
                                << ", userid: " << userid
                                <<", client: " << session->getSocket()->getLocalAddress();
    }

    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
    rsp->set("msg_type", "operatefriend");
    rsp->set("userid", std::to_string(userid));
    rsp->set("type", "5");

    //send to user who actively deleted
    SendMessage(session, rsp);

    //send to deleted user
    if (friendid < GROUPID_BOUBDARY) {
        std::list<tao::http::WSSession::ptr>sessions = get_sessions_by_userId(friendid);
        if (sessions.empty()) {
            //TODO: cache data
        } else {
            session_notify(rsp, sessions);
        }
    }

    

    return 0;
}

int32_t ChatServlet::OnChatResponse(tao::http::HttpRequest::ptr header, tao::http::WSSession::ptr session, ChatMessage::ptr data)
{
    /*
    {
        "targetid": 001
        "chatdata":
        {
            "msgType": 1, // 0-Unknown 1-text 2-shaking window  3-file
            "time": 2434167,
            "clientType": 0,		// 0-Unknown 1-pc 2-ios 3-android
            "font":["fontname", fontSize, fontColor, fontBold, fontItalic, fontUnderline],
            "content":
            [
                {"msgText": "text1"},
                {"msgText": "text2"},
                {"faceID": 101},
                {"faceID": 102},
                {"pic": ["name", "server_path", 400, w, h]},
                {"remotedesktop": 1},
                {"shake": 1},
                {"file":["name", "server_path", 400, onlineflag]}		//onlineflag: 0-offline file，other-online file
            ]
        }
    }*/
    std::string id = header->getHeader("$id");
    tao::RWMutex::ReadLock lock(m_mutex);
    auto userOline = m_sessions[id].second;
    m_mutex.unlock();

    int32_t targetid = stoi(data->get("targetid"));
    std::string chatdata = data->get("chatdata");

    ChatMessage::ptr rsp = std::make_shared<ChatMessage>();
    rsp->set("msg_type", "chat");
    rsp->set("sender", std::to_string(userOline->userid));
    rsp->set("receiver", std::to_string(targetid));
    rsp->set("chatdata", chatdata);

    if (!UserMgr::GetInstance()->saveChatMsgToDb(userOline->userid, targetid, chatdata)) {
        TAO_LOG_ERROR(g_logger) << "Write chat msg to db error, senderid: " << userOline->userid 
                                << ", targetid: " << targetid
                                << ", chatmsg: "<< data->toString()
                                << ", client: " << session->getSocket()->getLocalAddress();
    }

    //1t1 chat
    if (targetid < GROUPID_BOUBDARY) {
        std::list<tao::http::WSSession::ptr>sessions = get_sessions_by_userId(targetid);
        //offline
        if (sessions.empty()) {
            //TODO: cache chat msg
        } else {
            session_notify(rsp, sessions);
        }
    } else { //group chat
        std::list<User> friends;
        UserMgr::GetInstance()->getFriendInfoByUserId(targetid, friends);
        for (const auto& iter : friends) {
            //ignore self
            if (iter.userId == userOline->userid) {
                continue;
            }
            std::list<tao::http::WSSession::ptr> sessions = get_sessions_by_userId(targetid);
            //offline
            if (sessions.empty()) {
                //TODO: cache chat data
            } else {
                session_notify(rsp, sessions);
            }

        }
    }

    return 0;
}
}