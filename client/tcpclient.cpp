#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "mainwidget.h"
#include "loginwidget.h"
#include "addfriend.h"
#include "addfriendrequest.h"
#include "register.h"
#include "userinfowidget.h"

TCPClient::TCPClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TCPClient)
{
    ui->setupUi(this);

    toLoadConfig();
    // 登录界面
    LoginWidget::getInstance().show();
    //MainWidget::getInstance();
    //SendFileList::getInstance();
    //AddFriend::getInstance();
    //AddFriendRequest::getInstance();
    //Register::getInstance();
    //UserInfoWidget::getInstance();

    // 连接服务器,连接成功发出信号:connected()
    //tp.setMaxThreadCount(8);
    client_socket.connectToHost(QHostAddress(server_ip), server_port);
    recv_flush = new QTimer();
    connect(recv_flush, SIGNAL(timeout()),this, SLOT(readRecvBuff()));
    recv_flush->start(1);
    connect(&client_socket, SIGNAL(connected()),
            this, SLOT(connectState()));
    //connect(&client_socket, SIGNAL(readyRead()),\
            this, SLOT(handleRecvMsg()));
    connect(&client_socket, SIGNAL(readyRead()),\
    this, SLOT(recvMsg()));

    // 定时write
    QObject::connect(&write_flush, &QTimer::timeout, [&]() {
            unsigned int size = 0;
            Write((char*)&size, sizeof(unsigned int)); // 每 100 毫秒写入一次数据
            qDebug() << "write flush 4 bytes";
        });
}

TCPClient::~TCPClient()
{
    delete ui;
}

void TCPClient::toLoadConfig()
{
    QFile file(":/configFile/client.config");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString line;
        QTextStream in(&file);  //用文件构造流
        while (!in.atEnd())
        {
            QString line = in.readLine();
            //qDebug()<<line;
            QString key, value;
            int index = 0;
            while(index < line.size() && line[index] != ':') {
                key += line[index];
                index++;
            }
            index++;
            while(index < line.size() && line[index] == ' ') index++;
            while(index < line.size() && line[index] != ' ') {
                value += line[index];
                index++;
            }

            if(key == "server_ip") {
                server_ip = value;
            }
            if(key == "server_port") {
                server_port = value.toUShort();
            }
        }
    }
    else {
        QMessageBox::critical(this, "打开配置文件", "打开失败！");
    }
    file.close();
    //qDebug() << server_ip << server_port;
}

TCPClient &TCPClient::getInstance()
{
    static TCPClient instance;
    return instance;
}

const UserInfo &TCPClient::getMyselfInfo()
{
    return myself_info;
}

void TCPClient::setMyselfInfo(const UserInfo &user)
{
    myself_info.id = user.id;
    strcpy(myself_info.name, user.name);
    myself_info.online = user.online;
}

void TCPClient::connectState()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功！");
}

void TCPClient::handleRecvMsg()
{
    QtConcurrent::run(TCPClient::recvMsg);
}

