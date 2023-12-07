#include "chatservice.hpp"

#include <muduo/base/Logging.h>

#include <vector>

#include "public.hpp"

using namespace muduo;

// 获取单例对象的接口函数
auto ChatService::instance() -> ChatService * {
  static ChatService service;
  return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService() {
  // 用户基本业务管理相关事件处理回调注册
  _msgHandlerMap.insert({LOGIN_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
                           this->login(std::forward<decltype(PH1)>(PH1),
                                       std::forward<decltype(PH2)>(PH2),
                                       std::forward<decltype(PH3)>(PH3));
                         }});
  _msgHandlerMap.insert(
      {LOGINOUT_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
         this->loginout(std::forward<decltype(PH1)>(PH1),
                        std::forward<decltype(PH2)>(PH2),
                        std::forward<decltype(PH3)>(PH3));
       }});
  _msgHandlerMap.insert({REG_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
                           this->reg(std::forward<decltype(PH1)>(PH1),
                                     std::forward<decltype(PH2)>(PH2),
                                     std::forward<decltype(PH3)>(PH3));
                         }});
  _msgHandlerMap.insert(
      {ONE_CHAT_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
         this->oneChat(std::forward<decltype(PH1)>(PH1),
                       std::forward<decltype(PH2)>(PH2),
                       std::forward<decltype(PH3)>(PH3));
       }});
  _msgHandlerMap.insert(
      {ADD_FRIEND_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
         this->addFriend(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2),
                         std::forward<decltype(PH3)>(PH3));
       }});

  // 群组业务管理相关事件处理回调注册
  _msgHandlerMap.insert(
      {CREATE_GROUP_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
         this->createGroup(std::forward<decltype(PH1)>(PH1),
                           std::forward<decltype(PH2)>(PH2),
                           std::forward<decltype(PH3)>(PH3));
       }});
  _msgHandlerMap.insert(
      {ADD_GROUP_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
         this->addGroup(std::forward<decltype(PH1)>(PH1),
                        std::forward<decltype(PH2)>(PH2),
                        std::forward<decltype(PH3)>(PH3));
       }});
  _msgHandlerMap.insert(
      {GROUP_CHAT_MSG, [this](auto &&PH1, auto &&PH2, auto &&PH3) {
         this->groupChat(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2),
                         std::forward<decltype(PH3)>(PH3));
       }});

  // 连接redis服务器
  if (_redis.connect()) {
    // 设置上报消息的回调
    _redis.init_notify_handler([this](auto &&PH1, auto &&PH2) {
      this->handleRedisSubscribeMessage(std::forward<decltype(PH1)>(PH1),
                                        std::forward<decltype(PH2)>(PH2));
    });
  }
}

// 服务器异常，业务重置方法
void ChatService::reset() {
  // 把online状态的用户，设置成offline
  _userModel.resetState();
}

// 获取消息对应的处理器
auto ChatService::getHandler(int msgid) -> MsgHandler {
  // 记录错误日志，msgid没有对应的事件处理回调
  auto it = _msgHandlerMap.find(msgid);
  if (it == _msgHandlerMap.end()) {
    // 返回一个默认的处理器，空操作
    return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
      LOG_ERROR << "msgid:" << msgid << " can not find handler!";
    };
  }
  return _msgHandlerMap[msgid];
}

