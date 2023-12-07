#include "chatserver.hpp"

#include <functional>
#include <iostream>
#include <string>

#include "chatservice.hpp"
#include "json.hpp"
// using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象
ChatServer::ChatServer(muduo::net::EventLoop *loop,
                       const muduo::net::InetAddress &listenAddr,
                       const std::string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop) {
  // 注册链接回调
  _server.setConnectionCallback([this](auto &&PH1) {
    this->onConnection(std::forward<decltype(PH1)>(PH1));
  });

  // 注册消息回调
  _server.setMessageCallback([this](auto &&PH1, auto &&PH2, auto &&PH3) {
    onMessage(std::forward<decltype(PH1)>(PH1),
              std::forward<decltype(PH2)>(PH2),
              std::forward<decltype(PH3)>(PH3));
  });

  // 设置线程数量
  _server.setThreadNum(4);
}

// 启动服务
void ChatServer::start() { _server.start(); }

// 上报链接相关信息的回调函数
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
  // 客户端断开链接
  if (!conn->connected()) {
    ChatService::instance()->clientCloseException(conn);
    conn->shutdown();
  }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buffer, muduo::Timestamp time) {
  std::string buf = buffer->retrieveAllAsString();

  // 测试，添加json打印代码
  cout << buf << endl;

  // 数据的反序列化
//  json js = json::parse(buf);
  // 达到的目的：完全解耦网络模块的代码和业务模块的代码
  // 通过js["msgid"] 获取=》业务handler=》conn  js  time
  //auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
  // 回调消息绑定好的事件处理器，来执行相应的业务处理
  //msgHandler(conn, js, time);
}
