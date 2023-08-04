#pragma once

#include <netinet/in.h> // sockaddr_in
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h> // epoll
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include "ThreadPool.hpp"
#include "Protocol.hpp"
#include "DBOperator.hpp"
#include "RecvFile.hpp"
#include "SendFileList.hpp"

#define CONFIG_FILE_PATH "./configFile/server.config"
#define USER_FILE_DIRROOT "./userFile/"

#define MAX_EVENTS 1024


class CloudStorageServer
{
  private:
    int listen_fd;                            // 监听的fd
    int listen_backlog;                       // listen队列大小
    std::string server_port;                  // 绑定的端口
    int epoll_fd;                             // epoll fd
    std::mutex mtx_login_user_map;                           // 互斥锁
    std::mutex mtx_send;
    std::mutex mtx_recv;
    std::mutex transfer_file_info_map_mutex;
    map<int, std::pair<int, std::string>> login_user_map;           // 记录登录的用户，以fd作为键，登录类型，和登录名作为值
    map<std::string, int> login_fd_map;                             // 记录登录的用户， 以用户名为键，fd为值
    map<std::string, std::map<unsigned int, TransferFileInfo>> transfer_file_info_map;  // 用户名，和它上传未完成文件的信息
  public:
    CloudStorageServer();
    ~CloudStorageServer();
  private:
    void Socket();
    void Setsockopt();
    void Bind();
    void Listen();
    void EpollCreate();
    void Accept();

    int Send(int fd, void* buf, size_t len, int flag = 0);
    // MSG_WAITALL表示在接收到指定长度的数据之前一直等待，直到所有数据都到达了接收缓冲区。
    int Recv(int fd, void* buf, size_t len, int flag = 0);
  public:
    void Run();
    void handleConnetClose(int fd);
    static CloudStorageServer& getInstance();
    static void recvMsg(int fd);
    
    std::string getUserName(int fd);
    int getUserFd(std::string);

    int sendFile(std::string user_name, int num, int fd);

  public:
    void handleLoginRequest(MessagePack* msg_pack, int fd);
    void handleRegisterRequest(MessagePack* msg_pack, int fd);
    void handleFindUserRequest(MessagePack* msg_pack, int fd);  
    void handleAddFriendRequest(DataPack* pack, int fd);
    void handleAddFriendRespond(DataPack* pack, int fd);
    void handleFlushFriendRequest(DataPack* pack, int fd);
    void handleDeleteFriendRequest(DataPack* pack, int fd);
    void handlePrivateChatRequest(DataPack* pack, int fd);
    void handlePrivateChatRespond(DataPack* pack, int fd); 
    void handleCreateDirRequest(MessagePack* msg_pack, int fd);
    void handleFlushFileListResquest(MessagePack* msg_pack, int fd);
    void handleDeleteFileRequest(MessagePack* msg_pack, int fd);
    void handleUploadFileRequest(MessagePack* msg_pack, int fd);
    void handleDownloadFileRequest(MessagePack* msg_pack, int fd);
    void handleDownloadFileRespond(MessagePack* msg_pack, int fd);
    void handleShareFileRequest(DataPack* pack, int fd);
    void handleShareFileRespondt(MessagePack* msg_pack, int fd);
  private:
    int toLoadConfig();
    void setConfigValue(std::string s);

    void handleMsgPack(DataPack* pack, int fd);
    void handleFilePack(DataPack* pack, int fd);
};


void CloudStorageServer::setConfigValue(std::string s) {  
  std::string key;
  std::string value;
  int i = 0;
  for(; i < s.size(); i++) {
    if(s[i] == ':') break;
    key += s[i];
  }
  i++;
  while(i < s.size() && s[i] == ' ') i++;
  for(; i < s.size(); i++) {
    value += s[i];
  }
  if(key == "server_port") {
    server_port = value;
  }
  if(key == "listen_backlog")  {
    listen_backlog = atoi(value.c_str());
  }
}


int CloudStorageServer::toLoadConfig() {
  std::fstream file(CONFIG_FILE_PATH);
  if(file.is_open()) {
    std::string line;
    while (getline(file, line)) {
      setConfigValue(line);
    }
  }
  else {
    std::cout << "配置文件server.config打开失败，请检查本程序当前在路径目录是否有configFile,配置文件应该在其中！";
    return -1;
    exit(-1);
  }
  file.close();
  return 0;
}

CloudStorageServer::CloudStorageServer()
{
  toLoadConfig();
  sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);  
  server_addr.sin_port = htons(atoi(server_port.c_str()));

  listen_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) 
  {
    perror("Create Socket Failed!");
    exit(1); 
  }
  // 端口复用
  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

CloudStorageServer::~CloudStorageServer()
{
  close(listen_fd);
  close(epoll_fd);
}

void CloudStorageServer::Socket() {
  // 创建监听套接字
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }
}

void CloudStorageServer::Setsockopt() {
  // 端口复用   
  // 设置 SO_REUSEADDR 选项
  int reuse = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
    close(listen_fd);
    return exit(EXIT_FAILURE);
  }
}

void CloudStorageServer::Bind()
{
  // 绑定地址和端口号
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(atoi(server_port.c_str()));
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("Failed to bind socket");
    exit(EXIT_FAILURE);  
  }
}

void CloudStorageServer::Listen()
{
  // 监听连接
  if (listen(listen_fd, SOMAXCONN) == -1) {
    perror("Failed to listen on socket");
    exit(EXIT_FAILURE);  
  }
}

void CloudStorageServer::EpollCreate() {
  // 创建 epoll 文件描述符
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {    
    perror("Failed to create epoll file descriptor");    
    exit(EXIT_FAILURE);  
  }
  // 添加监听套接字到 epoll 中  
  struct epoll_event event;  
  event.data.fd = listen_fd;
  event.events = EPOLLIN;  
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {    
    perror("Failed to add listen socket to epoll");    
    exit(EXIT_FAILURE);
  }
}

void CloudStorageServer::Accept()
{
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd < 0)
  {
    perror("Failed to accept on socket");
    close(client_fd);
    exit(1);
  }

  std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) \
    << ":" << ntohs(client_addr.sin_port) << std::endl;

  // 在epoll_fd中注册新建立的连接
  struct epoll_event event{};
  event.data.fd = client_fd;
  event.events = EPOLLIN;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
    std::cerr << "Failed to add client_fd to epoll\n";
    close(client_fd);  
  }
}

