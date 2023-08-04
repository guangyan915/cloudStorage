#ifndef ADDFRIENDREQUEST_H
#define ADDFRIENDREQUEST_H

#include "protoclo.h"
#include <QWidget>
#include "tcpclient.h"

namespace Ui {
class AddFriendRequest;
}

class AddFriendRequest : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriendRequest(QWidget *parent = nullptr);
    ~AddFriendRequest();

    static AddFriendRequest& getInstance();

    void setButtonAttribute();
    // 鼠标悬停
    bool eventFilter(QObject *obj, QEvent *event) override;

    void setRecvUser(const UserInfo& user);

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_mini_clicked();

    void on_pushButton_close2_clicked();

    void on_pushButton_send_clicked();

private:
    Ui::AddFriendRequest *ui;
    UserInfo recv_user;
};

#endif // ADDFRIENDREQUEST_H
