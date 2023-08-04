#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QSizeGrip>
#include <QAbstractButton>
#include "tcpclient.h"
#include "addfriend.h"


MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    setLabel();

    QString basePath = QString(CHAT_MANAGER) + QString(TCPClient::getInstance().getMyselfInfo().name);
    //chatManager = new ChatManager(TCPClient::getInstance().getMyselfInfo().name, basePath);

    setWidgetAttribute();
    setButtonAttribute();

    // 好友分组
    setFriendGroup();

    setTabAttribute();

    // 允许右键动作
    ui->listWidget_friend_request->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget_file->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget_friend->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget_friend_group->setContextMenuPolicy(Qt::CustomContextMenu);
}

MainWidget::~MainWidget()
{
    delete ui;
    //delete chatManager;
}

MainWidget &MainWidget::getInstance()
{
    static MainWidget instance;
    return instance;
}

QString MainWidget::getCurDirPath()
{
    return file_path_cur;
}

QListWidget *MainWidget::getFriendRequestListWidget()
{
    return ui->listWidget_friend_request;
}

QListWidget *MainWidget::getFriendListWidget()
{
    return ui->listWidget_friend;
}

QTreeWidget *MainWidget::getFileTreeWidget()
{
    return ui->treeWidget_file;
}

QListWidget *MainWidget::getMessageListWidget()
{
    return ui->listWidget_message;
}

QString MainWidget::getFileDownloadPath()
{
    return file_download_path;
}

std::map<QString, ChatWidget> &MainWidget::getChatMap()
{
    return chat_map;
}


void MainWidget::friendTreeWidgetClear()
{
    while(onlineItem->childCount() > 0) {
        QTreeWidgetItem* childItem = onlineItem->child(0); // 获取第一个子项
        onlineItem->removeChild(childItem); // 移除该子项
        delete childItem; // 释放内存
    }
    while(offlineItem->childCount() > 0) {
        QTreeWidgetItem* childItem = offlineItem->child(0); // 获取第一个子项
        offlineItem->removeChild(childItem); // 移除该子项
        delete childItem; // 释放内存
    }
}

void MainWidget::updateMessageListWidget(MessagePack *msg_pack)
{
    QListWidgetItem* item = new QListWidgetItem();
    if(msg_pack->msg_type == MSG_TYPE_ADD_FRIEND_RESPOND) {
        QString send_name(msg_pack->common);
        send_name += QString(" 拒绝了你的好友请求");
        item->setText(send_name);
    }

    QListWidget* listWidget = MainWidget::getInstance().getMessageListWidget();
    listWidget->addItem(item);
}

void MainWidget::updateFriendRequestListWidget(MessagePack *msg_pack, int flag)
{
    if(flag == 0) {
        QListWidgetItem* item = new QListWidgetItem();
        QString send_name(msg_pack->common);
        item->setIcon(QIcon(":/add_friend/img/add_friend/addFriendIcon.png"));
        item->setText(send_name);

        QListWidget* listWidget = MainWidget::getInstance().getFriendRequestListWidget();
        listWidget->addItem(item);
    }
    else {

        QList<QListWidgetItem*> itemsToDelete;
        int itemCount = ui->listWidget_friend_request->count();
        QString target(msg_pack->common);
        //qDebug() << "删除：" << target;
        for(int i = 0; i < itemCount; i++) {
            QListWidgetItem* item = ui->listWidget_friend_request->item(i);
            int index = 0;
            QString text = item->text();
            for(; index < text.size(); index++) {
                if(text[index] == ' ' || text[index] == ':') break;
            }
            if(strncmp(text.toStdString().c_str(), target.toStdString().c_str(), index) == 0) itemsToDelete.append(item);
        }

        while(!itemsToDelete.isEmpty()) {
            QListWidgetItem* item = itemsToDelete.takeFirst();
            ui->listWidget_friend_request->takeItem(ui->listWidget_friend_request->row(item));
            delete item;
        }
    }
}



void MainWidget::setFilePathCur(QString path)
{
    file_path_cur = path;
}

void MainWidget::updateFriendListWidget(UserInfo* user_info)
{
    QListWidgetItem* item = new QListWidgetItem();
    if(user_info->online == '1') {
        item->setIcon(QIcon(":/img/offlone.png"));
    }
    else item->setIcon(QIcon(":/img/online.png"));
    item->setText(user_info->name);
    ui->listWidget_friend->addItem(item);
}

void MainWidget::updateFriendTreeWidget(UserInfo *user_info)
{
    // 在线
    QString name(user_info->name);
    if(user_info->online == '1') {
        // 添加在线好友子项
        QTreeWidgetItem *onlineFriend = new QTreeWidgetItem();
        onlineFriend->setIcon(0, QIcon(":/img/offlone.png"));
        onlineFriend->setText(0, name);
        onlineItem->addChild(onlineFriend);
    }
    else {
        // 添加离线好友子项
        QTreeWidgetItem *offlineFriend = new QTreeWidgetItem();
        offlineFriend->setIcon(0, QIcon(":/img/online.png"));
        offlineFriend->setText(0, name);
        offlineItem->addChild(offlineFriend);
    }
}

