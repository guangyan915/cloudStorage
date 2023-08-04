#include "sendfilelist.h"


SendFileList::SendFileList()
{
}

TransferFileInfo SendFileList::getUploadFileInfo(unsigned int num)
{
    if(transfer_file_info_map.find(num) == transfer_file_info_map.end()) {
        QMessageBox::warning(nullptr, "获得上传文件信息", "获得失败！(SendFileList::getUploadFileInfo)");
        TransferFileInfo res;
        return res;
    }
    return transfer_file_info_map[num];
}

bool SendFileList::removeUploadFileInfo(unsigned int num)
{
    auto e = transfer_file_info_map.find(num);
    if(e == transfer_file_info_map.end()) return false;
    transfer_file_info_map.erase(e);
    return true;
}

SendFileList &SendFileList::getInstance()
{
    static SendFileList instance;
    return instance;
}

unsigned int SendFileList::getFilePackDataSize(long long file_size)
{
    return 4096;        // 测试服务器带宽小，先这样设置
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

void SendFileList::setBeUploadCountMax(int count)
{
    if(count < 1 || count > 6) {
        // 不能超过这个范围
        QMessageBox::warning(nullptr, "上传文件", "同时上传个数范围（1 ~ 6）！");
        return;
    }
    be_transfer_count_max = count;
}

bool SendFileList::addFile(QString file_path)
{
    //qDebug() << "addFilePath:" << file_path;
    QFileInfo file_info(file_path);
    TransferFileInfo transfer_file_info;
    transfer_file_info.file_name = file_info.fileName().toStdString();
    transfer_file_info.transfer_file_path = file_path.toStdString();
    transfer_file_info.file_size = file_info.size();
    transfer_file_info.transfer_size = 0;
    // 得到分块大小
    transfer_file_info.data_size = SendFileList::getFilePackDataSize(file_info.size());
    transfer_file_info.pack_count = transfer_file_info.file_size / transfer_file_info.data_size;
    transfer_file_info.transfer_status = false;                         // 还没有开始上传
    transfer_file_info.transfer_num = 0;
    int num = 0;
    // 给文件包找一个合适的编号
    while(transfer_file_info_map.find(num) != transfer_file_info_map.end()) {
        num++;
    }
    transfer_file_info.num = num;
    transfer_file_info_map[transfer_file_info.num] = transfer_file_info;
    //QElapsedTimer timer;
    //elapsed_timer_map[transfer_file_info.num] = timer;
    return true;
}

TransferFileInfo SendFileList::setUploadFileInfo(FilePack *file_pack)
{
    if(transfer_file_info_map.find(file_pack->num) == transfer_file_info_map.end()) {
        QMessageBox::warning(nullptr, "更新上传文件信息", "更新失败！(SendFileList::setUploadFileInfo)");
        TransferFileInfo res;
        return res;
    }
    TransferFileInfo& transfer_file_info = transfer_file_info_map[file_pack->num];
    transfer_file_info.transfer_size += file_pack->data_size;
    transfer_file_info.transfer_num++;

    return transfer_file_info;
}


TransferFileInfo SendFileList::setUploadFileInfo(unsigned int num, bool transfer_status)
{
    if(transfer_file_info_map.find(num) == transfer_file_info_map.end()) {
        QMessageBox::warning(nullptr, "更新上传文件信息", "更新失败！(SendFileList::setUploadFileInfo)");
        TransferFileInfo res;
        return res;
    }
    TransferFileInfo& transfer_file_info = transfer_file_info_map[num];
    transfer_file_info.transfer_status = transfer_status;
    return transfer_file_info;
}

TransferFileInfo SendFileList::setUploadFileInfo(unsigned int num, unsigned int transfer_size, unsigned int transfer_num)
{
    if(transfer_file_info_map.find(num) == transfer_file_info_map.end()) {
        QMessageBox::warning(nullptr, "更新上传文件信息", "更新失败！(SendFileList::setUploadFileInfo)");
        TransferFileInfo res;
        return res;
    }
    TransferFileInfo& transfer_file_info = transfer_file_info_map[num];
    transfer_file_info.transfer_size = transfer_size;
    transfer_file_info.transfer_num = transfer_num;
    return transfer_file_info;
}


bool SendFileList::uploadFile(unsigned int num)
{
    if(be_transfer_count == be_transfer_count_max) {
        // 同时上传个数已达最大
        //qDebug() << "上传个数已达到最大";
        return false;
    }
    if(transfer_file_info_map.find(num) == transfer_file_info_map.end()) {
        // 没有找到文件信息
        //qDebug() << "未找到num对应upload_file_info_map";
        return false;
    }

    TransferFileInfo& transfer_file_info = transfer_file_info_map[num];
    QFile file(transfer_file_info.transfer_file_path.c_str());
    if (file.open(QIODevice::ReadOnly)) {
        // 添加文件上传信息
        MainWidget::getInstance().updateFileUploadListWidget(transfer_file_info);

        transfer_file_info.transfer_status = true;
        int read_bytes = 0;
        std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackFile(transfer_file_info.data_size);
        FilePack* file_pack = (FilePack*)pack->pack_data;
        do {

            // 每次读取 data_size大小数据块
            read_bytes = file.read(file_pack->data, transfer_file_info.data_size);
            // 发送读取到的数据块
            if(read_bytes > 0) {
                file_pack->data_size = read_bytes;
                file_pack->num = transfer_file_info.num;
                pack->total_size = pack->total_size - transfer_file_info.data_size + file_pack->data_size;
                qDebug() << file_pack->num << "已发送：" << TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
            }
            else if(read_bytes == 0){
                // 文件发送完毕
                qDebug() << "文件发送完毕！";
            }
            else {
                // 文件读取错误
                qDebug() << "文件读取错误!";
            }
        } while (read_bytes > 0);
        file.close();
    }
    else {
        qDebug() << "文件打开失败！";
        return false;
    }
    return true;
}

