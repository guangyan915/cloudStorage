#include "addfriend.h"
#include "addfriendrequest.h"
#include "qlistwidget.h"
#include "ui_addfriend.h"
#include "tcpclient.h"
#include "userinfowidget.h"

AddFriend::AddFriend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);
    // 去掉窗口边框和标题栏
    setWindowFlag(Qt::FramelessWindowHint);
    setButtonAttribute();
    // 设置右键菜单
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

}

AddFriend::~AddFriend()
{
    delete ui;
}

AddFriend &AddFriend::getInstance()
{
    static AddFriend instance;
    return instance;
}

void AddFriend::setButtonAttribute()
{
    // 按键透明度设为零添加Icon
    ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_close->setIcon(QIcon(":/img/close2.png"));
    ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_mini->setIcon(QIcon(":/img/mini2.png"));
    ui->pushButton_find->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_find->setIcon(QIcon(":/add_friend/img/add_friend/find.png"));

    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->pushButton_mini->installEventFilter(this);
    ui->pushButton_find->installEventFilter(this);

}

bool AddFriend::eventFilter(QObject *obj, QEvent *event)
{    if (event->type() == QEvent::HoverEnter) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: #ffa02c; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_mini) {
            ui->pushButton_mini->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_find) {
            ui->pushButton_find->setStyleSheet("background-color: #e1e1e1; color: white;");
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
        else if(obj == ui->pushButton_find) {
            ui->pushButton_find->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
    }
    return QObject::eventFilter(obj, event);

}

void AddFriend::updateUserListWidget(const std::vector<UserInfo> &ret)
{
    ui->treeWidget->clear();
    // 将数据存储到user_ret里
    for(auto user : ret) {
        // 信息存入map里
        user_info_map[user.id] = user;
        QTreeWidgetItem* item = new QTreeWidgetItem(); // 创建一个TreeWidget的item实例
        item->setText(0, QString::number(user.id)); // 设置第一列的文本
        item->setText(1, QString(user.name)); // 设置第二列的文本
        //qDebug() << "在线转态：" << user.online;
        if(user.online == '1') {
            item->setText(2, "在线"); // 设置第三列的文本
        }
        else {
            item->setText(2, "不在线"); // 设置第三列的文本
        }
        ui->treeWidget->addTopLevelItem(item); // 添加到TreeWidget中
    }
}

void AddFriend::updateFriendRequest(MessagePack *msg)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setIcon(QIcon(":/add_friend/img/add_friend/addFriendIcon.png"));
    QString request_text;
    AddFriendPack* add_friend_pack = (AddFriendPack*)msg->data;
    if(msg->data_size > sizeof(AddFriendPack)) request_text = QString(add_friend_pack->data);
    QString send_name(add_friend_pack->send);
    if(request_text.isEmpty()) request_text = QString("%1 发来了好友申请").arg(send_name);
    else request_text = send_name + QString(": ") + request_text;
    item->setText(request_text);
    add_friend_request_map[item->text()] = send_name;

    QListWidget* listWidget = MainWidget::getInstance().getFriendRequestListWidget();
    listWidget->addItem(item);

}

std::map<QString, QString> &AddFriend::getRecvNameMap()
{
    return add_friend_request_map;
}



void AddFriend::on_pushButton_close_clicked()
{
    close();
}


void AddFriend::on_pushButton_mini_clicked()
{
    showMinimized();
}


void AddFriend::on_pushButton_find_clicked()
{
    if(!ui->checkBox_id->isChecked() && !ui->checkBox_name->isChecked()) {
        QMessageBox::warning(this, "查找好友", "请选择条件！");
        return;
    }

    QString user = ui->lineEdit_people->text();
    if(user.isEmpty()) {
        QMessageBox::warning(this, "查找好友", "请输入好友信息！");
        return;
    }

    if(ui->checkBox_id->isChecked()) {
        for(auto c : ui->lineEdit_people->text()) {
            if(c < '0' || c >= '9') {
                QMessageBox::warning(this, "查找好友", "ID不能含有非数字");
                return;
            }
        }
    }

    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    msg_pack->msg_type = MSG_TYPE_FIND_USER_REQUEST;
    if(ui->checkBox_id->isChecked()) {
        strcpy(msg_pack->common, FIIND_CRITERIA_ID);
    }
    else {
        strcpy(msg_pack->common, FIIND_CRITERIA_NAME);
    }
    strcpy(msg_pack->common + 32, user.toStdString().c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}


void AddFriend::on_checkBox_id_stateChanged(int arg1)
{
    if(ui->checkBox_id->isChecked() && ui->checkBox_name->isChecked()) {
        ui->checkBox_name->setChecked(false);
    }
}


void AddFriend::on_checkBox_name_stateChanged(int arg1)
{
    if(ui->checkBox_id->isChecked() && ui->checkBox_name->isChecked()) {
        ui->checkBox_id->setChecked(false);
    }
}


void AddFriend::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    // 处理双击事件
    UserInfoWidget::getInstance().setWidgetList(user_info_map[item->text(0).toUInt()]);
    UserInfoWidget::getInstance().show();
    //qDebug() << "双击了：" << item->text(0);
}


void AddFriend::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem* item = ui->treeWidget->itemAt(pos);
        if (item) {
            // 创建菜单
            //打开右键菜单属性
            ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
            QMenu menu(ui->tabWidget);
            QAction* viewAction = new QAction("查看好友信息", &menu);
            QAction* addAction = new QAction("添加好友", &menu);
            menu.addAction(viewAction);
            menu.addAction(addAction);

            // 连接菜单项的信号槽
            connect(viewAction, &QAction::triggered, this, [=](){
                // 处理查看好友信息的操作
                UserInfoWidget::getInstance().setWidgetList(user_info_map[item->text(0).toUInt()]);
                UserInfoWidget::getInstance().show();
            });
            connect(addAction, &QAction::triggered, this, [=](){
                // 处理添加好友的操作
                //qDebug() << "添加好友：" << item->text(0);
                AddFriendRequest::getInstance().setRecvUser(user_info_map[item->text(0).toUInt()]);
                AddFriendRequest::getInstance().show();
            });

            // 显示菜单
            menu.exec(ui->treeWidget->mapToGlobal(pos));
        }
}

