#ifndef USERINFOWIDGET_H
#define USERINFOWIDGET_H

#include <QWidget>
#include "tcpclient.h"

namespace Ui {
class UserInfoWidget;
}

class UserInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserInfoWidget(QWidget *parent = nullptr);
    ~UserInfoWidget();

    static UserInfoWidget& getInstance();

    void setButtonAttribute();
    // 鼠标悬停
    bool eventFilter(QObject *obj, QEvent *event) override;

    void setWidgetList(const UserInfo& user);

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_mini_clicked();

    void on_pushButton_add_friend_clicked();

private:
    Ui::UserInfoWidget *ui;
};

#endif // USERINFOWIDGET_H
