#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QLineEdit>
#include <QCryptographicHash>
#include "tcpclient.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr, QString organization = "lgy", QString application = "CloudStorageClient");
    ~LoginWidget();

    static LoginWidget& getInstance();

    // 加载以保存的用户
    void toLoadUserInfo();


    // 设置按键属性等
    void setButtonAttribute();

    // 重写事件过滤器函数，实现鼠标悬停时按钮背景颜色变色的效果
    bool eventFilter(QObject* obj, QEvent* event);

    void setBackgroundPhoto(const QString& path = ":/img/xk.jpg");

    int getLoginManner();
    const QString& getLoginName();

    // 将账号密码设置进来
    void setLoginNamePasswd(QString name, QString passwd);

private slots:

    void on_pushButton_close_clicked();

    void on_pushButton_mini_clicked();

    void on_pushButton_login_clicked();

    void on_checkBox_remember_stateChanged(int arg1);

    void on_comboBox_user_name_currentIndexChanged(int index);

    void on_pushButton_handof_captche_clicked();

    void on_pushButton_QQ_clicked();

    void on_pushButton_wx_clicked();

    void on_pushButton_wb_clicked();

    void on_pushButton_register_clicked();

private:
    Ui::LoginWidget *ui;
    QLineEdit *lineEdit_user_name;
    // 创建QSettings对象，指定保存的组名和应用程序名称
    QSettings settings;

    // 登录方式：
    // 0 用户名登录
    // 1 验证码登录
    // 2 qq 登录
    // 3 微信登录
    // 4 微博登录
    int login_manner = 0;
    QString login_name;

};

#endif // LOGINWIDGET_H
