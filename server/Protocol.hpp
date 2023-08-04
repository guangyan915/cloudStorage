#ifndef PROTOCLO_H
#define PROTOCLO_H

#include <stdlib.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#define SYETEM_ERROR "system error"

#define LOGIN_SUCCEED "login succeed"
#define LOGIN_USER_ONLINE "user online"
#define LOGIN_NAMEORPASSWD_ERROR "name or passwd error"

#define REGISTER_USER_EXIST "name exist"
#define REGISTER_SUCCEED "register succeed"

//查找条件
#define FIIND_CRITERIA_ID "id"
#define FIIND_CRITERIA_NAME "name"

#define USER_NOT_EXIST "user not exist"
#define USER_EXIST "user exist"

#define ADD_FRIEND_REQUEST_SEND "add friend send"
#define ADD_FRIEND_REQUEST_SEND_ERROR "add friend send error"
#define IS_USER_FRIEND "is your friend"

#define ADD_FRIEND_RESPOND_REJECTED "rejected request"  // 同意请求
#define ADD_FRIEND_RESPOND_ACCEPT "accept request"      // 拒绝请求

#define DIRECTORY_EXISTS "directory exists"
#define DIRECTORY_NOT_EXISTS "directory not exists"

#define CREATE_DIRECTORY_ERROR "create directory error"
#define CREATE_DIRECTORY_SUCCEED "create directory succeed"


#define OPEN_DIRECTORY_ERROR "open_directory_error"
#define READ_DIRECTORY_ERROR "read_directory_error"

#define DELETE_DIR_OR_FILE_ERROR "delete file error"
#define DELETE_DIR_OR_FILE_SUCCEED "delete file succeed"

#define UPLOAD_FILE_SUCCEED "upload file succeed"
#define UPLOAD_FILE_ERROR "upload file error"
#define UPLOAD_FILE_BEGIN "upload file begin"

#define DOWNLOAD_FILE_SUCCEED "download file succeed"
#define DOWNLOAD_FILE_ERROR "download file error"
#define DOWNLOAD_FILE_BEGIN "download file begin"

#define FILE_NOT_EXISTS "file not exist"

#define DELETE_FRIEND_SUCCEED "delete friend succeed"
#define DELETE_FRIEND_ERROR "delete friend error"


#define SAVE_FILE_SUCCEED "save file succeed"
#define SAVE_FILE_ERROR "save file error"


//#define FILE_PACK_SIZE 4*1024*1024

struct FileInfo {
  char name[64]; // 文件名
  time_t mtime;   // 文件修改时间
  bool is_dir;    // 是否是目录
  long long size;     // 文件大小
};

struct UserSelfInfo {

};

class UserInfo{
public:
    unsigned int id;
    char name[32];
    char online;
    // 待拓展 
    
        
    UserInfo& operator= (const UserInfo& other) {
      id = other.id;
      strcpy(name, other.name);
      online = other.online;  
      return *this;
    }
};

enum PACK_TYPE {
    PACK_TYPE_MIN = 0,

    PACK_TYPE_MSG,                  // 消息包
    PACK_TYPE_UPLOAD_FILE,                 // 文件包
    PACK_TYPE_DOWNLOAD_FILE,        // 文件下载包
};

struct TransferFileInfo {
  long long file_size;              // 文件总大小
  unsigned int pack_count;          // 文件包总个数
  unsigned int data_size;           // 每次上传文件块大小
  unsigned transfer_size;             // 已上传大小
  unsigned int transfer_num;          // 已上传包编号
  unsigned int num;                 // 所属组别
  bool transfer_status;               // 上传状态
  std::string file_name;            // 文件名
  std::string transfer_file_path;     // 文件存储路径
};

struct FilePackInfo {
    long long file_size;            // 文件总大小
    unsigned int pack_count;        // 文件包总个数
    unsigned int num;               // 所属组别
    unsigned int data_size;         // 每个文件包的大小
    char file_name[64];             // 文件名
    char transfer_file_path[];        // 上传的路径
};

struct FilePack {
  unsigned int data_size;         // data的大小  
  unsigned int num;               // 包所属组别
  char data[];                    // 包数据
};

enum MSG_TYPE {
    MSG_TYPE_MIN = 0,

