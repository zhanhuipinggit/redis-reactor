#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_EVENTS 10
#define PORT 6370

// 初始化套接字
int init_socket(int port) {
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) == -1) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// 处理新连接
void handle_accept(int listen_fd, int kqueue_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("Accept failed");
        return;
    }

    printf("Accepted new connection: %d\n", client_fd);

    // 将客户端的文件描述符加入 kqueue 监听中
    struct kevent ev_set;
    EV_SET(&ev_set, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kqueue_fd, &ev_set, 1, NULL, 0, NULL) == -1) {
        perror("kevent failed");
        close(client_fd);
    }
}

// 处理客户端请求
void handle_read(int client_fd) {
    char buffer[1024];
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        perror("Read failed");
        close(client_fd);
        return;
    }

    if (bytes_read == 0) {
        printf("Client disconnected: %d\n", client_fd);
        close(client_fd);
        return;
    }

    buffer[bytes_read] = '\0';  // 确保是一个字符串
    printf("Received: %s\n", buffer);

    // 发送回应
    if (write(client_fd, buffer, bytes_read) == -1) {
        perror("Write failed");
        close(client_fd);
    }
}

int main() {
    int listen_fd, kqueue_fd;
    struct kevent ev_set, events[MAX_EVENTS];

    // 初始化监听套接字
    listen_fd = init_socket(PORT);

    // 创建 kqueue 实例
    kqueue_fd = kqueue();
    if (kqueue_fd == -1) {
        perror("kqueue failed");
        exit(EXIT_FAILURE);
    }

    // 将监听套接字加入 kqueue 监听
    EV_SET(&ev_set, listen_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kqueue_fd, &ev_set, 1, NULL, 0, NULL) == -1) {
        perror("kevent failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // 事件循环
    while (1) {
        int num_events = kevent(kqueue_fd, NULL, 0, events, MAX_EVENTS, NULL);
        if (num_events == -1) {
            perror("kevent wait failed");
            exit(EXIT_FAILURE);
        }

        // 处理所有的事件
        for (int i = 0; i < num_events; i++) {
            if (events[i].ident == listen_fd) {
                // 有新的连接请求
                handle_accept(listen_fd, kqueue_fd);
            } else if (events[i].filter == EVFILT_READ) {
                // 有数据可读
                handle_read(events[i].ident);
            }
        }
    }

    close(listen_fd);
    close(kqueue_fd);

    return 0;
}
