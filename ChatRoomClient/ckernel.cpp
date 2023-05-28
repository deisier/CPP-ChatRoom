#include "ckernel.h"
#include"TcpClientMediator.h"
#include<QMessageBox>
#include<qdebug.h>
#include"md5.h"
#include <QMetaType>
#include<QCoreApplication>
//初始化静态类对象指针
CKernel *CKernel::kernel = nullptr;
//获取类对象指针
CKernel *CKernel::GetInstance()
{
    if(kernel){
        return kernel;
    }
    kernel = new CKernel();
    return kernel;
}

#define NetPackMap(a) m_netPackMap[a - _DEF_PROTOCOL_BASE]
//创建协议映射表函数
void CKernel::setNetMap()
{
    memset(m_netPackMap,0,sizeof(PFUN)*_DEF_PROTOCOL_COUNT);
    NetPackMap( _DEF_PACK_REGISTER_RS ) = &CKernel::slot_dealRegisterRs;
    NetPackMap( _DEF_PACK_LOGIN_RS ) = &CKernel::slot_dealLoginRs;
    NetPackMap( _DEF_PACK_FILE_INFO ) = &CKernel::slot_dealFileInfo;
    NetPackMap( _DEF_PACK_FILE_HEAD_RQ ) = &CKernel::slot_dealFileHeadRq;
    NetPackMap( _DEF_PACK_FILE_CONTENT_RQ ) = &CKernel::slot_dealFileContentRq;
    NetPackMap( _DEF_PACK_UPLOAD_FILE_RS ) = &CKernel::slot_dealUploadFileRs;
    NetPackMap( _DEF_PACK_FILE_CONTENT_RS ) = &CKernel::slot_dealFileContentRs;
    NetPackMap( _DEF_PACK_ADD_FOLDER_RS ) = &CKernel::slot_dealAddFolderRs;
    NetPackMap( _DEF_PACK_QUICK_UPLOAD_RS ) = &CKernel::slot_dealQuickUpdateRs;
    NetPackMap( _DEF_PACK_DELETE_FILE_RS ) = &CKernel::slot_dealDeleteFileRs;
    NetPackMap( _DEF_PACK_SHARE_FILE_RS ) = &CKernel::slot_dealShareFileRs;
    NetPackMap( _DEF_PACK_MY_SHARE_RS ) = &CKernel::slot_dealMyshareFileRs;
    NetPackMap( _DEF_PACK_GET_SHARE_RS ) = &CKernel::slot_dealGetshareFileRs;
    NetPackMap( _DEF_PACK_FOLDER_HEAD_RQ ) = &CKernel::slot_dealFolderHeadRq;
    NetPackMap( _DEF_PACK_FRIEND_INFO ) = &CKernel::slot_dealFriendInfoRq;
    NetPackMap( _DEF_PACK_CHAT_RQ ) = &CKernel::slot_dealChatRq;
    NetPackMap( _DEF_PACK_ADDFRIEND_RQ ) = &CKernel::slot_dealAddFriendRq;
    NetPackMap( _DEF_PACK_ADDFRIEND_RS ) = &CKernel::slot_dealAddFriendRs;
    NetPackMap( _DEF_PACK_MESSAGES_RQ ) = &CKernel::slot_dealMessageRq;
    NetPackMap( _DEF_PACK_DELFRIEND_RQ ) = &CKernel::slot_dealDelFriendRq;
    NetPackMap( _DEF_PACK_DELFRIEND_RS ) = &CKernel::slot_dealDelFriendRs;
    NetPackMap( _DEF_PACK_GROUP_INFO ) = &CKernel::slot_dealGroupInfoRq;
    NetPackMap( _DEF_PACK_GROUP_BROADCAST_RQ ) = &CKernel::slot_dealGroupBroadcastRq;
    NetPackMap( _DEF_PACK_GROUP_OFFLINE_RQ ) = &CKernel::slot_dealGroupOfflineRq;
    NetPackMap( _DEF_PACK_GROUP_ADD_RQ ) = &CKernel::slot_dealGroupAddRq;
    NetPackMap( _DEF_PACK_GROUP_ADD_RS ) = &CKernel::slot_dealGroupAddRs;
    NetPackMap( _DEF_PACK_DELGROUP_RQ ) = &CKernel::slot_dealGroupDelRq;
    NetPackMap( _DEF_PACK_DELGROUP_RS ) = &CKernel::slot_dealGroupDelRs;
    NetPackMap( _DEF_PACK_IM_SEND_FILE_RQ ) = &CKernel::slot_dealIMSendFileRq;
    NetPackMap( _DEF_PACK_IM_SEND_FILE_RS ) = &CKernel::slot_dealIMSendFileRs;
    NetPackMap( _DEF_PACK_IM_SEND_FILE_CONTENT_RQ ) = &CKernel::slot_dealIMSendFileContentRq;
    NetPackMap( _DEF_PACK_IM_SEND_FILE_CONTENT_RS ) = &CKernel::slot_dealIMSendFileContentRs;

}

//md5

#define MD5_KEY "1234"

string getMD5(QString val){

    QString str = QString("%1_%2").arg(val).arg(MD5_KEY);
    MD5 md(str.toStdString());
    qDebug()<<"md5:"<<md.toString().c_str();

    return md.toString();
}

string getFileMd5(QString path){

    //转码，打开文件
    char buf[1000];
    CKernel::utf8toGB2312(buf,1000,path);

    FILE * file = fopen(buf,"rb");
    if(!file){
        return string();
    }

    int len = 0;
    MD5 md;
    do{
        len = fread(buf, 1 , 1000 , file);
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        md.update(buf,len);

    }while(len > 0);
    qDebug()<<md.toString().c_str();
    return md.toString();

}

