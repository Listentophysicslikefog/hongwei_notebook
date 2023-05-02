/**
 * @file message_info.cpp
 */
#include "message_info.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include "shm_log.h"
#include "util.h"
namespace shm::framework::memory
{
    // uint64_t 8 Bytes
    constexpr uint8_t ID_SIZE = 8;
    // message info size,m_seqNum + m_senderId + m_topicName
    const std::size_t MessageInfo::VAILD_MESSAGEINFO_SIZE = 2 * ID_SIZE + MAX_TOPIC_LEN * sizeof(char);

    MessageInfo::MessageInfo(const MessageInfo &another)
        : m_seqNum(another.m_seqNum)
    {
        memcpy(m_topicName, another.m_topicName, MAX_TOPIC_LEN);
    }

    MessageInfo::~MessageInfo() {}

    MessageInfo &MessageInfo::operator=(const MessageInfo &another)
    {
        if (this != &another)
        {
            memcpy(m_topicName, another.m_topicName, MAX_TOPIC_LEN);
            m_seqNum = another.m_seqNum;
        }
        return *this;
    }

    bool MessageInfo::operator==(const MessageInfo &another) const
    {

        return (m_topicName == another.m_topicName) && (m_seqNum == another.m_seqNum);
    }

    bool MessageInfo::operator!=(const MessageInfo &another) const
    {

        return !(*this == another);
    }

    bool MessageInfo::SerializeTo(std::string *const dst) const
    {
        dst->assign(reinterpret_cast<const char *>(&m_seqNum), sizeof(m_seqNum));
        return true;
    }

    bool MessageInfo::SerializeTo(const std::size_t &len, char *const dst) const
    {
        if (!dst || (VAILD_MESSAGEINFO_SIZE > len))
        {
            return false;
        }
        char *ptr = dst;
        std::memcpy(ptr, reinterpret_cast<const char *>(&m_senderId), sizeof(m_senderId));
        ptr += sizeof(m_senderId);
        std::memcpy(ptr, reinterpret_cast<const char *>(&m_seqNum), sizeof(m_seqNum));
        return true;
    }

    bool MessageInfo::DeserializeFrom(const std::string &src)
    {
        return DeserializeFrom(src.data(), src.size());
    }

    bool MessageInfo::DeserializeFrom(const char *const src, const std::size_t &len)
    {
        RETURN_VAL_IF_NULL(src, false);
        if (VAILD_MESSAGEINFO_SIZE != len)
        {
            printf("src size mismatch , give:%ld target: %ld", len, VAILD_MESSAGEINFO_SIZE);
            return false;
        }
        char *ptr = const_cast<char *>(src);
        std::memcpy(reinterpret_cast<char *>(&m_senderId), ptr, sizeof(m_senderId));
        ptr += sizeof(m_senderId);
        std::memcpy(reinterpret_cast<char *>(&m_seqNum), ptr, sizeof(m_seqNum));
        return true;
    }

    bool MessageInfo::SetTopicName(const std::string &topicName)
    {
        if (MAX_TOPIC_LEN < topicName.size())
        {
            return false;
        }
        memcpy(m_topicName, &topicName[0], topicName.size());
        return true;
    }
} // namespace shm::framework::memory