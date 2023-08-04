#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>

namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();


    // 重写事件过滤器函数，实现鼠标悬停时按钮背景颜色变色的效果
    bool eventFilter(QObject* obj, QEvent* event);

    static Register& getInstance();
    QString user_name;
    QString user_passwd;
private slots:
    void on_register_pushButton_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::Register *ui;

};

#endif // REGISTER_H
