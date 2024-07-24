#include "src/module.h"

namespace chat {

class MyModule : public tao::Module {
public:
    typedef std::shared_ptr<MyModule> ptr;
    MyModule();
    bool onLoad() override;
    bool onUnload() override;
    bool onServerReady() override;
    bool onServerUp() override;
};

}
