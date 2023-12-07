#ifndef USER_H
#define USER_H

#include <string>

// User表的ORM类
class User {
 public:
  explicit User(int id = -1, std::string name = "", std::string pwd = "",
                std::string state = "offline") {
    this->id = id;
    this->name = name;
    this->password = pwd;
    this->state = state;
  }

  void setId(int id) { this->id = id; }
  void setName(std::string name) { this->name = name; }
  void setPwd(std::string pwd) { this->password = pwd; }
  void setState(std::string state) { this->state = state; }

  auto getId() -> int { return this->id; }
  auto getName() -> std::string { return this->name; }
  auto getPwd() -> std::string { return this->password; }
  auto getState() -> std::string { return this->state; }

 protected:
  int id;
  std::string name;
  std::string password;
  std::string state;
};

#endif