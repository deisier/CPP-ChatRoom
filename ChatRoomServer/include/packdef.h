#ifndef _PACKDEF_H
#define _PACKDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "err_str.h"
#include <malloc.h>

#include<iostream>
#include<map>
#include<list>


//边界值
#define _DEF_SIZE 45
#define _DEF_BUFFERSIZE 1000
#define _DEF_PORT 8004
#define _DEF_SERVERIP "0.0.0.0"  //TODO

/*-------------数据库信息-----------------*/
#define _DEF_DB_NAME    "NetDisk"
#define _DEF_DB_IP      "localhost"
#define _DEF_DB_USER    "root"
#define _DEF_DB_PWD     "123"
/*--------------------------------------*/

#define TRUE true
#define FALSE false


#define _DEF_LISTEN 128
#define _DEF_EPOLLSIZE 4096
#define _DEF_IPSIZE 16
#define _DEF_COUNT 10
#define _DEF_TIMEOUT 10
#define _DEF_SQLIEN 400



#define BOOL bool
#define DEF_PACK_BASE  (10000)
#define DEF_PACK_COUNT  (100)

#define _DEF_BUFFER  ( 4096 )
#define _DEF_CONTENT_SIZE	(1024)
#define _MAX_SIZE	(40)

//自定义协议   先写协议头 再写协议结构
//登录 注册 获取好友信息 添加好友 聊天 发文件 下线请求
#define _DEF_PROTOCOL_BASE	DEF_PACK_BASE
#define _DEF_PROTOCOL_COUNT DEF_PACK_COUNT

//注册
#define _DEF_PACK_REGISTER_RQ	(_DEF_PROTOCOL_BASE + 0 )
#define _DEF_PACK_REGISTER_RS	(_DEF_PROTOCOL_BASE + 1 )
//登录
#define _DEF_PACK_LOGIN_RQ	(_DEF_PROTOCOL_BASE + 2 )
#define _DEF_PACK_LOGIN_RS	(_DEF_PROTOCOL_BASE + 3 )


//返回的结果
//注册请求的结果
#define tel_is_exist		(0)
#define name_is_exist		(1)
#define register_success	(2)
//登录请求的结果
#define user_not_exist		(0)
#define password_error		(1)
#define login_success		(2)


typedef int PackType;

//协议结构
//注册
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ():type(_DEF_PACK_REGISTER_RQ)
    {
        memset( tel  , 0, sizeof(tel));
        memset( name  , 0, sizeof(name));
        memset( password , 0, sizeof(password) );
    }
    //需要手机号码 , 密码, 昵称
    PackType type;
    char tel[_MAX_SIZE];
    char name[_MAX_SIZE];
    char password[_MAX_SIZE];

}STRU_REGISTER_RQ;

typedef struct STRU_REGISTER_RS
{
    //回复结果
    STRU_REGISTER_RS(): type(_DEF_PACK_REGISTER_RS) , result(register_success)
    {
    }
    PackType type;
    int result;

}STRU_REGISTER_RS;

//登录
typedef struct STRU_LOGIN_RQ
{
    //登录需要: 手机号 密码
    STRU_LOGIN_RQ():type(_DEF_PACK_LOGIN_RQ)
    {
        memset( tel , 0, sizeof(tel) );
        memset( password , 0, sizeof(password) );
    }
    PackType type;
    char tel[_MAX_SIZE];
    char password[_MAX_SIZE];

}STRU_LOGIN_RQ;

typedef struct STRU_LOGIN_RS
{
    // 需要 结果 , 用户的id
    STRU_LOGIN_RS(): type(_DEF_PACK_LOGIN_RS) , result(login_success),userid(0)
    {
        memset(name,0,_MAX_SIZE);
    }
    PackType type;
    int result;
    int userid;
    char name[_MAX_SIZE];

}STRU_LOGIN_RS;

struct UserInfo{

    int clientfd;
    int userId;
    std::string name;
};

