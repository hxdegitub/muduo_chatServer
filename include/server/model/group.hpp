#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <utility>
#include <vector>

#include "groupuser.hpp"

// User表的ORM类
class Group {
 public:
  Group(int id = -1, std::string name = "", std::string desc = "") {
    this->id = id;
    this->name = std::move(name);
    this->desc = std::move(desc);
  }

  void setId(int id) { this->id = id; }
  void setName(std::string name) { this->name = std::move(name); }
  void setDesc(std::string desc) { this->desc = std::move(desc); }

  auto getId() -> int { return this->id; }
  auto getName() -> std::string { return this->name; }
  auto getDesc() -> std::string { return this->desc; }
  auto getUsers() -> std::vector<GroupUser>& { return this->users; }

 private:
  int id;
  std::string name;
  std::string desc;
  std::vector<GroupUser> users;
};

#endif