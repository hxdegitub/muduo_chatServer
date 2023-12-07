#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>

#include <functional>
#include <thread>

class Redis {
 public:
  Redis();
  ~Redis();

  // 连接redis服务器
  auto connect() -> bool;

  // 向redis指定的通道channel发布消息
  auto publish(int channel, std::string message) -> bool;

  // 向redis指定的通道subscribe订阅消息
  auto subscribe(int channel) -> bool;

  // 向redis指定的通道unsubscribe取消订阅消息
  auto unsubscribe(int channel) -> bool;

  // 在独立线程中接收订阅通道中的消息
  void observer_channel_message();

  // 初始化向业务层上报通道消息的回调对象
  void init_notify_handler(std::function<void(int, std::string)> fn);

 private:
  // hiredis同步上下文对象，负责publish消息
  redisContext *_publish_context;

  // hiredis同步上下文对象，负责subscribe消息
  redisContext *_subcribe_context;

  // 回调操作，收到订阅的消息，给service层上报
  std::function<void(int, std::string)> _notify_message_handler;
};

#endif