// 处理登录业务  id  pwd   pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js,
                        Timestamp time) {
  int id = js["id"].get<int>();
  std::string pwd = js["password"];

  User user = _userModel.query(id);
  if (user.getId() == id && user.getPwd() == pwd) {
    if (user.getState() == "online") {
      // 该用户已经登录，不允许重复登录
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 2;
      response["errmsg"] = "this account is using, input another!";
      conn->send(response.dump());
    } else {
      // 登录成功，记录用户连接信息
      {
        lock_guard<mutex> lock(_connMutex);
        _userConnMap.insert({id, conn});
      }

      // id用户登录成功后，向redis订阅channel(id)
      _redis.subscribe(id);

      // 登录成功，更新用户状态信息 state offline=>online
      user.setState("online");
      _userModel.updateState(user);

      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 0;
      response["id"] = user.getId();
      response["name"] = user.getName();
      // 查询该用户是否有离线消息
      std::vector<std::string> vec = _offlineMsgModel.query(id);
      if (!vec.empty()) {
        response["offlinemsg"] = vec;
        // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
        _offlineMsgModel.remove(id);
      }

      // 查询该用户的好友信息并返回
      std::vector<User> user_vec = _friendModel.query(id);
      if (!user_vec.empty()) {
        std::vector<std::string> vec2;
        for (User &user : user_vec) {
          json js;
          js["id"] = user.getId();
          js["name"] = user.getName();
          js["state"] = user.getState();
          vec2.push_back(js.dump());
        }
        response["friends"] = vec2;
      }

      // 查询用户的群组信息
      std::vector<Group> groupuser_vec = _groupModel.queryGroups(id);
      if (!groupuser_vec.empty()) {
        // group:[{groupid:[xxx, xxx, xxx, xxx]}]
        std::vector<std::string> group_v;
        for (Group &group : groupuser_vec) {
          json grpjson;
          grpjson["id"] = group.getId();
          grpjson["groupname"] = group.getName();
          grpjson["groupdesc"] = group.getDesc();
          std::vector<std::string> user_v;
          for (GroupUser &user : group.getUsers()) {
            json js;
            js["id"] = user.getId();
            js["name"] = user.getName();
            js["state"] = user.getState();
            js["role"] = user.getRole();
            user_v.push_back(js.dump());
          }
          grpjson["users"] = user_v;
          group_v.push_back(grpjson.dump());
        }

        response["groups"] = group_v;
      }

      conn->send(response.dump());
    }
  } else {
    // 该用户不存在，用户存在但是密码错误，登录失败
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "id or password is invalid!";
    conn->send(response.dump());
  }
}

// 处理注册业务  name  password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  std::string name = js["name"];
  std::string pwd = js["password"];

  User user;
  user.setName(name);
  user.setPwd(pwd);
  bool state = _userModel.insert(user);
  if (state) {
    // 注册成功
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 0;
    response["id"] = user.getId();
    conn->send(response.dump());
  } else {
    // 注册失败
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    conn->send(response.dump());
  }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js,
                           Timestamp time) {
  int userid = js["id"].get<int>();

  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
      _userConnMap.erase(it);
    }
  }

  // 用户注销，相当于就是下线，在redis中取消订阅通道
  _redis.unsubscribe(userid);

  // 更新用户的状态信息
  User user(userid, "", "", "offline");
  _userModel.updateState(user);
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
  User user;
  {
    lock_guard<mutex> lock(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
      if (it->second == conn) {
        // 从map表删除用户的链接信息
        user.setId(it->first);
        _userConnMap.erase(it);
        break;
      }
    }
  }

  // 用户注销，相当于就是下线，在redis中取消订阅通道
  _redis.unsubscribe(user.getId());

  // 更新用户的状态信息
  if (user.getId() != -1) {
    user.setState("offline");
    _userModel.updateState(user);
  }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js,
                          Timestamp time) {
  int toid = js["toid"].get<int>();

  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if (it != _userConnMap.end()) {
      // toid在线，转发消息   服务器主动推送消息给toid用户
      it->second->send(js.dump());
      return;
    }
  }

  // 查询toid是否在线
  User user = _userModel.query(toid);
  if (user.getState() == "online") {
    _redis.publish(toid, js.dump());
    return;
  }

  // toid不在线，存储离线消息
  _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js,
                            Timestamp time) {
  int userid = js["id"].get<int>();
  int friendid = js["friendid"].get<int>();

  // 存储好友信息
  _friendModel.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js,
                              Timestamp time) {
  int userid = js["id"].get<int>();
  std::string name = js["groupname"];
  std::string desc = js["groupdesc"];

  // 存储新创建的群组信息
  Group group(-1, name, desc);
  if (_groupModel.createGroup(group)) {
    // 存储群组创建人信息
    _groupModel.addGroup(userid, group.getId(), "creator");
  }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js,
                           Timestamp time) {
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();
  _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js,
                            Timestamp time) {
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();
  std::vector<int> userid_vec = _groupModel.queryGroupUsers(userid, groupid);

  lock_guard<mutex> lock(_connMutex);
  for (int id : userid_vec) {
    auto it = _userConnMap.find(id);
    if (it != _userConnMap.end()) {
      // 转发群消息
      it->second->send(js.dump());
    } else {
      // 查询toid是否在线
      User user = _userModel.query(id);
      if (user.getState() == "online") {
        _redis.publish(id, js.dump());
      } else {
        // 存储离线群消息
        _offlineMsgModel.insert(id, js.dump());
      }
    }
  }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, std::string msg) {
  lock_guard<mutex> lock(_connMutex);
  auto it = _userConnMap.find(userid);
  if (it != _userConnMap.end()) {
    it->second->send(msg);
    return;
  }

  // 存储该用户的离线消息
  _offlineMsgModel.insert(userid, msg);
}