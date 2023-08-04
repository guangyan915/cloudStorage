#ifndef ADDFRIEND_H
#define ADDFRIEND_H

#include "protoclo.h"
#include <QWidget>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QMenu>
#include "mainwidget.h"

namespace Ui {
class AddFriend;
}

class AddFriend : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriend(QWidget *parent = nullptr);
    ~AddFriend();
    static AddFriend& getInstance();

    void setButtonAttribute();
    // 鼠标悬停
    bool eventFilter(QObject *obj, QEvent *event) override;

    void updateUserListWidget(const std::vector<UserInfo>& ret);

    void updateFriendRequest(MessagePack* msg);

    std::map<QString, QString>& getRecvNameMap();



private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_mini_clicked();

    void on_pushButton_find_clicked();

    void on_checkBox_id_stateChanged(int arg1);

    void on_checkBox_name_stateChanged(int arg1);

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

private:
    Ui::AddFriend *ui;
    std::map<unsigned int, UserInfo> user_info_map;

    std::map<QString, QString> add_friend_request_map;
};

#endif // ADDFRIEND_H
