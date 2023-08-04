#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include "Protocol.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <mutex>


class RecvFile {
private:
  std::map<std::string, int> fd_map;
public:
  static RecvFile& getInstance();
  int writeFile(TransferFileInfo& transfer_file_info, FilePack* file_pack);
};

RecvFile& RecvFile::getInstance() {
  static RecvFile instance;
  return instance;
}

int RecvFile::writeFile(TransferFileInfo& transfer_file_info, FilePack* file_pack) {
  // 已追加的方式打开
  if(fd_map.find(transfer_file_info.transfer_file_path.c_str()) == fd_map.end()) {
    int fd = open(transfer_file_info.transfer_file_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    if(fd < 0) {
      std::cout << "文件打开失败！\n";
      return -1;
    }
    fd_map[transfer_file_info.transfer_file_path.c_str()] = fd;
    transfer_file_info.transfer_status = true;
  }
  
  write(fd_map[transfer_file_info.transfer_file_path.c_str()], file_pack->data, file_pack->data_size);
  transfer_file_info.transfer_size += file_pack->data_size;
  transfer_file_info.transfer_num++;
  
  if(transfer_file_info.file_size == transfer_file_info.transfer_size) {
    std::cout << "文件上传完成!\n";
    std::cout << "路径：" << transfer_file_info.transfer_file_path  << std::endl;
    close(fd_map[transfer_file_info.transfer_file_path.c_str()]);
    fd_map.erase(fd_map.find(transfer_file_info.transfer_file_path.c_str()));
  }

  return transfer_file_info.transfer_size;
}