void MainWidget::updateMessageListWidget(QString name, QString message)
{
    int index = 0;
    for(; index < ui->listWidget_message->count(); index++) {
        QListWidgetItem* item = ui->listWidget_message->item(index);
        QString str = item->text();
        QStringList strList = str.split("\n"); // 使用split函数分割字符串
        QString res = strList[0]; // 获取分割后的第一个元素
        if(res == name) break;
    }
    if(index < ui->listWidget_message->count()) {
        ui->listWidget_message->item(index)->setText(name + '\n' + message);
        ui->listWidget_message->item(index)->setIcon(QIcon(":/img/message.png"));
    }
    else {
        QListWidgetItem* item = new QListWidgetItem();
        item->setIcon(QIcon(":/img/message.png"));
        item->setText(name + '\n' + message);
        ui->listWidget_message->addItem(item);
    }
}

void MainWidget::updateFileListWidget(const std::vector<FileInfo> &file_list)
{
    ui->treeWidget_file->clear();
    for(auto fileInfo : file_list) {
        QTreeWidgetItem *item = new QTreeWidgetItem();    // 获取FileInfo对象
        item->setText(0, fileInfo.name);      // 设置文件名显示文本
        item->setText(3, formatTime(fileInfo.mtime));  // 设置格式化后的时间显示文本
        QString type;
        QString file_name(fileInfo.name);
        int index = file_name.size() - 1;
        while(index >= 0 && file_name[index--] != '.');
        if(index < 0) type = '?';
        else type = file_name.mid(index + 2);
        if(fileInfo.is_dir) {
            item->setIcon(0, QIcon(":/file_type/img/file_type/dir.png"));
        }
        else if(type == "jpg" || type == "JPG" || type == "png" || type == "PNG") {
            item->setIcon(0, QIcon(":/file_type/img/file_type/photo.png"));
        }
        else if(type == "mp3" || type == "MP3") {
            item->setIcon(0, QIcon(":/file_type/img/file_type/MP3.png"));
        }
        else {
            item->setIcon(0, QIcon(":/file_type/img/file_type/what.png"));
        }
        item->setText(2, fileInfo.is_dir ? "目录" : type);  // 设置文件类型显示文本
        if(fileInfo.is_dir) {
            item->setText(1, "???");
        }
        else item->setText(1, formatFileSize(fileInfo.size));     // 设置格式化后的文件大小显示文本
        ui->treeWidget_file->addTopLevelItem(item);
    }
}

void MainWidget::updateFileUploadListWidget(TransferFileInfo &transfer_file_info)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget_upload);
    item->setText(0, transfer_file_info.file_name.c_str());
    item->setText(1, QString::number(transfer_file_info.file_size));
    QProgressBar* progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    //item->setData(2, Qt::UserRole, QVariant::fromValue(progressBar));
    //ui->treeWidget_upload->addTopLevelItem(item);
    // 将控件添加到TreeWidget里
    ui->treeWidget_upload->setItemWidget(item, 2, progressBar);
    QPushButton* button_cancle = new QPushButton("取消", ui->treeWidget_upload);
    QPushButton* button_stop_continue = new QPushButton("暂停", ui->treeWidget_upload);
    // 设置宽度
    button_cancle->setFixedWidth(100);
    button_stop_continue->setFixedWidth(100);
    ui->treeWidget_upload->setItemWidget(item, 3, button_cancle);
    ui->treeWidget_upload->setItemWidget(item, 4, button_stop_continue);
}

void MainWidget::updateFileUploadListWidget(QString file_name, int val)
{
    //qDebug() << "更新进度条！";
    QTreeWidgetItem *item = ui->treeWidget_upload->invisibleRootItem();
    for(int i = 0;i < item->childCount(); i++)
    {
        //对子节点做对应的操作
        QString name  = item->child(i)->text(0);
        if(name == file_name) {
            QWidget* widget = ui->treeWidget_upload->itemWidget(item->child(i), 2);
            QProgressBar* progressBar = qobject_cast<QProgressBar*>(widget);
            // 设置上传进度条的值
            progressBar->setValue(val);
            if(val == 100) {
                // 其他操作
                //qDebug() << "进度条加载完成！";
            }
        }
    }
}


void MainWidget::updateFileDonwloadListWidget(const TransferFileInfo &transfer_file_info)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget_download);
    item->setText(0, transfer_file_info.file_name.c_str());
    item->setText(1, QString::number(transfer_file_info.file_size));
    QProgressBar* progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    //item->setData(2, Qt::UserRole, QVariant::fromValue(progressBar));
    //ui->treeWidget_upload->addTopLevelItem(item);
    // 将控件添加到TreeWidget里
    ui->treeWidget_download->setItemWidget(item, 2, progressBar);
    QPushButton* button_cancle = new QPushButton("取消", ui->treeWidget_download);
    QPushButton* button_stop_continue = new QPushButton("暂停", ui->treeWidget_download);
    // 设置宽度
    button_cancle->setFixedWidth(100);
    button_stop_continue->setFixedWidth(100);
    ui->treeWidget_download->setItemWidget(item, 3, button_cancle);
    ui->treeWidget_download->setItemWidget(item, 4, button_stop_continue);
}


