#include "recvfile.h"
#include "ui_recvfile.h"
#include "tcpclient.h"

RecvFile::RecvFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecvFile)
{
    ui->setupUi(this);
}

RecvFile::~RecvFile()
{
    delete ui;
}

RecvFile &RecvFile::getInstance()
{
    static RecvFile instance;
    return instance;
}

void RecvFile::updateFileListWidget()
{
    ui->listWidget->clear();
    QTreeWidget* treeWidget = MainWidget::getInstance().getFileTreeWidget();
    QTreeWidgetItem *item = treeWidget->invisibleRootItem();
    for(int i = 0;i < item->childCount(); i++)
    {
        //对子节点做对应的操作
        item->child(i)->text(0);
        if(item->child(i)->text(2) == "目录") {
            QListWidgetItem* item2 = new QListWidgetItem();
            item2->setText(item->child(i)->text(0));
            item2->setIcon(QIcon(":/file_type/img/file_type/dir.png"));
            ui->listWidget->addItem(item2);
        }
    }
}

void RecvFile::setOldPath(QString path)
{
    old_path = path;
}

void RecvFile::setNewPath(QString path)
{
    new_path = path;
}


void RecvFile::on_pushButton_cancle_clicked()
{
    close();
    MainWidget::getInstance().show();
}


void RecvFile::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    MainWidget::getInstance().doubleButton(item->text());
    MainWidget::getInstance().hide();
    //MainWidget::getInstance().flushButton();
    updateFileListWidget();
}


void RecvFile::on_pushButton_save_clicked()
{
    QString path = MainWidget::getInstance().getCurDirPath();
    if(path.isEmpty()) path = TCPClient::getInstance().getMyselfInfo().name;
    else path = QString(TCPClient::getInstance().getMyselfInfo().name) + '/' + path;
    setNewPath(path);

    std::string send_path = old_path.toStdString();
    std::string recv_path = new_path.toStdString();

    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(send_path.size() + recv_path.size() + 2);
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->msg_type = MSG_TYPE_SHARE_FILE_RESPOND;
    memcpy(msg->data, send_path.c_str(), send_path.size());
    memcpy(msg->data + send_path.size() + 1, recv_path.c_str(), recv_path.size());

    TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);

    MainWidget::getInstance().show();
    close();
}


void RecvFile::on_pushButton_return_clicked()
{
    MainWidget::getInstance().returnButton();
    updateFileListWidget();
    MainWidget::getInstance().hide();
}


void RecvFile::on_pushButton_flush_clicked()
{
    updateFileListWidget();
    MainWidget::getInstance().hide();
}

