#ifndef __TAO_HTTP_RESOURCE_SERVLET_H__
#define __TAO_HTTP_RESOURCE_SERVLET_H__

#include "src/http/servlet.h"

namespace tao {
namespace http {

class ResourceServlet : public Servlet {
public:
    using ptr = std::shared_ptr<ResourceServlet>;
    ResourceServlet(const std::string& path);
    virtual int32_t handle(tao::http::HttpRequest::ptr request
                            ,tao::http::HttpResponse::ptr response
                            ,tao::http::HttpSession::ptr session) override;

private:
    std::string m_path;
};

}
    
} 

#endif