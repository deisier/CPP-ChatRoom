#ifndef MYCHAT_H
#define MYCHAT_H

#include <QMainWindow>
#include"useritem.h"
#include<QVBoxLayout>
#include<QMenu>
namespace Ui {
class MyChat;
}

class MyChat : public QMainWindow
{
    Q_OBJECT

signals:
    void SIG_AddFriendBytel(QString);
    void SIG_DelFriendBytel(QString);
    void SIG_CreateGroup(QString);
    void SIG_AddGroup(QString);
    void SIG_DelGroup(QString);

public:
    explicit MyChat(QWidget *parent = nullptr);
    ~MyChat();

public:
    void slot_addFriend(Useritem * item);
    void slot_addGroup(Useritem * item);
    void setInfo();

public slots:
    void slot_dealMenu(QAction * action);

public:
    QString m_name;
    QString m_feeling;
    QString m_icon;
    QString m_tel;
    QMenu * m_menu;
    QVBoxLayout * m_layout;
    QVBoxLayout * m_glayout;

private slots:
    void on_pb_menu_clicked();

private:
    Ui::MyChat *ui;




};

#endif // MYCHAT_H