#define _MAX_PATH_SIZE	(260) //文件路径最大长度
//用户文件列表请求
#define _DEF_PACK_FILE_LIST_RQ	(_DEF_PROTOCOL_BASE + 4 )
//文件信息
#define _DEF_PACK_FILE_INFO	(_DEF_PROTOCOL_BASE + 5 )


//文件列表请求
typedef struct STRU_FILE_LIST_RQ
{
    // 需要 结果 , 用户的id
    STRU_FILE_LIST_RQ(): type(_DEF_PACK_FILE_LIST_RQ) , userId(0)
    {
        memset(dir,0,_MAX_SIZE);
    }
    PackType type;
    int userId;
    char dir[_MAX_SIZE];

}STRU_FILE_LIST_RQ;

//文件信息
typedef struct STRU_FILE_INFO
{
    // 需要 结果 , 用户的id
    STRU_FILE_INFO(): type(_DEF_PACK_FILE_INFO) , userId(0)
    {
        memset(dir,0,sizeof(dir));
        memset(fileName,0,sizeof(fileName));
        memset(uploadTIme,0,sizeof(uploadTIme));
        memset(md5,0,sizeof(md5));
        memset(fileType,0,sizeof(fileType));
    }
    PackType type;
    int userId;
    int fileId;
    int size;//约定 小型网盘，上传不能超过2gb（可以改成int64来上传更大）
    // 2 147 483 647 2gb

    char fileName[_MAX_PATH_SIZE];
    char uploadTIme[_MAX_SIZE];// "2022-02-28 15:12:45"

    char dir[_MAX_PATH_SIZE]; //用于防止当双击进别的文件夹时，上一个文件夹中没来的及插入的文件夹，插入到当前文件夹
    char md5[_MAX_SIZE];
    char fileType[_MAX_SIZE]; //"file" "dir" (文件 or 文件夹)


}STRU_FILE_INFO;




//文件下载
//文件下载请求
#define _DEF_PACK_FILE_DOWNLOAD_RQ      (_DEF_PROTOCOL_BASE + 6 )
//文件下载回复
#define _DEF_PACK_FILE_DOWNLOAD_RS      (_DEF_PROTOCOL_BASE + 7 )
//文件头请求
#define _DEF_PACK_FILE_HEAD_RQ          (_DEF_PROTOCOL_BASE + 8 )
//文件头回复
#define _DEF_PACK_FILE_HEAD_RS          (_DEF_PROTOCOL_BASE + 9 )
//文件内容请求
#define _DEF_PACK_FILE_CONTENT_RQ       (_DEF_PROTOCOL_BASE + 10 )
//文件内容回复
#define _DEF_PACK_FILE_CONTENT_RS       (_DEF_PROTOCOL_BASE + 11 )