CKernel::CKernel(QObject *parent) : QObject(parent),m_id(0),m_curDir("/"),m_quit(0),m_messageId(1),m_gmessageId(1)
{
    //初始化协议映射表
    setNetMap();

    //初始化配置文件
    setConfig();

    //中介类类初始化
    m_TcpClient = new TcpClientMediator;
    //连接接收数据处理
    connect( m_TcpClient , SIGNAL(SIG_ReadyData(uint,char*,int))
             , this , SLOT(slot_ReadyData(uint,char*,int)) );
    m_TcpClient->OpenNet( m_ip.toStdString().c_str() , m_port );

    //主窗口
    m_mainDialog = new MainDialog;
    //连接主窗口结束信号槽
    connect(m_mainDialog,SIGNAL(SIG_close()),
            this,SLOT( DestoryInstance() ));
    connect( m_mainDialog , SIGNAL(SIG_downloadFile(int))
             , this , SLOT( slot_downloadFile(int) ) );
    //下载进度条
    connect(this,SIGNAL(SIG_updateFileProgress(int ,int))
            , m_mainDialog, SLOT( slot_updateFileProgress(int ,int) ) );

    qRegisterMetaType<FileInfo>("FileInfo");//进行注册
    connect(this,SIGNAL( SIG_insertComplete(FileInfo ) )
            , m_mainDialog , SLOT( slot_insertComplete(FileInfo ) ) );
    //上传进度条
    connect(this,SIGNAL(SIG_updateUploadFileProgress(int ,int))
            , m_mainDialog, SLOT( slot_updateUploadFileProgress(int ,int) ) );
    connect(this,SIGNAL( SIG_uploadFileinsertComplete(FileInfo ) )
            , m_mainDialog , SLOT( slot_uploadFileinsertComplete(FileInfo ) ) );

    connect(m_mainDialog, SIGNAL(SIG_uploadFile(QString)),
             this, SLOT(slot_uploadFile(QString)) );
    connect(m_mainDialog, SIGNAL(SIG_uploadFolder(QString,QString)),
             this, SLOT(slot_uploadFolder(QString,QString)) );

    connect(m_mainDialog,SIGNAL(SIG_addFolder(QString)) ,
            this,SLOT(slot_addFolder(QString)) );

    connect(m_mainDialog,SIGNAL(SIG_changeDir(QString)) ,
            this,SLOT(slot_changeDir(QString)) );

    connect(m_mainDialog,SIGNAL(SIG_deleteFile(QString , QVector<int>) ) ,
            this,SLOT(slot_deleteFile(QString , QVector<int>)) );
    connect(m_mainDialog,SIGNAL(SIG_sharedFile(QString , QVector<int>) ) ,
            this,SLOT(slot_sharedFile(QString , QVector<int>)) );

    connect(m_mainDialog,SIGNAL(SIG_getShareFile(QString , QString)),
            this,SLOT(slot_getShareFile(QString , QString)) );

    connect(m_mainDialog,SIGNAL(SIG_setUploadPauseStatus(int ,int ) ),
            this,SLOT(slot_setUploadPauseStatus(int ,int ) ) );
    connect(m_mainDialog,SIGNAL(SIG_setDownloadPauseStatus(int ,int ) ),
            this,SLOT(slot_setDownloadPauseStatus(int ,int ) ) );
    connect(m_mainDialog,SIGNAL(SIG_showMychat() ),
            this,SLOT(slot_showMychat() ) );

    //m_mainDialog->show();//显示主窗口


    //登录注册窗口
    m_loginDialog = new LoginDialog;
    //关闭
    connect(m_loginDialog,SIGNAL(SIG_close()),
            this,SLOT(DestoryInstance()));
    //登录提交
    connect(m_loginDialog,SIGNAL(SIG_loginCommit(QString,QString)),
            this,SLOT(slot_loginCommit(QString,QString)));
    //注册提交
    connect(m_loginDialog,SIGNAL(SIG_registerCommit(QString,QString,QString)),
            this,SLOT(slot_registerCommit(QString,QString,QString)));
    m_loginDialog->show();


    //IM
    //显示好友列表
    m_mychat = new MyChat;
   // m_mychat->show();
    connect(m_mychat , SIGNAL(SIG_AddFriendBytel(QString)) ,
            this , SLOT(slot_AddFriendBytel(QString)) );
    connect(m_mychat , SIGNAL(SIG_DelFriendBytel(QString)) ,
            this , SLOT(slot_DelFriendBytel(QString)) );

    //群聊
    connect(m_mychat , SIGNAL(SIG_CreateGroup(QString)) ,
            this , SLOT(slot_CreateGroup(QString)) );
    connect(m_mychat , SIGNAL(SIG_AddGroup(QString)) ,
            this , SLOT(slot_AddGroup(QString)) );
    connect(m_mychat , SIGNAL(SIG_DelGroup(QString)) ,
            this , SLOT(slot_DelGroup(QString)) );


}

CKernel::~CKernel()
{

}



//回收槽函数
void CKernel::DestoryInstance()
{
    qDebug()<<__func__;

    //发送一个离线通知
    STRU_OFFLINE_RQ rq;
    rq.userId = m_id;
    SendData((char *)&rq,sizeof(rq));


    m_TcpClient->CloseNet();//关闭网络
    delete m_TcpClient;//回收网络客户端
    delete m_mainDialog;//回收主窗口
    delete m_loginDialog;//回收登录注册窗口

    //退出死循环
    m_quit = 1;

    //回收聊天窗口
    for(auto ite=m_mapIdtoChatdialog.begin();ite!=m_mapIdtoChatdialog.end();){
        chatdialog *chat=(*ite).second;
        chat->hide();                   //先隐藏窗口，后delete
        delete chat;
        chat=nullptr;
        ite=m_mapIdtoChatdialog.erase(ite);   //别忘了在map中删除掉

    }
    //回收好友列表中的控件
    //回收聊天室窗口
    delete m_mychat;
}


//处理接收数据槽函数
void CKernel::slot_ReadyData(unsigned int lSendIP, char *buf, int nlen)
{
    int type = *(int *)buf;
    if(type >=_DEF_PROTOCOL_BASE && type < _DEF_PROTOCOL_BASE + _DEF_PROTOCOL_COUNT){
        PFUN pf = NetPackMap(type);
        if(pf)
            (this->*pf)(lSendIP, buf, nlen);
    }

    //注意：处理完数据后，要记得释放
    delete []buf;
}

void CKernel::SendData(char *buf, int nlen)
{
    m_TcpClient->SendData(0,buf,nlen);
}

#include<QCoreApplication>
#include<QSettings>
#include<QFileInfo>
#include<QDir>

void CKernel::setConfig()
{
    ///  windows *.ini   --> config.ini
    /// [net] 组名
    /// key = value 键值对
    /// ip = "192.128.123"
    /// port = 8004
    /// 保存在哪里 .exe文件下
    /// exe文件目录获取
    /// D:/kelin/Qt_Thrid_stage/project/PlayHall/build  (注意最后面没有右斜杠)

    QString path = QCoreApplication::applicationDirPath() + "/config.ini";
    QFileInfo info(path);//根据文件路径获取到文件，然后判断文件是否存在

    //设置默认端口和ip
    m_ip = "192.168.43.21";
    m_port = 8004;
    if(info.exists()){//有就读取
        //创建该对象，对访问的文件不存在会自动创建
        QSettings settings(path,QSettings::IniFormat,nullptr);//IniFormat:ini文件
        settings.beginGroup("net");
        //可以通过key获取到写入文件中对应的value
        QVariant strip = settings.value("ip","");//参数：（key，默认值）
        if(!strip.toString().isEmpty()) m_ip = strip.toString();
        QVariant strport = settings.value("port",0);
        if(strport.toInt() != 0) m_port = strport.toInt();
        settings.endGroup();

    }else{//没有就写入默认值
        QSettings settings(path,QSettings::IniFormat,nullptr);//IniFormat:ini文件
        settings.beginGroup( "net" ); //变量是组名
        settings.setValue("ip",m_ip);
        settings.setValue("port",m_port);
        settings.endGroup();

    }
    qDebug()<<"ip:"<<m_ip<<"port:"<<m_port;

    //查看是否有默认路径，exe同级路径（没有就创建一个）
    QString syspath = QCoreApplication::applicationDirPath() + "/NetDisk/";
    QDir dir; //用于判断该文件目录是否存在的，没有可以用它去创建
    if(!dir.exists(syspath)){
        dir.mkdir(syspath);
    }
    //创建接收文件文件夹
    QString filePath = QCoreApplication::applicationDirPath() + "/IMfile";
    if(!dir.exists(filePath)){
        dir.mkdir(filePath);
    }
    m_sysPath = QCoreApplication::applicationDirPath() + "/NetDisk";
}

void CKernel::utf8toGB2312(char *gbbuf, int nlen, QString &uft8)
{
    //设置成gb2312类型的
    QTextCodec* gb2312=QTextCodec::codecForName("gb2312");
    QByteArray ba=gb2312->fromUnicode(uft8);
    strcpy(gbbuf,ba.data());
}

QString CKernel::GB2321toutf8(char *gbbuf)
{
    //设置成gb2312类型的
    //论2312的重要性，写错了就会报“程序异常的错误”，然后调试到return的时候提示一个信号错误，很离谱
    QTextCodec* gb2312=QTextCodec::codecForName("gb2312");
    return gb2312->toUnicode(gbbuf);
}

void CKernel::slot_updateFileList()
{
    //删除所有文件信息
    m_mainDialog->slot_deleteAllFileInfo();
    STRU_FILE_LIST_RQ rq;
    rq.userId = m_id;
    std::string dir = m_curDir.toStdString();
    strcpy(rq.dir,dir.c_str());
    SendData((char *)&rq,sizeof(rq));
}



