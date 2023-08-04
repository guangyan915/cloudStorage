#include "userinfowidget.h"
#include "ui_userinfowidget.h"
#include "addfriendrequest.h"

UserInfoWidget::UserInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserInfoWidget)
{
    ui->setupUi(this);
    // 去掉窗口边框和标题栏
    setWindowFlag(Qt::FramelessWindowHint);
    setButtonAttribute();
}

UserInfoWidget::~UserInfoWidget()
{
    delete ui;
}

UserInfoWidget &UserInfoWidget::getInstance()
{
    static UserInfoWidget instance;
    return instance;
}


void UserInfoWidget::setButtonAttribute()
{
    // 按键透明度设为零添加Icon
    ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_close->setIcon(QIcon(":/img/close2.png"));
    ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_mini->setIcon(QIcon(":/img/mini2.png"));
    ui->pushButton_add_friend->setStyleSheet("background-color: rgb(143,192,231); color: black; border: none;border-radius: 5px;");

    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->pushButton_mini->installEventFilter(this);
    ui->pushButton_add_friend->installEventFilter(this);

}

bool UserInfoWidget::eventFilter(QObject *obj, QEvent *event)
{    if (event->type() == QEvent::HoverEnter) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: #ffa02c; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_mini) {
            ui->pushButton_mini->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_add_friend) {
            ui->pushButton_add_friend->setStyleSheet("background-color: rgb(85,170,255); color: white;border-radius: 5px;");
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
        else if(obj == ui->pushButton_add_friend) {
            ui->pushButton_add_friend->setStyleSheet("background-color: rgb(143,192,231); color: black; border: none;border-radius: 5px;");
            return true;
        }
    }
    return QObject::eventFilter(obj, event);

}

void UserInfoWidget::setWidgetList(const UserInfo &user)
{
    // 设置要发送的对象
    AddFriendRequest::getInstance().setRecvUser(user);
    ui->listWidget_user_info->clear();
    QListWidgetItem* item_id = new QListWidgetItem();
    QListWidgetItem* item_name = new QListWidgetItem();
    QListWidgetItem* item_online = new QListWidgetItem();
    item_id->setText(QString(QString("用户ID：") + QString::number(user.id)));
    ui->listWidget_user_info->addItem(item_id);
    item_name->setText(QString(QString("用户名：") + QString(user.name)));
    ui->listWidget_user_info->addItem(item_name);
    if(user.online == 1) {
        item_online->setText("状态：在线");
    }
    else {
        item_online->setText("状态：不在线");
    }
    ui->listWidget_user_info->addItem(item_online);
}



void UserInfoWidget::on_pushButton_close_clicked()
{
    close();
}


void UserInfoWidget::on_pushButton_mini_clicked()
{
    showMinimized();
}


void UserInfoWidget::on_pushButton_add_friend_clicked()
{
    AddFriendRequest::getInstance().show();
}

