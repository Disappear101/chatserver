#include "my_module.h"
#include "src/config.h"
#include "src/log.h"
#include "src/http/http_server.h"
#include "src/http/ws_server.h"
#include "src/application.h"
#include "resource_servlet.h"
#include "src/env.h"
#include "chat_servlet.h"
#include "src/db/mysql.h"
#include "user.h"

namespace chat {

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

MyModule::MyModule()
    :tao::Module("chatserver", "1.0", "") {
}

bool MyModule::onLoad() {
    TAO_LOG_INFO(g_logger) << "onLoad";
    return true;
}

bool MyModule::onUnload() {
    TAO_LOG_INFO(g_logger) << "onUnload";
    return true;
}

bool MyModule::onServerReady() {
    TAO_LOG_INFO(g_logger) << "onServerReady";

    //init database
    std::string abs_path = tao::EnvMgr::GetInstance()->getAbsolutePath("sql/chat.sql");
    tao::MySQLMgr::GetInstance()->executeFromFile("chat1", abs_path);

    //init user mananger
    UserMgr::GetInstance()->init();

    return true;
}


bool MyModule::onServerUp() {
    TAO_LOG_INFO(g_logger) << "onServerUp";

    std::vector<tao::TcpServer::ptr> svrs;
    if (!tao::Application::GetInstance()->getSever("http", svrs)) {
        TAO_LOG_INFO(g_logger) << "no httpserver alive";
        return false;
    }

    for (auto & i : svrs) {
        tao::http::HttpServer::ptr http_server = std::dynamic_pointer_cast<tao::http::HttpServer>(i);
        if (!http_server) {
            continue;
        }
        auto slt_dispatch = http_server->getServletDispatch();
        tao::http::ResourceServlet::ptr slt = std::make_shared<tao::http::ResourceServlet>(
            tao::EnvMgr::GetInstance()->getCwd()
        );
        slt_dispatch->addGlobServlet("/html/*", slt);
        TAO_LOG_INFO(g_logger) << "add Servlet";
    }

    svrs.clear();
    if (!tao::Application::GetInstance()->getSever("ws", svrs)) {
        TAO_LOG_INFO(g_logger) << "no ws alive";
        return false;
    }

    for(auto& i : svrs) {
        tao::http::WSServer::ptr ws_server =
            std::dynamic_pointer_cast<tao::http::WSServer>(i);

        tao::http::ServletDispatch::ptr slt_dispatch = ws_server->getWSServletDispatch();
        ChatServlet::ptr slt = std::make_shared<ChatServlet>();
        slt_dispatch->addServlet("/tao/chat", slt);
    }
    return true;
}

}

extern "C" {

tao::Module* CreateModule() {
    tao::Module* module = new chat::MyModule;
    TAO_LOG_INFO(chat::g_logger) << "CreateModule " << module;
    return module;
}

void DestoryModule(tao::Module* module) {
    TAO_LOG_INFO(chat::g_logger) << "CreateModule " << module;
    delete module;
}

}
