#include "db.h"

#include <muduo/base/Logging.h>

#include "commonConnectionPool.h"
#include "connection.h"

// 初始化数据库连接
MySQL::MySQL() = default;

// 释放数据库连接资源
MySQL::~MySQL() = default;

// 连接数据库
bool MySQL::connect() {
  return nullptr !=
         (conn_ = ConnectionPool::getConnectionPool()->getConnection());
}

// 更新操作
auto MySQL::update(const std::string& sql) -> bool {
  if (conn_) {
    return conn_->update(sql);
  }
  return true;
}

// 查询操作
auto MySQL::query(const std::string& sql) -> MYSQL_RES* {
  if (conn_) {
    return conn_->query(sql);

    return nullptr;
  }
}

// 获取连接
MYSQL* MySQL::getConnection() { return conn_->getRawConnection(); }