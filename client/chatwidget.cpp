#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "tcpclient.h"


ChatWidget::ChatWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWidget)
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
    ui->pushButton_mini->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: black; border: none;");
    ui->pushButton_mini->setIcon(QIcon(":/img/mini.png"));

    // 为按钮添加鼠标悬停事件监听器
    ui->pushButton_close->installEventFilter(this);
    ui->pushButton_mini->installEventFilter(this);
    ui->pushButton_send->installEventFilter(this);
    // 设置边角半径5像素、以及颜色
    ui->pushButton_send->setStyleSheet("background-color: #55aaff;color: white;border-radius: 5px;");

    QString name = chat_name;
    ui->label->setText(name);

    // 创建 QFont 对象并设置字体大小
    QFont font;
    font.setPointSize(21); // 设置字体大小为21
    // 设置字体颜色
    QColor fontColor(Qt::white); // 设置字体颜色为黑色
    // 设置背景颜色
    QColor backgroundColor(Qt::blue); // 设置背景颜色为绿色
    // 设置 QLabel 的字体、字体颜色和背景颜色
    ui->label->setFont(font);
    //ui->label->setStyleSheet("color: " + fontColor.name() + "; background-color: " + backgroundColor.name());

}

ChatWidget::ChatWidget(QString name)
{
    chat_name = name;
    ui->label->setText(name);

    // 创建 QFont 对象并设置字体大小
    QFont font;
    font.setPointSize(21); // 设置字体大小为21
    // 设置字体颜色
    QColor fontColor(Qt::white); // 设置字体颜色为黑色
    // 设置背景颜色
    QColor backgroundColor(Qt::blue); // 设置背景颜色为绿色
    // 设置 QLabel 的字体、字体颜色和背景颜色
    ui->label->setFont(font);
    //ui->label->setStyleSheet("color: " + fontColor.name() + "; background-color: " + backgroundColor.name());
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

bool ChatWidget::eventFilter(QObject *obj, QEvent *event)
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

        else if(obj == ui->pushButton_send) {
            ui->pushButton_send->setStyleSheet("background-color: #009dec;color:white;border-radius: 5px;");
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
        else if(obj == ui->pushButton_send) {
            ui->pushButton_send->setStyleSheet("background-color: #55aaff;color: white;border-radius: 5px;");
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void ChatWidget::setChatName(QString name)
{
    chat_name = name;
    ui->label->setText(chat_name);
}

void ChatWidget::addMessage(QString name, QString message)
{
    QListWidgetItem* item = new QListWidgetItem();
    QString text = name + '\n' + message;
    item->setText(text);

    // 创建 QPalette 对象
       QPalette palette;


       // 设置字体大小
       QFont font;
       font.setPointSize(12); // 设置字体大小为12
       item->setFont(font);
       // 设置文本颜色为黑色
       palette.setColor(QPalette::Text, Qt::black);
       item->setForeground(palette.color(QPalette::Text));

       // 设置背景色为白色
       palette.setColor(QPalette::Base, Qt::white);
       item->setBackground(palette.color(QPalette::Base));
    if(name != chat_name) {
        // 设置文本颜色为黑色
        palette.setColor(QPalette::Text, Qt::black);
        item->setForeground(palette.color(QPalette::Text));

        // 设置背景色为绿色
        palette.setColor(QPalette::Base, Qt::green);
        item->setBackground(palette.color(QPalette::Base));

        item->setTextAlignment(Qt::AlignRight); // 设置文本对齐方式为右对齐
    }

    ui->listWidget->addItem(item);
}


void ChatWidget::on_pushButtonclose_clicked()
{
    close();
}


void ChatWidget::on_pushButton_mini_clicked()
{
    showMinimized();
}


void ChatWidget::on_pushButton_clicked()
{
    std::string text = ui->textEdit->toPlainText().toStdString();
    if(text.empty()) {
        QMessageBox::warning(this, "聊天", "请输入聊天信息！");
        return;
    }

    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(text.size() + 1);
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->msg_type = MSG_TYPE_PRIVATE_CHAT_REQUEST;
    strcpy(msg->common, TCPClient::getInstance().getMyselfInfo().name);
    strcpy(msg->common + 32, chat_name.toStdString().c_str());
    strcpy(msg->data, text.c_str());
    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);

    ui->textEdit->clear();

    addMessage(TCPClient::getInstance().getMyselfInfo().name, text.c_str());

}


void ChatWidget::on_pushButton_close_clicked()
{
    close();
}


void ChatWidget::on_pushButton_send_clicked()
{
    on_pushButton_clicked();
}