//文件下载请求
struct STRU_DOWNLOAD_RQ
{
    STRU_DOWNLOAD_RQ(): type(_DEF_PACK_FILE_DOWNLOAD_RQ)
      , userid(0), fileid(0){
        memset( dir , 0, sizeof(dir) );
    }
    PackType type;
    int userid;
    int fileid;
    char dir[_MAX_PATH_SIZE];//路
};
//文件下载回复
struct STRU_DOWNLOAD_RS
{
    STRU_DOWNLOAD_RS(): type(_DEF_PACK_FILE_DOWNLOAD_RS)
      , userid(0), fileid(0),result(1){

    }
    PackType type;
    int userid;
    int fileid;
    int result;
};
//文件头请求
struct STRU_FILE_HEAD_RQ
{
    STRU_FILE_HEAD_RQ():type(_DEF_PACK_FILE_HEAD_RQ)
      ,fileid(0),size(0){
        memset( fileName , 0, sizeof(fileName) );
        memset( dir , 0, sizeof(dir) );
        memset( md5 , 0, sizeof(md5) );
        memset( fileType , 0, sizeof(fileType) );
    }
    PackType type;
    int fileid;
    char fileName[_MAX_PATH_SIZE];
    int size;//大小
    char dir[_MAX_PATH_SIZE];//路径
    char md5[_MAX_SIZE];
    char fileType[_MAX_SIZE];//文件类型
};
///////////////////////文件夹下载////////////////////////////
//文件夹 头请求
#define _DEF_PACK_FOLDER_HEAD_RQ          (_DEF_PROTOCOL_BASE + 25 )
//文件头请求
struct STRU_FOLDER_HEAD_RQ
{
    STRU_FOLDER_HEAD_RQ():type(_DEF_PACK_FOLDER_HEAD_RQ)
      ,fileid(0){
        memset( fileName , 0, sizeof(fileName) );
        memset( dir , 0, sizeof(dir) );
    }
    PackType type;
    int fileid;
    char fileName[_MAX_PATH_SIZE];
    char dir[_MAX_PATH_SIZE];//路径
};
//文件头回复
struct STRU_FILE_HEAD_RS
{
    STRU_FILE_HEAD_RS(): type(_DEF_PACK_FILE_HEAD_RS)
      , userid(0), fileid(0),result(1){

    }
    PackType type;
    int userid;
    int fileid;
    int result;
};
//文件内容请求
struct STRU_FILE_CONTENT_RQ
{
    STRU_FILE_CONTENT_RQ():type(_DEF_PACK_FILE_CONTENT_RQ),
        userid(0),fileid(0),len(0){
        memset( content , 0 , sizeof(content));
    }
    PackType type;
    int userid;//1)userid
    int fileid;//2)fileid
    char content[_DEF_BUFFER];//3)文件内容
    int len;//4)长度
};
//文件内容回复
struct STRU_FILE_CONTENT_RS
{
    STRU_FILE_CONTENT_RS():type(_DEF_PACK_FILE_CONTENT_RS),
        userid(0),fileid(0),result(1),len(0){

    }
    PackType type;
    int userid;//1)userid
    int fileid;//2)fileid
    int result;//3)result
    int len;

};


struct FileInfo{


    FileInfo():fileid(0),size(0),filefd(0),pos(0){

        }

    int fileid;
    std::string name;
    std::string dir;
    std::string time;
    int size;
    std::string md5;
    std::string type;
    std::string absolutePath;

    int pos;//传到那里了



    //文件指针
    int filefd;

};



///////////////文件上传///////////
//文件内容请求
#define _DEF_PACK_UPLOAD_FILE_RQ       (_DEF_PROTOCOL_BASE + 12 )
//文件内容回复
#define _DEF_PACK_UPLOAD_FILE_RS       (_DEF_PROTOCOL_BASE + 13 )


//上传文件头请求
struct STRU_UPLOAD_FILE_HEAD_RQ
{
    STRU_UPLOAD_FILE_HEAD_RQ():type(_DEF_PACK_UPLOAD_FILE_RQ)
      ,userid(0),size(0){
        memset( fileName , 0, sizeof(fileName) );
        memset( dir , 0, sizeof(dir) );
        memset( md5 , 0, sizeof(md5) );
        memset( fileType , 0, sizeof(fileType) );
        memset( time , 0, sizeof(time) );
    }
    PackType type;
    int userid;
    char fileName[_MAX_PATH_SIZE];
    int size;//大小
    char dir[_MAX_PATH_SIZE];//路径
    char md5[_MAX_SIZE];
    char fileType[_MAX_SIZE];//文件类型
    char time[_MAX_SIZE]; //上传时间
};
//上传文件头回复
struct STRU_UPLOAD_FILE_HEAD_RS
{
    STRU_UPLOAD_FILE_HEAD_RS(): type(_DEF_PACK_UPLOAD_FILE_RS)
      , userid(0), fileid(0),result(1){
        memset( md5 , 0, sizeof(md5) );
    }
    PackType type;
    int userid;
    int fileid;
    int result;
    char md5[_MAX_SIZE];
};