void TCPClient::recvMsg()
{
    unsigned int total_size = 0;
    if(TCPClient::getInstance().getBytesAvailable() >= sizeof(total_size)) {
        qint64 bytesReceived = TCPClient::getInstance().client_socket.peek((char*)(&total_size), sizeof(total_size));  // 读取前4个字节的数据，但不将这些数据从缓冲区中移除
        if (bytesReceived < sizeof(total_size) || TCPClient::getInstance().getBytesAvailable() < total_size)
        {
            qDebug() << "缓冲区数据：" << TCPClient::getInstance().getBytesAvailable();
            qDebug() << "taotal_size:" << total_size;
            return ;
        }
    }

    qDebug() << "缓冲区中数据：" << TCPClient::getInstance().getBytesAvailable();
    qDebug() << "收到消息";
    // 获取包大小
    int res = TCPClient::getInstance().Read((char*)&total_size, sizeof(unsigned int));

    //qDebug() << "total_size:" << total_size;
    if(total_size == 0) {
        while(total_size == 0) {
            TCPClient::getInstance().Read((char*)&total_size, sizeof(unsigned int));

        }
    }
    unsigned int pack_size = total_size - sizeof(DataPack);
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPack(pack_size);
    unsigned int pack_type = pack->pack_type;
    TCPClient::getInstance().Read((char*)pack.get() + sizeof(unsigned int), total_size - sizeof(unsigned int));

    qDebug() << "data包类型：" << pack->pack_type;
    if(pack->pack_type == PACK_TYPE_MSG) {
        TCPClient::getInstance().handleMsgPack(pack.get());
    }
    else if(pack->pack_type == PACK_TYPE_UPLOAD_FILE) {
        TCPClient::getInstance().handleFilePack(pack.get());
    }
    else if(pack->pack_type == PACK_TYPE_DOWNLOAD_FILE) {
        qDebug() << "收到文件包!";
        TCPClient::getInstance().handleDownloadFilePack(pack.get());
    }
    else {
        qDebug() << "pack_type 没有定义";
    }
}

qint64 TCPClient::getBytesAvailable()
{
    return client_socket.bytesAvailable();
}

TransferFileInfo &TCPClient::getDownloadFileInfo(unsigned int num)
{
    //if(download_file_map.find(num) == download_file_map.end())
    return download_file_map[num];
}

bool TCPClient::removeDownloadFileInfo(unsigned int num)
{
    auto e = download_file_map.find(num);
    if(e == download_file_map.end()) return false;
    download_file_map.erase(e);
    return true;
}

void TCPClient::handleMsgPack(DataPack *pack)
{
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;

    //qDebug() << "msg_type:" << msg_pack->msg_type;
    switch(msg_pack->msg_type) {
    case MSG_TYPE_LOGIN_RESPOND :
        handleLoginRespond(pack);
        break;
    case MSG_TYPE_REGISTER_RESPOND :
        handleRegister(pack);
        break;
    case MSG_TYPE_FIND_USER_RESPOND :
        handleFindUserRespond(pack);
        break;
    case MSG_TYPE_ADD_FRIEND_REQUEST :
        handleAddFrendRequest(pack);
        break;
    case MSG_TYPE_ADD_FRIEND_RESPOND :
        handleAddFrendRespond(pack);
        break;
    case MSG_TYPE_FLUSH_FRIEND_RESPOND :
        handleFlushFrendRespond(pack);
        break;
    case MSG_TYPE_DELETE_FRIEND_RESPOND :
        handleDeleteFrendRespond(pack);
        break;
    case MSG_TYPE_PRIVATE_CHAT_RESPOND :
        handlePrivateChatRespond(pack);
        break;
    case MSG_TYPE_CREATE_DIR_RESPOND :
        handleCreateDirRespond(pack);
        break;
    case MSG_TYPE_FLUSH_FILE_LIST_RESPOND :
        handleFlushFileListrequest(pack);
        break;
    case MSG_TYPE_DELETE_FILE_RESPOND :
        handleDeleteFileRequest(pack);
        break;
    case MSG_TYPE_UPLOAD_FILE_RESPOND :
        handleUploadFileRequest(pack);
        break;
    case MSG_TYPE_DOWNLOAD_FILE_RESPOND :
        handleDownloadFileRequest(pack);
        break;
    case MSG_TYPE_SHARE_FILE_REQUEST :
        handleShareFileRequest(pack);
        break;
    case MSG_TYPE_SHARE_FILE_RESPOND :
        handleShareFileRespond(pack);
        break;
    default :

        break;
    }
}

void TCPClient::handleFilePack(DataPack *pack)
{
    FilePack* file_pack = (FilePack*)pack->pack_data;
    qDebug() << "服务器接受到：" << file_pack->num << "文件：" << file_pack->data_size << "bytes";
    TransferFileInfo transfer_file_info = SendFileList::getInstance().setUploadFileInfo(file_pack);
    if(transfer_file_info.file_name.empty()) {
        QMessageBox::warning(nullptr, "更新进度条", "更新失败(TCPClient::handleFilePack)");
        return;
    }
    int val = transfer_file_info.transfer_size * 100 / transfer_file_info.file_size;
    MainWidget::getInstance().updateFileUploadListWidget(transfer_file_info.file_name.c_str(), val);
}


