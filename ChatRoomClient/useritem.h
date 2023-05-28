#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>
#include<QMouseEvent>

namespace Ui {
class Useritem;
}

class Useritem : public QWidget
{
    Q_OBJECT

signals:
    void SIG_ClickUserItem(int );
    void SIG_ClickGroupItem(int );

public:
    explicit Useritem(QWidget *parent = nullptr);
    ~Useritem();
    void slot_setInfo(int id,QString name,QString headimage,int state,QString feeling,QString tel);
    void slot_setGroupInfo(int id,QString name,int identity);

    //添加一个双击实践
    void mouseDoubleClickEvent(QMouseEvent * event);
public:
    QString m_name;
    QString m_headImage;
    int m_state;
    QString m_feeling;
    QString m_tel;
    int m_id;
    //群聊
    int m_identity;
    int m_gid;
private slots:
    void on_pb_headImage_clicked();

private:
    Ui::Useritem *ui;





};

#endif // USERITEM_H
