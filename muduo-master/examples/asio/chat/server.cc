#include "examples/asio/chat/codec.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer : noncopyable
{
 public:
  ChatServer(EventLoop* loop,
             const InetAddress& listenAddr)
  : server_(loop, listenAddr, "ChatServer"),
    codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
  {
    server_.setConnectionCallback(    //连接到来回调
        std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(  //消息到来的回调函数  回调的是LengthHeaderCodec 的onMessage函数
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
  }

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
      connections_.insert(conn);
    }
    else
    {
      connections_.erase(conn);
    }
  }

  void onStringMessage(const TcpConnectionPtr&,
                       const string& message,
                       Timestamp)
  {
    for (ConnectionList::iterator it = connections_.begin();
        it != connections_.end();
        ++it)
    {
      codec_.send(get_pointer(*it), message);
    }
  }

  typedef std::set<TcpConnectionPtr> ConnectionList;
  TcpServer server_;   //tcp服务器
  LengthHeaderCodec codec_;   //消息编解码
  ConnectionList connections_;   //链接列表
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 1)   //参数 端口号
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));   //解析端口号
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);   //构造server对象
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s port\n", argv[0]);
  }
}

