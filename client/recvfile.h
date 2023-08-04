#ifndef RECVFILE_H
#define RECVFILE_H

#include <QWidget>
#include "mainwidget.h"
#include <QTimer>
namespace Ui {
class RecvFile;
}

class RecvFile : public QWidget
{
    Q_OBJECT

public:
    explicit RecvFile(QWidget *parent = nullptr);
    ~RecvFile();
    static RecvFile& getInstance();
    void updateFileListWidget();
    void setOldPath(QString path);
    void setNewPath(QString path);
private slots:


    void on_pushButton_cancle_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_save_clicked();

    void on_pushButton_return_clicked();

    void on_pushButton_flush_clicked();

private:
    Ui::RecvFile *ui;
    QString old_path;
    QString new_path;
};

#endif // RECVFILE_H
