### 代码说明
 - init_socket：

   - 该函数用于初始化服务器的监听套接字，绑定到指定端口，并开始监听客户端连接。
 - handle_accept：

   - 用于接收新的客户端连接，将新的客户端文件描述符加入到 epoll 的事件监听中，等待该客户端的数据操作。
 - handle_read：
   - 用于读取客户端发送的数据，并进行简单的响应。在这个例子中，服务器会回显客户端发送的数据，模拟了 Redis 的 ECHO 命令。
 - main：
   - 创建一个监听套接字，并将其添加到 epoll 中进行监控。然后进入事件循环，通过 epoll_wait 等待事件的发生。
   当有新连接时，调用 handle_accept 处理；当有数据到达时，调用 handle_read 进行读取操作。
 - 主要特性
   - 单线程处理多客户端连接：
     - 该实现使用了单线程 Reactor 模型，在一个线程中处理所有连接的读写事件。epoll 会在有事件发生时通知主线程进行处理。
     I/O 多路复用：

     - 使用 epoll 来监听多个文件描述符，支持高效地处理大量并发连接，而不会像传统的阻塞式 I/O 模型那样为每个连接都需要创建一个线程。
   - 事件驱动：
     - 当 epoll_wait 监听到文件描述符上有事件发生时，触发相应的回调函数（如 handle_read）进行处理。
   - 高效性：
     - 通过 epoll 的多路复用机制，避免了传统的 select 的性能瓶颈，尤其是在高并发环境下。
 - 总结
   - 该代码展示了一个简单的单线程 Reactor 模型的网络服务器实现。通过 epoll 多路复用和事件驱动机制，服务器能够高效地处理多个客户端的连接和数据。虽然这个实现相对简单，但它展示了 Redis 等高性能服务器的核心设计思想：使用单线程处理大量并发连接，同时确保高效的事件处理。

### 使用客户端（如 telnet）测试：

```shell
telnet 127.0.0.1 6379

```
- 输入内容，服务器将回显：
```shell
> Hello
< Hello
```

