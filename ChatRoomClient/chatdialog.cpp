#include "chatdialog.h"
#include "ui_chatdialog.h"
#include<QDateTime>
#include<QDebug>

chatdialog::chatdialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chatdialog)
{
    m_id = 0;
    ui->setupUi(this);
}

chatdialog::~chatdialog()
{
    delete ui;
}

void chatdialog::setInfo(int id, QString name)
{
    m_id = id;
    m_name = name;
    //设置左上角标题
    this->setWindowTitle(QString("与【%1】的聊天室").arg(name));

}

void chatdialog::setGroupInfo(int id, QString name, int identity)
{
    m_gid = id;
    m_name = name;
    m_identity = identity;
    //设置左上角标题
    this->setWindowTitle(QString("【%1】的群聊聊天室 群id【%2】").arg(name).arg(m_gid));
}

void chatdialog::setGroupMessage(QString nowtime,QString saycontent,QString name,int identity)
{
    if(identity == 1){
        ui->tb_chat->append("【群主】"+name + "：" + nowtime);
    }else{
        ui->tb_chat->append("【成员】"+name + "：" + nowtime);
    }

    ui->tb_chat->append(saycontent);
}
void chatdialog::setMessage(QString nowtime,QString saycontent)
{

    ui->tb_chat->append(m_name + "：" + nowtime);
    ui->tb_chat->append(saycontent);
}

void chatdialog::setISayMessage(QString nowtime, QString saycontent)
{

    ui->tb_chat->append("[我]：" + nowtime);
    ui->tb_chat->append(saycontent);
}
void chatdialog::setISayGroupMessage(QString nowtime, QString saycontent,QString name)
{
    if(m_identity == 1){
        ui->tb_chat->append("【群主】[我]："+ nowtime);
    }else{
        ui->tb_chat->append("【成员】[我]："+ nowtime);
    }
    ui->tb_chat->append(saycontent);
}




void chatdialog::on_pb_send_clicked()
{
    if(ui->te_chat->toPlainText().isEmpty()){
        return;
    }

    QString sayContent = ui->te_chat->toPlainText();
    QString nowtime = QDateTime::currentDateTime().
            toString("yyyy-MM-dd hh:mm:ss");
    ui->tb_chat->append("[我]：" + nowtime);
    ui->tb_chat->append(sayContent);
    //清空输入框
    ui->te_chat->clear();

    //将聊天内容发送给核心类处理，让其发送到对端
    if(m_id !=0)
        Q_EMIT SIG_sendContent(m_id,sayContent);
    else
        Q_EMIT SIG_sendGroupContent(m_gid,sayContent);
}

#include<QFileDialog>
void chatdialog::on_pb_1_clicked()
{
    qDebug()<<"发送文件";
    //开弹窗，点击返回文件路径，不点击返回空
    QString path = QFileDialog::getOpenFileName(this,"上传文件");//参数：父窗口指针，左上角标题名,默认打开路径，过滤器（只打开MP3之类的）
    QFileInfo info(path);

    if(path.isEmpty()){
        return;
    }
    //判断是否是下载中的文件，是就不下载否则下载 todo

    //发送
    Q_EMIT SIG_sendfile(m_id,path);
}