//处理注册提交槽函数
void CKernel::slot_registerCommit(QString tel, QString password, QString name)
{
    STRU_REGISTER_RQ rq;
    std::string telStr = tel.toStdString();
    strcpy_s(rq.tel,telStr.c_str());

    std::string passwordStr = getMD5(password)/*password.toStdString()*/;
    strcpy_s(rq.password,passwordStr.c_str());

    std::string nameStr = name.toStdString();
    strcpy_s(rq.name,nameStr.c_str());

    SendData((char *)&rq,sizeof(rq));
}
//处理登录提交槽函数
void CKernel::slot_loginCommit(QString tel, QString password)
{
    STRU_LOGIN_RQ rq;
    std::string telStr = tel.toStdString();
    strcpy_s(rq.tel,telStr.c_str());

    std::string passwordStr = getMD5(password)/*password.toStdString()*/;
    strcpy_s(rq.password,passwordStr.c_str());


    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_downloadFile(int fileid)
{
    //封包
    STRU_DOWNLOAD_RQ rq;
    rq.fileid = fileid;
    rq.userid = m_id;
    std::string tmp = m_curDir.toStdString();
    strcpy(rq.dir,tmp.c_str());
    //发送
    SendData( (char *)&rq , sizeof(rq) );

}
#include<QFileInfo>
#include<QDateTime>
void CKernel::slot_uploadFile(QString path)
{
    qDebug()<<__func__;
    //创建文件信息结构体
    QFileInfo fileinfo(path);
    FileInfo info;
    info.absolutePath = path;
    info.dir = m_curDir;
    //根据文件内容获取到该文件的md5
    info.md5 = QString::fromStdString(getFileMd5(path));
    info.name = fileinfo.fileName();
    info.size = fileinfo.size();
    info.time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    info.type = "file";

    //打开文件获取文件描述符
    char pathbuf[1000] = "";
    utf8toGB2312(pathbuf,1000,path);
    info.pFile = fopen(pathbuf,"rb");
    if(!info.pFile){
        qDebug()<<"打开失败";
        return ;
    }

    //添加到map中
    m_mapFileMD5ToFileInfo[info.md5.toStdString()] = info;

    //封包
    STRU_UPLOAD_FILE_HEAD_RQ rq;
    strcpy(rq.dir,info.dir.toStdString().c_str());
    string name = info.name.toStdString();
    strcpy(rq.fileName,name.c_str());
    strcpy(rq.fileType,info.type.toStdString().c_str());
    strcpy(rq.md5,info.md5.toStdString().c_str());
    rq.size = info.size;
    strcpy(rq.time , info.time.toStdString().c_str());
    rq.userid = m_id;
    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_uploadFile(QString path, QString dir)
{
    qDebug()<<__func__;
    //创建文件信息结构体
    QFileInfo fileinfo(path);
    FileInfo info;
    info.absolutePath = path;
    info.dir = dir;
    //根据文件内容获取到该文件的md5
    info.md5 = QString::fromStdString(getFileMd5(path));
    info.name = fileinfo.fileName();
    info.size = fileinfo.size();
    info.time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    info.type = "file";

    //打开文件获取文件描述符
    char pathbuf[1000] = "";
    utf8toGB2312(pathbuf,1000,path);
    info.pFile = fopen(pathbuf,"rb");
    if(!info.pFile){
        qDebug()<<"打开失败";
        return ;
    }

    //添加到map中
    m_mapFileMD5ToFileInfo[info.md5.toStdString()] = info;

    //封包
    STRU_UPLOAD_FILE_HEAD_RQ rq;
    strcpy(rq.dir,info.dir.toStdString().c_str());
    string name = info.name.toStdString();
    strcpy(rq.fileName,name.c_str());
    strcpy(rq.fileType,info.type.toStdString().c_str());
    strcpy(rq.md5,info.md5.toStdString().c_str());
    rq.size = info.size;
    strcpy(rq.time , info.time.toStdString().c_str());
    rq.userid = m_id;
    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_uploadFolder(QString path,QString dir)
{
    qDebug()<<__func__;
    QFileInfo info(path);
    QDir dr(path);
    //先创建该文件夹
    slot_addFolder(info.fileName(),dir);
    //获取该文件目录下的所有文件夹及文件信息
    QFileInfoList lst = dr.entryInfoList();
    dir += info.fileName() + "/";
    for(int i = 0;i<lst.size();i++){
        const QFileInfo & file = lst.at(i);
        if(file.fileName() == ".") continue;
        if(file.fileName() == "..") continue;
        if(file.isFile()){
            slot_uploadFile(file.absoluteFilePath(),dir);
        }
        if(file.isDir()){
            slot_uploadFolder(file.absoluteFilePath(),dir);
        }
    }

}

void CKernel::slot_addFolder(QString name)
{
    qDebug()<<__func__;
    //封包
    STRU_ADD_FOLDER_RQ rq;
    strcpy(rq.dir,m_curDir.toStdString().c_str());
    string sname = name.toStdString();
    strcpy(rq.fileName,sname.c_str());
    strcpy(rq.fileType,"dir");
    QString time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.time , time.toStdString().c_str());
    rq.userid = m_id;

    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_addFolder(QString name, QString dir)
{
    qDebug()<<__func__;
    //封包
    STRU_ADD_FOLDER_RQ rq;
    strcpy(rq.dir,dir.toStdString().c_str());
    string sname = name.toStdString();
    strcpy(rq.fileName,sname.c_str());
    strcpy(rq.fileType,"dir");
    QString time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.time , time.toStdString().c_str());
    rq.userid = m_id;

    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_changeDir(QString path)
{
    m_curDir = path;
    slot_updateFileList();
}

void CKernel::slot_deleteFile(QString path, QVector<int> fileIdArray)
{
    //根据fid数量确定数组的大小，进而确定结构体的大小
    int len = sizeof(STRU_DELETE_FILE_RQ) + sizeof(int)*fileIdArray.size();
    STRU_DELETE_FILE_RQ *rq = (STRU_DELETE_FILE_RQ *)malloc( len );
    //因为没有构造函数，所以先执行下初始化函数
    rq->init();
    std::string strPath = path.toStdString();
    strcpy(rq->dir,strPath.c_str());
    rq->fileCount = fileIdArray.size();
    rq->userid = m_id;
    for(int i = 0 ; i < rq->fileCount ; i++){
       rq->fileidArray[i] = fileIdArray[i];
    }
    //发送大小不能为sizeof（），因为存在柔性数组
    SendData((char *)rq,len);

    //别忘了释放
    free(rq);
}

void CKernel::slot_sharedFile(QString path, QVector<int> fileIdArray)
{
    qDebug()<<__func__;
    //根据fid数量确定数组的大小，进而确定结构体的大小

    int len = sizeof(STRU_SHARE_FILE_RQ) + fileIdArray.size()*sizeof(int);
    STRU_SHARE_FILE_RQ *rq = (STRU_SHARE_FILE_RQ *)malloc( len );
    qDebug()<<"len:"<<len;
    //因为没有构造函数，所以先执行下初始化函数
    rq->init();
    std::string strPath = path.toStdString();
    strcpy(rq->dir,strPath.c_str());
    rq->itemCount = fileIdArray.size();
    rq->userid = m_id;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq->shareTime,time.toStdString().c_str());
    qDebug()<<len;
    qDebug()<<rq->itemCount;
    for(int i = 0 ; i < rq->itemCount ; i++){
       rq->fileidArray[i] = fileIdArray[i];
       qDebug()<<fileIdArray[i];
    }
    //发送大小不能为sizeof（），因为存在柔性数组
    SendData((char *)rq,len);

    //别忘了释放
    free(rq);
}

void CKernel::slot_refreshMyShare()
{
    qDebug()<<__func__;
    STRU_MY_SHARE_RQ rq;
    rq.userid = m_id;

    SendData((char *)&rq,sizeof (rq));
}

void CKernel::slot_getShareFile(QString link, QString dir)
{
    //封包
    STRU_GET_SHARE_RQ rq;
    //设置包内容
    string strDir = dir.toStdString();
    strcpy(rq.dir,strDir.c_str());
    rq.shareLink = link.toInt();
    rq.userid = m_id;

    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_setUploadPauseStatus(int fileId, int status)
{
    //map file正常有，可以进行暂停和恢复
    if(m_mapFileIdToFileInfo.count(fileId) > 0){

        m_mapFileIdToFileInfo[fileId].isPause = status;

    }else{
        //没有 ， 断点续传：把上传的信息写入到数据库，程序启动登录后加载到上传列表中，只可以继续
        if(status == 0){
            //断点续传...todo
        }

    }
}

void CKernel::slot_setDownloadPauseStatus(int fileId, int status)
{
    //map file正常有，可以进行暂停和恢复
    if(m_mapFileIdToFileInfo.count(fileId) > 0){

        m_mapFileIdToFileInfo[fileId].isPause = status;

    }else{
        //没有 ， 断点续传：把下载的信息写入到数据库，程序启动登录后加载到下载列表中，只可以继续
        if(status == 0){
            //断点续传...todo
        }

    }
}

void CKernel::slot_showMychat()
{
    static int showOrhide = 1;
    if(showOrhide == 1){
        m_mychat->show();
        showOrhide = 0;
    }else{
        m_mychat->hide();
        showOrhide = 1;
    }
}

void CKernel::slot_ClickUserItem(int id)
{
    //显示对应的聊天窗口
    m_mapIdtoChatdialog[id]->show();
}
void CKernel::slot_ClickGroupItem(int id)
{
    //显示对应的聊天窗口
    m_mapGroupIdtoChatdialog[id]->show();
}

void CKernel::slot_sendContent(int id, QString saycontent)
{
    qDebug()<<__func__;
    qDebug()<<saycontent;
    STRU_CHAT_RQ rq;
    string con = saycontent.toStdString();
    strcpy(rq.content,con.c_str());
    rq.friendId = id;
    rq.userId = m_id;
    QString nowtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.nowtime,nowtime.toStdString().c_str());
    SendData((char *)&rq,sizeof(rq));

}
void CKernel::slot_sendGroupContent(int id, QString saycontent)
{
    qDebug()<<__func__;
    qDebug()<<saycontent;
    STRU_GROUP_CHAT_RQ rq;
    string con = saycontent.toStdString();
    strcpy(rq.content,con.c_str());
    rq.gId = id;
    rq.sendId = m_id;
    QString nowtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.nowtime,nowtime.toStdString().c_str());
    SendData((char *)&rq,sizeof(rq));

}

void CKernel::slot_CreateGroup(QString name)
{
    qDebug()<<__func__;
    //发送创建群聊请求
    STRU_GROUP_CREATE_RQ rq;
    rq.uid = m_id;
    string strname = name.toStdString();
    strcpy(rq.name,strname.c_str());

    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_AddGroup(QString id)
{
    qDebug()<<__func__;
    //发送群添加请求
    STRU_GROUP_GROUP_ADD_RQ rq;
    rq.gid = id.toInt();
    rq.uid = m_id;
    strcpy(rq.tel,m_mychat->m_tel.toStdString().c_str());

    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_DelGroup(QString id)
{
    qDebug()<<__func__;
    STRU_DEL_GROUP_RQ rq;
    rq.uid = m_id;
    rq.gid = id.toInt();
    for(auto ite=m_mapGroupIdtoUseritem.begin();ite!=m_mapGroupIdtoUseritem.end();){
        qDebug()<<ite->second->m_tel<<endl;
        if(ite->second->m_gid == rq.gid){
            rq.identity = ite->second->m_identity;
            qDebug()<<ite->second->m_name<<" "<<ite->second->m_identity<<endl;
            Useritem *item=(*ite).second;
            //群主给个解散群聊的提示信息
            if(item->m_identity == 1){
                if(QMessageBox::question(m_mychat,"解散群聊","尊贵的群主，您是否决定解散群聊") == QMessageBox::No){
                    return;
                }
            }
            m_mychat->m_glayout->removeWidget(item);
            item->hide();                   //先隐藏窗口，后delete
            delete item;
            item=nullptr;
            ite=m_mapGroupIdtoUseritem.erase(ite);   //别忘了在map中删除掉
            //对应的聊天窗口也应该删除掉，这里就不删了，麻烦。。。还得遍历
            //发包
            SendData((char *)&rq,sizeof(rq));
            return;
        }
        ite++;

    }
    QMessageBox::about(m_mychat,"退出群聊","未找到该群聊");
}

void CKernel::slot_sendfile(int id, QString path)
{
    qDebug()<<__func__;

    STRU_IM_SEND_FILE_HEAD_RQ rq;
    rq.sendid = m_id;
    rq.recvid = id;
    //创建文件信息结构体
    QFileInfo fileinfo(path);
    FileInfo info;
    info.absolutePath = path;
    //根据文件内容获取到该文件的md5
    info.name = fileinfo.fileName();
    info.size = fileinfo.size();

    //打开文件获取文件描述符
    char pathbuf[1000] = "";
    utf8toGB2312(pathbuf,1000,path);
    info.pFile = fopen(pathbuf,"rb");
    if(!info.pFile){
        qDebug()<<"打开失败";
        return ;
    }

    //添加到map中
    m_mapRecvidtoFileinfo[id] = info;
    strcpy(rq.fileName , info.name.toStdString().c_str());
    rq.size = info.size;
    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_AddFriendBytel(QString tel)
{
    STRU_ADD_FRIEND_RQ rq;

    strcpy(rq.usertel,m_mychat->m_tel.toStdString().c_str());
    strcpy(rq.friendtel,tel.toStdString().c_str());
    rq.userId = m_id;
    SendData((char *)&rq,sizeof(rq));

}

void CKernel::slot_DelFriendBytel(QString tel)
{
    STRU_DEL_FRIEND_RQ rq;
    rq.userId = m_id;
    strcpy(rq.usertel , m_mychat->m_tel.toStdString().c_str());
    strcpy(rq.friendtel , tel.toStdString().c_str());
    for(auto ite=m_mapIdtoUseritem.begin();ite!=m_mapIdtoUseritem.end();){
        qDebug()<<ite->second->m_tel<<endl;
        if(ite->second->m_tel == tel){
            qDebug()<<ite->second->m_name<<endl;
            Useritem *item=(*ite).second;
            m_mychat->m_layout->removeWidget(item);
            item->hide();                   //先隐藏窗口，后delete
            delete item;
            item=nullptr;
            ite=m_mapIdtoUseritem.erase(ite);   //别忘了在map中删除掉

            //发包
            SendData((char *)&rq,sizeof(rq));
            return;
        }
        ite++;

    }
    QMessageBox::about(m_mychat,"删除好友","未找到该好友");

}
//注册请求的结果
//#define tel_is_exist		(0)
//#define name_is_exist		(1)
//#define register_success	(2)



//处理注册回复
void CKernel::slot_dealRegisterRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS *)buf;
    //根据不同结果显示不同提示
    switch(rs->result){
    case tel_is_exist:
        QMessageBox::about(m_loginDialog,"注册提示","该手机号已注册");
        break;
    case name_is_exist:
        QMessageBox::about(m_loginDialog,"注册提示","该昵称已存在");
        break;
    case register_success:
        QMessageBox::about(m_loginDialog,"注册提示","注册成功");
        break;
    }

}

//登录请求的结果
//#define user_not_exist		(0)
//#define password_error		(1)
//#define login_success		(2)
//处理登录回复
void CKernel::slot_dealLoginRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_LOGIN_RS * rs = (STRU_LOGIN_RS *)buf;
    //根据不同结果显示不同提示
    switch(rs->result){
    case user_not_exist:
        QMessageBox::about(m_loginDialog,"注册提示","该用户不存在");
        break;
    case password_error:
        QMessageBox::about(m_loginDialog,"注册提示","密码错误");
        break;
    case login_success:

        //ui界面切换 前台
        m_loginDialog->hide();
        m_mainDialog->show();

        //后台
        m_name = rs->name;//记录用户名和用户id
        m_id = rs->userid;

        m_mainDialog->slot_setInfo(m_name);//设置主界面中的用户姓名

        //登录成功 追加请求
        //获取根目录 “/” 下所有文件
        m_curDir = "/";
        //更新目录
        slot_updateFileList();
        //更新分享列表
        slot_refreshMyShare();
        QString myid = QString::number(m_id);
        QDir dir;
        //创建接收文件目录
        m_IMfilePath = QCoreApplication::applicationDirPath() + "/IMfile/" + myid + "/";
        if(!dir.exists(m_IMfilePath)){
            dir.mkdir(m_IMfilePath);
        }
        break;
    }
}

void CKernel::slot_dealFileInfo(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_FILE_INFO * info = (STRU_FILE_INFO *)buf;

    //判断该文件是否为当前路径下文件（有可能出现已经换文件了，但是上个文件目录下的文件信息才来，导致加载错文件目录）
    if(info->dir == m_curDir){
        FileInfo fileInfo;

        fileInfo.fileid = info->fileId;
        fileInfo.size = info->size;
        fileInfo.name = QString::fromStdString(info->fileName);
        fileInfo.time = QString::fromStdString(info->uploadTIme);
        fileInfo.md5  = QString::fromStdString(info->md5);
        fileInfo.type = QString::fromStdString(info->fileType);

        //更新到文件列表表格控件中
        m_mainDialog->slot_InsertFileInfo( fileInfo );
    }





}

void CKernel::slot_dealFileHeadRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_FILE_HEAD_RQ *rq = (STRU_FILE_HEAD_RQ *) buf;

    //获取文件信息
    FileInfo fileInfo;

    fileInfo.fileid = rq->fileid;
    fileInfo.size = rq->size;
    fileInfo.name = QString::fromStdString(rq->fileName);
    fileInfo.md5  = QString::fromStdString(rq->md5);
    fileInfo.type = QString::fromStdString(rq->fileType);
    fileInfo.dir = QString::fromStdString(rq->dir);
    char tmpName[_MAX_PATH_SIZE];
    memset(tmpName,0,_MAX_PATH_SIZE);

    fileInfo.absolutePath = m_sysPath + fileInfo.dir + fileInfo.name;

    QDir dir;
    //获取路径文件夹名集合
    QStringList subStr = fileInfo.dir.split("/");
    QString pathSum = m_sysPath + "/";//每个文件夹的路径
    for(int i =0 ; i < subStr.size() ; i++){
        // /0314/ split在分割的时候首尾有两个空格，所以要判断下是否为空
        if(subStr.at(i).isEmpty()) continue;
        //拼路径上文件夹路径
        pathSum +=subStr.at(i) + "/";
        //判断该文件夹是否存在，不存在就创建
        if(!dir.exists(pathSum)){
            dir.mkdir(pathSum);
        }
    }

    utf8toGB2312(tmpName,fileInfo.absolutePath.size(),fileInfo.absolutePath);
    //打开文件
    fileInfo.pFile = fopen(tmpName,"wb");//二进制写的方式打开，加个二进制
    if(!fileInfo.pFile){
       qDebug()<<"打开文件失败："<<fileInfo.absolutePath;
       return;
    }
    //打开文件成功，创建下载信息（进度条）
    m_mainDialog->slot_InsertDownLoadFile(fileInfo);
    //插入到map中
    m_mapFileIdToFileInfo[rq->fileid] = fileInfo;
    STRU_FILE_HEAD_RS rs;
    rs.fileid = rq->fileid;
    rs.userid = m_id;
    rs.result = 1;
    //发送回复
    SendData((char *)&rs , sizeof(rs));
    qDebug()<<"--------------";
}
#include<QThread>
void CKernel::slot_dealFileContentRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_FILE_CONTENT_RQ *rq = (STRU_FILE_CONTENT_RQ *)buf;
    qDebug()<<"文件id："<<rq->fileid;

    if(m_mapFileIdToFileInfo.count(rq->fileid) == 0 ) return;

    //获取文件信息
    FileInfo & info = m_mapFileIdToFileInfo[rq->fileid];
    while(info.isPause){
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        //程序退出时，退出循环
        if(m_quit) return;
    }
    //向文件中写入
    int len = fwrite(rq->content,1,rq->len,info.pFile);
    qDebug()<<len;

    STRU_FILE_CONTENT_RS rs;

    if(len != rq->len){
        //写入失败,跳回写入之前的位置
        fseek(info.pFile,-1*len,SEEK_CUR);
        rs.result = 0;
    }else{
        //写入成功
        info.pos +=len;
        rs.result = 1;
        //写入成功，发送给主窗口目前写入多少
        Q_EMIT SIG_updateFileProgress(info.fileid,info.pos);
    }
    //已经全部写入
    //qDebug()<<"pos:"<<info.pos<<"size:"<<info.size;
    if(info.pos == info.size){
        fclose(info.pFile);
        m_mapFileIdToFileInfo.erase(info.fileid);
        qDebug()<<"fasong";
        //完成之后，将文件信息添加到已完成目录中
//        QVariant DataInfo;
//        FileInfo tmp = info;
//        DataInfo.setValue(tmp);

        //Q_EMIT SIG_insertComplete(info);
    }
    qDebug()<<info.pos<<"size:"<<info.size;
    rs.fileid = rq->fileid;
    rs.userid = rq->userid;
    rs.len = len;

    SendData((char *)&rs,sizeof(rs));


}

