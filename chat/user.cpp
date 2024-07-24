#include "user.h"
#include "src/log.h"
#include <sstream>
#include <jsoncpp/json/json.h>

namespace chat {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("chat");

UserManager::UserManager()
{
    
}

UserManager::~UserManager()
{

}

bool UserManager::init(const char* dbServer, const char* dbUserName, const char* dbPassword, const char* dbName)
{
    m_strDbServer = dbServer;
    m_strDbUserName = dbUserName;
    if (dbPassword != NULL)
        m_strDbPassword = dbPassword;
    m_strDbName = dbName;

    if (!loadUsersFromDb())
        return false;

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

}