///////////////新建文件夹///////////
//文件内容请求
#define _DEF_PACK_ADD_FOLDER_RQ       (_DEF_PROTOCOL_BASE + 14 )
//文件内容回复
#define _DEF_PACK_ADD_FOLDER_RS       (_DEF_PROTOCOL_BASE + 15 )


//新建文件夹 文件头请求
struct STRU_ADD_FOLDER_RQ
{
    STRU_ADD_FOLDER_RQ():type(_DEF_PACK_ADD_FOLDER_RQ)
      ,userid(0){
        memset( fileName , 0, sizeof(fileName) );
        memset( dir , 0, sizeof(dir) );
        memset( fileType , 0, sizeof(fileType) );
        memset( time , 0, sizeof(time) );
    }
    PackType type;
    int userid;
    char fileName[_MAX_PATH_SIZE];
    char dir[_MAX_PATH_SIZE];//路径
    char fileType[_MAX_SIZE];//文件类型
    char time[_MAX_SIZE]; //上传时间
};
//新建文件夹 文件头回复
struct STRU_ADD_FOLDER_RS
{
    STRU_ADD_FOLDER_RS(): type(_DEF_PACK_ADD_FOLDER_RS)
      ,result(1){

    }
    PackType type;
    int result;
};


///////////////快传//////////////////////////

//快传回复
#define _DEF_PACK_QUICK_UPLOAD_RS       (_DEF_PROTOCOL_BASE + 16 )

struct STRU_QUICK_UPLOAD_RS
{
    STRU_QUICK_UPLOAD_RS(): type(_DEF_PACK_QUICK_UPLOAD_RS)
      ,result(1){
       memset( md5 , 0, sizeof(md5) );
    }
    PackType type;
    char md5[_MAX_SIZE];
    int result;
};


//////////////////删除文件///////////////////
//删除文件请求
#define _DEF_PACK_DELETE_FILE_RQ       (_DEF_PROTOCOL_BASE + 17 )
//删除文件回复
#define _DEF_PACK_DELETE_FILE_RS       (_DEF_PROTOCOL_BASE + 18 )
//删除文件请求 : 某人 删除某路径下的 某文件
//struct STRU_DELETE_FILE_RQ
//{
//    STRU_DELETE_FILE_RQ():type(_DEF_PACK_DELETE_FILE_RQ)
//      ,userid(0),fileid(0) {
//        memset( dir , 0 , sizeof(dir) );
//    }
//    PackType type;
//    int userid;
//    int fileid;
//    char dir[_MAX_PATH_SIZE];
//};

struct STRU_DELETE_FILE_RQ
{
    void init()
    {
        type = _DEF_PACK_DELETE_FILE_RQ;
        userid = 0;
        fileCount = 0;
        memset( dir , 0 , sizeof(dir) );
    }
    PackType type;
    int userid;
    char dir[_MAX_PATH_SIZE];
    int fileCount;
    int fileidArray[];
};

//删除文件回复
struct STRU_DELETE_FILE_RS
{
    STRU_DELETE_FILE_RS():type(_DEF_PACK_DELETE_FILE_RS)
      ,result(1){
        memset( dir , 0 , sizeof(dir) );
    }
    PackType type;
    int result;
    char dir[_MAX_PATH_SIZE];
};


//////////////////分享文件///////////////////
//分享文件请求
#define _DEF_PACK_SHARE_FILE_RQ       (_DEF_PROTOCOL_BASE + 19 )
//分享回复
#define _DEF_PACK_SHARE_FILE_RS       (_DEF_PROTOCOL_BASE + 20 )

struct STRU_SHARE_FILE_RQ
{
    void init(){
        type = _DEF_PACK_SHARE_FILE_RQ; userid = 0; itemCount = 0;
        memset( dir , 0 , sizeof(dir));
        memset( shareTime , 0 , sizeof(shareTime));
    }
    PackType type;
    int userid;
    char dir[_MAX_PATH_SIZE];
    char shareTime[_MAX_PATH_SIZE];
    int itemCount;
    int fileidArray[];
};


