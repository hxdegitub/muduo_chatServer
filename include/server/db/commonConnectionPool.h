#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

using namespace std;
#include "connection.h"

class ConnectionPool {
 public:
  // ��ȡ���ӳض���ʵ��
  static ConnectionPool* getConnectionPool();

  shared_ptr<Connection> getConnection();

 private:
  ConnectionPool();

  // �������ļ��м���������
  bool loadConfigFile();

  // �����ڶ������߳��У�ר�Ÿ�������������
  void produceConnectionTask();

  void scannerConnectionTask();

  string _ip;              // mysql��ip��ַ
  unsigned short _port;    // mysql�Ķ˿ں� 3306
  string _username;        // mysql��¼�û���
  string _password;        // mysql��¼����
  string _dbname;          // ���ӵ����ݿ�����
  int _initSize;           // ���ӳصĳ�ʼ������
  int _maxSize;            // ���ӳص����������
  int _maxIdleTime;        // ���ӳ�������ʱ��
  int _connectionTimeout;  // ���ӳػ�ȡ���ӵĳ�ʱʱ��

  queue<Connection*> _connectionQue;  // �洢mysql���ӵĶ���
  mutex _queueMutex;  // ά�����Ӷ��е��̰߳�ȫ������
  atomic_int _connectionCnt;  // ��¼������������connection���ӵ�������
  condition_variable cv;  // ���������������������������̺߳����������̵߳�ͨ��
};