    MSG_TYPE_LOGIN_REQUEST,         // 登录请求
    MSG_TYPE_LOGIN_RESPOND,         // 登录回复

    MSG_TYPE_REGISTER_REQUEST,    // 注册请求
    MSG_TYPE_REGISTER_RESPOND,      // 注册回复
    
    MSG_TYPE_FIND_USER_REQUEST,        // 查找用户请求
    MSG_TYPE_FIND_USER_RESPOND,        // 查找用户回复
    

    MSG_TYPE_ADD_FRIEND_REQUEST,        // 加好友请求
    MSG_TYPE_ADD_FRIEND_RESPOND,        // 加好友回复


    MSG_TYPE_FLUSH_FRIEND_REQUEST,      // 刷新好友列表请求
    MSG_TYPE_FLUSH_FRIEND_RESPOND,      // 刷新好友列表响应

    MSG_TYPE_DELETE_FRIEND_REQUEST,      // 删除好友请求
    MSG_TYPE_DELETE_FRIEND_RESPOND,      // 删除好友响应
    
    MSG_TYPE_PRIVATE_CHAT_REQUEST,      // 私聊好友
    MSG_TYPE_PRIVATE_CHAT_RESPOND,      // 私聊好友
    
    MSG_TYPE_CREATE_DIR_REQUEST,        // 创建目录请求
    MSG_TYPE_CREATE_DIR_RESPOND,        // 创建目录响应

    MSG_TYPE_FLUSH_FILE_LIST_REQUEST,   // 创建文件列表请求
    MSG_TYPE_FLUSH_FILE_LIST_RESPOND,   // 刷新文件列表响应

    MSG_TYPE_DELETE_FILE_REQUEST,       // 删除文件请求
    MSG_TYPE_DELETE_FILE_RESPOND,       // 删除文件相应

    MSG_TYPE_UPLOAD_FILE_REQUEST,       // 上传文件请求
    MSG_TYPE_UPLOAD_FILE_RESPOND,       // 上传文件响应

    MSG_TYPE_DOWNLOAD_FILE_REQUEST,     // 下载文件请求
    MSG_TYPE_DOWNLOAD_FILE_RESPOND,     // 下载文件响应
    
    MSG_TYPE_SHARE_FILE_REQUEST,        // 分享文件请求
    MSG_TYPE_SHARE_FILE_RESPOND,        // 分享文件响应

};


// 添加好友申请、回复包
struct AddFriendPack {
    char send[32];
    char recv[32];
    char data[];
};

struct MessagePack {
    unsigned int data_size;         // data大小
    unsigned int msg_type;          // 消息包类型
    char common[64];                // 可以存放一些不大于64字节的消息： 比如用户名、密码之类
    char data[];                    // 大于64字节的消息放在这里
};

struct DataPack
{
    unsigned int total_size;        // 数据总大小
    unsigned int pack_type;         // 数据包类型
    char pack_data[];               // 数据包
};



// 使用智能指针来管理

struct FreeDeleter {
  void operator()(void* ptr) const {
    std::free(ptr);
  }
};

std::unique_ptr<DataPack, FreeDeleter> makeAddFriendPack(unsigned int request_size) {
    unsigned int total_size = sizeof(DataPack) + sizeof(MessagePack) + sizeof(AddFriendPack) + request_size;
    
    std::unique_ptr<DataPack, FreeDeleter> pack((DataPack*)std::malloc(total_size));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    memset(pack.get(), 0, total_size);
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->data_size = sizeof(AddFriendPack) + request_size;
    pack->total_size = total_size;
    pack->pack_type = PACK_TYPE_MSG;
    return pack;
}

std::unique_ptr<DataPack, FreeDeleter> makeDataPack(unsigned int pack_size) {
    unsigned int total_size = sizeof(DataPack) + pack_size;
    std::cout << "make total_size:" << total_size << std::endl;
    std::unique_ptr<DataPack, FreeDeleter> pack((DataPack*)std::malloc(total_size));
    if(pack == nullptr) {
        printf("makeDataPack申请内存失败！\n");
        exit(EXIT_FAILURE);
    }
    memset(pack.get(), 0, total_size);
    pack->total_size = total_size;
    return pack;
}