void MainWidget::updateFileDonwloadListWidget(QString file_name, int val)
{
    //qDebug() << "更新进度条！";
    QTreeWidgetItem *item = ui->treeWidget_download->invisibleRootItem();
    for(int i = 0;i < item->childCount(); i++)
    {
        //对子节点做对应的操作
        QString name  = item->child(i)->text(0);
        if(name == file_name) {
            QWidget* widget = ui->treeWidget_download->itemWidget(item->child(i), 2);
            QProgressBar* progressBar = qobject_cast<QProgressBar*>(widget);
            // 设置上传进度条的值
            progressBar->setValue(val);
            if(val == 100) {
                // 其他操作
                //qDebug() << "进度条加载完成！";
            }
        }
    }
}

void MainWidget::uploadOver(int num)
{
    const auto& e = SendFileList::getInstance().getUploadFileInfo(num);
    QString file_name(e.file_name.c_str());
    QTreeWidgetItem *item = ui->treeWidget_upload->invisibleRootItem();
    for(int i = 0;i < item->childCount(); i++)
    {
        //对子节点做对应的操作
        QString name  = item->child(i)->text(0);
        if(name == file_name) {
            ////qDebug() << "移出" << name;
            delete ui->treeWidget_upload->takeTopLevelItem(i);

            // 完成里面添加
            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget_finish);
            item->setText(0, file_name);
            item->setText(1, formatFileSize(e.file_size));
            item->setText(2, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            item->setText(3, "上传");

            transfer_finish_path_map[file_name] = e.transfer_file_path.c_str();
            // 移出相关管理信息
            SendFileList::getInstance().removeUploadFileInfo(num);
        }
    }
}

void MainWidget::downloadOver(int num)
{
    const auto& e = TCPClient::getInstance().getDownloadFileInfo(num);
    QString file_name(e.file_name.c_str());
    QTreeWidgetItem *item = ui->treeWidget_download->invisibleRootItem();
    for(int i = 0;i < item->childCount(); i++)
    {
        //对子节点做对应的操作
        QString name  = item->child(i)->text(0);
        if(name == file_name) {
            ////qDebug() << "移出" << name;
            delete ui->treeWidget_download->takeTopLevelItem(i);

            // 完成里面添加
            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget_finish);
            item->setText(0, file_name);
            item->setText(1, formatFileSize(e.file_size));
            item->setText(2, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            item->setText(3, "下载");

            //transfer_finish_path_map[file_name] = e.transfer_file_path.c_str();
            // 移出相关管理信息
            TCPClient::getInstance().removeDownloadFileInfo(num);
            SendFileList::getInstance().removeUploadFileInfo(num);
        }
    }
}

void MainWidget::returnButton()
{
    on_pushButton_return_clicked();
}

void MainWidget::flushButton()
{
    on_pushButton_flush_file_list_clicked();
}

void MainWidget::doubleButton(QString name)
{
    QTreeWidget* treeWidget = MainWidget::getInstance().getFileTreeWidget();
    QTreeWidgetItem *item = treeWidget->invisibleRootItem();
    for(int i = 0;i < item->childCount(); i++)
    {
       if(item->child(i)->text(0) == name) {
           on_treeWidget_file_itemDoubleClicked(item->child(i), 0);
           break;
       }
    }


}

void MainWidget::setLableUserName(QString name)
{
    ui->label_user_name->setText(name);
}

void MainWidget::flushFriendList()
{
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->msg_type = MSG_TYPE_FLUSH_FRIEND_REQUEST;
    strcpy(msg->common, TCPClient::getInstance().getMyselfInfo().name);
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}

void MainWidget::deleteFriend(QString name)
{
    int ret = QMessageBox::question(this, tr("删除好友"), tr("是否删除该好友？"));
    if (ret == QMessageBox::No) {
        //qDebug() << "不删除";
        return;
    }

    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
    std::string friend_name = name.toStdString();
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->msg_type = MSG_TYPE_DELETE_FRIEND_REQUEST;
    strcpy(msg->common, friend_name.c_str());
    strcpy(msg->common + 32, TCPClient::getInstance().getMyselfInfo().name);
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);

}





