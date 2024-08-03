#include "user.h"
#include "src/log.h"
#include <sstream>
#include <jsoncpp/json/json.h>
#include "src/db/mysql.h"
#include "src/config.h"
#include "src/util.h"

namespace chat {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("chat");

static tao::ConfigVar<std::map<std::string, std::map<std::string, std::string> > >::ptr g_mysql_dbs
    = tao::Config::Lookup("mysql.dbs", std::map<std::string, std::map<std::string, std::string> >()
            , "mysql dbs");

UserManager::UserManager()
{
    
}

UserManager::~UserManager()
{

}

bool UserManager::load_user_callback(MYSQL_ROW row, int field_count, int row_no)
{
    if (field_count < 13) {
        TAO_LOG_INFO(g_logger) << "field_count is less than expectation";
        return false;
    }
    User usr;
    usr.userId = std::stoi(row[0]);
    usr.userName = std::string(row[1]);
    usr.nickName = std::string(row[2]);
    usr.passWord = std::string(row[3]);
    usr.faceType = std::stoi(row[4]);
    usr.customAvatar = row[5] ? std::string(row[5]) : "";
    usr.gender = std::stoi(row[6]);
    usr.birthday = std::stoi(row[7]);
    usr.signature = row[8] ? std::string(row[8]) : "";
    usr.address = row[9] ? std::string(row[9]) : "";
    usr.phoneNumber = row[10] ? std::string(row[10]) : "";
    usr.mail = row[11] ? std::string(row[11]) : "";
    usr.teaminfo = row[12] ? std::string(row[12]) : "";

    m_allCachedUsers.emplace_back(usr);

    if (usr.userId > m_baseUserId) {
        m_baseUserId = usr.userId;
    }
    return true;
    
}


bool UserManager::init()
{
    auto dbs = g_mysql_dbs->getValue();
    if (dbs.empty()) {
        TAO_LOG_ERROR(g_logger) << "no database config files";
        return false;
    }
    m_strDbServer = dbs.begin()->first;
    auto param = dbs[m_strDbServer];
    m_strDbUserName = tao::GetParamValue<std::string>(param, "user");
    m_strDbPassword = tao::GetParamValue<std::string>(param, "password");
    m_strDbName = tao::GetParamValue<std::string>(param, "dbname");

    if (!loadUsersFromDb()) {
        return false;
    }

    for (auto& iter : m_allCachedUsers)
    {
        if (!loadRelationshipFromDb(iter.userId, iter.friends))
        {
            TAO_LOG_ERROR(g_logger) << "Load relationship from db error, userid:" << iter.userId;
            continue;
        }
    }

    return true;
}

bool UserManager::addUser(User &u)
{
    ++m_baseUserId;
    char sql[256] = { 0 };
    snprintf(sql, 256, "INSERT INTO t_user(f_user_id, f_username, f_nickname, f_password, f_register_time) VALUES(%d, '%s', '%s', '%s', NOW())"
                , m_baseUserId.load()
                , u.userName.c_str()
                , u.nickName.c_str()
                , u.passWord.c_str());
    std::string sql_str(sql);
    if (tao::MySQLMgr::GetInstance()->execute("chat1", sql_str)) {
        TAO_LOG_ERROR(g_logger) << "insert user error, sql: " << sql_str;
        return false;
    }

    u.userId = m_baseUserId;
    u.faceType = 0;
    u.birthday = 19700101;
    u.gender = 0;
    u.ownerid = 0;

    {
        MutexType::Lock lock(m_mutex);
        m_allCachedUsers.push_back(u);
    }

    return true;
}

bool UserManager::makeFriendRelationshipInDB(int32_t smallUserid, int32_t greaterUserid)
{
    return false;
}
bool UserManager::releaseFriendRelationshipInDBAndMemory(int32_t smallUserid, int32_t greaterUserid)
{
    return false;
}
bool UserManager::isFriend(int32_t userid, int32_t friendid)
{
    return false;
}
bool UserManager::deleteFriendToUser(int32_t userid, int32_t friendid)
{
    return false;
}
bool UserManager::addFriendToUser(int32_t userid, int32_t friendid)
{
    return false;
}
bool UserManager::updateUserRelationshipInMemory(int32_t userid, int32_t target, FRIEND_OPERATION operation)
{
    return false;
}
bool UserManager::updateUserInfoInDb(int32_t userid, const User &newuserinfo)
{
    return false;
}

bool UserManager::deleteTeam(int32_t userid, const std::string &deletedteamname)
{
    return false;
}

bool UserManager::modifyTeamName(int32_t userid, const std::string &newteamname, const std::string &oldteamname)
{
    return false;
}

bool UserManager::updateMarknameInDb(int32_t userid, int32_t friendid, const std::string &newmarkname)
{
    return false;
}

bool UserManager::saveChatMsgToDb(int32_t senderid, int32_t targetid, const std::string &chatmsg)
{
    return false;
}

bool UserManager::addGroup(const char *groupname, int32_t ownerid, int32_t &groupid)
{
    return false;
}

bool UserManager::moveFriendToOtherTeam(int32_t userid, int32_t friendid, const std::string &newteamname)
{
    return false;
}

bool UserManager::updateUserTeamInfoInDbAndMemory(int32_t userid, const std::string &newteaminfo)
{
    return false;
}

bool UserManager::modifyUserPassword(int32_t userid, const std::string &newpassword)
{
    return false;
}

bool UserManager::getUserInfoByUsername(const std::string &username, User &u)
{
    MutexType::Lock lock(m_mutex);
    for (const auto& iter : m_allCachedUsers) {
        if (iter.userName == username) {
            u = iter;
            return true;
        }
    }
    return false;
}

bool UserManager::getUserInfoByUserId(int32_t userid, User &u)
{
    MutexType::Lock lock(m_mutex);
    for (const auto& iter : m_allCachedUsers) {
        if (iter.userName == u.userName) {
            u = iter;
            return true;
        }
    }
    return false;
}
bool UserManager::getFriendInfoByUserId(int32_t userid, std::list<User> &friends)
{
    std::list<FriendInfo> friendInfo;
    MutexType::Lock lock(m_mutex);
    for (const auto & iter : m_allCachedUsers) {
        if (iter.userId == userid) {
            friendInfo = iter.friends;
        }
    }

    for (const auto & iter : friendInfo) {
        User u;
        for (const auto & iter2 : m_allCachedUsers) {
            if (iter2.userId == iter.friendid) {
                u = iter2;
                friends.push_back(u);
                break;
            }
        }
    }
    return false;
}

bool UserManager::loadUsersFromDb()
{
    tao::ISQLData::ptr res = tao::MySQLMgr::GetInstance()->query("chat1"
        , std::string("SELECT f_user_id, f_username, f_nickname, f_password,  f_facetype, f_customavatar, f_gender, f_birthday, f_signature, f_address, f_phonenumber, f_mail, f_teaminfo FROM t_user ORDER BY  f_user_id DESC"));
    if (nullptr == res) {
        TAO_LOG_INFO(g_logger) << "UserManager::query error, dbname: " << m_strDbName;
        return false;  
    }

    auto mysqlres = std::dynamic_pointer_cast<tao::MySQLRes>(res);
    return mysqlres->foreach(std::bind(&UserManager::load_user_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
bool UserManager::loadRelationshipFromDb(int32_t userid, std::list<FriendInfo> &r)
{
    char sql[256] = { 0 };
    snprintf(sql, 256, "SELECT f_user_id1, f_user_id2, f_user1_markname, f_user2_markname, f_user1_teamname, f_user2_teamname FROM t_user_relationship WHERE f_user_id1 = %d OR f_user_id2 = %d ", userid, userid);
    tao::ISQLData::ptr res = tao::MySQLMgr::GetInstance()->query("chat1"
        , std::string(sql)); 
    if (nullptr == res) {
        TAO_LOG_INFO(g_logger) << "UserManager::query error, dbname: " << m_strDbName;
        return false;
    }

    auto mysqlres = std::dynamic_pointer_cast<tao::MySQLRes>(res);
    int num_fields = mysqlres->getColumnCount();
    if (num_fields < 6) {
        TAO_LOG_INFO(g_logger) << "field_count is less than expectation";
        return false;
    }

    while (mysqlres->next()) {
        int32_t friendid1 = mysqlres->getInt32(0);
        int32_t friendid2 = mysqlres->getInt32(1);
        std::string markname1 = mysqlres->getString(2);
        std::string markname2 = mysqlres->getString(3);  
        std::string teamname1 = mysqlres->getString(4);
        std::string teamname2 = mysqlres->getString(5);
        if (teamname1.empty())
            teamname1 = DEFAULT_TEAMNAME;
        if (teamname2.empty())
            teamname2 = DEFAULT_TEAMNAME;
        FriendInfo fi;
        if (friendid1 == userid)
        {
            fi.friendid = friendid2;
            fi.markname = markname1;
            fi.teamname = teamname1;
            r.emplace_back(fi);
            TAO_LOG_INFO(g_logger) << "userid = " << userid
                    << ", friendid = " << friendid2;
        }
        else {
            fi.friendid = friendid1;
            fi.markname = markname2;
            fi.teamname = teamname2;
            r.emplace_back(fi);
            TAO_LOG_INFO(g_logger) << "userid = " << userid
                    << ", friendid = " << friendid2;        
        }
    }

    
    return true;
}
bool UserManager::getTeamInfoByUserId(int32_t userid, std::string &teaminfo)
{
    std::set<int32_t> friendsId;
    MutexType::Lock lock(m_mutex);
    for (const auto& iter : m_allCachedUsers) {
        if (iter.userId == userid) {
            teaminfo = iter.teaminfo;
            return true;
        }
    }
    return false;
}
bool UserManager::getFriendMarknameByUserId(int32_t userid1, int32_t friendid, std::string &markname)
{
    return false;
}
bool UserManager::getUserInfoByUserId(int32_t userid, User *&u)
{
    return false;
}
}