void TCPClient::handleDownloadFilePack(DataPack *pack)
{
    FilePack* file_pack = (FilePack*)pack->pack_data;
    QString file_path = download_file_map[file_pack->num].transfer_file_path.c_str();
    //qDebug() << "pack_num:" << file_pack->num;
    //qDebug() << "num:" << download_file_map[file_pack->num].num;
    //qDebug() << "name:" << download_file_map[file_pack->num].file_name;
    //qDebug() << "path:" << file_path;
    if(file_path.isEmpty()) {
        //QMessageBox::warning(this, "下载", "下载错误！");
    }
    QFile file(file_path);
    if (file.open(QIODevice::Append | QIODevice::WriteOnly)) {
        file.write(file_pack->data, file_pack->data_size);
        download_file_map[file_pack->num].transfer_size += file_pack->data_size;
        download_file_map[file_pack->num].transfer_num++;
        qDebug() << "传输大小:" << download_file_map[file_pack->num].transfer_size;
        int val = download_file_map[file_pack->num].transfer_size * 100 / download_file_map[file_pack->num].file_size;

        // 更新进度条

        MainWidget::getInstance().updateFileDonwloadListWidget(download_file_map[file_pack->num].file_name.c_str(), val);

        // 文件每达到1mb或者文件接收完成，将文件写入

        if(download_file_map[file_pack->num].transfer_size == download_file_map[file_pack->num].file_size || \
            download_file_map[file_pack->num].transfer_size % 1 MB == 0) {
            qDebug() << "传输文件大小：" << download_file_map[file_pack->num].transfer_size;
            qDebug() << "总文件大小：" << download_file_map[file_pack->num].file_size;
            file.flush();
            file.close();
            if(download_file_map[file_pack->num].transfer_size == download_file_map[file_pack->num].file_size) {
                // 完成
                QMessageBox::information(this, "下载", "下载完成！");
                MainWidget::getInstance().downloadOver(file_pack->num);

            }
        }
        file.close();
    }
    else
    {
        QMessageBox::warning(this, "下载文件", "文件下载失败,创建本地文件失败！");
    }


}

int TCPClient::Write(char *ptr, int size)
{
    write_mutex.lock();
    int total_size = 0;
    while(total_size != size) {
        total_size += client_socket.write((char*)ptr + total_size, size - total_size);
    }
    write_mutex.unlock();
    return total_size;
}

int TCPClient::Read(char *ptr, int size)
{
    read_mutex.lock();
    if(getBytesAvailable() < size) return -2;
    qDebug() << size;
    int total_size = 0;
    while(total_size != size) {
        int read_bytes = client_socket.read((char*)ptr + total_size, size - total_size);
        total_size += read_bytes;
        qDebug() << "read:" << read_bytes;
        break;
    }
    read_mutex.unlock();
}


void TCPClient::handleLoginRespond(DataPack *pack)
{
    //qDebug() << "login";
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    qDebug() << (char*)msg_pack->common;
    if(strcmp(msg_pack->common, LOGIN_SUCCEED) == 0) {
        QMessageBox::information(this, "登录", "登录成功");
        LoginWidget::getInstance().close();
        // 登陆成功 先获得自己的信息
        std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
        MessagePack* msg_pack = (MessagePack*)pack->pack_data;
        msg_pack->msg_type = MSG_TYPE_FIND_USER_REQUEST;

        if(LoginWidget::getInstance().getLoginManner() == 0) {
            // 用户名登录
            strcpy(msg_pack->common, FIIND_CRITERIA_NAME);
        }

        strcpy(msg_pack->common + 32, LoginWidget::getInstance().getLoginName().toStdString().c_str());
        Write((char*)pack.get(), pack->total_size);

        //qDebug() << "好难呀";
        MainWidget::getInstance().setLableUserName("用户：" + LoginWidget::getInstance().getLoginName());
        MainWidget::getInstance().show();
        MainWidget::getInstance().flushFriendList();
        //qDebug() << "mainWidget Show";
    }
    else if(strcmp(msg_pack->common, LOGIN_USER_ONLINE) == 0) {
        QMessageBox::information(this, "登录", "登录失败！此账号已经登录！");
    }
    else {
        QMessageBox::information(this, "登录", "账号或密码错误！");
    }
}