void CKernel::slot_dealUploadFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_UPLOAD_FILE_HEAD_RS * rs = (STRU_UPLOAD_FILE_HEAD_RS *)buf;
    //添加到fid对应的映射
    m_mapFileIdToFileInfo[rs->fileid] = m_mapFileMD5ToFileInfo[rs->md5];
    //获取文件信息
    FileInfo & info = m_mapFileIdToFileInfo[rs->fileid];
    //之前的fid没有赋值，不赋值会导致上传文件进度条出错
    info.fileid = rs->fileid;
    //删除md5中对应的映射
    if(m_mapFileMD5ToFileInfo.count(rs->md5) != 0){
        m_mapFileMD5ToFileInfo.erase(rs->md5);
    }

    //添加进度条到上传界面
    m_mainDialog->slot_InsertUpLoadFile(info);

    STRU_FILE_CONTENT_RQ rq;
    //读文件内容
    rq.len = fread(rq.content,1,_DEF_BUFFER,info.pFile);

    rq.fileid = rs->fileid;
    rq.userid = rs->userid;
    SendData((char *)&rq,sizeof(rq));

}

void CKernel::slot_dealFileContentRs(unsigned int lSendIP, char *buf, int nlen)
{
    //qDebug()<<__func__;
    //拆包
    STRU_FILE_CONTENT_RS * rs = (STRU_FILE_CONTENT_RS *)buf;

    //获取文件信息
    if(m_mapFileIdToFileInfo.count(rs->fileid) == 0) return;
    FileInfo & info = m_mapFileIdToFileInfo[rs->fileid];
    //暂停循环
    while(info.isPause){
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        //程序退出时，退出循环
        if(m_quit) return;
    }
    //写入失败
    if(rs->result == 0){
        fseek(info.pFile,-rs->len,SEEK_CUR);
    //成功
    }else{
        info.pos +=rs->len;
        Q_EMIT SIG_updateUploadFileProgress(info.fileid,info.pos);
        if(info.pos >= info.size){
            //判断该上传文件是否为当前路径下文件，是就更新目录
            if(info.dir == m_curDir)
                slot_updateFileList();
            //关文件描述符，删节点
            fclose(info.pFile);
            m_mapFileIdToFileInfo.erase(rs->fileid);
            //上传完成，在完成列表添加控件
            Q_EMIT SIG_uploadFileinsertComplete(info);
            return;
        }
    }

    STRU_FILE_CONTENT_RQ rq;
    //读文件内容
    rq.len = fread(rq.content,1,_DEF_BUFFER,info.pFile);

    rq.fileid = rs->fileid;
    rq.userid = rs->userid;
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_dealAddFolderRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_ADD_FOLDER_RS *rs = (STRU_ADD_FOLDER_RS *)buf;
    if(rs->result == 1){
        slot_updateFileList();
    }
}