struct STRU_SHARE_FILE_RS
{
    STRU_SHARE_FILE_RS():type(_DEF_PACK_SHARE_FILE_RS){
        result = 1;
    }
    PackType type;
    int result;
};

//获取分享请求
#define _DEF_PACK_GET_SHARE_RQ       (_DEF_PROTOCOL_BASE + 21 )
//获取分享回复
#define _DEF_PACK_GET_SHARE_RS       (_DEF_PROTOCOL_BASE + 22 )
//获取自己的分享请求
#define _DEF_PACK_MY_SHARE_RQ        (_DEF_PROTOCOL_BASE + 23 )
//获取自己的分享回复
#define _DEF_PACK_MY_SHARE_RS        (_DEF_PROTOCOL_BASE + 24 )

//获取分享
struct STRU_GET_SHARE_RQ
{
    STRU_GET_SHARE_RQ():type(_DEF_PACK_GET_SHARE_RQ)
      ,userid(0), shareLink(0){
        memset(dir , 0 , sizeof(dir));
    }
    PackType type;
    int userid;
    int shareLink; // 8位 首位是1-9 数字
    char dir[_MAX_PATH_SIZE];
    //直接加载这个路径下面
};

//获取分享回复 :收到刷新
struct STRU_GET_SHARE_RS
{
    STRU_GET_SHARE_RS():type(_DEF_PACK_GET_SHARE_RS)
      ,result(0) {
        memset(dir , 0 , sizeof(dir));
    }
    PackType type;
    int result;
    char dir[_MAX_PATH_SIZE];
};

//获取自己的分享
struct STRU_MY_SHARE_RQ
{
    STRU_MY_SHARE_RQ():type(_DEF_PACK_MY_SHARE_RQ), userid(0){

    }
    PackType type;
    int userid;
    //考虑加入时间 查询半个月之内的分享
};

struct MY_SHARE_FILE
{
    char name[_MAX_PATH_SIZE];
    int size;
    char time[_MAX_SIZE];
    int shareLink;
};

//获取自己的分享
struct STRU_MY_SHARE_RS
{
    void init(){
        type = _DEF_PACK_MY_SHARE_RS;
        itemCount = 0;
    }
    PackType type;
    int itemCount;
    MY_SHARE_FILE items[];
};


////////// IM 通信 ///////////
#define _DEF_MAX_SIZE   (100)

//获取自己的分享回复
#define _DEF_PACK_FRIEND_INFO        (_DEF_PROTOCOL_BASE + 25 )

//好友状态  这里注意别谢写串了
#define status_online		(0)
#define status_offline		(1)

//好友信息
struct STRU_FRIEND_INFO{
    STRU_FRIEND_INFO():type(_DEF_PACK_FRIEND_INFO),userId(0),state(status_online){
        memset(icon,0,_DEF_MAX_SIZE);
        memset(tel,0,_DEF_MAX_SIZE);
        memset(name,0,_DEF_MAX_SIZE);
        memset(feeling,0,_DEF_MAX_SIZE);
    }

    //协议头，手机号，昵称，密码
    PackType type;
    int userId;
    char icon[_DEF_MAX_SIZE];
    int state;
    char tel[_DEF_MAX_SIZE];
    char name[_DEF_MAX_SIZE];
    char feeling[_DEF_MAX_SIZE];
};

//聊天
#define _DEF_PACK_CHAT_RQ	(_DEF_PROTOCOL_BASE + 26)
#define _DEF_PACK_CHAT_RS	(_DEF_PROTOCOL_BASE + 27)
#define _DEF_content_SIZE (1000)
//聊天请求
struct STRU_CHAT_RQ{
    STRU_CHAT_RQ():type(_DEF_PACK_CHAT_RQ),friendId(0),userId(0){
        memset(content,0,_DEF_content_SIZE);
        memset(nowtime,0,_DEF_content_SIZE);
    }