void TCPClient::handleRegister(DataPack *pack)
{
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    qDebug() << (char*)msg_pack->common;
    if(strcmp(msg_pack->common, REGISTER_SUCCEED) == 0) {
        //QMessageBox::information(this, "注册", "注册成功！");
        int ret = QMessageBox::question(this, tr("注册成功"), tr("是否将账号密码填入登录框？"));
        if (ret == QMessageBox::No) {
            return;
        }
        QString name = Register::getInstance().user_name;
        QString passwd = Register::getInstance().user_passwd;
        LoginWidget::getInstance().setLoginNamePasswd(name, passwd);
        Register::getInstance().hide();
    }
    else if(strcmp(msg_pack->common, REGISTER_USER_EXIST) == 0) {
        QMessageBox::information(this, "注册", "注册失败！用户名已存在！");
    }
    else {
        QMessageBox::information(this, "注册", "注册失败！");
    }
}

void TCPClient::handleFindUserRespond(DataPack *pack)
{
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    if(!setMyself) {
        //qDebug() << "setMyself";
        setMyself = true;
        UserInfo* user;
        user = (UserInfo*)msg_pack->data;
        myself_info.id = user->id;
        strcpy(myself_info.name, user->name);
        myself_info.online = user->online;
        return;
    }
    if(strcmp(msg_pack->common, USER_EXIST) == 0) {
        int user_count = msg_pack->data_size / sizeof(UserInfo);
        qDebug() << "用户数：" << user_count;
        std::vector<UserInfo> ret(user_count);
        for(int i = 0; i < user_count; i++) {
            UserInfo* user;
            user = (UserInfo*)msg_pack->data + i * sizeof(UserInfo);
            ret[i].id = user->id;
            strcpy(ret[i].name, user->name);
            ret[i].online = user->online;
        }
        AddFriend::getInstance().updateUserListWidget(ret);
    }
    else {
        QMessageBox::warning(this, "添加好友", "用户不存在！");
    }
}

void TCPClient::handleAddFrendRequest(DataPack *pack)
{
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    MainWidget::getInstance().tabFriendRequestStartDiscolor();
    AddFriend::getInstance().updateFriendRequest(msg_pack);
}

void TCPClient::handleAddFrendRespond(DataPack *pack)
{
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;

    if(strcmp(msg_pack->common, USER_NOT_EXIST) == 0) {
        QMessageBox::warning(this, "添加好友", "用户不存在！");
    }
    else if(strcmp(msg_pack->common, IS_USER_FRIEND) == 0) {
        QMessageBox::warning(this, "添加好友", "你们已经是好友！");
    }
    else if(strcmp(msg_pack->common, ADD_FRIEND_REQUEST_SEND) == 0) {
        QMessageBox::information(this, "添加好友", "请求发送成功！");
    }
    else if(strcmp(msg_pack->common, ADD_FRIEND_REQUEST_SEND_ERROR) == 0) {
        QMessageBox::information(this, "添加好友", "请求发送失败！");
    }
    else if(strcmp(msg_pack->common, ADD_FRIEND_RESPOND_ACCEPT) == 0){
        // 同意，更新消息列表和好友列表，并将好友申请移出
        MainWidget::getInstance().flushFriendList();
    }
    else if(strcmp(msg_pack->common, ADD_FRIEND_RESPOND_REJECTED) == 0) {
        // 拒绝，将好友申请信息更新移出
    }
    else {
        QMessageBox::warning(this, "添加好友", "其他信息");
    }
}

