#ifndef __CHAT_MESSAGE_H__
#define __CHAT_MESSAGE_H__

#include <memory>
#include <map>
#include <string>

namespace chat {

class ChatMessage {
public:
    using ptr = std::shared_ptr<ChatMessage>;

    /// @brief create ChatMessage from string
    /// @param v string
    /// @return ChatMessage
    static ChatMessage::ptr Create(const std::string& v);
    ChatMessage();

    /// @brief get value according to key
    /// @param name key
    /// @return value
    std::string get(const std::string& name);

    /// @brief set key-value pair
    /// @param name key
    /// @param val value
    void set(const std::string& name, const std::string& val);

    std::string toString() const;

    void clear();
private:
    std::map<std::string, std::string>m_datas;
};

}


#endif