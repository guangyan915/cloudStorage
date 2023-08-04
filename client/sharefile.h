#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include "mainwidget.h"

namespace Ui {
class ShareFile;
}

class ShareFile : public QWidget
{
    Q_OBJECT

public:
    static ShareFile& getInstance();
    explicit ShareFile(QWidget *parent = nullptr);
    ~ShareFile();

    void setSelectFile(QString file);
    void updateFriend();

private slots:
    void on_pushButton_all_clicked();

    void on_pushButton_cancle_clicked();

    void on_pushButton_NO_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_pushButton_OK_clicked();

private:
    Ui::ShareFile *ui;

    QString select_file;
};

#endif // SHAREFILE_H