void MainWidget::flushCurFileList()
{
    std::string cur_path = file_path_cur.toStdString();
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(cur_path.size() + 1);
    MessagePack* msg_pack = (MessagePack*) pack->pack_data;
    msg_pack->msg_type = MSG_TYPE_FLUSH_FILE_LIST_REQUEST;
    strcpy(msg_pack->data, cur_path.c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}


void MainWidget::openDir(const QString &dir)
{
    if(file_path_cur.size() == 0) file_path_cur += dir;
    else file_path_cur += QString("/") + dir;
    flushCurFileList();
}

void MainWidget::uploadFile(const QString &transfer_file_path)
{
    QFileInfo file_info(transfer_file_path);
    if(file_info.size() > FILE_SIZE_MAX) {
        QMessageBox::warning(this, "上传文件", "单个文件不能大于2GB");
        return;
    }
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(file_path_cur.size() + 1 + sizeof(FilePackInfo));
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->msg_type = MSG_TYPE_UPLOAD_FILE_REQUEST;
    FilePackInfo* file_pack_info = (FilePackInfo*)msg->data;
    file_pack_info->file_size = file_info.size();
    file_pack_info->data_size = SendFileList::getFilePackDataSize(file_pack_info->data_size);
    strcpy(file_pack_info->file_name, file_info.fileName().toStdString().c_str());
    strcpy(file_pack_info->transfer_file_path, file_path_cur.toStdString().c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
    SendFileList::getInstance().addFile(transfer_file_path);
}

void MainWidget::deleteFile(const QString &file)
{
    int ret = QMessageBox::question(this, tr("删除文件"), tr("是否删除该文件？"));
    if (ret == QMessageBox::No) {
        //qDebug() << "不删除";
        return;
    }
    std::string cur_path;
    if(file_path_cur.isEmpty()) cur_path = file_path_cur.toStdString() + file.toStdString();
    else cur_path = QString(file_path_cur + QString("/") + file).toStdString();
    //qDebug() <<cur_path;
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(cur_path.size() + 1);
    MessagePack* msg_pack = (MessagePack*) pack->pack_data;
    msg_pack->msg_type = MSG_TYPE_DELETE_FILE_REQUEST;
    strcpy(msg_pack->data, cur_path.c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}

void MainWidget::downloadFile(const QString file_name)
{
    QDir dir;
    if(dir.exists(file_download_path)) {
        //qDebug() << "下载文件夹存在";
    }
    else {
        if(!dir.mkdir(file_download_path)) qDebug() << "文件夹创建失败！";
    }
    std::string cur_path;
    if(file_path_cur.isEmpty()) cur_path = file_path_cur.toStdString() + file_name.toStdString();
    else cur_path = QString(file_path_cur + QString("/") + file_name).toStdString();
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(cur_path.size() + 1);
    MessagePack* msg_pack = (MessagePack*) pack->pack_data;
    msg_pack->msg_type = MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    strcpy(msg_pack->data, cur_path.c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}


void MainWidget::setWidgetAttribute()
{
    // 去掉窗口边框和标题栏
    setWindowFlag(Qt::FramelessWindowHint);
    //放大缩小
    // 创建 QSizeGrip
    QSizeGrip *sizeGrip = new QSizeGrip(this);
    sizeGrip->setFixedSize(20, 20);
    sizeGrip->move(width() - sizeGrip->width(), height() - sizeGrip->height());


    // 设置应用程序图标
    setWindowIcon(QIcon(":/img/logo.png"));
    // 设置应用程序名称
    QApplication::setApplicationName("栗子的云盘");

    // 设置窗口可以移动
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground); // 设置窗口背景为透明
    setAttribute(Qt::WA_NoSystemBackground, false); // 优化窗口重绘

    // 处理鼠标按下事件
    connect(this, &MainWidget::mousePressed, this, &MainWidget::onMousePressed);
    // 处理鼠标移动事件
    connect(this, &MainWidget::mouseMoved, this, &MainWidget::onMouseMoved);

}

void MainWidget::setButtonAttribute()
{
    // 按键透明度设为零添加Icon
    ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_close->setIcon(QIcon(":/img/close2.png"));
    ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_mini->setIcon(QIcon(":/img/mini2.png"));
    ui->pushButton_m_s->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_m_s->setIcon(QIcon(":/img/magnify2.png"));
    ui->pushButton_set_up->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_set_up->setIcon(QIcon(":/img/set_up2.png"));

    ui->pushButton_friend->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_friend->setIcon(QIcon(":/left/img/friend.png"));
    ui->pushButton_file->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_file->setIcon(QIcon(":/left/img/file.png"));
    ui->pushButton_transmit->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_transmit->setIcon(QIcon(":/left/img/transmit.png"));

    ui->pushButton_upload->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_upload->setIcon(QIcon(":/file_opt/img/file_op/upload.png"));
    ui->pushButton_flush_file_list->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_flush_file_list->setIcon(QIcon(":/file_opt/img/file_op/flush.png"));
    ui->pushButton_return->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_return->setIcon(QIcon(":/file_opt/img/file_op/return.png"));
    ui->pushButton_create_dir->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_create_dir->setIcon(QIcon(":/file_opt/img/file_op/create.png"));
    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->pushButton_m_s->installEventFilter(this);
    ui->pushButton_mini->installEventFilter(this);
    ui->pushButton_set_up->installEventFilter(this);

    ui->pushButton_friend->installEventFilter(this);
    ui->pushButton_file->installEventFilter(this);
    ui->pushButton_transmit->installEventFilter(this);



}

void MainWidget::setFriendGroup()
{
    ui->treeWidget_friend_group->setHeaderLabel("好友分组");
    // 添加在线好友项
    onlineItem = new QTreeWidgetItem();
    onlineItem->setText(0, "在线好友");
    ui->treeWidget_friend_group->addTopLevelItem(onlineItem);

    // 添加离线好友项
    offlineItem = new QTreeWidgetItem();
    offlineItem->setText(0, "离线好友");
    ui->treeWidget_friend_group->addTopLevelItem(offlineItem);
}

void MainWidget::setTabAttribute()
{
    // 选中蓝色 正常灰色
    ui->tabWidget_friend->setAttribute(Qt::WA_StyledBackground);
    ui->tabWidget_friend->setStyleSheet("QTabWidget#tabWidget{background-color:rgb(245,245,245);}\
                                     QTabBar::tab{background-color:rgb(200,200,200);color:rgb(0,0,0);font:8pt '新宋体'}\
                                     QTabBar::tab::selected{background-color:rgb(0,100,200);color:rgb(0,0,0);font:8pt '新宋体'}");

    // 选中蓝色 正常灰色
    ui->tabWidget_transmit->setAttribute(Qt::WA_StyledBackground);
    ui->tabWidget_transmit->setStyleSheet("QTabWidget#tabWidget{background-color:rgb(245,245,245);}\
                                     QTabBar::tab{background-color:rgb(200,200,200);color:rgb(0,0,0);font:8pt '新宋体'}\
                                     QTabBar::tab::selected{background-color:rgb(0,100,200);color:rgb(0,0,0);font:8pt '新宋体'}");


    timer_tab_friend_request = new QTimer(this);
    timer_tab_friend_request->setInterval(500);
    timer_tab_message = new QTimer(this);
    timer_tab_message->setInterval(500);
    // 设置定时器的时间间隔，单位是毫秒

    // 根据标签页获取index
    int index_friend_request = ui->tabWidget_friend->indexOf(ui->tab_friend_request);
    int index_message = ui->tabWidget_friend->indexOf(ui->tab_message);
    if (index_friend_request == -1 || index_message == -1) {
        //qDebug() << "标签索引获取失败";
    }

    connect(timer_tab_friend_request, &QTimer::timeout, this, [=]() {
        QTabBar* tabBar = ui->tabWidget_friend->tabBar();
        static bool isColor1 = true; // 用一个标志变量记录当前的颜色
        if (isColor1) {
            //qDebug() << index_friend_request << "yellow";
            tabBar->setTabTextColor(index_friend_request, QColor(Qt::yellow));
        } else {
            //qDebug() << index_friend_request << "white";
            tabBar->setTabTextColor(index_friend_request, QColor(Qt::white));
        }
        isColor1 = !isColor1;
    });
    connect(timer_tab_message, &QTimer::timeout, this, [=]() {
        QTabBar* tabBar = ui->tabWidget_friend->tabBar();
        static bool isColor2 = true; // 用一个标志变量记录当前的颜色
        if (isColor2) {
            //qDebug() << index_friend_request << "message yellow";
            tabBar->setTabTextColor(index_message, QColor(Qt::yellow));
        } else {
            //qDebug() << index_friend_request << "message white";
            tabBar->setTabTextColor(index_message, QColor(Qt::white));
        }
        isColor2 = !isColor2;
    });
}

// 悬停事件
bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::HoverEnter) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: #ffa02c; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_mini) {
            ui->pushButton_mini->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_m_s) {
            ui->pushButton_m_s->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_set_up) {
            ui->pushButton_set_up->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
    }
    else if (event->type() == QEvent::HoverLeave) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_mini) {
            ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_m_s) {
            ui->pushButton_m_s->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_set_up) {
            ui->pushButton_set_up->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWidget::onMousePressed(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mousePressPosition = event->globalPosition().toPoint() - this->frameGeometry().topLeft();
        event->accept();
    }
}

