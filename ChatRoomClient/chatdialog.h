#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>

namespace Ui {
class chatdialog;
}

class chatdialog : public QWidget
{
    Q_OBJECT

signals:
    void SIG_sendContent(int id ,QString saycontent);

    void SIG_sendGroupContent(int id ,QString saycontent);

    void SIG_sendfile(int id ,QString path);
public:
    explicit chatdialog(QWidget *parent = nullptr);
    ~chatdialog();
public:
    void setInfo(int id ,QString name);
    void setGroupInfo(int id ,QString name,int identity);
    void setMessage(QString nowtime, QString saycontent);
    void setGroupMessage(QString nowtime,QString saycontent,QString name,int identity);
    void setISayMessage(QString nowtime, QString saycontent);
    void setISayGroupMessage(QString nowtime, QString saycontent,QString name);

public:
    int m_id;
    int m_gid;
    int m_identity;
    QString m_name;

private slots:
    void on_pb_send_clicked();

    void on_pb_1_clicked();

private:
    Ui::chatdialog *ui;
};

#endif // CHATDIALOG_H
