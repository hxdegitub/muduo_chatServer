
#include "connection.h"

#include <iostream>

#include "public.h"

using namespace std;

Connection::Connection() { _conn = mysql_init(nullptr); }

Connection::~Connection() {
  if (_conn != nullptr) mysql_close(_conn);
}

bool Connection::connect(string ip, unsigned short port, string username,
                         string password, string dbname) {
  MYSQL* p =
      mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(),
                         dbname.c_str(), port, nullptr, 0);
  return p != nullptr;
}

auto Connection::update(string sql) -> bool {
  if (mysql_query(_conn, sql.c_str()) != 0) {
    LOG("update sql  fail : " + sql);
    return false;
  }
  return true;
}

MYSQL_RES* Connection::query(string sql) {
  if (mysql_query(_conn, sql.c_str())) {
    LOG("query fail : " + sql);
    return nullptr;
  }
  return mysql_use_result(_conn);
}