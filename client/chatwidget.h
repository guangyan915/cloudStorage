#pragma once
#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include "protoclo.h"

#define MYSELF "myself"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:

    ChatWidget(QWidget *parent = nullptr);
    ChatWidget(QString name);
    ~ChatWidget();
    // 重写事件过滤器函数，实现鼠标悬停时按钮背景颜色变色的效果
    bool eventFilter(QObject* obj, QEvent* event);

    void setChatName(QString name);
    void addMessage(QString name, QString message);
private slots:
    void on_pushButtonclose_clicked();

    void on_pushButton_mini_clicked();

    void on_pushButton_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_send_clicked();

private:
    Ui::ChatWidget *ui;
    QString chat_name;
};

#endif // CHATWIDGET_H
