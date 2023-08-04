#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "tcpclient.h"

LoginWidget::LoginWidget(QWidget *parent, QString organization, QString application) :
    QWidget(parent),
    ui(new Ui::LoginWidget),
    settings(organization, application)
{
    ui->setupUi(this);
    // 去掉窗口边框和标题栏
    setWindowFlag(Qt::FramelessWindowHint);
    // 设置应用程序图标
    setWindowIcon(QIcon(":/img/logo.png"));
    // 设置应用程序名称和显示应用名称
    QApplication::setApplicationName("CloudStorageClient");
    QApplication::setApplicationDisplayName("栗子的云盘");

    // 设置左边背景图片
    setBackgroundPhoto();
    // 其他登录提示标签文本居中
    ui->label_other_login->setAlignment(Qt::AlignCenter);
    // comboxBox_user_name 设置提示词以及提示词大小
    lineEdit_user_name = new QLineEdit(ui->comboBox_user_name);
    QFont font;
    font.setPointSize(12);
    lineEdit_user_name->setFont(font);
    lineEdit_user_name->setPlaceholderText("用户名");
    ui->comboBox_user_name->setLineEdit(lineEdit_user_name);
    // 加载已保存用户密码信息
    toLoadUserInfo();
    // 设置按键属性
    setButtonAttribute();
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

LoginWidget &LoginWidget::getInstance()
{
    static LoginWidget instance;
    return instance;
}

void LoginWidget::toLoadUserInfo()
{
    // 将用户名填充到相应的输入框中
    QStringList users = settings.value("users").toStringList();
    for (QString user : users) {
        ui->comboBox_user_name->addItem(user);
    }
}

void LoginWidget::setButtonAttribute()
{
    // 设置边角半径5像素、以及颜色
    ui->pushButton_login->setStyleSheet("background-color: #55aaff;color: white;border-radius: 5px;");

    // 按键透明度设为零添加Icon
    ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_close->setIcon(QIcon(":/img/close_black.png"));
    ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_mini->setIcon(QIcon(":/img/mini.png"));
    ui->pushButton_set_up->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_set_up->setIcon(QIcon(":/img/set_up.png"));
    ui->pushButton_QQ->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_QQ->setIcon(QIcon(":/img/QQ.png"));
    ui->pushButton_wx->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_wx->setIcon(QIcon(":/img/wx.png"));
    ui->pushButton_wb->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_wb->setIcon(QIcon(":/img/wb.png"));

    // 文字颜色设置
    ui->pushButton_register->setStyleSheet("color: grey; border: none;");
    ui->pushButton_forget_passwd->setStyleSheet("color: grey; border: none;");
    ui->pushButton_handof_captche->setStyleSheet("color: grey; border: none;");
    ui->checkBox_remember->setStyleSheet("color: grey; border: none;");

    // 按键只显示文本、透明度为0
    ui->pushButton_forget_passwd->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: grey; border: none;");
    ui->pushButton_register->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: grey; border: none;");
    ui->pushButton_handof_captche->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: grey; border: none;");

    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->pushButton_set_up->installEventFilter(this);
    ui->pushButton_mini->installEventFilter(this);
    ui->pushButton_QQ->installEventFilter(this);
    ui->pushButton_wx->installEventFilter(this);
    ui->pushButton_wb->installEventFilter(this);
    ui->pushButton_handof_captche->installEventFilter(this);
    ui->pushButton_forget_passwd->installEventFilter(this);
    ui->pushButton_register->installEventFilter(this);
    ui->pushButton_login->installEventFilter(this);
    ui->checkBox_remember->installEventFilter(this);
}

bool LoginWidget::eventFilter(QObject *obj, QEvent *event)
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
        else if(obj == ui->pushButton_set_up) {
            ui->pushButton_set_up->setStyleSheet("background-color: #e1e1e1; color: white;");
            return true;
        }
        else if(obj == ui->pushButton_QQ) {
            ui->pushButton_QQ->setStyleSheet("background-color: #d8d8d8; color : white;");
            return true;
        }
        else if(obj == ui->pushButton_wx) {
            ui->pushButton_wx->setStyleSheet("background-color: #d8d8d8; color : white;");
            return true;
        }
        else if(obj == ui->pushButton_wb) {
            ui->pushButton_wb->setStyleSheet("background-color: #d8d8d8; color : white;");
            return true;
        }
        else if(obj == ui->pushButton_register) {
            // 按键不变色，文本变色
            ui->pushButton_register->setStyleSheet("background-color: rgba(0, 0, 0, 0);color: blue;");
            return true;
        }
        else if(obj == ui->pushButton_forget_passwd) {
            ui->pushButton_forget_passwd->setStyleSheet("background-color: rgba(0, 0, 0, 0);color: blue;");
            return true;
        }
        else if(obj == ui->pushButton_handof_captche) {
            ui->pushButton_handof_captche->setStyleSheet("background-color: rgba(0, 0, 0, 0);color: blue;");
            return true;
        }
        else if(obj == ui->pushButton_login) {
            ui->pushButton_login->setStyleSheet("background-color: #009dec;color:white;border-radius: 5px;");
            return true;
        }
        else if(obj == ui->checkBox_remember) {
            ui->checkBox_remember->setStyleSheet("color: blue;");
            return true;
        }
    } else if (event->type() == QEvent::HoverLeave) {
        if (obj == ui->pushButton_close) {
            ui->pushButton_close->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_mini) {
            ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_set_up) {
            ui->pushButton_set_up->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_QQ) {
            ui->pushButton_QQ->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_wx) {
            ui->pushButton_wx->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_wb) {
            ui->pushButton_wb->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
            return true;
        }
        else if (obj == ui->pushButton_register) {
            ui->pushButton_register->setStyleSheet("color: grey; border: none;");
            return true;
        }
        else if (obj == ui->pushButton_forget_passwd) {
            ui->pushButton_forget_passwd->setStyleSheet("color: grey; border: none;");
            return true;
        }
        else if (obj == ui->pushButton_handof_captche) {
            ui->pushButton_handof_captche->setStyleSheet("color: grey; border: none;");
            return true;
        }
        else if(obj == ui->pushButton_login) {
            ui->pushButton_login->setStyleSheet("background-color: #55aaff;color: white;border-radius: 5px;");
            return true;
        }
        else if (obj == ui->checkBox_remember) {
            ui->checkBox_remember->setStyleSheet("color: grey; border: none;");
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void LoginWidget::setBackgroundPhoto(const QString &path)
{
    QPixmap pixmap(path);
    ui->label_left->setPixmap(pixmap);
    ui->label_left->setScaledContents(true);
}

int LoginWidget::getLoginManner()
{
    return login_manner;
}

const QString &LoginWidget::getLoginName()
{
    return login_name;
}

void LoginWidget::setLoginNamePasswd(QString name, QString passwd)
{
    lineEdit_user_name->setText(name);
    ui->lineEdit_passwd->setText(passwd);
    //qDebug() << "name:" << name;
    //qDebug() << "passwd:" << passwd;
}



void LoginWidget::on_pushButton_close_clicked()
{
    close();
}


void LoginWidget::on_pushButton_mini_clicked()
{
    showMinimized();
}


void LoginWidget::on_pushButton_login_clicked()
{
    QString name = ui->comboBox_user_name->currentText();
    QString passwd = ui->lineEdit_passwd->text();

    if(name.isEmpty() || passwd.isEmpty()) {
        QMessageBox::warning(this, "登录", "用户名或密码不能为空！");
        return;
    }

    if(name.size() >= 31) {
        QMessageBox::warning(this, "登录", "用户名过长！");
        return;
    }

    if(passwd.size() >= 31) {
        QMessageBox::warning(this, "登录", "密码过长！");
        return;
    }

    login_name = name;

    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(sizeof(unsigned int));
    MessagePack* msg_pack = (MessagePack*)pack->pack_data;
    msg_pack->msg_type = MSG_TYPE_LOGIN_REQUEST;
    memcpy(msg_pack->common, login_name.toStdString().c_str(), login_name.size() + 1);
    memcpy(msg_pack->common + 32, passwd.toStdString().c_str(), passwd.size() + 1);
    int login_manner = LoginWidget::getInstance().getLoginManner();
    memcpy(msg_pack->data, &login_manner, sizeof(unsigned int));
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
}


void LoginWidget::on_checkBox_remember_stateChanged(int arg1)
{
    if (ui->checkBox_remember->isChecked()) {
           QString user = lineEdit_user_name->text().trimmed();
           QString password = ui->lineEdit_passwd->text();

           // 检查用户名是否合法
           if (user.isEmpty()) {
               return;
           }
           // 加密密码
           //QByteArray encryptedPassword = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

           QStringList users = settings.value("users").toStringList();
           if (!users.contains(user)) {
               users << user;
               settings.setValue("users", users);
           }
           //settings.setValue(user, encryptedPassword.toHex());
           bool exists = false;
           for(auto s : users) {
               if(s != user) exists = true;
           }
           settings.setValue(user, password);
           if(!exists) ui->comboBox_user_name->addItem(user);
       } else {
           QStringList users = settings.value("users").toStringList();
           QString user = lineEdit_user_name->text().trimmed();
           if (users.contains(user)) {
               users.removeAll(user);
               settings.setValue("users", users);
               settings.remove(user);
           }
           lineEdit_user_name->clear();
           ui->lineEdit_passwd->clear();
       }
}

// 当选择用户名时，如果已保存密码，则将密码加载到输入框中
void LoginWidget::on_comboBox_user_name_currentIndexChanged(int index)
{
   QString user_name = ui->comboBox_user_name->currentText();
   QString passwd = settings.value(user_name).toString();
   if(passwd.isEmpty()) return;
   ui->lineEdit_passwd->setText(passwd);
}


void LoginWidget::on_pushButton_handof_captche_clicked()
{
    login_manner = 1;
}


void LoginWidget::on_pushButton_QQ_clicked()
{
    login_manner = 2;
}


void LoginWidget::on_pushButton_wx_clicked()
{
    login_manner = 3;
}


void LoginWidget::on_pushButton_wb_clicked()
{
    login_manner = 4;
}


void LoginWidget::on_pushButton_register_clicked()
{
    Register::getInstance().show();
}

