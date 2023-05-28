#include "mychat.h"
#include "ui_mychat.h"
#include<QIcon>
#include<qdebug.h>
#include<QInputDialog>
#include<QMessageBox>

MyChat::MyChat(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyChat)
{
    ui->setupUi(this);
    this->setWindowTitle("聊天室");

   //添加层
    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0,0,0,0);//设置上下左右的边距
    m_layout->setSpacing(3);//设置控件间距
    //设置控件的层
    ui->wdg_list->setLayout(m_layout);

    //添加群聊层
     m_glayout = new QVBoxLayout;
     m_glayout->setContentsMargins(0,0,0,0);//设置上下左右的边距
     m_glayout->setSpacing(3);//设置控件间距
     //设置控件的层
     ui->g_wbg_list->setLayout(m_glayout);

    m_menu = new QMenu;
    m_menu->addAction("添加好友");
    m_menu->addAction("删除好友");
    m_menu->addAction("创建群聊");
    m_menu->addAction("加入群聊");
    m_menu->addAction("退出群聊");
    connect(m_menu,SIGNAL(triggered(QAction *)) ,
            this,SLOT(slot_dealMenu(QAction *)) );

}

MyChat::~MyChat()
{
    delete ui;
}

void MyChat::slot_addFriend(Useritem *item)
{
    //往层上添加控件
    m_layout->addWidget(item);
}

void MyChat::slot_addGroup(Useritem *item)
{
    //往层上添加控件
    m_glayout->addWidget(item);
}

void MyChat::setInfo()
{
    ui->lb_name->setText(m_name);
    ui->le_feeling->setText(m_feeling);
    ui->pb_headImage->setIcon(QIcon(":/face/" + m_icon));
}

void MyChat::slot_dealMenu(QAction *action)
{
    qDebug()<<__func__;
    if(action->text() == "添加好友"){
        qDebug()<<"添加好友";
        //获取添加好友的电话号
        QString tel = QInputDialog::getText(this,"添加好友","请输入手机号");

        //验证
        //查看下是否已经是好友了，就是简单的遍历下当前的好友列表，看看tel是否有存在的
        // 获取 QVBoxLayout 上添加的控件
        for (int i = 0; i < m_layout->count(); i++) {
            QLayoutItem *item = m_layout->itemAt(i);
            if (item->widget() != nullptr) {
                // 如果该控件是 QWidget 子类，则进行相应的操作
                QWidget *widget = item->widget();
                if (widget->inherits("Useritem")) {
                    Useritem *myitem = qobject_cast<Useritem *>(widget);
                    // 对 label 执行相应的操作
                    if(tel == myitem->m_tel){
                        QMessageBox::about( this , "添加好友提示" , "该好友已经添加过了" );
                        return;
                    }
                }
                // 如果还有其他类型的 QWidget 子类，可以继续添加对应的判断
            }
        }
        //验证是否有空
        if(tel.isEmpty()){
            QMessageBox::about( this , "添加好友提示" , "输入不可为空！" );
            return;
        }
        //验证手机号是否合法 -- 正则表达式验证
        QRegExp exp(QString("^(1[356789])[0-9]\{9\}$"));//正则表达式
        bool ret = exp.exactMatch(tel);//精准匹配
        if(!ret){
            QMessageBox::about( this , "添加好友提示" , "手机号输入格式有误！" );
            return;
        }

        //发送tel给客户端
        Q_EMIT SIG_AddFriendBytel(tel);


    }else if(action->text() == "删除好友"){
        qDebug()<<"删除好友";

        //获取添加好友的电话号
        QString tel = QInputDialog::getText(this,"删除好友","请输入手机号");

        //验证
        //验证是否有空
        if(tel.isEmpty()){
            QMessageBox::about( this , "添加好友提示" , "输入不可为空！" );
            return;
        }
        //验证手机号是否合法 -- 正则表达式验证
        QRegExp exp(QString("^(1[356789])[0-9]\{9\}$"));//正则表达式
        bool ret = exp.exactMatch(tel);//精准匹配
        if(!ret){
            QMessageBox::about( this , "添加好友提示" , "手机号输入格式有误！" );
            return;
        }

        //发送tel给客户端
        Q_EMIT SIG_DelFriendBytel(tel);


    }else if(action->text() == "创建群聊"){
        qDebug()<<"创建群聊";

        //获取添加好友的电话号
        QString name = QInputDialog::getText(this,"创建群聊","请输入要创建的群聊名");

        //验证
        //验证是否有空
        if(name.isEmpty()){
            QMessageBox::about( this , "添加好友提示" , "输入不可为空！" );
            return;
        }


        //发送tel给客户端
        Q_EMIT SIG_CreateGroup(name);


    }else if(action->text() == "加入群聊"){
        qDebug()<<"加入群聊";

        //获取群聊号
        QString id = QInputDialog::getText(this,"加入群聊","请输入群聊号");

        //验证
        //验证是否有空
        if(id.isEmpty()){
            QMessageBox::about( this , "添加好友提示" , "输入不可为空！" );
            return;
        }
        //验证群聊号是否合法 -- 正则表达式验证
        QRegExp exp(QString("[0-9]+"));//正则表达式
        bool ret = exp.exactMatch(id);//精准匹配
        if(!ret){
            QMessageBox::about( this , "加入群聊提示" , "群聊号只能为数字" );
            return;
        }

        //发送tel给客户端
        Q_EMIT SIG_AddGroup(id);

    }else if(action->text() == "退出群聊"){
        qDebug()<<"退出群聊";

        //获取群聊号
        QString id = QInputDialog::getText(this,"退出群聊","请输入群聊号");

        //验证
        //验证是否有空
        if(id.isEmpty()){
            QMessageBox::about( this , "添加好友提示" , "输入不可为空！" );
            return;
        }
        //验证群聊号是否合法 -- 正则表达式验证
        QRegExp exp(QString("[0-9]+"));//正则表达式
        bool ret = exp.exactMatch(id);//精准匹配
        if(!ret){
            QMessageBox::about( this , "加入群聊提示" , "群聊号只能为数字" );
            return;
        }

        //发送tel给客户端
        Q_EMIT SIG_DelGroup(id);

    }
}

void MyChat::on_pb_menu_clicked()
{
    QPoint p = QCursor::pos();//获取鼠标位置
    QSize size = m_menu->sizeHint();//获取菜单的属性类吧，可以从中获取大小
    m_menu->exec(QPoint(p.x(),p.y() - size.height()));
}

