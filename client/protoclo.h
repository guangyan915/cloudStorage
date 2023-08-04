#ifndef PROTOCLO_H
#define PROTOCLO_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory>
#include <string>

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

//#define FILE_PACK_SIZE  4*1024*1024     // 4Mb

struct FileInfo {
    char name[64];  // 文件名
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
    PACK_TYPE_UPLOAD_FILE,          // 文件包
    PACK_TYPE_DOWNLOAD_FILE,        // 文件下载包
};


struct TransferFileInfo {
    long long file_size;              // 文件总大小
    unsigned int pack_count;          // 文件包总个数
    unsigned int data_size;           // 每次上传文件块大小
    unsigned transfer_size;             // 已传输大小
    unsigned int transfer_num;          // 已传输传包编号
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
    char transfer_file_path[];      // 文件路径
};

struct FilePack {
    unsigned int data_size;    // data的大小
    unsigned int num;          // 包所属组别
    char data[];               // 包数据
};

enum MSG_TYPE {
    MSG_TYPE_MIN = 0,

    MSG_TYPE_LOGIN_REQUEST,         // 登录请求
    MSG_TYPE_LOGIN_RESPOND,         // 登录回复

    MSG_TYPE_REGISTER_REQUEST,         // 注册请求
    MSG_TYPE_REGISTER_RESPOND,         // 注册回复

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

    MSG_TYPE_FLUSH_FILE_LIST_REQUEST,   // 刷新文件列表请求
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


struct FreeDeleter {
  void operator()(void* ptr) const {
    std::free(ptr);
  }
};

std::unique_ptr<DataPack, FreeDeleter> makeAddFriendPack(unsigned int request_size);
std::unique_ptr<DataPack, FreeDeleter> makeDataPack(unsigned int pack_size);
std::unique_ptr<DataPack, FreeDeleter> makeDataPackMsg(unsigned int data_size);
std::unique_ptr<DataPack, FreeDeleter> makeDataPackFile(unsigned int data_size);

#endif // PROTOCLO_H