void MainWidget::onMouseMoved(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - mousePressPosition);
        event->accept();
    }
}



void MainWidget::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed(event);
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoved(event);
}

void MainWidget::on_pushButton_close_clicked()
{
    close();
    QCoreApplication::quit();
}



void MainWidget::on_pushButton_mini_clicked()
{
    //qDebug() << "mini";
    showMinimized();
}


void MainWidget::on_pushButton_m_s_clicked()
{
    if (isMaximized) {
        // 还原
        showNormal();
        isMaximized = false;
        ui->pushButton_m_s->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
        ui->pushButton_m_s->setIcon(QIcon(":/img/magnify2.png"));
    } else {
        // 放大
        showMaximized();
        isMaximized = true;
        ui->pushButton_m_s->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
        ui->pushButton_m_s->setIcon(QIcon(":/img/shrink.png"));
    }
}


void MainWidget::on_pushButton_friend_clicked()
{
    // 切换到第一页
    ui->stackedWidget->setCurrentWidget(ui->page_1);
    flushFriendList();
}


void MainWidget::on_pushButton_file_clicked()
{
    // 切换到第二页
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    // 刷新文件列表
    flushCurFileList();
}


void MainWidget::on_pushButton_transmit_clicked()
{
    // 切换到第三页
    ui->stackedWidget->setCurrentWidget(ui->page_3);
}





