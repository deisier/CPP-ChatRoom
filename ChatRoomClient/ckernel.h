#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include"maindialog.h"
#include<qdebug.h>
#include"INetMediator.h"
#include"packdef.h"
#include"logindialog.h"
#include<QTextCodec>
#include"mychat.h"
#include"chatdialog.h"

class CKernel;
//类函数指针类型
typedef void (CKernel::*PFUN)( unsigned int lSendIP , char* buf , int nlen );

//单例：不会无缘无故新建一个
class CKernel : public QObject
{
    Q_OBJECT
public:
    static CKernel * GetInstance();

    static void utf8toGB2312(char *gbbuf,int nlen,QString &uft8);

    static QString GB2321toutf8(char *gbbuf);

signals:

public slots:
    //控件处理
    //更新文件目录
    void slot_updateFileList();

    //回收核心类资源槽函数
    void DestoryInstance();
    //注册提交槽函数
    void slot_registerCommit( QString tel , QString password , QString name);
    //登录提交槽函数
    void slot_loginCommit( QString tel , QString password);

    void slot_downloadFile(int fileid);
    //上传文件槽函数
    void slot_uploadFile(QString path);
    void slot_uploadFile(QString path,QString dir);
    //上传文件夹槽函数
    void slot_uploadFolder(QString path,QString dir);
    //新建文件夹槽函数
    void slot_addFolder(QString name);
    void slot_addFolder(QString name,QString dir);
    //路径跳转
    void slot_changeDir(QString);
    //删除处理
    void slot_deleteFile(QString path, QVector<int> fileIdArray);

    void slot_sharedFile(QString path, QVector<int> fileIdArray);
    //获取分享列表
    void slot_refreshMyShare();
    //获取分享文件
    void slot_getShareFile(QString link, QString dir);
    //上传暂停槽函数
    void slot_setUploadPauseStatus(int fileId,int status);
    //下载暂停槽函数
    void slot_setDownloadPauseStatus(int fileId,int status);

    ///////////////////IM/////////////////////
    //显示聊天室
    void slot_showMychat();
    void slot_ClickUserItem(int id);
    void slot_sendContent(int id ,QString saycontent);
    void slot_AddFriendBytel(QString tel);
    void slot_DelFriendBytel(QString tel);

    //群聊
    void slot_ClickGroupItem(int id);
    void slot_sendGroupContent(int id ,QString saycontent);
    void slot_CreateGroup(QString name);
    void slot_AddGroup(QString id);
    void slot_DelGroup(QString id);
    void slot_sendfile(int id ,QString path);
    //网络处理
    //处理接收数据槽函数
    void slot_ReadyData( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealRegisterRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealLoginRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileInfo( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileHeadRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileContentRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealUploadFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileContentRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealAddFolderRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealQuickUpdateRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealDeleteFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealShareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealMyshareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGetshareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFolderHeadRq( unsigned int lSendIP , char* buf , int nlen );

    ///////////////////IM/////////////////////
    void slot_dealFriendInfoRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealChatRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealChatRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealAddFriendRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealAddFriendRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealMessageRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealDelFriendRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealDelFriendRs( unsigned int lSendIP , char* buf , int nlen );
    //void slot_dealOffLineRq( unsigned int lSendIP , char* buf , int nlen );
    //群聊
    void slot_dealGroupInfoRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGroupBroadcastRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGroupOfflineRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGroupAddRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGroupAddRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGroupDelRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGroupDelRs( unsigned int lSendIP , char* buf , int nlen );
    //文件发送
    void slot_dealIMSendFileRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealIMSendFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealIMSendFileContentRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealIMSendFileContentRs( unsigned int lSendIP , char* buf , int nlen );
signals:
    void SIG_updateFileProgress(int ,int);
    void SIG_insertComplete(FileInfo info);

    void SIG_updateUploadFileProgress(int ,int);
    void SIG_uploadFileinsertComplete(FileInfo info);

private:
    void SendData(char* buf , int nlen);
    void setConfig();//设置配置文件
    //QString->GB2312


private:
    explicit CKernel(QObject *parent = 0);
    ~CKernel();
    CKernel(const CKernel & kernel){}
    CKernel & operator = (const CKernel & kernel){
        return *this;
    }

    //创建协议映射表函数
    void setNetMap();

    //成员属性 网络对象 ui对象
    static CKernel * kernel;
    MainDialog *m_mainDialog;//主窗口类
    INetMediator * m_TcpClient;
    LoginDialog * m_loginDialog;
    PFUN m_netPackMap[_DEF_PROTOCOL_COUNT];

    //当前所在路径
    QString m_curDir;

    QString m_name; // 昵称
    int m_id; //uid


    std::map<int,FileInfo> m_mapFileIdToFileInfo;

    std::map<std::string,FileInfo> m_mapFileMD5ToFileInfo;

    QString m_sysPath; //这个要在那个设置配置文件函数中写，（.ini文件中写）

    QString m_ip;
    int m_port;
    int m_quit;//程序退出标志位

    //IM
    MyChat * m_mychat;
    std::map<int,Useritem*> m_mapIdtoUseritem;
    std::map<int,chatdialog*> m_mapIdtoChatdialog;
    std::map<int,Useritem*> m_mapGroupIdtoUseritem;
    std::map<int,chatdialog*> m_mapGroupIdtoChatdialog;
    int m_messageId;
    int m_gmessageId;

    QString m_IMfilePath;
    //存发送的文件信息，key是接收方的id，一次只能发送一个文件
    std::map<int,FileInfo> m_mapRecvidtoFileinfo;

    std::map<int,FileInfo> m_mapSendidtoFileinfo;
};

//

#endif // CKERNEL_H