void CKernel::slot_dealQuickUpdateRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_QUICK_UPLOAD_RS *rs = (STRU_QUICK_UPLOAD_RS *)buf;
    if(rs->result == 0) return;
    //根据md5，获取info
    if(m_mapFileMD5ToFileInfo.count(rs->md5) == 0) return;
    FileInfo &info = m_mapFileMD5ToFileInfo[rs->md5];
    //更新当前列表
    slot_updateFileList();
    //添加到上传目录下
    m_mainDialog->slot_uploadFileinsertComplete(info);
    m_mapFileMD5ToFileInfo.erase(rs->md5);
}

void CKernel::slot_dealDeleteFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DELETE_FILE_RS * rs = (STRU_DELETE_FILE_RS *)buf;
    //删除失败
    if(rs->result == 0) return;
    //是当前目录，更新下
    if(QString::fromStdString(rs->dir) == m_curDir){
        slot_updateFileList();
    }
}

void CKernel::slot_dealShareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_SHARE_FILE_RS *rs = (STRU_SHARE_FILE_RS *)buf;

    if(rs->result == 0) return;
    //刷新分享列表
    //删除所有信息
    m_mainDialog->slot_deleteSharedAllFileInfo();

    //获取分享列表
    slot_refreshMyShare();
}

void CKernel::slot_dealMyshareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_MY_SHARE_RS * rs = (STRU_MY_SHARE_RS *)buf;
    int len = rs->itemCount;
    for(int i = 0;i<len ;i++){
        //插入每个文件信息
        qDebug()<<rs->items[i].name<<" "<<rs->items[i].size<<" "<<rs->items[i].time<<" "<<rs->items[i].shareLink<<endl;
        m_mainDialog->slot_insertSharedFile(rs->items[i].name,rs->items[i].size,rs->items[i].time,rs->items[i].shareLink);
    }
}