void MainWidget::on_pushButton_add_friend_clicked()
{
    AddFriend::getInstance().show();
}

void MainWidget::tabFriendRequestStartDiscolor()
{
    timer_tab_friend_request->start();
    timer_tab_friend_request_run = true;
}

void MainWidget::tabMessageStartDiscolor()
{
    timer_tab_message->start();
    timer_tab_message_run= true;
}

void MainWidget::tabFriendRequestSTopDiscolor()
{
    timer_tab_friend_request->stop();
    timer_tab_friend_request_run = false;
}

void MainWidget::tabMessageStopDiscolor()
{
   timer_tab_message->stop();
   timer_tab_message_run= false;
}

bool MainWidget::tabMesssageTimerIsRun()
{
    return timer_tab_message_run;
}

bool MainWidget::tabFriendRequestTimerIsRun()
{
    return timer_tab_friend_request_run;
}

void MainWidget::setLabel()
{
    // 加载图片
    QPixmap pixmap;
    pixmap.load(":/img/log.png");
    // 调整图片大小以适应标签
    pixmap = pixmap.scaled(ui->label_logo->width(), ui->label_logo->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    // 在标签中显示图像
    ui->label_logo->setPixmap(pixmap);
}


void MainWidget::on_tabWidget_friend_currentChanged(int index)
{
    if(index == ui->tabWidget_friend->indexOf(ui->tab_friend_request)) {
        if(tabFriendRequestTimerIsRun()) tabFriendRequestSTopDiscolor();
    }
    if(index == ui->tabWidget_friend->indexOf(ui->tab_message)) {
        if(tabMesssageTimerIsRun()) tabMessageStopDiscolor();
    }
    // 变回原来的颜色
    ui->tabWidget_friend->setAttribute(Qt::WA_StyledBackground);
    ui->tabWidget_friend->setStyleSheet("QTabWidget#tabWidget{background-color:rgb(245,245,245);}\
                                     QTabBar::tab{background-color:rgb(200,200,200);color:rgb(0,0,0);font:8pt '新宋体'}\
                                     QTabBar::tab::selected{background-color:rgb(0,100,200);color:rgb(0,0,0);font:8pt '新宋体'}");

}





void MainWidget::on_listWidget_friend_request_itemDoubleClicked(QListWidgetItem *item)
{
        //qDebug() << "双击了：" << item->text();
}


void MainWidget::on_listWidget_friend_request_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* item = ui->listWidget_friend_request->itemAt(pos);
        if (item) {
            // 右键
            QMenu *menu = new QMenu(this);
            QAction *acceptAction = new QAction(tr("同意"), this);
            QAction *rejectAction = new QAction(tr("拒绝"), this);
            QAction *viewAction = new QAction(tr("查看好友信息"), this);
            menu->addAction(acceptAction);
            menu->addAction(rejectAction);
            menu->addAction(viewAction);
            // 连接菜单项的信号槽
            connect(viewAction, &QAction::triggered, this, [=](){
                // 处理查看好友信息的操作
                //qDebug() << "查看好友信息";

            });
            connect(acceptAction, &QAction::triggered, this, [=](){
                // 处理添加好友的操作
                //qDebug() << "同意：" << item->text();
                std::unique_ptr<DataPack, FreeDeleter> pack(makeAddFriendPack(0));
                MessagePack* msg_pack = (MessagePack*) pack->pack_data;
                AddFriendPack* add_friend_pack = (AddFriendPack*)msg_pack->data;
                msg_pack->msg_type = MSG_TYPE_ADD_FRIEND_RESPOND;
                QString recv_name = AddFriend::getInstance().getRecvNameMap()[item->text()];
                strcpy(add_friend_pack->send, TCPClient::getInstance().getMyselfInfo().name);
                strcpy(add_friend_pack->recv, recv_name.toStdString().c_str());
                strcpy(msg_pack->common, ADD_FRIEND_RESPOND_ACCEPT);

                TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
                //MainWidget::getInstance().updateFriendRequestListWidget(msg_pack, 1);
                ui->listWidget_friend_request->takeItem(ui->listWidget_friend_request->row(item));
                delete item;
                // 刷新好友列表
                flushFriendList();
            });
            connect(rejectAction, &QAction::triggered, this, [=](){
                // 处理添加好友的操作
                //qDebug() << "拒绝：" << item->text();

                std::unique_ptr<DataPack, FreeDeleter> pack(makeAddFriendPack(0));
                MessagePack* msg_pack = (MessagePack*) pack->pack_data;
                AddFriendPack* add_friend_pack = (AddFriendPack*)msg_pack->data;
                msg_pack->msg_type = MSG_TYPE_ADD_FRIEND_RESPOND;
                QString recv_name = AddFriend::getInstance().getRecvNameMap()[item->text()];
                strcpy(add_friend_pack->send, TCPClient::getInstance().getMyselfInfo().name);
                strcpy(add_friend_pack->recv, recv_name.toStdString().c_str());
                strcpy(msg_pack->common, ADD_FRIEND_RESPOND_REJECTED);

                TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
                ui->listWidget_friend_request->takeItem(ui->listWidget_friend_request->row(item));
                delete item;
                //MainWidget::getInstance().updateFriendRequestListWidget(msg_pack, 1);
                // 刷新好友列表
                flushFriendList();
            });
            menu->exec(ui->listWidget_friend_request->mapToGlobal(pos));
        }
}


