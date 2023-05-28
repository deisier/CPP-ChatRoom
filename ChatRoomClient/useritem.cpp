#include "useritem.h"
#include "ui_useritem.h"
#include<QBitmap>
#include<QIcon>
#include"qdebug.h"

Useritem::Useritem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Useritem)
{
    m_id = 0;
    ui->setupUi(this);
}

Useritem::~Useritem()
{
    delete ui;
}

void Useritem::slot_setInfo(int id, QString name, QString headimage, int state, QString feeling, QString tel)
{
    m_id = id;
    m_name = name;
    m_headImage = headimage;
    m_state = state;
    m_feeling = feeling;
    m_tel = tel;

    QString path = ":/face/" + headimage;
    qDebug()<<path;
    //在线
    if(m_state == 1){
        //设置头像彩色
        ui->pb_headImage->setIcon(QIcon(path));
    //离线状态
    }else{
        //设置头像灰色
        QBitmap bmp;
        bmp.load(path);

        ui->pb_headImage->setIcon(bmp);
    }
    //设置用户名
    ui->lb_name->setText(m_name);
    //设置个性签名
    ui->lb_feeling->setText(m_feeling);

    //控件重绘
    this->repaint();

}

void Useritem::slot_setGroupInfo(int id, QString name, int identity)
{
    //btn_avatar_a10.png
    m_gid = id;
    m_name = name;
    m_identity= identity;
    QString path = ":/face/btn_avatar_a19.png";
    qDebug()<<path;
    //设置头像
    ui->pb_headImage->setIcon(QIcon(path));

    //设置用户名
    ui->lb_name->setText(m_name);
    //设置个性签名
    ui->lb_feeling->setText("。。。");

    //控件重绘
    this->repaint();
}

void Useritem::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(m_id != 0)
        Q_EMIT SIG_ClickUserItem(m_id);
    else
        Q_EMIT SIG_ClickGroupItem(m_gid);
}

void Useritem::on_pb_headImage_clicked()
{
    if(m_id != 0)
        Q_EMIT SIG_ClickUserItem(m_id);
    else
        Q_EMIT SIG_ClickGroupItem(m_gid);

}