void CKernel::slot_dealGetshareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_GET_SHARE_RS * rs = (STRU_GET_SHARE_RS *)buf;
    //获取失败
    if(rs->result == 0) return;
    //判断是否为当前路径，是就刷新
    if(rs->dir == m_curDir)
        slot_updateFileList();
}

void CKernel::slot_dealFolderHeadRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_FOLDER_HEAD_RQ * rq = (STRU_FOLDER_HEAD_RQ *)buf;
    //先创建该路径上没有的文件夹
    QDir dir;
    //获取路径文件夹名集合
    QStringList subStr = QString::fromStdString( rq->dir).split("/");
    QString pathSum = m_sysPath + "/";//每个文件夹的路径
    for(int i =0 ; i < subStr.size() ; i++){
        // /0314/ split在分割的时候首尾有两个空格，所以要判断下是否为空
        if(subStr.at(i).isEmpty()) continue;
        //拼路径上文件夹路径
        pathSum +=subStr.at(i) + "/";
        //判断该文件夹是否存在，不存在就创建
        if(!dir.exists(pathSum)){
            dir.mkdir(pathSum);
        }
    }
    //创建该文件夹
    pathSum += QString::fromStdString( rq->fileName) + "/";
    //判断该文件夹是否存在，不存在就创建
    if(!dir.exists(pathSum)){
        dir.mkdir(pathSum);
    }

}

void CKernel::slot_dealFriendInfoRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;

    STRU_FRIEND_INFO * info =(STRU_FRIEND_INFO *)buf;


    QString name = QString::fromStdString(info->name);
    QString icon = QString::fromStdString(info->icon);
    QString feeling = QString::fromStdString(info->feeling);
    QString tel = QString::fromStdString(info->tel);
    //是自己，更新聊天窗口信息并返回   
    if(m_id == info->userId){
        m_mychat->m_name = name;
        m_mychat->m_icon = icon;
        m_mychat->m_feeling = feeling;
        m_mychat->m_tel = tel;
        //更新自身信息
        m_mychat->setInfo();
        return;
    }
    //是好友信息
    if(m_mapIdtoUseritem.count(info->userId) == 0){
        //4、之前的好友列表没有，把好友添加到控件上
        //4.1、创建一个Userid控件
        Useritem *item=new Useritem;
        //4.2、给控件赋值

        item->slot_setInfo(info->userId,name,icon,info->state,feeling,tel);
        //4.3、给控件绑定点击的信号和槽函数
        connect(item,SIGNAL(SIG_ClickUserItem(int )),
                this,SLOT(slot_ClickUserItem(int)));
         m_mapIdtoUseritem[info->userId]=item;
         //4.7、把新创建的控件添加到好友列表中
         m_mychat->slot_addFriend(item);
        //4.4、创建聊天窗口
        chatdialog* chat=new chatdialog;
        chat->setInfo(info->userId,name);
        //4.5、聊天窗口绑定发送数据的信号和槽函数
        connect(chat,SIGNAL(SIG_sendContent(int ,QString )),
                this,SLOT(slot_sendContent(int,QString)));
        connect(chat,SIGNAL(SIG_sendfile(int ,QString )),
                this,SLOT(slot_sendfile(int ,QString )));
        //4.6、把聊天窗口放到map中用id控制
        m_mapIdtoChatdialog[info->userId]=chat;


    }else{
        //5、之前的好友列表中有这个好友
        //5.1、取出之前创建的好友控件
        Useritem *item=m_mapIdtoUseritem[info->userId];
        //5.2、判断之前好友是下线状态，现在是上线状态
        if(item && 0==item->m_state && 1==info->state){
            //5.3、发送一条弹窗，显示某某某以上线
            qDebug()<<QString("用户【%1】已上线").arg(info->name);
        }

        //5.4、更新好友控件属性
        item->slot_setInfo(info->userId,name,icon,info->state,feeling,tel);
    }

}

void CKernel::slot_dealChatRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;

    STRU_CHAT_RQ * rq = (STRU_CHAT_RQ *)buf;
    qDebug()<<rq->content;
    //找对应的聊天窗口
    if(m_mapIdtoChatdialog.count(rq->friendId) > 0){
        QString nowtime = QString::fromStdString(rq->nowtime);

        m_mapIdtoChatdialog[rq->friendId]->setMessage(nowtime,QString::fromStdString(rq->content));
    }else{
        qDebug()<<"没找到对应的聊天窗口哦";
    }
}

void CKernel::slot_dealChatRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_CHAT_RS * rs = (STRU_CHAT_RS *)buf;
    //这里在窗口中输出该好友不在线的消息即可
}