void MainWidget::on_pushButton_return_clicked()
{
    if(file_path_cur.isEmpty()) {
        QMessageBox::warning(this, "返回上一级", "首页不能再返回！");
        return;
    }
    while(!file_path_cur.isEmpty() && file_path_cur.back() != '/') file_path_cur.resize(file_path_cur.size() - 1);
    if(!file_path_cur.isEmpty()) file_path_cur.resize(file_path_cur.size() - 1);
    flushCurFileList();
}


void MainWidget::on_pushButton_flush_file_list_clicked()
{
    std::string cur_path = file_path_cur.toStdString();
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(cur_path.size() + 1);
    MessagePack* msg_pack = (MessagePack*) pack->pack_data;
    msg_pack->msg_type = MSG_TYPE_FLUSH_FILE_LIST_REQUEST;
    strcpy(msg_pack->data, cur_path.c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}


void MainWidget::on_pushButton_create_dir_clicked()
{

    QScopedPointer<QInputDialog> input_dialog(new QInputDialog());
    input_dialog->setWindowTitle("创建文件夹");
    input_dialog->setLabelText("请输入文件夹名：");
    input_dialog->setOkButtonText("确定");
    input_dialog->setCancelButtonText("取消");

    if(QDialog::Accepted == input_dialog->exec()) {
        QString text = input_dialog->textValue();
        std::string cur_path;
        if(!file_path_cur.isEmpty()) cur_path = file_path_cur.toStdString() + "/" + text.toStdString();
        else cur_path = text.toStdString();
        if(text.isEmpty()) {
            QMessageBox::warning(this, "创建文件夹", "请输入文件夹名！");
            return;
        }

        // 定义不允许输入的字符，包括 Windows 文件路径中的非法字符
        QString invalidChars("[\\\\/:*?\"<>|]");
        // 定义允许输入的字符集合，包括中英文、数字、下划线、点号和空格
        QString allowedChars("[\\w\\u4E00-\\u9FA5\\s.-]+");

        // 创建正则表达式，匹配不允许出现的字符
        QRegularExpression regExp(invalidChars);
        QRegularExpressionMatch match = regExp.match(text);

        // 检查文件名是否合法，如果不合法则弹出警告框
        if (match.hasMatch()) {
            QMessageBox::warning(nullptr, "创建文件夹", QString("不能含有字符：\n%1").arg(invalidChars));
            return;
        }

        if(text.size() * 2 > 63) {
            QMessageBox::warning(this, "创建文件夹", "文件夹名过长！");
            return;
        }

        //qDebug() << cur_path;
        std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(cur_path.size() + 1);
        MessagePack* msg_pack = (MessagePack*) pack->pack_data;
        msg_pack->msg_type = MSG_TYPE_CREATE_DIR_REQUEST;
        strcpy(msg_pack->data, cur_path.c_str());
        TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
    }
}


void MainWidget::on_pushButton_upload_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
                   this,
                   tr("打开文件"),
                   "/",
                   tr("所有文件 (*.*);;txt (*.txt);;jpg (*.jpg);;pdf (*.pdf)")
                   );

       if (filePath.isEmpty())
       {
           //qDebug() << "未选择文件";
       }
       else
       {
           uploadFile(filePath);
       }
}


void MainWidget::on_treeWidget_file_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(item->text(2) == QString("目录")) {
        // 进入目录
        openDir(item->text(0));
    }
    else {
        // 提示下载该文件
                int ret = QMessageBox::question(this, tr("下载文件"), tr("是否下载该文件？"));
                if (ret == QMessageBox::Yes) {
                    // 下载文件
                    //qDebug() << "下载文件";
                }
                else {
                    //qDebug() << "不下载";
                }
    }
}


