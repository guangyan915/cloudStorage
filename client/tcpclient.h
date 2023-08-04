#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QHostAddress>
#include <QtConcurrent/QtConcurrent>
#include <QByteArray>
#include "protoclo.h"
#include "register.h"
#include "sendfilelist.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TCPClient; }
QT_END_NAMESPACE

class TCPClient : public QWidget
{
    Q_OBJECT

private:
    TCPClient(QWidget *parent = nullptr);
    ~TCPClient();
    void toLoadConfig();
public:
    static TCPClient& getInstance();

    const UserInfo& getMyselfInfo();
    void setMyselfInfo(const UserInfo& user);
    qint64 getBytesAvailable();
    TransferFileInfo& getDownloadFileInfo(unsigned int num);
    bool removeDownloadFileInfo(unsigned int num);

public slots:
    void connectState();
    void handleRecvMsg();
    static void recvMsg();

private:
    void handleMsgPack(DataPack* pack);
    void handleFilePack(DataPack* pack);
    void handleDownloadFilePack(DataPack* pack);
public:
    int Write(char* ptr, int size);
    int Read(char* ptr, int size);

public:
    void handleLoginRespond(DataPack* pack);
    void handleRegister(DataPack* pack);
    void handleFindUserRespond(DataPack* pack);
    void handleAddFrendRequest(DataPack* pack);
    void handleAddFrendRespond(DataPack* pack);
    void handleFlushFrendRespond(DataPack* pack);
    void handleDeleteFrendRespond(DataPack* pack);
    void handlePrivateChatRespond(DataPack* pack);
    void handleCreateDirRespond(DataPack* pack);
    void handleFlushFileListrequest(DataPack* pack);
    void handleDeleteFileRequest(DataPack* pack);
    void handleUploadFileRequest(DataPack* pack);
    void handleDownloadFileRequest(DataPack* pack);
    void handleShareFileRequest(DataPack* pack);
    void handleShareFileRespond(DataPack* pack);
private slots:
    void readRecvBuff();

private:
    Ui::TCPClient *ui;
    QTcpSocket client_socket;
    QString server_ip;
    quint16 server_port;
    UserInfo myself_info;
    bool setMyself = false;
    QMutex write_mutex;
    QMutex read_mutex;

    std::map<unsigned int , TransferFileInfo> download_file_map;
    QTimer* recv_flush;
    QTimer write_flush;
    std::map<unsigned int , QTimer> write_flush_map;
    //QThreadPool tp;
};
#endif // TCPCLIENT_H
