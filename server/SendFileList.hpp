#pragma once
#include <string>
#include <map>
#include <mutex>
#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>
#include "Protocol.hpp"

#define KB *1024
#define MB *1024*1024
#define GB *1024*1024*1024

struct SendFilelistInfo {
  std::map<unsigned int, TransferFileInfo> transfer_file_info_map;
  int be_transfer_count = 0;                    // 当前传输文件的个数
  int be_transfer_count_max = 1;                // 传输文件的最大个数
};

class SendFileList {
public:
  std::map<std::string, SendFilelistInfo> user_transfer_file_info_map;
  std::mutex user_transfer_file_info_map_mutex;

public:
  SendFileList() {}
  TransferFileInfo getTransferFileInfo(std::string user_name, unsigned int num); // 获得传输文件信息管理的结构体
  static SendFileList& getInstance();     
  static unsigned int getFilePackDataSize(long long file_size);                  // 获得传输文件包的大小
  int setBeTransferCountMax(std::string user_name, int num , int count);         // 设置最大值
  
  int addFile(std::string user_name, std::string file_path);                    // 添加文件信息传输管理结构体到map里
  TransferFileInfo updateTransferFileInfo(std::string user_name, FilePack* file_pack); //更新TransferFileInfo信息;
};

SendFileList& SendFileList::getInstance() {
  static SendFileList instance;
  return instance;
}

unsigned int SendFileList::getFilePackDataSize(long long file_size) {

  return 4096;        // 服务器带宽不够，先这样设置

    if(file_size <= 1 MB) {
        // (0, 1MB]
        return 64 KB;
    }
    else if(file_size <= 10 MB) {
        return 128 KB;
    }
    else if(file_size <= 100 MB) {
        return 256 KB;
    }
    else if(file_size <= 500 MB) {
        return 512 KB;
    }
    else if(file_size <= 1 GB) {
        return  1 MB;
    }
    else  {
        // 允许上传的单个最大文件为2GB
        return 2 MB;
    }
}

TransferFileInfo SendFileList::getTransferFileInfo(std::string user_name, unsigned int num) {

  TransferFileInfo res;
  user_transfer_file_info_map_mutex.lock();
  auto it = user_transfer_file_info_map.find(user_name);
  if(it == user_transfer_file_info_map.end()) {
    return res;
  }
  else {
    if(it->second.transfer_file_info_map.find(num) == it->second.transfer_file_info_map.end()) return res;
    else res = it->second.transfer_file_info_map[num];
  }
  user_transfer_file_info_map_mutex.unlock();
  return res;
}

TransferFileInfo SendFileList::updateTransferFileInfo(std::string user_name, FilePack *file_pack) {
  TransferFileInfo res;
  
  user_transfer_file_info_map_mutex.lock();
  auto it = user_transfer_file_info_map.find(user_name);
  if(it == user_transfer_file_info_map.end()) {
    // 不存在返回一个file_name为空的TransferFileInfo
    return res;
  }
  else {
    if(it->second.transfer_file_info_map.find(file_pack->num) == it->second.transfer_file_info_map.end()) return res;
    else {
      // 存在的话更新TransferFileInfo的信息，并返回其拷贝
      TransferFileInfo& transfer_file_info = it->second.transfer_file_info_map[file_pack->num];
      transfer_file_info.transfer_num++;
      transfer_file_info.transfer_size += file_pack->data_size;
      transfer_file_info.transfer_status = true;
      res = transfer_file_info;
    }
  }
  user_transfer_file_info_map_mutex.unlock();
  return res;
}


int SendFileList::setBeTransferCountMax(std::string user_name, int num, int val) {
  if(val < 1 || val > 6) {
    return -3;
  }
  // -1 用户已经下线，或者用户不存在
  // -2 该TransferFileInfo不存在
  // -3 不在同时传输个数范围
  // 0 修改成功
  user_transfer_file_info_map_mutex.lock();
  auto it = user_transfer_file_info_map.find(user_name);
  if(it == user_transfer_file_info_map.end()) {
    return -1;
  }
  else {
    if(it->second.transfer_file_info_map.find(num) == it->second.transfer_file_info_map.end()) return -2;
    else {
      it->second.be_transfer_count_max = val;
    }
  }
  user_transfer_file_info_map_mutex.unlock();
  return 0;
}

int SendFileList::addFile(std::string user_name, std::string file_path) {
  // -1 path error
  // 
  // -2 get file info error
  
  std::string file_name;
  size_t pos = file_path.find_last_of('/');
  if (pos != std::string::npos)
  {
    file_name = file_path.substr(pos + 1);
  }
  else
  {
    std::cout << "获取文件名失败!\n";
    return -1;  
  }
  std::cout << "文件名:" << file_name << std::endl;
  int num = 0;
  user_transfer_file_info_map_mutex.lock();
  auto& it = user_transfer_file_info_map[user_name]; 
    struct stat file_info;
    
    if(stat(file_path.c_str(), &file_info) != 0) {
      std::cout << "获取文件信息失败!\n";
      // 获取文件信息失败
      return -2;
    }
    TransferFileInfo transfer_file_info;
    transfer_file_info.file_name = file_name;
    transfer_file_info.file_size = file_info.st_size;
    //std::cout << "file_size:" << transfer_file_info.file_size << std::endl;
    transfer_file_info.transfer_file_path = file_path;
    transfer_file_info.data_size = SendFileList::getFilePackDataSize(file_info.st_size);
    std::cout << "开始获得编号！"<< std::endl;
    while(it.transfer_file_info_map.find(num) != it.transfer_file_info_map.end()) {
      std::cout << num <<std::endl;
      if(num == INT32_MAX) {
        std::cout << "找不到合适的文件编号num(addFile)" << std::endl;
        return -4;
      }
      num++;
    }
    std::cout << "文件编号:" << num << std::endl;
  transfer_file_info.num = num;
  transfer_file_info.pack_count = transfer_file_info.file_size / transfer_file_info.data_size;
  transfer_file_info.transfer_num = 0;
  transfer_file_info.transfer_size = 0;
  transfer_file_info.transfer_status = false;
  it.transfer_file_info_map[transfer_file_info.num] = transfer_file_info;
  std::cout << "insert succeed\n";
  user_transfer_file_info_map_mutex.unlock();

  return num;
}

