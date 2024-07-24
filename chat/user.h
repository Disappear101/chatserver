#ifndef __CHAT_USER_H__
#define __CHAT_USER_H__

#include <stdint.h>
#include <string>
#include <list>
#include <set>
#include <atomic>
#include "src/mutex.h"
#include "src/singleton.h"

namespace chat {

#define GROUPID_BOUBDARY   0x0FFFFFFF 

#define DEFAULT_TEAMNAME  "My Friends"

enum FRIEND_OPERATION
{
    FRIEND_OPERATION_ADD,
    FRIEND_OPERATION_DELETE
};

struct FriendInfo
{
    int32_t friendid;
    std::string  markname;
    std::string  teamname;
};

//user or group
struct User
{
    int32_t             userId;      //above 0x0FFFFFFF is group，under is user
    std::string         userName;    //username is string format of userid
    std::string         passWord;
    std::string         nickName;    
    int32_t             faceType;
    std::string         customAvatar;
    std::string         customAvatarfmt;//customized avatar format
    int32_t             gender;
    int32_t             birthday;
    std::string         signature;
    std::string         address;
    std::string         phoneNumber;
    std::string         mail;
    std::string         teaminfo;       //teaminfo for user；null for group
    int32_t             ownerid;        //for group，it is userid of group owner
    std::list<FriendInfo>   friends;        
};

class UserManager final
{
public:
    using MutexType = tao::Mutex;
    UserManager();
    ~UserManager();

    bool init(const char* dbServer, const char* dbUserName, const char* dbPassword, const char* dbName);

    UserManager(const UserManager& rhs) = delete;
    UserManager& operator=(const UserManager& rhs) = delete;

    bool addUser(User& u);
    bool makeFriendRelationshipInDB(int32_t smallUserid, int32_t greaterUserid);
    bool releaseFriendRelationshipInDBAndMemory(int32_t smallUserid, int32_t greaterUserid);
    bool updateUserRelationshipInMemory(int32_t userid, int32_t target, FRIEND_OPERATION operation);
    bool addFriendToUser(int32_t userid, int32_t friendid);
    bool deleteFriendToUser(int32_t userid, int32_t friendid);

    bool isFriend(int32_t userid, int32_t friendid);
    
    bool updateUserInfoInDb(int32_t userid, const User& newuserinfo);
    bool modifyUserPassword(int32_t userid, const std::string& newpassword);

    bool updateUserTeamInfoInDbAndMemory(int32_t userid, const std::string& newteaminfo);
    bool deleteTeam(int32_t userid, const std::string& deletedteamname);
    bool modifyTeamName(int32_t userid, const std::string& newteamname, const std::string& oldteamname);
    
    bool updateMarknameInDb(int32_t userid, int32_t friendid, const std::string& newmarkname);
    bool moveFriendToOtherTeam(int32_t userid, int32_t friendid, const std::string& newteamname);

    bool addGroup(const char* groupname, int32_t ownerid, int32_t& groupid);

    bool saveChatMsgToDb(int32_t senderid, int32_t targetid, const std::string& chatmsg);

    bool getUserInfoByUsername(const std::string& username, User& u);
    bool getUserInfoByUserId(int32_t userid, User& u);
    bool getUserInfoByUserId(int32_t userid, User*& u);
    bool getFriendInfoByUserId(int32_t userid, std::list<User>& friends);

    bool getFriendMarknameByUserId(int32_t userid1, int32_t friendid, std::string& markname);
    bool getTeamInfoByUserId(int32_t userid, std::string& teaminfo);

private:
    bool loadUsersFromDb();
    bool loadRelationshipFromDb(int32_t userid, std::list<FriendInfo>& r);

private:
    std::atomic_int         m_baseUserId{ 0 };        //m_baseUserId, 
    std::atomic<int>        m_baseGroupId{0x0FFFFFFF};
    std::list<User>         m_allCachedUsers;
    MutexType               m_mutex;

    std::string             m_strDbServer;
    std::string             m_strDbUserName;
    std::string             m_strDbPassword;
    std::string             m_strDbName;
};

using UserMgr = tao::Singleton<UserManager>;

}

#endif