void TCPClient::handleFlushFrendRespond(DataPack *pack)
{
    MainWidget::getInstance().getFriendListWidget()->clear();
    MainWidget::getInstance().friendTreeWidgetClear();
    MessagePack* msg = (MessagePack*)pack->pack_data;
    int size = msg->data_size / sizeof(UserInfo);
    for(int i = 0; i < size; i++) {
        UserInfo* user_info = (UserInfo*)(msg->data + sizeof(UserInfo) * i);
        MainWidget::getInstance().updateFriendListWidget(user_info);
        MainWidget::getInstance().updateFriendTreeWidget(user_info);

    }
}

void TCPClient::handleDeleteFrendRespond(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    if(strcmp(msg->common, DELETE_FRIEND_SUCCEED) == 0) {
        QMessageBox::information(this, "删除好友", "删除成功！");
        MainWidget::getInstance().flushFriendList();
    }
    else QMessageBox::warning(this, "删除好友", "删除错误！");
}

void TCPClient::handlePrivateChatRespond(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    QString name = msg->common;
    QString message = msg->data;
    qDebug() << name;
    qDebug() << msg->data;
    MainWidget::getInstance().getChatMap()[name].setChatName(name);
    MainWidget::getInstance().getChatMap()[name].addMessage(name, msg->data);
    MainWidget::getInstance().updateMessageListWidget(name, message);
}

void TCPClient::handleCreateDirRespond(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    if(strcmp(msg->common, DIRECTORY_EXISTS) == 0) {
        QMessageBox::warning(this, "创建文件夹", "创建失败！文件夹已存在！");
    }
    else if(strcmp(msg->common, CREATE_DIRECTORY_ERROR) == 0) {
        QMessageBox::warning(this, "创建文件夹", "创建失败！系统错误！");
    }
    else if(strcmp(msg->common, CREATE_DIRECTORY_SUCCEED) == 0) {
        QMessageBox::information(this, "创建文件夹", "创建成功！");
        MainWidget::getInstance().flushCurFileList();
    }
}

void TCPClient::handleFlushFileListrequest(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    int file_count = msg->data_size / sizeof(FileInfo);
    std::vector<FileInfo> file_list(file_count);
    for(int i = 0; i < file_count; i++) {
        strcpy(file_list[i].name, ((FileInfo*)msg->data + i)->name);
        file_list[i].is_dir = ((FileInfo*)msg->data + i)->is_dir;
        file_list[i].mtime = ((FileInfo*)msg->data + i)->mtime;
        file_list[i].size = ((FileInfo*)msg->data + i)->size;
    }

    MainWidget::getInstance().updateFileListWidget(file_list);
}

void TCPClient::handleDeleteFileRequest(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    if(strcmp(msg->common, DELETE_DIR_OR_FILE_SUCCEED) == 0) {
        QMessageBox::information(this, "删除文件", "删除成功!");
        MainWidget::getInstance().flushCurFileList();
    }
    else if(strcmp(msg->common, DELETE_DIR_OR_FILE_ERROR) == 0) {
        QMessageBox::warning(this, "删除文件", "删除失败！");
    }
    else {
        QMessageBox::warning(this, "删除文件", "删除失败！文件不存在！");
    }
}

void TCPClient::handleUploadFileRequest(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    unsigned int num = *((unsigned int*)msg->data);
    QString file_name = SendFileList::getInstance().getUploadFileInfo(num).file_name.c_str();
    if(strcmp(msg->common, UPLOAD_FILE_BEGIN) == 0) {
        QMessageBox::information(this, "上传文件", "开始上传");
        if(!SendFileList::getInstance().uploadFile(num)) qDebug() << "上传出错";
    }
    else if (strcmp(msg->common, UPLOAD_FILE_ERROR) == 0) {
        QMessageBox::information(this, "上传文件", "上传出错");
    }
    else if(strcmp(msg->common, UPLOAD_FILE_SUCCEED) == 0) {
        QMessageBox::information(this, "上传文件", "上传成功");
        // 将进度条更新为上传已完成!
        MainWidget::getInstance().updateFileUploadListWidget(file_name, 100);
        MainWidget::getInstance().uploadOver(num);
    }
}

