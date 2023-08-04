#include "register.h"
#include "ui_register.h"
#include "tcpclient.h"

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    // 去掉窗口边框和标题栏
    setWindowFlag(Qt::FramelessWindowHint);
    // 设置应用程序图标
    setWindowIcon(QIcon(":/img/logo.png"));
    // 设置应用程序名称和显示应用名称

    // 按键透明度设为零添加Icon
    ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_close->setIcon(QIcon(":/img/close_black.png"));

    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->register_pushButton->installEventFilter(this);

    // 设置边角半径5像素、以及颜色
    ui->register_pushButton->setStyleSheet("background-color: #55aaff;color: white;border-radius: 5px;");

}

Register::~Register()
{
    delete ui;
}

bool Register::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::HoverEnter) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: #ffa02c; color: white;");
            return true;
        }
        else if(obj == ui->register_pushButton) {
            ui->register_pushButton->setStyleSheet("background-color: #009dec;color:white;border-radius: 5px;");
            return true;
        }
    }
    else if (event->type() == QEvent::HoverLeave) {
            if (obj == ui->pushButton_close) {
                ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
                return true;
            }
            else if(obj == ui->register_pushButton) {
                ui->register_pushButton->setStyleSheet("background-color: #55aaff;color: white;border-radius: 5px;");
                return true;
            }
    }
}

Register &Register::getInstance()
{
    static Register instance;
    return instance;
}

void Register::on_register_pushButton_clicked()
{
    QString name = ui->name_lineEdit->text();
    QString passwd1 = ui->passwd1_lineEdit->text();
    QString passwd2 = ui->passwd2_lineEdit->text();

    if(name.isEmpty()) {
        QMessageBox::warning(this, "注册", "用户名不能为空！");
        return;
    }
    if(name.size() >= 31) {
        QMessageBox::warning(this, "注册", "用户名过长！");
        return;
    }

    if(passwd1.isEmpty()) {
        QMessageBox::warning(this, "注册", "密码不能为空！");
        return;
    }
    if(passwd1.size() >= 31) {
        QMessageBox::warning(this, "注册", "密码过长！");
        return;
    }
    if(passwd2.isEmpty()) {
        QMessageBox::warning(this, "注册", "确认密码不能为空！");
        return;
    }

    if(passwd1 != passwd2) {
        QMessageBox::warning(this, "注册", "请您检查那输入的密码，两次输入的密码不一样！");
        return;
    }

    user_name = name;
    user_passwd = passwd1;
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(0);
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    strcpy(msg_pack->common, name.toStdString().c_str());
    strcpy(msg_pack->common + 32, passwd1.toStdString().c_str());
    msg_pack->msg_type = MSG_TYPE_REGISTER_REQUEST;

    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);

}


void Register::on_pushButton_close_clicked()
{
    close();
}

