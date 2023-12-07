#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include <string>
#include <vector>

#include "group.hpp"

// 维护群组信息的操作接口方法
class GroupModel {
 public:
  // 创建群组
  auto createGroup(Group &group) -> bool;
  // 加入群组
  void addGroup(int userid, int groupid, std::string role);
  // 查询用户所在群组信息
  auto queryGroups(int userid) -> std::vector<Group>;
  // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
  auto queryGroupUsers(int userid, int groupid) -> std::vector<int>;
};

#endif