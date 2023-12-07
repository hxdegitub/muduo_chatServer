#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>

#include <string>

#include "commonConnectionPool.h"
#include "connection.h"
// 数据库操作类
class MySQL {
 public:
  // 初始化数据库连接
  MySQL();
  // 释放数据库连接资源
  ~MySQL();
  // 连接数据库
  auto connect() -> bool;
  // 更新操作
  auto update(const std::string &sql) -> bool;
  // 查询操作
  auto query(const std::string &sql) -> MYSQL_RES *;
  // 获取连接
  auto getConnection() -> MYSQL *;

 private:
  std::shared_ptr<Connection> conn_;
};

#endif