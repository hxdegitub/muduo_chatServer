## muduo

实现了muduo的主要功能：

* EventLoop TcpConnection组件。
* 使用C++11提供的新特性，去掉了Boost库的依赖，包括智能指针，Thread，function等使用。
* 实现了基于epoll的IO-Multiplexing。

## chatserver

可以工作在nginx tcp负载均衡环境中的集群聊天服务器和客户端源码  基于muduo实现，使用redis 实现订阅服务。

## 运行
 可以安装muduo来使用，也可以安装mymuduo来运行
 
