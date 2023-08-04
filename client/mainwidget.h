#pragma once
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "chatwidget.h"
#include "protoclo.h"
#include "qlistwidget.h"
#include "sharefile.h"
#include "recvfile.h"
#include <QWidget>
#include <QTabWidget>
#include <QTimer>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QRegularExpression>
#include <QProgressBar>
#include <QMap>
#include <map>
#include <QListWidgetItem>

#define CHAT_MANAGER "./chatmanager"
#define FILE_DOWNPATH_PATH "./DownloadFile"
#define FILE_SIZE_MAX 2 * 1024 * 1024 * 1024ll

#define NOT_CONNECT 0
#define MY_COMPUTER 1
#define OTHER_COMPUTER 2

// 定义格式化文件大小函数
inline QString formatFileSize(long long size)
{
    static QString units[] = {"B", "KB", "MB", "GB", "TB"};   // 文件大小单位
    double fileSize = size; // 文件大小
    int unitIdx = 0;        // 文件大小单位的索引
    while (fileSize > 1024.0 && unitIdx < 4) {
        fileSize /= 1024.0; // 转换单位
        ++unitIdx;
    }
    // 使用QString类提供的arg()函数格式化输出字符串
    return QString("%1 %2").arg(fileSize, 0, 'f', 2).arg(units[unitIdx]);
}

// 定义格式化时间函数
inline QString formatTime(time_t mtime)
{
    char timeStr[20] = {0}; // 时间字符串缓冲区
    strftime(timeStr, 20, "%Y-%m-%d %H:%M:%S", localtime(&mtime)); // 将时间格式化为字符串
    return QString(timeStr);
}



namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
    static MainWidget& getInstance();
    QString getCurDirPath();
    QListWidget* getFriendRequestListWidget();
    QListWidget* getFriendListWidget();
    QTreeWidget* getFriendTreeWidget();
    QTreeWidget* getFileTreeWidget();
    QListWidget* getMessageListWidget();
    QString getFileDownloadPath();
    std::map<QString, ChatWidget>& getChatMap();
    void friendTreeWidgetClear();
    void updateMessageListWidget(MessagePack* msg_pack);
    void updateFriendRequestListWidget(MessagePack* msg_pack, int flag = 0);
    //void updateMessageListWidget(Message msg);
    void updateFriendListWidget(UserInfo* user_info);
    void updateFriendTreeWidget(UserInfo* user_info);
    void updateMessageListWidget(QString name, QString message);



    void setFilePathCur(QString path);
    void updateFileListWidget(const std::vector<FileInfo>& file_list);
    void updateFileUploadListWidget(TransferFileInfo& transfer_file_info);
    void updateFileUploadListWidget(QString file_name, int val);
    void updateFileDonwloadListWidget(const TransferFileInfo& transfer_file_info);
    void updateFileDonwloadListWidget(QString file_name, int val);
    void uploadOver(int num);
    void downloadOver(int num);

public:
    void returnButton();
    void flushButton();
    void doubleButton(QString name);

    void setLableUserName(QString name);
    // 刷新好友列表
    void flushFriendList();
    // 删除好友
    void deleteFriend(QString name);
    // 刷新当前路径文件
    void flushCurFileList();
    // 打开目录
    void openDir(const QString &dir);
    // 上传文件
    void uploadFile(const QString& transfer_file_path);
    // 删除文件
    void deleteFile(const QString& file);
    // 下载文件
    void downloadFile(const QString file_name);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // 鼠标悬停
    bool eventFilter(QObject *obj, QEvent *event) override;
signals:
    void mousePressed(QMouseEvent *event);
    void mouseMoved(QMouseEvent *event);

private slots:
    void onMousePressed(QMouseEvent *event); // 声明 onMousePressed 为私有槽函数
    void onMouseMoved(QMouseEvent *event);

private slots:
    void on_pushButton_close_clicked();


    void on_pushButton_mini_clicked();

    void on_pushButton_m_s_clicked();

    void on_pushButton_friend_clicked();

    void on_pushButton_file_clicked();

    void on_pushButton_transmit_clicked();

    void on_pushButton_add_friend_clicked();

    void on_tabWidget_friend_currentChanged(int index);

    void on_listWidget_friend_request_itemDoubleClicked(QListWidgetItem *item);

    void on_listWidget_friend_request_customContextMenuRequested(const QPoint &pos);

    void on_pushButton_return_clicked();

    void on_pushButton_flush_file_list_clicked();

    void on_pushButton_create_dir_clicked();

    void on_pushButton_upload_clicked();

    void on_treeWidget_file_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_file_customContextMenuRequested(const QPoint &pos);

    void on_pushButton_flush_friend_list_clicked();

    void on_treeWidget_friend_group_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_friend_group_customContextMenuRequested(const QPoint &pos);

    void on_listWidget_message_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWidget *ui;

    // 控制标签变色的定时器
    //QTimer* timer_tab_my_friend;
    //QTimer* timer_tab_friend_group;
    QTimer* timer_tab_friend_request;
    QTimer* timer_tab_message;
    bool timer_tab_friend_request_run = false;
    bool timer_tab_message_run = false;

    std::map<QString, ChatWidget> chat_map;

    std::map<QString, QString> transfer_finish_path_map;

private:

private:
    QString file_path_cur;
    QString file_download_path = FILE_DOWNPATH_PATH;

public:
    // 启动定时器，关闭定时器，以及判断定时器是否启动
    void tabFriendRequestStartDiscolor();
    void tabMessageStartDiscolor();
    void tabFriendRequestSTopDiscolor();
    void tabMessageStopDiscolor();
    bool tabMesssageTimerIsRun();
    bool tabFriendRequestTimerIsRun();
private:
    // 设置label
    void setLabel();
    // 设置窗口属性
    void setWidgetAttribute();
    // 设置按钮属性
    void setButtonAttribute();
    // 设置好分组
    void setFriendGroup();
    // 设置tab
    void setTabAttribute();


    QPoint mousePressPosition;
    bool isMaximized;

    //在线好友
    QTreeWidgetItem *onlineItem;
    // 离线好友
    QTreeWidgetItem *offlineItem;
};

#endif // MAINWIDGET_H