int CloudStorageServer::Send(int fd, void* buf, size_t len, int flag) {
  if(len == 0) return 0;
  mtx_send.lock();
  // 保证一次send完数据，防止黏包
  int total_bytes = 0;
  int ret;                                                                                                         
  while (total_bytes < len) {
    ret = send(fd, (char*)buf + total_bytes, len - total_bytes, flag);
    if (ret <= 0) {
       break;
    }
    total_bytes += ret;
  }
  mtx_send.unlock();
  if(ret == 0) {
    std::cout << "用户:" << login_user_map[fd].second << " 退出！" << std::endl;
    handleConnetClose(fd);
    return 0;
  }
  if (ret == -1) {
    int errsv = errno;  // 保存 errno 的值
    std::cerr << "send error: " << std::strerror(errno) << std::endl;
    if (errsv == EAGAIN || errsv == EWOULDBLOCK) {
      // 非阻塞套接字操作无法立即完成
    } else if (errsv == EINTR) {
      // 系统调用被信号中断

    } else if (errsv == EINVAL) {
      // 参数不合法

    } else if (errsv == EMSGSIZE) {
      // 数据超出套接字缓冲区的大小

    } else if (errsv == ENOTCONN) {
      // 套接字未连接

    } else if (errsv == EPIPE) {
      // 套接字已经被关闭，还有数据要发送

    } else {
      // 其他错误类型
    }
    // 暂时先这样处理，关闭连接

    std::cout << "用户:" << login_user_map[fd].second << " 退出！" << std::endl;
    handleConnetClose(fd);
    return -1;
  }
  return len;
}

int CloudStorageServer::Recv(int fd, void* buf, size_t len, int flag) {
  mtx_recv.lock();
  // 保证读完数据，防止黏包
  int total_bytes = 0;
  int ret;
  while (total_bytes < len) {
    ret = recv(fd, (char*)buf + total_bytes, len - total_bytes, flag);
    if (ret <= 0) {
        break;
    }
    total_bytes += ret;
  }
  mtx_recv.unlock();
  if(ret == 0) {
    std::cout << "用户:" << login_user_map[fd].second << " 退出！" << std::endl;
    handleConnetClose(fd);
    return ret;
  }
  if (ret == -1) {
    int errsv = errno;  // 保存 errno 的值
    std::cerr << "recv error: " << std::strerror(errno) << std::endl;
    if (errsv == EAGAIN || errsv == EWOULDBLOCK) {
      // 非阻塞套接字操作无法立即完成

    } else if (errsv == EINTR) {
      // 系统调用被信号中断

    } else if (errsv == EINVAL) {
      // 参数不合法

    } else if (errsv == ECONNRESET) {
      // 远程端点重置了连接。

    } else if (errsv == ENOTCONN) {
      // 套接字未连接

    } else if (errsv == EBADF) {
      // 无效的文件描述符
    } else if(errsv == ENOMEM){
      // 内存不足
    } else if(errsv == ENOTSOCK){
      // 描述符不是一个socket
    } else {
      // 其他错误
    }
    // 暂时先这样处理，关闭连接
    handleConnetClose(fd);
    std::cout << "用户:" << login_user_map[fd].second << " 退出！" << std::endl;
    return ret;
  }
  return total_bytes;
}

void CloudStorageServer::Run()
{
  Socket();
  Setsockopt();
  Bind();
  Listen();
  EpollCreate();
  // 将所有用户设置为不在线
  DBOperator::getInstance().setAllUserOntOnline();
  // 创建线程池
  // 最小线程数量，最大线程数量， 任务队列最大长度， 线程空闲时间：ms
  ThreadPool thread_poll(4, 12, 100, 5000);
  // 处理事件循环
  std::vector<epoll_event> events(MAX_EVENTS);
  while (true) {    
    // 等待事件
    int nfds = epoll_wait(epoll_fd, events.data(), events.size(), -1);
    if (nfds < 0) {
      std::cerr << "Failed to wait for events\n";
      continue;
    }
    // 处理事件
    for (int i = 0; i < nfds; i++) {
      auto fd = events[i].data.fd;
      if(fd == listen_fd && events[i].events & EPOLLIN) {
        // 处理新连接
        Accept();
      }
      else {
        // 处理客户端事件        
        std::cout << fd << ":客户端发送消息\n";
        if(events[i].events & EPOLLIN) {
          //Task task(recvMsg, fd);
          //thread_poll.AddTask(task);
          //std::thread t(recvMsg, fd);
          //t.detach();
          recvMsg(fd);
          //t.join();
        }
        //else 
      }
    }

  }
}

void CloudStorageServer::handleConnetClose(int fd) {
  // 关掉描述符
  close(fd);

  DBOperator::getInstance().setNotOnline(login_user_map[fd].first, login_user_map[fd].second);

  mtx_login_user_map.lock();
  if(login_user_map.find(fd) != login_user_map.end()) {
    login_fd_map.erase(login_user_map[fd].second);
    login_user_map.erase(fd);
  }
  // 将该fd也从epoll中注销
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN;
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
  mtx_login_user_map.unlock();
}

CloudStorageServer& CloudStorageServer::getInstance() {
  static CloudStorageServer instance;
  return instance;
}

