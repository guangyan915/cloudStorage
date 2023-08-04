#include "sharefile.h"
#include "ui_sharefile.h"
#include <QMessageBox>
#include <vector>
#include "protoclo.h"
#include "tcpclient.h"

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

ShareFile::ShareFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareFile)
{
    ui->setupUi(this);
}

ShareFile::~ShareFile()
{
    delete ui;
}

void ShareFile::setSelectFile(QString file)
{
    select_file = file;
}

void ShareFile::updateFriend()
{
    ui->listWidget->clear();
    MainWidget::getInstance().flushFriendList();

    QListWidget* listWiget = MainWidget::getInstance().getFriendListWidget();
    for(int i = 0; i <listWiget->count(); i++) {
        QListWidgetItem* item = listWiget->item(i)->clone();
        item->setCheckState(Qt::Unchecked);
        ui->listWidget->addItem(item);
    }


}

void ShareFile::on_pushButton_all_clicked()
{
    for(int i = 0; i < ui->listWidget->count(); i++) {
        ui->listWidget->item(i)->setCheckState(Qt::Checked);
    }
}


void ShareFile::on_pushButton_cancle_clicked()
{
    for(int i = 0; i < ui->listWidget->count(); i++) {
        ui->listWidget->item(i)->setCheckState(Qt::Unchecked);
    }
}


void ShareFile::on_pushButton_NO_clicked()
{
    hide();
}


void ShareFile::on_listWidget_itemClicked(QListWidgetItem *item)
{
    if(item->checkState()) {
        item->setCheckState(Qt::Unchecked);
    }
    else item->setCheckState(Qt::Checked);
}


void ShareFile::on_pushButton_OK_clicked()
{
    std::vector<std::string> friend_list;

    for(int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem* item = ui->listWidget->item(i);
        if(item->checkState()) {
            friend_list.push_back(item->text().toStdString());
            qDebug() << friend_list.back();
        }
    }

    if(friend_list.empty()) {
        QMessageBox::warning(this, "分享文件", "未选择好友！");
        return;
    }

    QString path = MainWidget::getInstance().getCurDirPath();
    if(path.isEmpty()) {
        path = QString(TCPClient::getInstance().getMyselfInfo().name) + '/'  + select_file;
    }
    else path = QString(TCPClient::getInstance().getMyselfInfo().name) + '/' + MainWidget::getInstance().getCurDirPath() + '/' + select_file;

    std::string file_path = path.toStdString();
    qDebug() << "path:" << file_path;

    for(auto& e : friend_list) {
        std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPackMsg(file_path.size() +1);
        MessagePack* msg = (MessagePack*)pack->pack_data;
        msg->msg_type = MSG_TYPE_SHARE_FILE_REQUEST;
        strcpy(msg->common, e.c_str());
        memcpy(msg->data, file_path.c_str(), file_path.size());
        TCPClient::getInstance().Write((char*)pack.get(), pack->total_size);
    }
    hide();
    QMessageBox::information(this, "分享文件", "文件分享请求已发送！");
}

