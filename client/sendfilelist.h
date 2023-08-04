#ifndef SENDFILECACHE_H
#define SENDFILECACHE_H
#include <vector>
#include <map>
#include <unordered_map>
#include "protoclo.h"
#include "tcpclient.h"
#include "mainwidget.h"
#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <mutex>
#include <QFile>
#include <QFileInfo>
#include <fstream>
#include <string>
#include <QMessageBox>
#include <QElapsedTimer>

#define KB *1024
#define MB *1024*1024
#define GB *1024*1024*1024

class SendFileList
{
private:
  std::map<unsigned int, TransferFileInfo> transfer_file_info_map;
  //std::map<unsigned int, QElapsedTimer> elapsed_timer_map;
  int be_transfer_count = 0;
  int be_transfer_count_max = 1;
public:
  SendFileList();
  TransferFileInfo getUploadFileInfo(unsigned int num);
  bool removeUploadFileInfo(unsigned int num);
  static SendFileList& getInstance();
  static unsigned int getFilePackDataSize(long long file_size);
  void setBeUploadCountMax(int count);
  bool addFile(QString file_path);
  TransferFileInfo setUploadFileInfo(FilePack* file_pack);
  TransferFileInfo setUploadFileInfo(unsigned int num, bool transfer_status);
  TransferFileInfo setUploadFileInfo(unsigned int num, unsigned transfer_size, unsigned int transfer_num);
  bool uploadFile(unsigned int num);
};

#endif // SENDFILECACHE_H
