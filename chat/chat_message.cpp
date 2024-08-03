#include "chat_message.h"

namespace chat {

ChatMessage::ptr ChatMessage::Create(const std::string &v)
{
    Json::Value json;
    if (!tao::JsonUtil::FromString(json, v)) {
        return nullptr;
    }
    ChatMessage::ptr rt = std::make_shared<ChatMessage>();
    auto names = json.getMemberNames();
    for (auto&i : names) {
        rt->m_datas[i] = json[i].asString();
    }
    return rt;
}

ChatMessage::ChatMessage()
{

}

std::string ChatMessage::get(const std::string &name)
{
    auto it = m_datas.find(name);
    return it == m_datas.end() ? "" : it->second;
}

void ChatMessage::set(const std::string &name, const std::string &val)
{
    m_datas[name] = val;
}

std::string ChatMessage::toString() const
{
    Json::Value json;
    for(auto& i : m_datas) {
        json[i.first] = i.second;
    }
    return tao::JsonUtil::ToString(json);
}
void ChatMessage::clear()
{
    m_datas.clear();
}

ChatMessage1::ptr ChatMessage1::Create(const std::string &v)
{
    Json::Value json;
    if (!tao::JsonUtil::FromString(json, v)) {
        return nullptr;
    }
    ChatMessage1::ptr rt = std::make_shared<ChatMessage1>();
    rt->root = json;
    return rt;
}

ChatMessage1::ChatMessage1()
{

}

std::string ChatMessage1::toString() const
{
    return tao::JsonUtil::ToString(root);
}
void ChatMessage1::clear()
{
    root.clear();
}
}