    //协议头，聊天内容，发送方id，接收方id
    PackType type;
    int userId;
    int friendId;
    char content[_DEF_content_SIZE];
    char nowtime[_DEF_content_SIZE];
};
//聊天回复
struct STRU_CHAT_RS{
    STRU_CHAT_RS():type(_DEF_PACK_CHAT_RS),result(register_success),friendId(0),userId(0){
    }

    //协议头，注册结果，发送方id，接收方id
    PackType type;
    int userId;
    int friendId;
    int result;
};

//添加好友
#define _DEF_PACK_ADDFRIEND_RQ	(_DEF_PROTOCOL_BASE + 28)
#define _DEF_PACK_ADDFRIEND_RS	(_DEF_PROTOCOL_BASE + 29)

//添加好友的结果
#define no_this_user		(0)
#define user_refuse			(1)
#define user_offline		(2)
#define add_success			(3)

//添加好友请求
struct STRU_ADD_FRIEND_RQ{
    STRU_ADD_FRIEND_RQ():type(_DEF_PACK_ADDFRIEND_RQ),userId(0){
        memset(usertel,0,_DEF_MAX_SIZE);
        memset(friendtel,0,_DEF_MAX_SIZE);
    }

    //协议头，用户1，用户1昵称，用户2昵称
    PackType type;
    int userId;
    char usertel[_DEF_MAX_SIZE];
    char friendtel[_DEF_MAX_SIZE];
};
//添加好友回复
struct STRU_ADD_FRIEND_RS{
    STRU_ADD_FRIEND_RS():type(_DEF_PACK_ADDFRIEND_RS),result(add_success),friendId(0),userId(0){
        memset(friendName,0,_DEF_MAX_SIZE);
    }

    //协议头，注册结果
    PackType type;
    int userId;
    int friendId;
    char friendName[_DEF_MAX_SIZE];
    int result;
};

//离线
#define _DEF_PACK_OFFLINE_RQ	(_DEF_PROTOCOL_BASE + 30)

//离线
struct STRU_OFFLINE_RQ{
    STRU_OFFLINE_RQ():type(_DEF_PACK_OFFLINE_RQ),userId(0){
    }

    //协议头，注册结果
    PackType type;
    int userId;
};


//消息记录
#define _DEF_PACK_MESSAGES_RQ	(_DEF_PROTOCOL_BASE + 31)

//消息记录
struct STRU_MESSAGES_RQ{
    STRU_MESSAGES_RQ():type(_DEF_PACK_MESSAGES_RQ),id(0),sendId(0),recvId(0){
        memset(content,0,_DEF_content_SIZE);
        memset(nowtime,0,_DEF_content_SIZE);
    }

    //协议头，注册结果
    PackType type;
    int id;
    int sendId;
    int recvId;
    char content[_DEF_content_SIZE];
    char nowtime[_DEF_content_SIZE];
};


//删除好友
#define _DEF_PACK_DELFRIEND_RQ	(_DEF_PROTOCOL_BASE + 32)
#define _DEF_PACK_DELFRIEND_RS	(_DEF_PROTOCOL_BASE + 33)

//删除好友的结果
#define del_success 		(0)
#define del_fail			(1)


//删除好友请求
struct STRU_DEL_FRIEND_RQ{
    STRU_DEL_FRIEND_RQ():type(_DEF_PACK_DELFRIEND_RQ),userId(0){
        memset(usertel,0,_DEF_MAX_SIZE);
        memset(friendtel,0,_DEF_MAX_SIZE);
    }

    //协议头，用户1，用户1昵称，用户2昵称
    PackType type;
    int userId;
    char usertel[_DEF_MAX_SIZE];
    char friendtel[_DEF_MAX_SIZE];
};
//删除好友回复
struct STRU_DEL_FRIEND_RS{
    STRU_DEL_FRIEND_RS():type(_DEF_PACK_DELFRIEND_RS),result(del_success){
    }