void CKernel::slot_dealAddFriendRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_ADD_FRIEND_RQ * rq = (STRU_ADD_FRIEND_RQ *)buf;

    STRU_ADD_FRIEND_RS rs;
    //弹窗是否同意
    QString str = QString("手机号为：%1 请求添加您为好友，是否同意").arg(rq->usertel);
    if(QMessageBox::question(m_mychat,"添加好友请求",str) == QMessageBox::Yes){
        rs.result = add_success;

    }else{
        rs.result = user_refuse;
    }
    rs.friendId = m_id;
    strcpy(rs.friendName , m_mychat->m_name.toStdString().c_str());
    rs.userId = rq->userId;

    SendData((char *)&rs,sizeof(rs));
}

void CKernel::slot_dealAddFriendRs(unsigned int lSendIP, char *buf, int nlen)
{
    //no_this_user
    //user_refuse
    //user_offline
    //add_success
    //1.拆包
    STRU_ADD_FRIEND_RS *rs=(STRU_ADD_FRIEND_RS *)buf;

    //2.转码好友的用户名
    QString strAddName=QString::fromStdString( rs->friendName);
    //3.根据结果显示提示信息
    switch(rs->result){
        case no_this_user:
            QMessageBox::about(m_mychat,"提示",QString("该用户不存在，添加好友失败"));
        break;
        case user_offline:
            QMessageBox::about(m_mychat,"提示",QString("该用户不在线，添加好友失败"));
        break;
        case user_refuse:
            QMessageBox::about(m_mychat,"提示",QString("用户[%1]拒绝，添加好友失败").arg(strAddName));
        break;
        case add_success:
            QMessageBox::about(m_mychat,"提示",QString("用户[%1]同意，添加好友成功").arg(strAddName));
        break;

    }
}

void CKernel::slot_dealMessageRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_MESSAGES_RQ * rq = (STRU_MESSAGES_RQ *)buf;
    QString nowtime = QString::fromStdString(rq->nowtime);
    QString mycontent = QString::fromStdString(rq->content);

    qDebug()<<nowtime<<" "<<mycontent<<" "<<rq->sendId<<" "<<rq->recvId;
    while(rq->id != m_messageId){
        QThread::msleep(1);
         QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
    m_messageId++;
    //是自己发送的
    if(rq->sendId == m_id){
        //判断该窗口是否存在，存在就添加消息
        if(m_mapIdtoChatdialog.count(rq->recvId) > 0){
            m_mapIdtoChatdialog[rq->recvId]->setMessage(nowtime,mycontent);
        }
    }else{
        //判断该窗口是否存在，存在就添加消息
        if(m_mapIdtoChatdialog.count(rq->sendId) > 0){
            m_mapIdtoChatdialog[rq->sendId]->setISayMessage(nowtime,mycontent);
        }
    }

}

void CKernel::slot_dealDelFriendRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DEL_FRIEND_RQ * rq = (STRU_DEL_FRIEND_RQ *)buf;
    QString tel = QString::fromStdString(rq->usertel);
    for(auto ite=m_mapIdtoUseritem.begin();ite!=m_mapIdtoUseritem.end();){
        //qDebug()<<ite->second->m_tel<<endl;
        if(ite->second->m_tel == tel){
            qDebug()<<ite->second->m_name<<endl;
            Useritem *item=(*ite).second;
            m_mychat->m_layout->removeWidget(item);
            item->hide();                   //先隐藏窗口，后delete
            delete item;
            item=nullptr;
            ite=m_mapIdtoUseritem.erase(ite);   //别忘了在map中删除掉
            return;
        }
        ite++;
    }
}

void CKernel::slot_dealDelFriendRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DEL_FRIEND_RS * rs = (STRU_DEL_FRIEND_RS *)buf;
    if(rs->result == del_success){
        QMessageBox::about(m_mychat,"删除好友","删除好友成功!");
    }else{
        QMessageBox::about(m_mychat,"删除好友","删除好友失败!");
    }
}

void CKernel::slot_dealGroupInfoRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;

    STRU_GROUP_INFO * info =(STRU_GROUP_INFO *)buf;


    QString name = QString::fromStdString(info->gname);

    //是好友信息
    if(m_mapGroupIdtoUseritem.count(info->gid) == 0){
        //4、之前的好友列表没有，把好友添加到控件上
        //4.1、创建一个Userid控件
        Useritem *item=new Useritem;
        //4.2、给控件赋值

        item->slot_setGroupInfo(info->gid,name,info->identity);
        //4.3、给控件绑定点击的信号和槽函数
        connect(item,SIGNAL(SIG_ClickGroupItem(int)),
                this,SLOT(slot_ClickGroupItem(int)));
         m_mapGroupIdtoUseritem[info->gid]=item;
         //4.7、把新创建的控件添加到好友列表中
         m_mychat->slot_addGroup(item);
        //4.4、创建聊天窗口
        chatdialog* chat=new chatdialog;
        chat->setGroupInfo(info->gid,name,info->identity);
        //4.5、聊天窗口绑定发送数据的信号和槽函数
        connect(chat,SIGNAL(SIG_sendGroupContent(int ,QString )),
                this,SLOT(slot_sendGroupContent(int,QString)));
        //4.6、把聊天窗口放到map中用id控制
        m_mapGroupIdtoChatdialog[info->gid]=chat;


    }else{

    }
}

void CKernel::slot_dealGroupBroadcastRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_GROUP_BROADCAST_CHAT_RQ * rq = (STRU_GROUP_BROADCAST_CHAT_RQ *)buf;
    qDebug()<<rq->content;
    //找对应的聊天窗口
    if(m_mapGroupIdtoChatdialog.count(rq->gId) > 0){
        QString nowtime = QString::fromStdString(rq->nowtime);
        QString name = QString::fromStdString(rq->name);
        m_mapGroupIdtoChatdialog[rq->gId]->setGroupMessage(nowtime,QString::fromStdString(rq->content),name,rq->identity);
    }else{
        qDebug()<<"没找到对应的聊天窗口哦";
    }
}

void CKernel::slot_dealGroupOfflineRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_GROUP_OFFLINE_CHAT_RQ * rq = (STRU_GROUP_OFFLINE_CHAT_RQ *)buf;
    QString nowtime = QString::fromStdString(rq->nowtime);
    QString mycontent = QString::fromStdString(rq->content);
    QString name = QString::fromStdString(rq->name);

    qDebug()<<nowtime<<" "<<mycontent<<" "<<rq->gId<<" "<<rq->identity;
    while(rq->id != m_gmessageId){
        QThread::msleep(1);
         QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
    m_gmessageId++;
    //是自己发送的
    if(rq->uid == m_id){
        //判断该窗口是否存在，存在就添加消息
        if(m_mapGroupIdtoChatdialog.count(rq->gId) > 0){
            m_mapGroupIdtoChatdialog[rq->gId]->setISayGroupMessage(nowtime,mycontent,name);

        }
    }else{
        //判断该窗口是否存在，存在就添加消息
        if(m_mapGroupIdtoChatdialog.count(rq->gId) > 0){
            m_mapGroupIdtoChatdialog[rq->gId]->setGroupMessage(nowtime,mycontent,name,rq->identity);
        }
    }
}

void CKernel::slot_dealGroupAddRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //1.拆包
    STRU_GROUP_GROUP_ADD_RQ *rq=(STRU_GROUP_GROUP_ADD_RQ *)buf;
    STRU_GROUP_GROUP_ADD_RS rs;
    //弹窗是否同意
    QString str = QString("手机号为：%1 请求加入您群号为[%2]的群聊，是否同意").arg(rq->tel).arg(rq->gid);
    if(QMessageBox::question(m_mychat,"添加群聊请求",str) == QMessageBox::Yes){
        rs.result = add_success;

    }else{
        rs.result = user_refuse;
    }
    rs.gid = rq->gid;
    rs.senderid = rq->uid;

    SendData((char *)&rs,sizeof(rs));
}