void TCPClient::handleDownloadFileRequest(DataPack *pack)
{

    MessagePack* msg = (MessagePack*)pack->pack_data;
    if(strcmp(msg->common, DOWNLOAD_FILE_BEGIN) == 0) {
        QMessageBox::information(this, "下载文件", "开始下载");
        FilePackInfo* file = (FilePackInfo*)msg->data;
        TransferFileInfo transfer_file_info;
        transfer_file_info.file_name = file->file_name;
        transfer_file_info.transfer_file_path = MainWidget::getInstance().getFileDownloadPath().toStdString().c_str() + std::string("/") + file->file_name;
        transfer_file_info.file_size = file->file_size;
        transfer_file_info.transfer_size = 0;
        transfer_file_info.transfer_status = false;
        transfer_file_info.data_size = file->data_size;
        transfer_file_info.num = file->num;
        transfer_file_info.transfer_num = 0;
        download_file_map[transfer_file_info.num] = transfer_file_info;

        qDebug() << "文件开始下载" << download_file_map[transfer_file_info.num].transfer_file_path;
        qDebug() << "num" << download_file_map[transfer_file_info.num].num;

        //unsigned int val = 0;
        //Write((char*)&val, sizeof(unsigned int));

        std::unique_ptr<DataPack, FreeDeleter> res_pack = makeDataPackMsg(sizeof(FilePackInfo));
        MessagePack* res_msg = (MessagePack*)res_pack->pack_data;
        res_msg->msg_type = MSG_TYPE_DOWNLOAD_FILE_RESPOND;
        memcpy(res_msg->data, file, sizeof(FilePackInfo));
        strcpy(res_msg->common, DOWNLOAD_FILE_BEGIN);
        MainWidget::getInstance().updateFileDonwloadListWidget(transfer_file_info);
        Write((char*)res_pack.get(), res_pack->total_size);

    }
    else if(strcmp(msg->common, DOWNLOAD_FILE_SUCCEED) == 0) {
        QMessageBox::warning(this, "下载文件", "文件下载成功");
    }
    else if(strcmp(msg->common, DOWNLOAD_FILE_ERROR) == 0) {
        QMessageBox::warning(this, "下载文件", "系统错误");
    }
    else {
        QMessageBox::warning(this, "下载文件", "系统错误");
    }
}

void TCPClient::handleShareFileRequest(DataPack *pack)
{
    // 接收文件操作
    MessagePack* msg = (MessagePack*)pack->pack_data;
    QString str = msg->common;
    str += "发来了文件是否接收该文件？";
    int ret = QMessageBox::question(this, tr("分享文件"), str);
    if (ret == QMessageBox::Yes) {
        // 接受文件
        RecvFile::getInstance().setOldPath(msg->data);
        RecvFile::getInstance().show();
    }
}

void TCPClient::handleShareFileRespond(DataPack *pack)
{
    MessagePack* msg = (MessagePack*)pack->pack_data;
    if(strcmp(msg->common, SAVE_FILE_SUCCEED) == 0) {
        QMessageBox::information(this, "保存文件", "保存成功");
        MainWidget::getInstance().flushCurFileList();
    }
    else {
        QMessageBox::warning(this, "保存文件", "保存失败！");
    }
}

void TCPClient::readRecvBuff()
{
    unsigned int total_size = 0;
    if(getBytesAvailable() >= sizeof(total_size)) {
        qint64 bytesReceived = client_socket.peek((char*)(&total_size), sizeof(total_size));  // 读取前4个字节的数据，但不将这些数据从缓冲区中移除
        if (bytesReceived == sizeof(total_size) && getBytesAvailable() >= total_size) // 如果读取成功
        {
            TCPClient::recvMsg();
        }
    }
}