    //协议头，注册结果
    PackType type;
    int result;
};


////////////////////////群聊/////////////////////////////////

//添加群列表请求
#define _DEF_PACK_GROUP_INFO	(_DEF_PROTOCOL_BASE + 34)
//好友信息
struct STRU_GROUP_INFO{
    STRU_GROUP_INFO():type(_DEF_PACK_GROUP_INFO),gid(0),identity(0){
        memset(gname,0,_DEF_MAX_SIZE);
    }

    //协议头，手机号，昵称，密码
    PackType type;
    int gid;
    int identity;
    char gname[_DEF_MAX_SIZE];

};
//群聊消息记录
#define _DEF_PACK_GROUP_CHAT_RQ	(_DEF_PROTOCOL_BASE + 35)

//群聊消息记录
struct STRU_GROUP_CHAT_RQ{
    STRU_GROUP_CHAT_RQ():type(_DEF_PACK_GROUP_CHAT_RQ),sendId(0),gId(0){
        memset(content,0,_DEF_content_SIZE);
        memset(nowtime,0,_DEF_content_SIZE);
    }

    //协议头，注册结果
    PackType type;
    int sendId;
    int gId;
    char content[_DEF_content_SIZE];
    char nowtime[_DEF_content_SIZE];
};

#define _DEF_PACK_GROUP_BROADCAST_RQ	(_DEF_PROTOCOL_BASE + 36)

//群聊消息记录
struct STRU_GROUP_BROADCAST_CHAT_RQ{
    STRU_GROUP_BROADCAST_CHAT_RQ():type(_DEF_PACK_GROUP_BROADCAST_RQ),gId(0),identity(0){
        memset(name,0,_DEF_content_SIZE);
        memset(content,0,_DEF_content_SIZE);
        memset(nowtime,0,_DEF_content_SIZE);
    }

    //协议头，注册结果
    PackType type;
    int gId;
    int identity;
    char name[_DEF_content_SIZE];
    char content[_DEF_content_SIZE];
    char nowtime[_DEF_content_SIZE];
};

#define _DEF_PACK_GROUP_OFFLINE_RQ	(_DEF_PROTOCOL_BASE + 37)

//群聊消息记录
struct STRU_GROUP_OFFLINE_CHAT_RQ{
    STRU_GROUP_OFFLINE_CHAT_RQ():type(_DEF_PACK_GROUP_OFFLINE_RQ),id(0),gId(0),uid(0),identity(0){
        memset(name,0,_DEF_content_SIZE);
        memset(content,0,_DEF_content_SIZE);
        memset(nowtime,0,_DEF_content_SIZE);
    }

    //协议头，注册结果
    PackType type;
    int id;
    int gId;
    int uid;
    int identity;
    char name[_DEF_content_SIZE];
    char content[_DEF_content_SIZE];
    char nowtime[_DEF_content_SIZE];
};


#define _DEF_PACK_CREATE_RQ	(_DEF_PROTOCOL_BASE + 38)

//群聊创建
struct STRU_GROUP_CREATE_RQ{
    STRU_GROUP_CREATE_RQ():type(_DEF_PACK_CREATE_RQ),uid(0){
        memset(name,0,_DEF_content_SIZE);
    }

    //协议头，注册结果
    PackType type;
    int uid;
    char name[_DEF_content_SIZE];
};


#define _DEF_PACK_GROUP_ADD_RQ	(_DEF_PROTOCOL_BASE + 39)
#define _DEF_PACK_GROUP_ADD_RS	(_DEF_PROTOCOL_BASE + 40)
//群聊添加请求
struct STRU_GROUP_GROUP_ADD_RQ{
    STRU_GROUP_GROUP_ADD_RQ():type(_DEF_PACK_GROUP_ADD_RQ),uid(0),gid(0){
        memset(tel,0,sizeof(tel));
    }