void CloudStorageServer::recvMsg(int fd)
{
  // 获取包大小
  unsigned int total_size = 0;
  //std::cout << "读取头4字节\n";
  int ret = CloudStorageServer::getInstance().Recv(fd, &total_size, sizeof(total_size), 0);
  //std::cout << "读取成功:" << total_size << std::endl;
  if(ret <= 0) {
    std::cout << "recv <= 0";
    return;
  }
  if(total_size == 0) {
    CloudStorageServer::getInstance().Send(fd, (char*)&total_size, sizeof(total_size));
    //shutdown(fd, SHUT_WR);
    //std::cout << "\n";
    //send(fd, nullptr, 0, 0);//刷新缓冲区，将数据发送出去
    return;
  }
  // 获取包
  std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPack(total_size - sizeof(DataPack));
  //std::cout << "包大小：" << total_size << std::endl;
  // 需要偏移记录包大小的变量字节数,也要少读这字节数
  //std::cout << "读取后面偏移量\n";
  ret = CloudStorageServer::getInstance().Recv(fd, (char*)pack.get() + sizeof(total_size), total_size - sizeof(total_size), 0);
  //std::cout << "读取到：" << ret  << "bytes" << std::endl; 
  if(ret <= 0){
    if(ret <= 0) std::cout << "recv <= 0\n";
    return;
  }
  if(pack->pack_type == PACK_TYPE_MSG) {
    CloudStorageServer::getInstance().handleMsgPack(pack.get(), fd);
  }
  else if(pack->pack_type == PACK_TYPE_UPLOAD_FILE){  
    CloudStorageServer::getInstance().handleFilePack(pack.get(), fd);
  }
  else if(pack->pack_type == PACK_TYPE_DOWNLOAD_FILE) {
    std::cout << "上传文件回复\n";
  }
  else {
    std::cout << "包有问题！\n";
  }
  return;
}

std::string CloudStorageServer::getUserName(int fd) {
  std::string user_name;
  mtx_login_user_map.lock(); 
  if(login_user_map.find(fd) == login_user_map.end()) {
    std::cout << "用户不存在!(getUsrName)" << std::endl;
  }
  else {
    user_name = login_user_map[fd].second;
  }
  mtx_login_user_map.unlock();
  return user_name;
}

int CloudStorageServer::getUserFd(std::string user_name) {
  if(user_name.empty()) return -1;
  int fd;
  mtx_login_user_map.lock();
  if(login_fd_map.find(user_name) == login_fd_map.end()) {
    return -1;
  }
  else {
    fd = login_fd_map[user_name];
  }
  mtx_login_user_map.unlock();
  return fd;
}

void CloudStorageServer::handleMsgPack(DataPack* pack, int fd) {
  MessagePack* msg_pack = (MessagePack*)pack->pack_data;
  switch(msg_pack->msg_type) {
    case MSG_TYPE_LOGIN_REQUEST :
      handleLoginRequest(msg_pack, fd);
      break;
    case MSG_TYPE_REGISTER_REQUEST :
      handleRegisterRequest(msg_pack, fd);
      break;
    case MSG_TYPE_FIND_USER_REQUEST :
      handleFindUserRequest(msg_pack, fd);
      break;
    case MSG_TYPE_ADD_FRIEND_REQUEST :
      handleAddFriendRequest(pack, fd);
      break;
    case MSG_TYPE_ADD_FRIEND_RESPOND :
      handleAddFriendRespond(pack, fd);
      break;
    case MSG_TYPE_FLUSH_FRIEND_REQUEST :
      handleFlushFriendRequest(pack, fd);
      break;
    case MSG_TYPE_DELETE_FRIEND_REQUEST :
      handleDeleteFriendRequest(pack, fd);
      break;
    case MSG_TYPE_PRIVATE_CHAT_REQUEST :
      handlePrivateChatRequest(pack, fd);
      break;
    case MSG_TYPE_PRIVATE_CHAT_RESPOND :
      handlePrivateChatRespond(pack, fd);
      break;
    case MSG_TYPE_CREATE_DIR_REQUEST :
      handleCreateDirRequest(msg_pack, fd);
      break;
    case MSG_TYPE_FLUSH_FILE_LIST_REQUEST :
      handleFlushFileListResquest(msg_pack, fd);
      break;
    case MSG_TYPE_DELETE_FILE_REQUEST :
      handleDeleteFileRequest(msg_pack, fd);
      break;
    case MSG_TYPE_UPLOAD_FILE_REQUEST :
      handleUploadFileRequest(msg_pack, fd);
      break;
    case MSG_TYPE_DOWNLOAD_FILE_REQUEST :
      handleDownloadFileRequest(msg_pack, fd);
      break;
    case MSG_TYPE_DOWNLOAD_FILE_RESPOND :
      handleDownloadFileRespond(msg_pack, fd);
      break;
    case MSG_TYPE_SHARE_FILE_REQUEST :
      handleShareFileRequest(pack, fd);
      break;
    case MSG_TYPE_SHARE_FILE_RESPOND :
      handleShareFileRespondt(msg_pack, fd);
      break;
    default :
      break;
  }
}

