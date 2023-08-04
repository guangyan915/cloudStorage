#include "addfriendrequest.h"
#include "ui_addfriendrequest.h"


AddFriendRequest::AddFriendRequest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriendRequest)
{
    ui->setupUi(this);
    // 去掉窗口边框和标题栏
    setWindowFlag(Qt::FramelessWindowHint);
    setButtonAttribute();
}

AddFriendRequest::~AddFriendRequest()
{
    delete ui;
}

AddFriendRequest &AddFriendRequest::getInstance()
{
    static AddFriendRequest instance;
    return instance;
}

void AddFriendRequest::setButtonAttribute()
{
    // 按键透明度设为零添加Icon
    ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_close->setIcon(QIcon(":/img/close2.png"));
    ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_mini->setIcon(QIcon(":/img/mini2.png"));
    ui->pushButton_send->setStyleSheet("background-color: rgb(244,244,244); color: black; border: none;border-radius: 5px;");
    ui->pushButton_close2->setStyleSheet("background-color: rgb(244,244,244); color: black; border: none;border-radius: 5px;");

    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->pushButton_mini->installEventFilter(this);
    ui->pushButton_send->installEventFilter(this);
    ui->pushButton_close2->installEventFilter(this);
}

bool AddFriendRequest::eventFilter(QObject *obj, QEvent *event)
{    if (event->type() == QEvent::HoverEnter) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: #ffa02c; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_mini) {
            ui->pushButton_mini->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_send) {
            ui->pushButton_send->setStyleSheet("background-color: rgb(85,170,255); color: white;border-radius: 5px;");
            return true;
        }
        else if(obj == ui->pushButton_close2) {
            ui->pushButton_close2->setStyleSheet("background-color: rgb(85,170,255); color: white;border-radius: 5px;");
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
        else if(obj == ui->pushButton_send) {
            ui->pushButton_send->setStyleSheet("background-color: rgb(244,244,244); color: black; border: none;border-radius: 5px;");
            return true;
        }
        else if(obj == ui->pushButton_close2) {
            ui->pushButton_close2->setStyleSheet("background-color: rgb(244,244,244); color: black; border: none;border-radius: 5px;");
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void AddFriendRequest::setRecvUser(const UserInfo &user)
{
    recv_user = user;
}


void AddFriendRequest::on_pushButton_close_clicked()
{
    close();
}


void AddFriendRequest::on_pushButton_mini_clicked()
{
    showMinimized();
}


void AddFriendRequest::on_pushButton_close2_clicked()
{
    close();
}


void AddFriendRequest::on_pushButton_send_clicked()
{
    // 发送好友请求
    // 获取验证信息

    std::string text = ui->textEdit->toPlainText().toStdString();
    std::unique_ptr<DataPack, FreeDeleter> pack;
    MessagePack* msg;
    AddFriendPack* msg_data;
    if(text.empty()) {
        // 释放原来对象，设置新的对象
        pack.reset(makeAddFriendPack(0).release());
        msg = (MessagePack*)(pack->pack_data);
        msg_data = (AddFriendPack*)msg->data;
    }
    else {
        pack.reset(makeAddFriendPack(text.size() + 1).release());
        msg = (MessagePack*)(pack->pack_data);
        msg_data = (AddFriendPack*)msg->data;
        memcpy(msg_data->data, text.c_str(), text.size() +1);
    }

    msg->msg_type = MSG_TYPE_ADD_FRIEND_REQUEST;
    // 拷贝发送请求方
    strcpy(msg_data->send, TCPClient::getInstance().getMyselfInfo().name);
    // 接受方
    strcpy(msg_data->recv, recv_user.name);
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}


