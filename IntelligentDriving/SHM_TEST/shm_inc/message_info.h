/**
 * @file message_info.h
 */
#ifndef MESSAGE_INFO_H_
#define MESSAGE_INFO_H_
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
namespace shm::framework::memory
{
    const uint8_t MAX_TOPIC_LEN = 255;
    class MessageInfo
    {
    public:
        /**
         * @brief Construct a new message info object
         */
        MessageInfo() = default;
        /**
         * @brief Construct a new message info object
         * @param another other message info
         */
        MessageInfo(const MessageInfo &another);

        /**
         * @brief Destory the message info object
         */
        virtual ~MessageInfo();

        /**
         * @brief operator=
         * @param another other message info
         * @return MessageInfo&
         */
        MessageInfo &operator=(const MessageInfo &another);

        /**
         * @brief operator==
         * @param another other message info
         * @return true/false
         */
        bool operator==(const MessageInfo &another) const;

        /**
         * @brief operator!=
         * @param another other message info
         * @return true/false
         */
        bool operator!=(const MessageInfo &another) const;

        /**
         * @brief Serialize message info
         * @param dst target message info addr
         * @return false/true fail or success
         */
        bool SerializeTo(std::string *const dst) const;

        /**
         * @brief Serialize message info with length
         * @param len length
         * @param dst target message info addr
         * @return false/true fail or success
         */
        bool SerializeTo(const std::size_t &len, char *const dst) const;

        /**
         * @brief Deserialize message info
         * @param src source message info
         * @return false/true fail or success
         */
        bool DeserializeFrom(const std::string &src);

        /**
         * @brief Deserialize message info with length
         * @param src source message info
         * @return false/true fail or success
         */
        bool DeserializeFrom(const char *const src, const std::size_t &len);

        /**
         * @brief get send notation
         * @return const uint64_t&
         */
        const uint64_t &SenderId() const { return m_senderId; }

        /**
         * @brief set send notation
         * @param senderId sender id
         */
        void SetSenderId(const uint64_t &senderId) { m_senderId = senderId; }

        /**
         * @brief get topic name
         * @return const char* topic name
         */
        const char *TopicName() { return m_topicName; }

        /**
         * @brief set topic name
         * @param topicName topic name
         * @return false/true fail or success
         */
        bool SetTopicName(const std::string &topicName);

        /**
         * @brief get current message index
         * @return uint64_t current message index
         */
        uint64_t SeqNum() const { return m_seqNum; }

        /**
         * @brief set the seq num object
         * @param seqNum seq num
         */
        void SetSeqNum(const uint64_t &seqNum) { m_seqNum = seqNum; }

        static const std::size_t VAILD_MESSAGEINFO_SIZE;

    private:
        char m_topicName[MAX_TOPIC_LEN]; // topic name
        uint64_t m_seqNum = 0;           // current message index
        uint64_t m_senderId = 0;         // send notation
    };

} // namespace shm::framework::memory
#endif