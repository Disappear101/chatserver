#ifndef __CHAT_MESSAGE_H__
#define __CHAT_MESSAGE_H__

#include <memory>
#include <map>
#include <string>
#include "src/util.h"

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

class ChatMessage1 {
public:
    using ptr = std::shared_ptr<ChatMessage1>;

    /// @brief create ChatMessage from string
    /// @param v string
    /// @return ChatMessage
    static ChatMessage1::ptr Create(const std::string& v);
    ChatMessage1();

    /// @brief get value according to key
    /// @param name key
    /// @return value
    template<class T>
    T get(const std::string& name) {
        return root[name].as<T>();
    }

    /// @brief set key-value pair
    /// @param name key
    /// @param val value
    template<class T>
    void set(const std::string& name, const T& val) {
        root[name] = val;
    }

    std::string toString() const;

    void clear();
private:
    Json::Value root;
};

}


#endif