void MainWidget::on_treeWidget_file_customContextMenuRequested(const QPoint &pos)
{
    // 获取当前鼠标光标位置的QTreeWidgetItem对象
    QTreeWidgetItem *item = ui->treeWidget_file->itemAt(pos);
    // 如果没有选中任何选项
    if (!item) {
        QMenu menu(this);
        QAction *flushAction = new QAction(tr("刷新"), this);
        QAction *uploadFileAction = new QAction(tr("上传文件"), this);
        QAction *returnAction = new QAction(tr("返回"), this);
        menu.addAction(flushAction);
        menu.addAction(uploadFileAction);
        menu.addAction(returnAction);

        // 弹出右键菜单
        QAction *selectedAction = menu.exec(ui->treeWidget_file->mapToGlobal(pos));
        if (!selectedAction) {
            return;
        }
        // 执行相应的操作
         if (selectedAction == flushAction) {
             flushCurFileList();
         }
         else if(selectedAction == uploadFileAction) {

         }
         else if(selectedAction == returnAction) {
             on_pushButton_return_clicked();
         }
        return;
    }

    QMenu menu(this);
    QAction *openAction = new QAction(tr("打开"), this);
    QAction *deleteAction = new QAction(tr("删除"), this);
    QAction *downloadAction = new QAction(tr("下载"), this);
    QAction *moveAction = new QAction(tr("移动"), this);
    QAction *shareAction = new  QAction(tr("分享"), this);

    // 检查该项是否为目录
    if (item->text(2) == "目录") {
        menu.addAction(openAction);
    }
    else {
        menu.addAction(downloadAction);
    }

    menu.addAction(deleteAction);
    menu.addAction(moveAction);
    menu.addAction(shareAction);

    // 弹出右键菜单
    QAction *selectedAction = menu.exec(ui->treeWidget_file->mapToGlobal(pos));

    if (!selectedAction) {
        return;
    }

    // 执行相应的操作
    if (selectedAction == openAction) {
        // 进入目录操作
        openDir(item->text(0));
    }
    else if (selectedAction == deleteAction) {
        // 删除文件操作
        deleteFile(item->text(0));
    }
    else if (selectedAction == downloadAction) {
        // 下载文件操作
        int ret = QMessageBox::question(this, tr("下载文件"), tr("是否下载该文件？"));
        if (ret == QMessageBox::Yes) {
            downloadFile(item->text(0));
        }
    }
    else if (selectedAction == moveAction) {
        // 移动文件操作
    }
    else if(selectedAction == shareAction) {
        // 分享文件
        if(item->text(0).isEmpty()) {
            QMessageBox::warning(this, "分享文件", "未选择文件");
            return;
        }
        ShareFile::getInstance().setSelectFile(item->text(0));
        ShareFile::getInstance().updateFriend();
        ShareFile::getInstance().show();
    }
}


void MainWidget::on_pushButton_flush_friend_list_clicked()
{
    flushFriendList();
}


void MainWidget::on_treeWidget_friend_group_itemDoubleClicked(QTreeWidgetItem *item, int column)
{

    // 双击事件处理
    if(item == onlineItem || item == offlineItem) return;
    QString name = item->text(0);
    // 在这里处理双击事件，例如打开聊天窗口
    chat_map[name].setChatName(name);
    chat_map[name].show();
}


void MainWidget::on_treeWidget_friend_group_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem* item = ui->treeWidget_friend_group->itemAt(pos);
    if(item == NULL || item == onlineItem || item == offlineItem) {
        return;
    }

    // 右键菜单
    QMenu* menu = new QMenu(this);

    QAction* privateChatAction = new QAction(tr("私聊"), this);
    QAction* infoAction = new QAction(tr("查看信息"), this);
    QAction* deleteFriendAction = new QAction(tr("删除好友"), this);

    menu->addAction(privateChatAction);
    menu->addAction(infoAction);
    menu->addAction(deleteFriendAction);

    // 行为的槽函数，例如删除好友或打开聊天窗口
    connect(deleteFriendAction, &QAction::triggered, [=]() {
        // 在这里处理删除好友操作
        //qDebug() << "删除:" << item->text(0);

        deleteFriend(item->text(0));
    });

    connect(infoAction, &QAction::triggered, [=]() {
        //
        //qDebug() << "查看:" << item->text(0);
    });

    connect(privateChatAction, &QAction::triggered, [=]() {
        // 在这里处理打开私聊窗口操作
        ////qDebug()() << "私聊:" << item->text(0);
    });

    // 显示右键菜单
    menu->exec(ui->treeWidget_friend_group->mapToGlobal(pos));
}


void MainWidget::on_listWidget_message_itemDoubleClicked(QListWidgetItem *item)
{
    if(item->text().isEmpty()) {
        return ;
    }
    QString str = item->text();
    QStringList strList = str.split("\n"); // 使用split函数分割字符串
    QString name = strList[0]; // 获取分割后的第一个元素
    chat_map[name].setChatName(name);
    chat_map[name].show();

}

