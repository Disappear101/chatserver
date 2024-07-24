#include "resource_servlet.h"
#include "src/log.h"
#include <fstream>
#include <iostream>

namespace tao {
namespace http {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

ResourceServlet::ResourceServlet(const std::string &path)
    :Servlet("ResourceServlet")
    ,m_path(path){
}

int32_t ResourceServlet::handle(tao::http::HttpRequest::ptr request, tao::http::HttpResponse::ptr response, tao::http::HttpSession::ptr session)
{
    auto path = m_path + "/" + request->getPath();
    TAO_LOG_INFO(g_logger) << "handle " << path;
    if (path.find("..") != std::string::npos) {
        response->setBody("invalid path");
        response->setStatus(tao::http::HttpStatus::NOT_FOUND);
        return 0;
    }

    std::ifstream ifs(path);
    if (!ifs) {
        response->setBody("invalid file");
        response->setStatus(tao::http::HttpStatus::NOT_FOUND);
        return 0;
    }
    std::stringstream ss;
    std::string line;

    while (std::getline(ifs, line)) {
        ss << line << std::endl;
    }
    response->setBody(ss.str());
    return 0;
}
}
}