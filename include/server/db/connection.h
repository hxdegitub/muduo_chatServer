#pragma once
#include <mysql/mysql.h>

#include <ctime>
#include <string>

using namespace std;

class Connection {
 public:
  Connection();

  ~Connection();

  auto connect(string ip, unsigned short port, string user, string password,
               string dbname) -> bool;

  auto update(string sql) -> bool;

  auto query(string sql) -> MYSQL_RES*;

  void refreshAliveTime() { _alivetime = clock(); }

  auto getAliveeTime() const -> clock_t { return clock() - _alivetime; }
  auto getRawConnection() -> MYSQL* { return _conn; }

 private:
  MYSQL* _conn;
  clock_t _alivetime;
};