void CloudStorageServer::handleLoginRequest(MessagePack* msg_pack, int fd) {
  std::string login_name = std::string(msg_pack->common); 
  std::string passwd = std::string(msg_pack->common + 32);
  unsigned int login_manner = *((unsigned int*)msg_pack->data);
  //std::cout << "login_manner:" << login_manner << std::endl;
  std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
  pack->pack_type = PACK_TYPE_MSG;
  MessagePack* msg = (MessagePack*)pack->pack_data;
  msg->msg_type = MSG_TYPE_LOGIN_RESPOND;
  int ret = DBOperator::getInstance().loginIsSucceed(login_name, passwd);
  if(ret == 0) {
    // 不在线,可以登录
    strcpy((char*)msg->common, LOGIN_SUCCEED);
    mtx_login_user_map.lock();
    login_user_map[fd].first = login_manner;
    login_user_map[fd].second = login_name;
    login_fd_map[login_name] = fd;
    // 用户名登录
    if(login_manner == 0) DBOperator::getInstance().setOnline(login_manner, login_name);
    else if(login_manner == 1) {}
    else if(login_manner == 2) {}
    else if(login_manner == 3) {}
    else {}
    mtx_login_user_map.unlock();
    std::string user_path = USER_FILE_DIRROOT + login_name;
    int status = mkdir_recursive(user_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    std::cout << login_name << "登录成功！" << std::endl;
  }
  else if(ret == 1) {
    // 在线，暂时不能登录
    strcpy((char*)msg->common, LOGIN_USER_ONLINE);
  }
  else {
    // 账号或密码错误
    strcpy((char*)msg->common, LOGIN_NAMEORPASSWD_ERROR);
  }

  ret = Send(fd, pack.get(), pack->total_size, 0);
}


void CloudStorageServer::handleRegisterRequest(MessagePack* msg_pack, int fd) {
  std::string login_name = std::string(msg_pack->common); 
  std::string passwd = std::string(msg_pack->common + 32);
  std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
  pack->pack_type = PACK_TYPE_MSG;
  MessagePack* msg = (MessagePack*)pack->pack_data;
  msg->msg_type = MSG_TYPE_REGISTER_RESPOND;
  if(DBOperator::getInstance().userNameIsExist(login_name)) {
    // 用户名存在
    strcpy(msg->common, REGISTER_USER_EXIST);
  }
  else {
    // 用户名不存在
    strcpy(msg->common, REGISTER_SUCCEED);
    if(!DBOperator::getInstance().insertUser(login_name, passwd)) {
      strcpy(msg->common, SYETEM_ERROR);
    }
  }
  Send(fd, pack.get(), pack->total_size, 0);
}

void CloudStorageServer::handleFindUserRequest(MessagePack* msg_pack, int fd) {
  std::string find_criteria(msg_pack->common);
  std::string user(msg_pack->common + 32);

  //std::cout << "find:" << find_criteria << std::endl;
  //std::cout << "user:" << user << std::endl;
  // 按用户名查找
  if(strcmp(find_criteria.c_str(), FIIND_CRITERIA_NAME) == 0) {
    // 用户不存在
    if(!DBOperator::getInstance().userNameIsExist(user)) {
      std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
      MessagePack* msg = (MessagePack*)pack->pack_data;
      msg->msg_type = MSG_TYPE_FIND_USER_RESPOND;
      strcpy(msg->common, USER_NOT_EXIST);
      if(Send(fd, pack.get(), pack->total_size, 0) <= 0) {
        return;
      }
    }
    // 用户存在
    else {
      std::vector<UserInfo> user_info;
      auto ret = DBOperator::getInstance().getUserInfo(find_criteria, user);
      for(auto e : ret) {
        UserInfo temp;
        temp.id = atoi(e[0].c_str());
        strcpy(temp.name, e[1].c_str());
        temp.online = e[3][0];
        user_info.push_back(temp);
      }

      //std::cout << "id:"<< ret[0][0] << endl;
      //std::cout << "name:"<< ret[0][1] << endl;
      //std::cout << "online:"<< ret[0][3] << endl;
      std::unique_ptr<DataPack,FreeDeleter> pack = makeDataPackMsg(user_info.size() * sizeof(UserInfo));
      MessagePack* msg = (MessagePack*)pack->pack_data;
      msg->msg_type = MSG_TYPE_FIND_USER_RESPOND;
      strcpy(msg->common, USER_EXIST);
      for(int i = 0; i < user_info.size(); i++) {
        memcpy(msg->data + i * sizeof(UserInfo), &user_info[i], sizeof(UserInfo));
      }
      if(Send(fd, pack.get(), pack->total_size, 0) <= 0) {
        return;
      }

    }
  }
  // 按id查找
  else if(strcmp(find_criteria.c_str(), FIIND_CRITERIA_ID) == 0) {
    // 用户不存在
    if(!DBOperator::getInstance().userIdIsExist(user)) {
      std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
      MessagePack* msg = (MessagePack*)pack->pack_data;
      msg->msg_type = MSG_TYPE_FIND_USER_RESPOND;
      strcpy(msg->common, USER_NOT_EXIST);
      if(Send(fd, pack.get(), pack->total_size, 0) <= 0) {
        return;
      }
    }
    else {
      // 用户存在
      std::vector<UserInfo> user_info;
      std::cout << "id exist!" << std::endl;
      auto ret = DBOperator::getInstance().getUserInfo(find_criteria, user);
      //std::cout << "id:"<< ret[0][0] << endl;
      //std::cout << "name:"<< ret[0][1] << endl;
      //std::cout << "online:"<< ret[0][3] << endl;
      //std::cout << "name:"<< ret[0][0] << endl;
      for(auto e : ret) {
        UserInfo temp;
        temp.id = atoi(e[0].c_str());
        strcpy(temp.name, e[1].c_str());
        temp.online = e[3][0];
        user_info.push_back(temp);
      }

      std::unique_ptr<DataPack,FreeDeleter> pack = makeDataPackMsg(user_info.size() * sizeof(UserInfo));
      MessagePack* msg = (MessagePack*)pack->pack_data;
      msg->msg_type = MSG_TYPE_FIND_USER_RESPOND;
      strcpy(msg->common, USER_EXIST);
      for(int i = 0; i < user_info.size(); i++) {
        memcpy(msg->data + i * sizeof(UserInfo), &user_info[i], sizeof(UserInfo));
      }

      Send(fd, pack.get(), pack->total_size, 0);
    }
  }
}

void CloudStorageServer::handleAddFriendRequest(DataPack* pack, int fd) {

  MessagePack* msg_pack = (MessagePack*)pack->pack_data;
  AddFriendPack* add_friend_pack = (AddFriendPack*)msg_pack->data;

  std::string send_name(add_friend_pack->send);
  std::string recv_name(add_friend_pack->recv);
  std::string request_data;
  if(msg_pack->data_size > sizeof(AddFriendPack)) {
    request_data = std::string(add_friend_pack->data);
  }

  int ret = DBOperator::getInstance().userIsOnline(recv_name);
  std::unique_ptr<DataPack, FreeDeleter> respond_pack(makeDataPackMsg(0));
  MessagePack* respond_msg = (MessagePack*)respond_pack->pack_data;
  respond_msg->msg_type = MSG_TYPE_ADD_FRIEND_RESPOND;
  int is_user_friend = DBOperator::getInstance().isUserFriend(send_name ,recv_name);

  if(ret == -1 || is_user_friend == -1) {
    // 用户不存在
    strcpy(respond_msg->common, USER_NOT_EXIST);
    Send(fd, respond_pack.get(), respond_pack->total_size);
    return;
  }
  if(is_user_friend == 1) {
    // 已经是好友
    strcpy(respond_msg->common, IS_USER_FRIEND);
    Send(fd, respond_pack.get(), respond_pack->total_size);
    return;
  }

  if(ret == 1) {
    // 用户在线，直接转发
    if(Send(login_fd_map[recv_name], pack, pack->total_size) <= 0){
      strcpy(respond_msg->common, ADD_FRIEND_REQUEST_SEND_ERROR);
    }
    else {
      strcpy(respond_msg->common, ADD_FRIEND_REQUEST_SEND);
    }
    // 转发情况发送给用户
    Send(fd, respond_pack.get(), respond_pack->total_size);
  }
  else {
    // 用户不在线
    int recv_id = DBOperator::getInstance().getUserId(0, recv_name);
    int send_id = DBOperator::getInstance().getUserId(0, send_name);
    std::string message (add_friend_pack->data);
    // 写入数据库
    if(DBOperator::getInstance().insertNotOnlineMessage(recv_id, send_id, MSG_TYPE_ADD_FRIEND_REQUEST, message)) {
      strcpy(respond_msg->common, ADD_FRIEND_REQUEST_SEND);
    }
    else {
      strcpy(respond_msg->common, ADD_FRIEND_REQUEST_SEND_ERROR);
    }
    Send(fd, respond_pack.get(), respond_pack->total_size);
  }
}

void CloudStorageServer::handleAddFriendRespond(DataPack* pack, int fd) {
  MessagePack* msg_pack = (MessagePack*)pack->pack_data;
  AddFriendPack* add_friend_pack = (AddFriendPack*)msg_pack->data;
  std::string recv_name(add_friend_pack->recv);
  std::string send_name(login_user_map[fd].second);
  int ret = DBOperator::getInstance().userIsOnline(recv_name);
  if(ret == -1) {
    // 用户不存在

  }
  else if(ret == 1) {
    std::cout << "在线\n";
    // 在线 直接发给他
    int recv_fd = login_fd_map[recv_name];
    Send(recv_fd, pack, pack->total_size);
    // 同意在好友关系里插入新数据
    if(strcmp(msg_pack->common, ADD_FRIEND_RESPOND_ACCEPT) == 0) {
      DBOperator::getInstance().insertFriend(recv_name, send_name);
    }
  }
  else {
    std::cout << "不在线\n";
    // 不在线 放到数据库里
    // 将请求写入数据库
    int recv_id = DBOperator::getInstance().getUserId(0, recv_name);
    int send_id = DBOperator::getInstance().getUserId(0, send_name);
    std::string message (add_friend_pack->data);
    DBOperator::getInstance().insertNotOnlineMessage(recv_id, send_id, MSG_TYPE_ADD_FRIEND_RESPOND, msg_pack->common);
    if(strcmp(msg_pack->common, ADD_FRIEND_RESPOND_ACCEPT) == 0) {
      DBOperator::getInstance().insertFriend(recv_name, send_name);
    }
  }
}


void CloudStorageServer::handleFlushFriendRequest( DataPack *pack, int fd ) {
  MessagePack* msg = (MessagePack*)pack->pack_data;
  std::string name = msg->common;
  auto res = DBOperator::getInstance().getFriendList(name);
  std::unique_ptr<DataPack, FreeDeleter> res_pack = makeDataPackMsg(sizeof(UserInfo) * res.size());
  MessagePack* res_msg = (MessagePack*)res_pack->pack_data;
  res_msg->msg_type = MSG_TYPE_FLUSH_FRIEND_RESPOND;
  for(int i = 0; i < res.size(); i++) {
    memcpy(res_msg->data + i * sizeof(UserInfo), &res[i], sizeof(UserInfo));
    std::cout << "id:" << ((UserInfo*)(res_msg->data + i * sizeof(UserInfo)))->id << std::endl;
    std::cout << "name:" << ((UserInfo*)(res_msg->data + i * sizeof(UserInfo)))->name << std::endl;
    std::cout << "online:" << ((UserInfo*)(res_msg->data + i * sizeof(UserInfo)))->online << std::endl;
    //std::cout << res[i].name << std::endl;
  }
  Send(fd, res_pack.get(), res_pack->total_size);
}

void CloudStorageServer::handleDeleteFriendRequest(DataPack* pack, int fd) {
  MessagePack* msg = (MessagePack*)pack->pack_data;
  std::string name1 = msg->common;
  std::string name2 = msg->common + 32;
  std::unique_ptr<DataPack, FreeDeleter> res_pack = makeDataPackMsg(0);
  MessagePack* res_msg = (MessagePack*)res_pack->pack_data;
  res_msg->msg_type = MSG_TYPE_DELETE_FRIEND_RESPOND;
  if(DBOperator::getInstance().deleteFriend(name1, name2)) {
    strcpy(res_msg->common, DELETE_FRIEND_SUCCEED);
  }
  else {
    strcpy(res_msg->common, DELETE_FRIEND_ERROR);
  }
  Send(fd, res_pack.get(), res_pack->total_size);
}

void CloudStorageServer::handlePrivateChatRequest( DataPack *pack, int fd ) {
  MessagePack* msg = (MessagePack*)pack->pack_data;
  std::string send_name(msg->common);
  std::string recv_name(msg->common + 32);
  //std::string text(msg->data);
  //std::cout << send_name << "->" << recv_name << ":" << text << std::endl;
  int res = DBOperator::getInstance().userIsOnline(recv_name);
  if(res == -1) {
    // 用户不存在
  }
  else if(res == 0) {
    // 用户不在线
  }
  else  {
    // 用户在线，直接转发消息
    msg->msg_type = MSG_TYPE_PRIVATE_CHAT_RESPOND;
    int recv_fd = login_fd_map[recv_name];
    Send(recv_fd, pack, pack->total_size);
  }
}

void CloudStorageServer::handlePrivateChatRespond( DataPack *pack, int fd ) {

}


void CloudStorageServer::handleCreateDirRequest(MessagePack* msg_pack, int fd) {
  std::cout << msg_pack->data_size << std::endl;
  std::string cur_path = USER_FILE_DIRROOT + login_user_map[fd].second + "/" + std::string(msg_pack->data);
  std::unique_ptr<DataPack, FreeDeleter> pack_respond = makeDataPackMsg(0);
  MessagePack* msg_respond = (MessagePack*)pack_respond->pack_data;
  msg_respond->msg_type = MSG_TYPE_CREATE_DIR_RESPOND;
  std::cout << "path:" << cur_path <<std::endl;
  if (access(cur_path.c_str(), F_OK) == 0) {  
    // dir exists
    strcpy(msg_respond->common, DIRECTORY_EXISTS);
    Send(fd, pack_respond.get(), pack_respond->total_size);
    return ;
  }
  int status = mkdir_recursive(cur_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (status != 0) {
    if (errno == EEXIST) {
      // 目录已存在
      // ...
      std::cout << "目录存在" << std::endl;
      strcpy(msg_respond->common, DIRECTORY_EXISTS);


    } else {
      // 创建目录失败
      // ...
      std::cout << "创建失败！" << std::endl;
      strcpy(msg_respond->common, CREATE_DIRECTORY_ERROR);
    }
  } else {
    // 创建目录成功
    // ...
    std::cout << "创建成功！" << std::endl;
    strcpy(msg_respond->common, CREATE_DIRECTORY_SUCCEED);
  }

  Send(fd, pack_respond.get(), pack_respond->total_size);
}

void CloudStorageServer::handleFlushFileListResquest(MessagePack* msg_pack, int fd) {
  std::string cur_path = USER_FILE_DIRROOT + login_user_map[fd].second + "/" + std::string(msg_pack->data);
  std::cout << cur_path << std::endl;
  std::unique_ptr<DataPack, FreeDeleter> pack_res = makeDataPackMsg(0);
  MessagePack* msg_res = (MessagePack*)pack_res->pack_data;
  msg_res->msg_type = MSG_TYPE_FLUSH_FILE_LIST_RESPOND;
  if (access(cur_path.c_str(), F_OK) != 0) {  
    // dir not exists
    std::cout << "目录不存在" << std::endl;
    strcpy(msg_res->common, DIRECTORY_EXISTS);
    Send(fd, pack_res.get(), pack_res->total_size);
    return ;
  }  

  DIR* p_dir = opendir(cur_path.c_str()); // 打开目录  
  if (!p_dir) { // 打开目录失败
    std::cout << "目录打开失败" << std::endl;
    strcpy(msg_res->common, OPEN_DIRECTORY_ERROR);
    Send(fd, pack_res.get(), pack_res->total_size);
    return ;  
  } 

  std::vector<FileInfo> file_list; // 文件列表  
  while (true) { // 遍历目录中的每一个文件    
    errno = 0; // 在调用readdir之前清空errno    
    dirent* p_entry = readdir(p_dir); // 读取目录中的一个文件
    if (!p_entry) { // 无更多文件，直接退出循环
      if (errno != 0) { // 如果调用readdir出错，输出错误信息
        strcpy(msg_res->common, READ_DIRECTORY_ERROR);
        Send(fd, pack_res.get(), pack_res->total_size);
        closedir(p_dir);
        return;
      }
      break; // 完整遍历目录结束
    }
    const char* p_name = p_entry->d_name; // 目录项的文件名
    if (strcmp(p_name, ".") == 0 || strcmp(p_name, "..") == 0) {
      continue; // 忽略 "." 和 ".."
    }
    // 获取文件信息
    std::string full_path = cur_path + "/" + p_name; // 获取文件的完整路径
    struct stat st;
    if (lstat(full_path.c_str(), &st) != 0) { // 获取文件信息失败
      std::cerr << "获取文件信息失败: " << full_path << ": " << strerror(errno) << std::endl;
      continue;
    }
    FileInfo file_info = { 0 };
    strcpy(file_info.name, p_name);
    file_info.mtime = st.st_mtime;
    file_info.is_dir = S_ISDIR(st.st_mode);
    file_info.size = st.st_size;
    file_list.push_back(file_info); 
    /*
       std::cout << "name:" << file_list.back().name <<std::endl;
       std::cout << "is_dir:" << file_list.back().is_dir <<std::endl;
       std::cout << "mitime:" << file_list.back().mtime <<std::endl;
       std::cout << "size:" << file_list.back().size <<std::endl;
       */
  }

  std::unique_ptr<DataPack, FreeDeleter> pack_respond = makeDataPackMsg(file_list.size() * sizeof(FileInfo));
  MessagePack* msg_respond = (MessagePack*)pack_respond->pack_data;
  msg_respond->msg_type = MSG_TYPE_FLUSH_FILE_LIST_RESPOND;
  memcpy(msg_respond->data, file_list.data(), file_list.size() * sizeof(FileInfo));
  /*for(int i = 0; i < msg_respond->data_size / sizeof(FileInfo); i++) {
    std::cout << "name:" << ((FileInfo*)msg_respond->data + i)->name << std::endl;
    std::cout << "is_dir" << ((FileInfo*)msg_respond->data + i)->is_dir << std::endl;
    std::cout << "mtime:" << ((FileInfo*)msg_respond->data + i)->mtime << std::endl;
    std::cout << "size:" << ((FileInfo*)msg_respond->data + i)->size << std::endl;
    }
    */
  Send(fd, pack_respond.get(), pack_respond->total_size);
  closedir(p_dir); // 关闭目录
}

void CloudStorageServer::handleDeleteFileRequest(MessagePack* msg_pack, int fd) {
  std::string cur_path = USER_FILE_DIRROOT + login_user_map[fd].second + "/" + std::string(msg_pack->data);
  std::cout << cur_path << std::endl;
  std::unique_ptr<DataPack, FreeDeleter> pack_res = makeDataPackMsg(0);
  MessagePack* msg_res = (MessagePack*)pack_res->pack_data;
  msg_res->msg_type = MSG_TYPE_DELETE_FILE_RESPOND;

  // 检查文件是否存在
  if(access(cur_path.c_str(), F_OK) == 0)
  {
    // 存在则删除
    if(remove(cur_path.c_str()) == 0)
    {
      std::cout << "文件删除成功！" << endl;
      strcpy(msg_res->common, DELETE_DIR_OR_FILE_SUCCEED);
    }
    else
    {
      // 可能是非空目录
      if(removeDir(cur_path.c_str())) {
        strcpy(msg_res->common, DELETE_DIR_OR_FILE_SUCCEED);
      }
      else {
        std::cout << "文件删除失败！" << endl;
        strcpy(msg_res->common, DELETE_DIR_OR_FILE_ERROR);
      }
    } 
  } 
  else
  {
    std::cout << "文件不存在！" << endl; 
    strcpy(msg_res->common, DIRECTORY_NOT_EXISTS);
  }
  Send(fd, pack_res.get(), pack_res->total_size);

}

void CloudStorageServer::handleUploadFileRequest(MessagePack* msg_pack, int fd)  {
  FilePackInfo* file_pack_info = (FilePackInfo*)msg_pack->data;
  TransferFileInfo transfer_file_info;
  transfer_file_info.data_size = file_pack_info->data_size;
  transfer_file_info.file_name = file_pack_info->file_name;
  transfer_file_info.file_size = file_pack_info->file_size;
  transfer_file_info.num = file_pack_info->num;
  transfer_file_info.pack_count = file_pack_info->pack_count;
  std::string user_name = login_user_map[fd].second;
  std::string file_path = std::string(USER_FILE_DIRROOT) + user_name + std::string("/") + std::string(file_pack_info->transfer_file_path);
  if(file_path.back() == '/') transfer_file_info.transfer_file_path = file_path + file_pack_info->file_name;
  else transfer_file_info.transfer_file_path = file_path + "/" + file_pack_info->file_name;
  transfer_file_info.transfer_num = 0;
  transfer_file_info.transfer_size = 0;
  transfer_file_info.transfer_status = false;

  std::cout << "transferFilePath: " << transfer_file_info.transfer_file_path << std::endl;

  transfer_file_info_map_mutex.lock();
  if(transfer_file_info_map.find(user_name) == transfer_file_info_map.end()) {
    std::map<unsigned int, TransferFileInfo> transfer_file_map;
    transfer_file_map[transfer_file_info.num] = transfer_file_info;
    transfer_file_info_map[user_name] = transfer_file_map;
  }
  else {
    transfer_file_info_map[user_name][transfer_file_info.num] = transfer_file_info;
  }
  transfer_file_info_map_mutex.unlock();
  
  std::unique_ptr<DataPack, FreeDeleter> pack_res = makeDataPackMsg(sizeof(file_pack_info->num));
  MessagePack* msg_res = (MessagePack*)pack_res->pack_data;
  msg_res->msg_type = MSG_TYPE_UPLOAD_FILE_RESPOND;
  strcpy(msg_res->common, UPLOAD_FILE_BEGIN);
  // 将文件编号也发送过去
  memcpy(msg_res->data, &file_pack_info->num, sizeof(file_pack_info->num));
  
  Send(fd, pack_res.get(), pack_res->total_size);
  
}

int CloudStorageServer::sendFile( std::string user_name, int num, int fd) {
  std::cout << "sendFile\n";
  std::string file_path;
  TransferFileInfo transfer_file_info;
  SendFileList::getInstance().user_transfer_file_info_map_mutex.lock();
  auto it = SendFileList::getInstance().user_transfer_file_info_map.find(user_name);
  if(it == SendFileList::getInstance().user_transfer_file_info_map.end()) {\
    std::cout << "未找到用户信息\n";
    return -1;
  }
  else {
    if(it->second.transfer_file_info_map.find(num) == it->second.transfer_file_info_map.end()) return -2;
    else {
      transfer_file_info = it->second.transfer_file_info_map[num];
    }
  }
  SendFileList::getInstance().user_transfer_file_info_map_mutex.unlock();

  // send file
  file_path = transfer_file_info.transfer_file_path;
  int file_fd = open(file_path.c_str(),  O_RDONLY);
  if(file_fd < 0) {
    std::cout << "下载文件：文件打开失败！path: " << file_path << std::endl;
    return -3;
  }
  
  int read_bytes = 0;
  
  std::unique_ptr<DataPack, FreeDeleter> data_pack = makeDataPackFile(transfer_file_info.data_size + 1);
  data_pack->pack_type = PACK_TYPE_DOWNLOAD_FILE;
  std::cout << "data_pack:" << data_pack->pack_type << std::endl;
  std::cout << "path:" << transfer_file_info.transfer_file_path << std::endl;
  std::cout << "size:" << transfer_file_info.file_size << std::endl;
  FilePack* file_pack = (FilePack*)data_pack->pack_data;
  file_pack->num = transfer_file_info.num;
  std::cout << "num:" << file_pack->num << std::endl;
  std::cout << "pack_size:" << data_pack->total_size << std::endl;
  do {
    memset(file_pack, 0, transfer_file_info.data_size + 1);
    read_bytes = read(file_fd, file_pack->data, transfer_file_info.data_size);
    if(read_bytes < 0) break;
    else if(read_bytes == 0) break;
    file_pack->data_size = read_bytes;
    file_pack->num = transfer_file_info.num;
    
    data_pack->total_size = data_pack->total_size - transfer_file_info.data_size + read_bytes;
    int res = Send(fd, data_pack.get(), data_pack->total_size);
    if(res > 0) std::cout << "发送文件-> " << file_path << ":" << read_bytes << "bytes\n";
    else std::cout << file_path << "发送错误\n";
  } while(read_bytes != 0);
  if(read_bytes < 0) {
    std::cout << "下载文件：文件读取错误！read()\n";
  }
  else if(read_bytes == 0) {
    std::cout << "文件发送完成!\n";
  }
  return 0;
}

void CloudStorageServer::handleDownloadFileRequest(MessagePack* msg_pack, int fd) {
  std::string user_name = getUserName(fd);
  if(user_name.empty()) {
    std::cout << "用户不存在或者不在线（handleDownloadFileRequest）";
    return;
  }
  std::string file_path = USER_FILE_DIRROOT + user_name + "/" + std::string(msg_pack->data);
  std::cout << "下载文件：" << file_path << std::endl;
  std::unique_ptr<DataPack, FreeDeleter> pack_res = makeDataPackMsg(sizeof(FilePackInfo));
  MessagePack* msg_res = (MessagePack*)pack_res->pack_data;
  msg_res->msg_type = MSG_TYPE_DOWNLOAD_FILE_RESPOND;

  // 检查文件是否存在
  if(access(file_path.c_str(), F_OK) == 0)
  {
    // 存在，开始传送
    
    // 加载文件
    std::cout << "加载文件\n";
    int num = SendFileList::getInstance().addFile(user_name, file_path);
    std::cout << "num:" << num << std::endl;
    if(num >= 0) {
      std::cout << "加载完成!\n";
      FilePackInfo* file = (FilePackInfo*)msg_res->data;
      TransferFileInfo transfer_file_info = SendFileList::getInstance().getTransferFileInfo(user_name, num);
      file->data_size = transfer_file_info.data_size;
      file->file_size = transfer_file_info.file_size;
      file->num = transfer_file_info.num;
      strncpy(file->file_name, transfer_file_info.file_name.c_str(), 64);
      strcpy(msg_res->common, DOWNLOAD_FILE_BEGIN);

      std::cout << "开始发送:\n";
      Send(fd, pack_res.get(), pack_res->total_size);
      

      std::cout << "发送文件：\n";
      // 发送文件
      // 
      //sleep(1);
      //sendFile(user_name, file_path, num, fd);

      return;
    }
    else {
      std::cout << "加载失败num:" << num << std::endl;
      strcpy(msg_res->common, DOWNLOAD_FILE_ERROR);
      Send(fd, pack_res.get(), pack_res->total_size);
    }
  } 
  else
  {
    std::cout << "文件不存在！" << endl; 
    strcpy(msg_res->common, FILE_NOT_EXISTS);
    Send(fd, pack_res.get(), pack_res->total_size);
  }
  
}

void CloudStorageServer::handleDownloadFileRespond(MessagePack* msg_pack, int fd) {
  if(strcmp(msg_pack->common, DOWNLOAD_FILE_BEGIN) == 0) {
    FilePackInfo* file_pack_info = (FilePackInfo*)msg_pack->data;
    std::string name = login_user_map[fd].second;
    sendFile(name, file_pack_info->num, fd);
    //std::cout << num << std::endl;
    return;
  }


  std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
  MessagePack* msg = (MessagePack*)msg_pack->data;
  msg->msg_type = MSG_TYPE_DOWNLOAD_FILE_RESPOND;
  strcpy(msg->common, DOWNLOAD_FILE_SUCCEED);
  Send(fd, pack.get(), pack->total_size);
  std::cout << "客户端已收到文件\n";
}

void CloudStorageServer::handleShareFileRequest( DataPack *pack, int fd ) {
  MessagePack* msg = (MessagePack*)pack->pack_data;
  std::string send_name = getUserName(fd);
  std::string recv_name = msg->common;
  
  std::cout << send_name << "向" << recv_name << "发送文件：" << msg->data << std::endl;

  int recv_fd = getUserFd(recv_name);
  if(recv_fd < 0) {
    // 用户不在线
  }
  else {
    strcpy(msg->common, send_name.c_str());
    Send(recv_fd, pack, pack->total_size);
  }
}

void CloudStorageServer::handleShareFileRespondt(MessagePack* msg_pack, int fd) {
  std::string source = msg_pack->data;
  std::string target = msg_pack->data + source.size() + 1;

  source = USER_FILE_DIRROOT + source;
  target = USER_FILE_DIRROOT + target;
  std::cout << source << std::endl;
  std::cout << target << std::endl;
     
  struct stat st;

  
  if (stat(source.c_str(), &st) != 0) {
    //cout << "Cannot access file or directory " << source << endl;
    std::cout << "源文件不存在" << std::endl;
  }
  //int index = source.size() - 1;
  //while(index >= 0 && source[index] != '/') index--;
  target += string("/") + string(basename(source.c_str()));
  //std::cout << target << std::endl;
	int ret = 0;
  if (S_ISREG(st.st_mode)) {
    ret = CopyFile(source.c_str(), target.c_str());
  } 
	else if (S_ISDIR(st.st_mode)) {
    //string dst_dir = target + string("/") + string(basename(source.c_str()));
    ret = CopyDir(source.c_str(), target.c_str());
  }
  
  std::unique_ptr<DataPack, FreeDeleter> res_pack = makeDataPackMsg(0);
  MessagePack* res_msg = (MessagePack*)res_pack->pack_data;     
  res_msg->msg_type = MSG_TYPE_SHARE_FILE_RESPOND;  
  if(ret == 0) {
    strcpy(res_msg->common, SAVE_FILE_SUCCEED);
  }
  else strcpy(res_msg->common, SAVE_FILE_ERROR);

  Send(fd, res_pack.get(), res_pack->total_size);

}

void CloudStorageServer::handleFilePack(DataPack* pack, int fd) {
  FilePack* file_pack = (FilePack*)pack->pack_data;
  std::cout << "接收到文件包" << file_pack->num << ":" << file_pack->data_size << "bytes!" << std::endl;
  if(login_user_map.find(fd) == login_user_map.end()) return;
  std::string user_name = login_user_map[fd].second;
  transfer_file_info_map_mutex.lock();
  if(transfer_file_info_map.find(user_name) == transfer_file_info_map.end()) return;
  if(transfer_file_info_map[user_name].find(file_pack->num) == transfer_file_info_map[user_name].end()) return;

  TransferFileInfo& transfer_file_info = transfer_file_info_map[user_name][file_pack->num];
  transfer_file_info_map_mutex.unlock();
  
  
  RecvFile::getInstance().writeFile(transfer_file_info, file_pack);
  
  

  std::unique_ptr<DataPack, FreeDeleter> pack_res = makeDataPackFile(0);
  FilePack* file_res = (FilePack*)pack_res->pack_data;
  file_res->num = file_pack->num;
  file_res->data_size = file_pack->data_size;
  Send(fd, pack_res.get(), pack_res->total_size);

  // 传输完成移出相关的文件描述信息，并发送完成信号
  if(transfer_file_info.file_size == transfer_file_info.transfer_size) {
    std::cout << "file_size:" << transfer_file_info.file_size << std::endl;
    std::cout << "transfer_size:" << transfer_file_info.transfer_size << std::endl;
    unsigned int num = transfer_file_info.num;
    transfer_file_info_map_mutex.lock();
    transfer_file_info_map[login_user_map[fd].second].erase(transfer_file_info_map[login_user_map[fd].second].find(transfer_file_info.num));
    transfer_file_info_map_mutex.unlock();

    std::unique_ptr<DataPack, FreeDeleter> pack_msg = makeDataPackMsg(sizeof(num));
    MessagePack* msg = (MessagePack*)pack_msg->pack_data;
    msg->msg_type = MSG_TYPE_UPLOAD_FILE_RESPOND;
    strcpy(msg->common, UPLOAD_FILE_SUCCEED);
    memcpy(msg->data, &num, sizeof(num));
    Send(fd, pack_msg.get(), pack_msg->total_size);
    std::cout << "已发送传输完成信号\n";
  }
}
