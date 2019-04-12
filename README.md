# UtilityEngine
c++网络通信静态库。
Windows使用iocp模型，Linux使用epoll模型（多线程ET）

namespace::net
网络相关，包括socket，IO service，session，client，server实现
namespace::mem
内存相关，包括循环buffer及一个物件池container
namespace::task
任务相关，包括调度物件基类task_object及两种调度模式pool及balance
pool模式不保证同一个task_object同时只被一条线程处理
balance模式保证同一个task_object同时只被一条线程处理
namespace::msg
消息处理相关，包括消息解析msg_pares，消息处理函数管理器handler_manager及两个已完成的消息处理方案