void CKernel::slot_dealGroupAddRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;

    //1.拆包
    STRU_GROUP_GROUP_ADD_RS *rs=(STRU_GROUP_GROUP_ADD_RS *)buf;

    //2.转码好友的用户名
    //3.根据结果显示提示信息
    switch(rs->result){
        case no_this_group:
            QMessageBox::about(m_mychat,"提示",QString("该群聊不存在，添加群聊失败"));
        break;
        case user_offline:
            QMessageBox::about(m_mychat,"提示",QString("该群聊群主不在线，无法验证群信息"));
        break;
        case user_refuse:
            QMessageBox::about(m_mychat,"提示",QString("群聊[%1]拒绝，添加好友失败").arg(rs->gid));
        break;
        case add_success:
            QMessageBox::about(m_mychat,"提示",QString("群聊[%1]同意，添加好友成功").arg(rs->gid));
        break;

    }
}

void CKernel::slot_dealGroupDelRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_DEL_GROUP_RQ *rq = (STRU_DEL_GROUP_RQ *)buf;
    for(auto ite=m_mapGroupIdtoUseritem.begin();ite!=m_mapGroupIdtoUseritem.end();){
        qDebug()<<ite->second->m_tel<<endl;
        if(ite->second->m_gid == rq->gid){
            //提示，该群聊已被解散
            QMessageBox::about(m_mychat,"群聊解散",QString("[%d]该群聊已被解散").arg(ite->second->m_name));
            qDebug()<<ite->second->m_name<<" "<<ite->second->m_identity<<endl;
            Useritem *item=(*ite).second;

            m_mychat->m_glayout->removeWidget(item);
            item->hide();                   //先隐藏窗口，后delete
            delete item;
            item=nullptr;
            ite=m_mapGroupIdtoUseritem.erase(ite);   //别忘了在map中删除掉
            //对应的聊天窗口也应该删除掉，这里就不删了，麻烦。。。还得遍历


            return;
        }
        ite++;

    }

}

void CKernel::slot_dealGroupDelRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DEL_GROUP_RS * rs = (STRU_DEL_GROUP_RS *)buf;
    if(rs->result == del_success){
        QMessageBox::about(m_mychat,"群聊退出","该群聊已退出成功");
    }
}

void CKernel::slot_dealIMSendFileRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_IM_SEND_FILE_HEAD_RQ *rq = (STRU_IM_SEND_FILE_HEAD_RQ *)buf;
    STRU_IM_SEND_FILE_HEAD_RS rs;
    rs.recvid = rq->recvid;
    rs.sendid = rq->sendid;
    //显示提示
    for(auto ite=m_mapIdtoChatdialog.begin();ite!=m_mapIdtoChatdialog.end();){
        if(ite->second->m_id == rq->sendid){
            //提示
            if(QMessageBox::question(ite->second,"接收文件",QString("是否接收来自【%1】发送的文件【%2】").arg(ite->second->m_name).arg(rq->fileName))
                    == QMessageBox::No){

                rs.result = 2;//不收
                SendData((char *)&rs,sizeof(rs));
                return;
            }
            break;
        }
        ite++;
    }
    //创建文件夹
    rs.result = 1;
    QString path = m_IMfilePath + QString::fromStdString(rq->fileName);
    FileInfo info;
    info.absolutePath = path;
    //根据文件内容获取到该文件的md5
    info.md5 = QString::fromStdString(getFileMd5(path));
    info.name = QString::fromStdString(rq->fileName);
    info.size = rq->size;
    info.pos= 0;

    //打开文件获取文件描述符
    char pathbuf[1000] = "";
    utf8toGB2312(pathbuf,1000,path);
    info.pFile = fopen(pathbuf,"wb");
    if(!info.pFile){
        qDebug()<<"打开失败";
        return ;
    }
    //添加到map中
    m_mapSendidtoFileinfo[rq->sendid] = info;

    SendData((char *)&rs,sizeof(rs));

}

void CKernel::slot_dealIMSendFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_IM_SEND_FILE_HEAD_RS *rs = (STRU_IM_SEND_FILE_HEAD_RS *)buf;

    if(rs->result == 0 || rs->result == 2){
        //显示提示
        for(auto ite=m_mapIdtoChatdialog.begin();ite!=m_mapIdtoChatdialog.end();){

            if(ite->second->m_id == rs->recvid){
                //提示
                if(rs->result == 0){
                    QMessageBox::question(ite->second,"文件发送提示","文件发送失败，对端不在线");
                    return;
                }else if(rs->result == 2){
                    QMessageBox::question(ite->second,"文件发送提示","文件发送失败，对端拒绝接收");
                    return;
                }
            }
            ite++;
        }
    }
    FileInfo & info = m_mapRecvidtoFileinfo[rs->recvid];
    //发送包内容
    STRU_IM_SEND_FILE_CONTENT_RQ lrq;
    lrq.recvid = rs->recvid;
    lrq.sendid = rs->sendid;
    lrq.len = fread(lrq.content,1,_DEF_BUFFER,info.pFile);

    SendData((char *)&lrq,sizeof(lrq));

}

void CKernel::slot_dealIMSendFileContentRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_IM_SEND_FILE_CONTENT_RQ * rq = (STRU_IM_SEND_FILE_CONTENT_RQ *)buf;
    //取出来
    FileInfo & info = m_mapSendidtoFileinfo[rq->sendid];
    //写入，记录写入大小
    int size = fwrite(rq->content,1,rq->len,info.pFile);
    STRU_IM_SEND_FILE_CONTENT_RS rs;
    rs.recvid = rq->recvid;
    rs.sendid = rq->sendid;
    rs.len = rq->len;
    if(size != rq->len){
        fseek(info.pFile,-size,SEEK_CUR);
        rs.result = 0;
    }else{
        rs.result = 1;
        info.pos += rq->len;
        qDebug()<<info.name<<" "<<info.pos<<" "<<info.size<<endl;
        if(info.pos >= info.size){
            fclose(info.pFile);
            SendData((char *)&rs,sizeof(rs));
            //提示接收完成
            for(auto ite=m_mapIdtoChatdialog.begin();ite!=m_mapIdtoChatdialog.end();ite++){
                if(ite->second->m_id == rs.sendid){
                   QMessageBox::about(ite->second,"文件发送",QString("文件【%1】已接受完成").arg(info.name));
                }
            }
            m_mapSendidtoFileinfo.erase(rq->sendid);
            return;
        }
    }
    SendData((char *)&rs,sizeof(rs));
}

void CKernel::slot_dealIMSendFileContentRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_IM_SEND_FILE_CONTENT_RS * rs = (STRU_IM_SEND_FILE_CONTENT_RS *)buf;

    FileInfo & info = m_mapRecvidtoFileinfo[rs->recvid];

    if(!rs->result){
        fseek(info.pFile,-rs->len,SEEK_CUR);
    }else{

        info.pos += rs->len;
        qDebug()<<info.name<<" "<<info.pos<<" "<<info.size<<endl;
        if(info.pos >= info.size){
            //提示接收完成
            for(auto ite=m_mapIdtoChatdialog.begin();ite!=m_mapIdtoChatdialog.end();ite++){
                qDebug()<<ite->second->m_id<<" "<<rs->recvid<<endl;
                if(ite->second->m_id == rs->recvid){
                   QMessageBox::about(ite->second,"文件发送",QString("文件【%1】已发送完成").arg(info.name));
                }
            }
            fclose(info.pFile);
            m_mapRecvidtoFileinfo.erase(rs->recvid);
            return;

        }
    }

    //发送包内容
    STRU_IM_SEND_FILE_CONTENT_RQ lrq;
    lrq.recvid = rs->recvid;
    lrq.sendid = rs->sendid;
    lrq.len = fread(lrq.content,1,_DEF_BUFFER,info.pFile);

    SendData((char *)&lrq,sizeof(lrq));

}