    //协议头，注册结果
    PackType type;
    int uid;
    int gid;
    char tel[_MAX_SIZE];

};
//添加好友的结果
#define no_this_group		(0)
#define user_refuse			(1)
#define user_offline		(2)
#define add_success			(3)
//群聊添加回复
struct STRU_GROUP_GROUP_ADD_RS{
    STRU_GROUP_GROUP_ADD_RS():type(_DEF_PACK_GROUP_ADD_RS),gid(0),senderid(0),result(3){
    }

    //协议头，注册结果
    PackType type;
    int gid;
    int senderid;
    int result;
};

//删除群聊
#define _DEF_PACK_DELGROUP_RQ	(_DEF_PROTOCOL_BASE + 41)
#define _DEF_PACK_DELGROUP_RS	(_DEF_PROTOCOL_BASE + 42)

//删除群聊的结果
#define del_success 		(0)
#define del_fail			(1)


//删除群聊请求
struct STRU_DEL_GROUP_RQ{
    STRU_DEL_GROUP_RQ():type(_DEF_PACK_DELGROUP_RQ),gid(0),uid(0),identity(0){
    }

    //协议头
    PackType type;
    int gid;
    int uid;
    int identity;
};
//删除群聊回复
struct STRU_DEL_GROUP_RS{
    STRU_DEL_GROUP_RS():type(_DEF_PACK_DELGROUP_RS),result(del_success){
    }

    //协议头，结果
    PackType type;
    int result;
};
///////////////文件发送///////////
//文件内容请求
#define _DEF_PACK_IM_SEND_FILE_RQ       (_DEF_PROTOCOL_BASE + 43 )
//文件内容回复
#define _DEF_PACK_IM_SEND_FILE_RS       (_DEF_PROTOCOL_BASE + 44 )


//上传文件头请求
struct STRU_IM_SEND_FILE_HEAD_RQ
{
    STRU_IM_SEND_FILE_HEAD_RQ():type(_DEF_PACK_IM_SEND_FILE_RQ)
      ,sendid(0),recvid(0),size(0){
        memset( fileName , 0, sizeof(fileName) );
    }
    PackType type;
    int sendid;
    int recvid;
    char fileName[_MAX_PATH_SIZE];
    int size;//大小
};
//上传文件头回复
struct STRU_IM_SEND_FILE_HEAD_RS
{
    STRU_IM_SEND_FILE_HEAD_RS(): type(_DEF_PACK_IM_SEND_FILE_RS)
      , sendid(0), recvid(0),result(1){
    }
    PackType type;
    int sendid;
    int recvid;
    int result;
};

//文件内容请求
#define _DEF_PACK_IM_SEND_FILE_CONTENT_RQ       (_DEF_PROTOCOL_BASE + 45 )
//文件内容回复
#define _DEF_PACK_IM_SEND_FILE_CONTENT_RS       (_DEF_PROTOCOL_BASE + 46 )


//上传文件头请求
struct STRU_IM_SEND_FILE_CONTENT_RQ
{
    STRU_IM_SEND_FILE_CONTENT_RQ():type(_DEF_PACK_IM_SEND_FILE_CONTENT_RQ)
      ,sendid(0),recvid(0),len(0){
        memset( content , 0, sizeof(content) );
    }
    PackType type;
    int sendid;
    int recvid;
    char content[_DEF_BUFFER];//3)文件内容
    int len;//大小
};
//上传文件头回复
struct STRU_IM_SEND_FILE_CONTENT_RS
{
    STRU_IM_SEND_FILE_CONTENT_RS(): type(_DEF_PACK_IM_SEND_FILE_CONTENT_RS)
      , sendid(0), recvid(0),result(1),len(0){
    }
    PackType type;
    int sendid;
    int recvid;
    int result;
    int len;
};












#endif