std::unique_ptr<DataPack, FreeDeleter> makeDataPackMsg(unsigned int data_size)
{
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPack(data_size + sizeof(MessagePack));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    pack->pack_type = PACK_TYPE_MSG;
    MessagePack* msg_page = (MessagePack*)pack->pack_data;
    msg_page->data_size = data_size;
    return pack;
}

std::unique_ptr<DataPack, FreeDeleter> makeDataPackFile(unsigned int data_size)
{
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPack(data_size + sizeof(FilePack));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    pack->pack_type = PACK_TYPE_UPLOAD_FILE;
    return pack;
}

// 递归创建目录

int mkdir_recursive(const char *path, mode_t mode)
{
    char *p = strdup(path);
    char *c = strchr(p + 1, '/');

    while (c) {
        *c = '\0';
        if (mkdir(p, mode) && errno != EEXIST) {
            perror("mkdir");
            free(p);
            return -1;
        }
        *c = '/';
        c = strchr(c + 1, '/');
    }

    if (mkdir(p, mode) && errno != EEXIST) {
        perror("mkdir");
        free(p);
        return -1;
    }

    free(p);
    return 0;
}

// 删除目录bool 
bool removeDir(const std::string& dirname)
{
    DIR* dir = opendir(dirname.c_str());
    if(dir == NULL)
    {
      std::cerr << "Unable to open directory: " << dirname << ", error code: " << errno << std::endl;
        return false;
    }

    dirent* entry;
    struct stat info;
    std::string path;

    while((entry = readdir(dir)) != NULL)
    {
        // 忽略特殊目录 . 和 ..
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        path = dirname + "/" + entry->d_name;
        if(lstat(path.c_str(), &info) != 0)
        {
          std::cerr << "Unable to get file status: " << path << ", error code: " << errno << std::endl;
            continue;
        }

        if(S_ISDIR(info.st_mode))
        {
            // 是目录，则递归删除
            if(!removeDir(path)) return false;
        }
        else
        {
            // 是文件，则直接删除
            if(!remove(path.c_str())) return false;
        }
    }

    if(rmdir(dirname.c_str()) != 0)
    {
      std::cerr << "Unable to remove directory: " << dirname << ", error code: " << errno << std::endl;
        return false;
    }

    closedir(dir);

    return true;
}


#define BUFFER_LENGTH 1024

int IsDir(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

// 复制文件
int CopyFile(const char* src_path, const char* dst_path) {
    char buffer[BUFFER_LENGTH];
    FILE* src_file = fopen(src_path, "rb");
    if (src_file == NULL) {
        cout << "Cannot open source file " << src_path << endl;
        return -1;
    }

    FILE* dst_file = fopen(dst_path, "wb");
    if (dst_file == NULL) {
        cout << "Cannot create destination file " << dst_path << endl;
        fclose(src_file);
        return -1;
    }

    size_t count;
    while ((count = fread(buffer, 1, BUFFER_LENGTH, src_file)) > 0) {
        fwrite(buffer, 1, count, dst_file);
    }

    fclose(src_file);
    fclose(dst_file);

    return 0;
}

// 复制文件夹

int CopyDir(const char* src_path, const char* dst_path) {
    DIR* dir = opendir(src_path);
    if (dir == NULL) {
        cerr << "Cannot open source directory " << src_path << endl;
        return -1;
    }

    struct stat st;
    if (stat(src_path, &st) == -1) {
        cerr << "Cannot get status of source directory " << src_path << endl;
        return -1;
    }

    if (mkdir(dst_path, st.st_mode) == -1) {
        cerr << "Cannot create destination directory " << dst_path << endl;
        closedir(dir);
        return -1;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        string entry_name = entry->d_name;
        if (entry_name == "." || entry_name == "..") {
            continue;
        }

        string src_file = src_path;
        src_file += "/";
        src_file += entry_name;
        string dst_file = dst_path;
        dst_file += "/";
        dst_file += entry_name;

        if (IsDir(src_file.c_str())) {
            return CopyDir(src_file.c_str(), dst_file.c_str());
        } else {
            return CopyFile(src_file.c_str(), dst_file.c_str());
        }
    }

    closedir(dir);
    return 0;
}


#endif // PROTOCLO_H
