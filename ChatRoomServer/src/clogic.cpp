#include "clogic.h"

void CLogic::setNetPackMap()
{
    NetPackMap(_DEF_PACK_REGISTER_RQ)    = &CLogic::RegisterRq;
    NetPackMap(_DEF_PACK_LOGIN_RQ)       = &CLogic::LoginRq;
    NetPackMap(_DEF_PACK_FILE_LIST_RQ)   = &CLogic::UserFileListRq;
    NetPackMap(_DEF_PACK_FILE_DOWNLOAD_RQ)   = &CLogic:: DownloadFileRq;
    NetPackMap(_DEF_PACK_FILE_HEAD_RS)   = &CLogic:: FileHeadRs;
    NetPackMap(_DEF_PACK_FILE_CONTENT_RS)   = &CLogic:: FileContentRs;
    NetPackMap(_DEF_PACK_UPLOAD_FILE_RQ)   = &CLogic::UploadFileRq;
    NetPackMap(_DEF_PACK_FILE_CONTENT_RQ)   = &CLogic::FileContentRq;
    NetPackMap(_DEF_PACK_ADD_FOLDER_RQ)   = &CLogic::AddFolderRq;
    NetPackMap(_DEF_PACK_DELETE_FILE_RQ)   = &CLogic::DeleteFileRq;
    NetPackMap(_DEF_PACK_MY_SHARE_RQ)   = &CLogic::ShareFileListRq;
    NetPackMap(_DEF_PACK_SHARE_FILE_RQ)   = &CLogic::ShareFileRq;
    NetPackMap(_DEF_PACK_GET_SHARE_RQ)   = &CLogic::GetShareFileRq;
    NetPackMap(_DEF_PACK_CHAT_RQ)   = &CLogic::ChatRq;
    NetPackMap(_DEF_PACK_ADDFRIEND_RQ)   = &CLogic::AddFriendRq;
    NetPackMap(_DEF_PACK_ADDFRIEND_RS)   = &CLogic::AddFriendRs;
    NetPackMap(_DEF_PACK_OFFLINE_RQ)   = &CLogic::OffLineRq;
    NetPackMap(_DEF_PACK_DELFRIEND_RQ)   = &CLogic::DelFriendRq;
    NetPackMap(_DEF_PACK_GROUP_CHAT_RQ)   = &CLogic::GroupChatRq;
    NetPackMap(_DEF_PACK_CREATE_RQ)   = &CLogic::GroupCreateRq;
    NetPackMap(_DEF_PACK_GROUP_ADD_RQ)   = &CLogic::GroupAddRq;
    NetPackMap(_DEF_PACK_GROUP_ADD_RS)   = &CLogic::GroupAddRs;
    NetPackMap(_DEF_PACK_DELGROUP_RQ)   = &CLogic::GroupDelRq;
    NetPackMap(_DEF_PACK_IM_SEND_FILE_RQ)   = &CLogic::ImSendFileRq;
    NetPackMap(_DEF_PACK_IM_SEND_FILE_RS)   = &CLogic::ImSendFileRs;
    NetPackMap(_DEF_PACK_IM_SEND_FILE_CONTENT_RQ)   = &CLogic::ImSendFileContentRq;
    NetPackMap(_DEF_PACK_IM_SEND_FILE_CONTENT_RS)   = &CLogic::ImSendFileContentRs;
}
//返回的结果
//注册请求的结果
//#define tel_is_exist		(0)
//#define name_is_exist		(1)
//#define register_success	(2)

//存储路径
#define DEF_PATH "/home/x/NetDisk/"
//注册
void CLogic::RegisterRq(sock_fd clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d RegisterRq\n", clientfd);
    //拆包，tel password name
    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;
    int uid =0;
    //查数据库 判断tel是否存在
    char sqlbuf[1000] ={};
    sprintf(sqlbuf,"select u_tel from t_user where u_tel = '%s'",rq->tel);
    list<string> lisRes;
    bool res = m_sql->SelectMysql(sqlbuf,1,lisRes);
    if( !res ){
        printf("sql select error:%s\n",sqlbuf);
        return;
    }

    //存在 -》 返回 tel_is_exist
    if(lisRes.size() > 0){
        rs.result = tel_is_exist;

    }else{
        //查数据库 判断name是否存在
        sprintf(sqlbuf,"select u_name from t_user where u_name = '%s'",rq->name);
        lisRes.clear();
        res = m_sql->SelectMysql(sqlbuf,1,lisRes);
        if( !res ){
            printf("sql select error:%s\n",sqlbuf);
            return;
        }
        //存在 -》 返回 name_is_exist
        if(lisRes.size() > 0){
            rs.result = name_is_exist;
        }else{
            //注册成功，将注册信息插入到数据库
            sprintf(sqlbuf,"insert into t_user ( u_tel , u_password , u_name ) values ( '%s' , '%s' , '%s' );",rq->tel,rq->password,rq->name);
            res = m_sql->UpdataMysql(sqlbuf);
            if( !res ){
                printf("sql insert error:%s\n",sqlbuf);
                return;
            }
            //获取用户id
            sprintf(sqlbuf,"select u_id from t_user where u_tel = '%s'",rq->tel);
            lisRes.clear();
            res = m_sql->SelectMysql(sqlbuf,1,lisRes);
            if(lisRes.size() > 0){
                uid = atoi(lisRes.front().c_str());
            }
            char strPath[260] ={};
            //获取用户路径
            sprintf(strPath,"%s%d",DEF_PATH,uid);
            //--> 网盘 创建存储的路径 /home/x/NetDisk/user_id/
            umask(0);
            mkdir(strPath,0777);
            rs.result = register_success;

            //将自己添加到好友表，用于更新自己数据
            sprintf(sqlbuf,"insert into t_friend values(%d,%d);",uid,uid);
            m_sql->UpdataMysql(sqlbuf);

        }
    }
    //发送回复包
    SendData(clientfd,(char *)&rs,sizeof(rs));
}

//登录请求的结果
//#define user_not_exist		(0)
//#define password_error		(1)
//#define login_success		(2)

//登录
void CLogic::LoginRq(sock_fd clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    //拆包 tel password
    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;
    //查数据库 获取password id name
    char sqlStr[1000] ={};
    list<string> lstRes;
    sprintf(sqlStr,"select u_password , u_id , u_name from t_user where u_tel = '%s'",rq->tel);
    m_sql->SelectMysql(sqlStr,3,lstRes);
    //查不到 返回 user_not_exist
    if( !lstRes.size() ){
        rs.result = user_not_exist;
    }else{
    //查到，判断密码是否一致
        //否， 返回 password_error
        if(strcmp(lstRes.front().c_str(),rq->password) != 0){
            rs.result = password_error;
        }else{
            //是，回复包中写name id
            lstRes.pop_front();
            int id = atoi(lstRes.front().c_str());
            lstRes.pop_front();

            rs.userid = id;
            strcpy(rs.name,lstRes.front().c_str());
            rs.result = login_success;

            // map[id] = UserInfo id 对用户信息的映射
            UserInfo * user;
            if(!m_mapIdToUserInfo.find(id,user)){
                user = new UserInfo;
            }else{
                //踢掉之前登陆的
                //首先判断新登陆的和之前存储的fd是否相等，不等，就给之前存储的fd发送退出包，从而实现只有一个用户登陆的效果
            }
            user->clientfd = clientfd;
            user->name = rs.name;
            user->userId = id;
            //更新映射关系
            m_mapIdToUserInfo.insert(id,user);
            //添加客户端连接映射
            if(m_mapUseridToSockfd.count(id) == 0){
                m_mapUseridToSockfd[id] = clientfd;
            }
            //发送回复包
            SendData(clientfd,(char *)&rs,sizeof(rs));
            //发送好友列表请求
            slot_addFriend(clientfd,id);
            //更新他的群聊列表
            slot_addGroup(clientfd,id);
            //获取自己的消息列表
            slot_getMessages(clientfd,id);
            //获取群离线消息
            slot_getGroupMessages(clientfd,id);
            //更新自己的状态
            sprintf(sqlStr,"update t_user set u_state = %d where u_id = %d;",1,id);
            m_sql->UpdataMysql(sqlStr);
            //查询他的所有好友，更新在线好友列表中该用户的状态
            list<string> lstRes;
            sprintf(sqlStr,"select f_Bid from t_friend where f_Aid = %d;",id);
            m_sql->SelectMysql(sqlStr,1,lstRes);
            while(lstRes.size() != 0){
                int friendid = stoi(lstRes.front());lstRes.pop_front();
                if(m_mapUseridToSockfd.count(friendid) > 0){
                    //更新他的好友列表
                    slot_addFriend(m_mapUseridToSockfd[friendid],friendid);

                }
            }
            return;
        }

    }
    //发送回复包
    SendData(clientfd,(char *)&rs,sizeof(rs));
}

void CLogic::UserFileListRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_FILE_LIST_RQ * rq = (STRU_FILE_LIST_RQ *)szbuf;

    //查询文件信息
    char sqlstr[1000];
    list<string> lstRes;
    sprintf(sqlstr,"select f_id , f_name , f_uploadtime , f_size , f_MD5 , f_type from t_file where f_id in ( select  f_id  from  t_user_file  where f_dir = '%s'  and u_id = '%d'  );",rq->dir,rq->userId);
    bool res = m_sql->SelectMysql(sqlstr,6,lstRes);
    if(!res){
        printf("sql select error:%s\n",sqlstr);
        return;
    }
    if(lstRes.size() == 0) return ;

    //循环获取文件信息，打包发送到客户端
    for(auto ite = lstRes.begin(); ite != lstRes.end();){
        string fileId = *ite;ite++;
        string fileName = *ite;ite++;
        string uploadtime = *ite;ite++;
        string size = *ite;ite++;
        string md5 = *ite;ite++;
        string fileType = *ite;ite++;

        STRU_FILE_INFO info;

        info.userId = rq->userId;
        info.fileId = atoi(fileId.c_str());
        strcpy(info.fileName,fileName.c_str());
        strcpy(info.uploadTIme,uploadtime.c_str());
        info.size = atoi(size.c_str());
        strcpy(info.md5,md5.c_str());
        strcpy(info.fileType,fileType.c_str());
        strcpy(info.dir,rq->dir);//注意别落下了
        cout<<info.fileId<<endl;
        //发送数据
        SendData(clientfd,(char *)&info,sizeof(info));
    }


}
//文件下载请求
void CLogic::DownloadFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("DownloadFileRq\n");
    STRU_DOWNLOAD_RQ * rq = (STRU_DOWNLOAD_RQ *)szbuf;


    char sqlStr[1000];
    sprintf(sqlStr,"select f_type ,f_dir , f_MD5, f_size , t_file.f_id , f_name , f_path from t_file inner join t_user_file on t_file.f_id = t_user_file.f_id where t_file.f_id = '%d' and t_user_file.u_id = '%d' and t_user_file.f_dir = '%s';",
            rq->fileid,rq->userid,rq->dir);
    list<string> lstRes;
    m_sql->SelectMysql(sqlStr,7,lstRes);
    //没查到，直接返回
    if(lstRes.size() == 0){
        //下载失败，失败处理 todo
        return;
    }
    //做映射，u_id f_id to fileInfo
    FileInfo * info = new FileInfo;
    for(auto ite = lstRes.begin(); ite != lstRes.end();){
        info->type = *ite; ite++;
        info->dir = *ite; ite++; //带文件的下载需要dir

        info->md5 = *ite; ite++;
        info->size = atoi((*ite).c_str());ite++;
        info->fileid = atoi((*ite).c_str());ite++;
        info->name = *ite; ite++;

        info->absolutePath = *ite; ite++;

    }
    if(info->type == "file"){
        //插入到映射中
        //make_pair(rq->userid,rq->fileid)
        m_mapFileIdToFileInfo.insert(make_pair(rq->userid,rq->fileid),info);
        //封包，发送文件头请求
        STRU_FILE_HEAD_RQ headRq;
        strcpy(headRq.dir,info->dir.c_str());
        strcpy(headRq.md5,info->md5.c_str());
        headRq.size = info->size;
        headRq.fileid = info->fileid;
        strcpy(headRq.fileName,info->name.c_str());
        strcpy(headRq.fileType,info->type.c_str());

        SendData(clientfd,(char *)&headRq,sizeof(headRq));
    }else{
        DownloadFolder(clientfd,rq->userid,rq->fileid,info->dir,info->name);
    }

}

void CLogic::DownloadFile(list<string> &lstRes,int userid,int clientfd)
{
    if(lstRes.size() == 0){
        //下载失败，失败处理 todo
        return;
    }
    //做映射，u_id f_id to fileInfo
    FileInfo * info = new FileInfo;
    info->type = lstRes.front(); lstRes.pop_front();
    info->dir = lstRes.front(); lstRes.pop_front(); //带文件的下载需要dir

    info->md5 = lstRes.front(); lstRes.pop_front();
    info->size = atoi(lstRes.front().c_str());lstRes.pop_front();
    info->fileid = atoi(lstRes.front().c_str());lstRes.pop_front();
    info->name = lstRes.front(); lstRes.pop_front();
    info->absolutePath = lstRes.front(); lstRes.pop_front();

    cout<<info->name<<endl;

        //插入到映射中
        //make_pair(rq->userid,rq->fileid)
        m_mapFileIdToFileInfo.insert(make_pair(userid,info->fileid),info);
        //封包，发送文件头请求
        STRU_FILE_HEAD_RQ headRq;
        strcpy(headRq.dir,info->dir.c_str());
        strcpy(headRq.md5,info->md5.c_str());
        headRq.size = info->size;
        headRq.fileid = info->fileid;
        strcpy(headRq.fileName,info->name.c_str());
        strcpy(headRq.fileType,info->type.c_str());

        SendData(clientfd,(char *)&headRq,sizeof(headRq));
}
void CLogic::DownloadFolder(int clientfd, int userid, int fileid,string dir, string filename)
{
    //先发文件夹头请求包
    STRU_FOLDER_HEAD_RQ rq;
    strcpy(rq.dir,dir.c_str());
    rq.fileid = fileid;
    strcpy(rq.fileName,filename.c_str());
    SendData(clientfd,(char *)&rq,sizeof(rq));
    //拼路径，查该文件夹下的文件和文件夹
    dir +=filename + "/";

    char sqlStr[1000];
    sprintf(sqlStr,"select f_type ,f_dir , f_MD5, f_size , t_file.f_id , f_name , f_path from t_file inner join t_user_file on t_file.f_id = t_user_file.f_id where t_user_file.u_id = '%d' and t_user_file.f_dir = '%s';",
            userid,dir.c_str());
    list<string> lstRes;
    m_sql->SelectMysql(sqlStr,7,lstRes);
    if(lstRes.size() == 0){
        //下载失败，失败处理 todo
        return;
    }

    while(lstRes.size() != 0){
        string type = lstRes.front();
        if(type == "file"){

            DownloadFile(lstRes,userid,clientfd);
        }else{

            auto ite = lstRes.begin();
            string type = lstRes.front(); lstRes.pop_front();
            string dir = lstRes.front(); lstRes.pop_front(); //带文件的下载需要dir
            string md5 = lstRes.front(); lstRes.pop_front();
            int size = atoi(lstRes.front().c_str());lstRes.pop_front();
            int fileid = atoi(lstRes.front().c_str());lstRes.pop_front();
            string name = lstRes.front(); lstRes.pop_front();
            string absolutePath = lstRes.front(); lstRes.pop_front();

            DownloadFolder(clientfd,userid,fileid,dir,name);

        }

    }


}

void CLogic::slot_addFriend(int clientfd, int userid)
{
    char sqlbuf[1000];
    sprintf(sqlbuf,"select f_Bid from t_friend where f_Aid = %d;",userid);
    list<string> lstRes;
    bool res =m_sql->SelectMysql(sqlbuf,1,lstRes);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    while(lstRes.size() != 0){
        int fid = stoi(lstRes.front());lstRes.pop_front();
        STRU_FRIEND_INFO info;

        sprintf(sqlbuf,"select u_tel,u_icon,u_name,u_feeling,u_state from t_user where u_id = %d;",fid);
        list<string> lst;
        bool res =m_sql->SelectMysql(sqlbuf,5,lst);
        if(!res){
            cout<<"select friendinfo error:"<<sqlbuf<<endl;
            return;
        }

        strcpy(info.tel,lst.front().c_str());lst.pop_front();
        strcpy(info.icon,lst.front().c_str());lst.pop_front();
        strcpy(info.name,lst.front().c_str());lst.pop_front();
        strcpy(info.feeling,lst.front().c_str());lst.pop_front();
        info.state = stoi(lst.front());lst.pop_front();
        info.userId = fid;
        SendData(clientfd,(char *)&info,sizeof(info));

    }
}
void CLogic::slot_addGroup(int clientfd, int userid)
{
    char sqlbuf[1000];
    sprintf(sqlbuf,"select g_id,identity from group_members where u_id = %d;",userid);
    list<string> lstRes;
    bool res =m_sql->SelectMysql(sqlbuf,2,lstRes);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    while(lstRes.size() != 0){
        STRU_GROUP_INFO info;
        info.gid = stoi(lstRes.front());lstRes.pop_front();
        info.identity = stoi(lstRes.front());lstRes.pop_front();


        sprintf(sqlbuf,"select name from groups where id = %d;",info.gid);
        list<string> lst;
        bool res =m_sql->SelectMysql(sqlbuf,1,lst);
        if(!res){
            cout<<"select friendinfo error:"<<sqlbuf<<endl;
            return;
        }

        strcpy(info.gname,lst.front().c_str());lst.pop_front();
        cout<<info.gname<<" "<<info.gid<<endl;
        SendData(clientfd,(char *)&info,sizeof(info));

    }
}

void CLogic::slot_getMessages(int clientfd, int userid)
{
    char sqlbuf[1000];
    sprintf(sqlbuf,"select sender_id , receiver_id , content ,sent_time from t_messages where sender_id = %d or receiver_id = %d;",userid,userid);
    list<string> lstRes;
    bool res =m_sql->SelectMysql(sqlbuf,4,lstRes);
    if(!res){
        cout<<"select t_messages error:"<<sqlbuf<<endl;
        return;
    }
    int id = 1;
    while(lstRes.size() != 0){
        STRU_MESSAGES_RQ rq;
        rq.id = id++;
        rq.sendId = stoi(lstRes.front());lstRes.pop_front();
        rq.recvId = stoi(lstRes.front());lstRes.pop_front();
        strcpy(rq.content,lstRes.front().c_str());lstRes.pop_front();
        strcpy(rq.nowtime,lstRes.front().c_str());lstRes.pop_front();
        cout<<rq.sendId<<" "<<rq.content<<endl;
        SendData(clientfd,(char *)&rq,sizeof(rq));

    }
}
void CLogic::slot_getGroupMessages(int clientfd, int userid)
{
    char sqlbuf[1000];
    //select g_id from group_members where u_id = 1;
    sprintf(sqlbuf,"select g_id from group_members where u_id = %d;",userid);
    list<string> lst;
    bool res =m_sql->SelectMysql(sqlbuf,1,lst);
    if(!res){
        cout<<"select t_messages error:"<<sqlbuf<<endl;
        return;
    }
    int id = 1;
    while(lst.size()!=0){
        int gid = stoi(lst.front());lst.pop_front();
        sprintf(sqlbuf,"select * from v_getmes where group_id = %d;",gid);
        list<string> lstRes;
        bool res =m_sql->SelectMysql(sqlbuf,6,lstRes);
        if(!res){
            cout<<"select t_messages error:"<<sqlbuf<<endl;
            return;
        }
        while(lstRes.size() != 0){
            STRU_GROUP_OFFLINE_CHAT_RQ rq;
            rq.id = id++;
            rq.gId = stoi(lstRes.front());lstRes.pop_front();
            rq.uid = stoi(lstRes.front());lstRes.pop_front();//用户id
            strcpy(rq.content,lstRes.front().c_str());lstRes.pop_front();
            strcpy(rq.nowtime,lstRes.front().c_str());lstRes.pop_front();
            strcpy(rq.name,lstRes.front().c_str());lstRes.pop_front();
            rq.identity = stoi(lstRes.front());lstRes.pop_front();
            cout<<rq.uid <<" "<<rq.content<<endl;
            SendData(clientfd,(char *)&rq,sizeof(rq));

        }
    }
}

void CLogic::ChatRq(sock_fd clientfd, char *szbuf, int nlen)
{


    STRU_CHAT_RQ * rq = (STRU_CHAT_RQ *)szbuf;
    //将该消息插入到数据库
    char sqlbuf[1000];

    sprintf(sqlbuf,"insert into t_messages(sender_id,receiver_id,content,sent_time) values(%d,%d,'%s','%s');",
            rq->userId,rq->friendId,rq->content,rq->nowtime);
    m_sql->UpdataMysql(sqlbuf);
    //先看看對端在不在线
    //在线
    cout<<rq->content<<endl;
    if(m_mapUseridToSockfd.count(rq->friendId) > 0){
        cout<<rq->content<<endl;
        STRU_CHAT_RQ frq;
        frq.userId = rq->friendId;
        frq.friendId = rq->userId;
        strcpy(frq.content,rq->content);
        strcpy(frq.nowtime,rq->nowtime);
        SendData(m_mapUseridToSockfd[rq->friendId],(char * )&frq,sizeof(frq));
    }else{
        //这里可以发送回复告诉发送端该好友不在线，我就不这样写了，我想写带离线消息的。
    }
}

void CLogic::AddFriendRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_ADD_FRIEND_RQ * rq = (STRU_ADD_FRIEND_RQ *)szbuf;

    char sqlbuf[100];
    sprintf(sqlbuf,"select u_id from t_user where u_tel = '%s';",rq->friendtel);
    list<string> lstRes;
    m_sql->SelectMysql(sqlbuf,1,lstRes);

    //
    STRU_ADD_FRIEND_RS rs;
    rs.userId = rq->userId;
    //用户不存在
    if(lstRes.size() == 0){
        rs.result = no_this_user;
        SendData(clientfd,(char *)&rs,sizeof(rs));
        return;
    }
    //没在线
    rs.friendId = stoi(lstRes.front());lstRes.pop_front();
    if(m_mapUseridToSockfd.count(rs.friendId) == 0){
        rs.result = user_offline;
        SendData(clientfd,(char *)&rs,sizeof(rs));
        cout<<rs.friendId<<endl;
        return;
    }
    //发送给对端
    SendData(m_mapUseridToSockfd[rs.friendId],szbuf,nlen);

}

void CLogic::AddFriendRs(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_ADD_FRIEND_RS * rs = (STRU_ADD_FRIEND_RS *)szbuf;
    if(rs->result == add_success){
        char sqlbuf[100];
        sprintf(sqlbuf,"insert into t_friend values(%d,%d),(%d,%d);",rs->userId,rs->friendId,rs->friendId,rs->userId);
        bool res = m_sql->UpdataMysql(sqlbuf);
        if(!res){
            cout<<"insert t_friend error:"<<sqlbuf<<endl;
        }
        //更新请求端好友列表
        if(m_mapUseridToSockfd.count(rs->userId) > 0){
            slot_addFriend(m_mapUseridToSockfd[rs->userId],rs->userId);
        }
        //更新接受端好友列表
        if(m_mapUseridToSockfd.count(rs->friendId) > 0){
            slot_addFriend(m_mapUseridToSockfd[rs->friendId],rs->friendId);
        }
    }
    //不存在就不回复了
    if(m_mapUseridToSockfd.count(rs->userId) == 0) return;

    int usersockfd =  m_mapUseridToSockfd[rs->userId];
    //直接转发
    SendData(usersockfd,szbuf,nlen);


    //SendData()
}

void CLogic::OffLineRq(sock_fd clientfd, char *szbuf, int nlen)
{
    cout<<"OffLineRq"<<endl;
    //拆包
    STRU_OFFLINE_RQ * rq = (STRU_OFFLINE_RQ *)szbuf;
    cout<<"OffLineRq:"<<rq->userId<<endl;
    //更新自己的状态
    char sqlStr[1000];
    sprintf(sqlStr,"update t_user set u_state = %d where u_id = %d;",0,rq->userId);
    m_sql->UpdataMysql(sqlStr);
    //查询他的所有好友，更新在线好友列表中该用户的状态
    list<string> lstRes;
    sprintf(sqlStr,"select f_Bid from t_friend where f_Aid = %d;",rq->userId);
    m_sql->SelectMysql(sqlStr,1,lstRes);
    while(lstRes.size() != 0){
        int friendid = stoi(lstRes.front());lstRes.pop_front();
        if(m_mapUseridToSockfd.count(friendid) > 0){
            //更新他的好友列表
            slot_addFriend(m_mapUseridToSockfd[friendid],friendid);
        }
    }
    //删除自己的socket映射信息
    m_mapUseridToSockfd.erase(rq->userId);
}

void CLogic::DelFriendRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_DEL_FRIEND_RQ * rq =(STRU_DEL_FRIEND_RQ *)szbuf;
    //查好友id
    char sqlbuf[1000];
    sprintf(sqlbuf,"select u_id from t_user where u_tel = '%s';",rq->friendtel);
    cout<<sqlbuf<<endl;
    list<string> lst;
    m_sql->SelectMysql(sqlbuf,1,lst);
    int friendid = stoi(lst.front());lst.clear();
    //删除好友列表中的好友关系
    sprintf(sqlbuf,"delete from t_friend where f_Aid = %d and f_Bid = %d;",rq->userId,friendid);
    m_sql->UpdataMysql(sqlbuf);
    cout<<sqlbuf<<endl;
    sprintf(sqlbuf,"delete from t_friend where f_Aid = %d and f_Bid = %d;",friendid,rq->userId);
    m_sql->UpdataMysql(sqlbuf);
    cout<<sqlbuf<<endl;

    //如果对端在线的话，给对端一个删除请求
    if(m_mapUseridToSockfd.count(friendid) > 0){
        SendData(m_mapUseridToSockfd[friendid],szbuf,nlen);
    }
    //会一个回复包
    STRU_DEL_FRIEND_RS rs;
    rs.result = del_success;
    SendData(clientfd,(char *)&rs,sizeof(rs));


}

void CLogic::GroupChatRq(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_GROUP_CHAT_RQ * rq = (STRU_GROUP_CHAT_RQ *)szbuf;
    //将该消息插入到数据库
    char sqlbuf[1000];

    sprintf(sqlbuf,"insert into group_messages(group_id,sender_id,message) values(%d,%d,'%s');",
            rq->gId,rq->sendId,rq->content);
    m_sql->UpdataMysql(sqlbuf);
    //查自己的名字和身份
    sprintf(sqlbuf,"select t_user.u_name,identity from group_members inner join t_user on group_members.u_id = t_user.u_id where g_id = %d and t_user.u_id =%d;",
            rq->gId,rq->sendId);
    list<string> lst;
    bool res =m_sql->SelectMysql(sqlbuf,3,lst);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    char myname[1024];
    strcpy(myname,lst.front().c_str());lst.pop_front();
    int myidentity = stoi(lst.front());lst.pop_front();
    sprintf(sqlbuf,"select t_user.u_id,t_user.u_name,identity from group_members inner join t_user on group_members.u_id = t_user.u_id where g_id = %d and t_user.u_id != %d;",rq->gId,rq->sendId);
    list<string> lstRes;
    res =m_sql->SelectMysql(sqlbuf,3,lstRes);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    char name[1024];
    while(lstRes.size() != 0){
        int fid = stoi(lstRes.front());lstRes.pop_front();
        //这个名字和身份没啥用了，但是有不想碰，怕出别的错误，开辟原则
        strcpy(name,lstRes.front().c_str());lstRes.pop_front();
        int identity = stoi(lstRes.front());lstRes.pop_front();
        cout<<fid<<" "<<myname<<" "<<myidentity<<endl;
        //先看看群成员在不在线
        //在线
        cout<<rq->content<<endl;
        if(m_mapUseridToSockfd.count(fid) > 0){
            cout<<rq->content<<endl;
            STRU_GROUP_BROADCAST_CHAT_RQ frq;
            strcpy(frq.content,rq->content);
            strcpy(frq.nowtime,rq->nowtime);
            strcpy(frq.nowtime,myname);
            frq.gId = rq->gId;
            frq.identity = myidentity;
            SendData(m_mapUseridToSockfd[fid],(char * )&frq,sizeof(frq));
        }else{
            //这里可以发送回复告诉发送端该好友不在线，我就不这样写了，我想写带离线消息的。
        }

    }



}

void CLogic::GroupCreateRq(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_GROUP_CREATE_RQ * rq = (STRU_GROUP_CREATE_RQ *)szbuf;
    //插入到群聊信息里
    char sqlbuf[1024];
    sprintf(sqlbuf,"insert into groups(name) values('%s');",rq->name);
    m_sql->UpdataMysql(sqlbuf);

    //获取群id (名字不允许重复)
    sprintf(sqlbuf,"select id from groups where name ='%s';",rq->name);
    list<string> lstRes;
    bool res =m_sql->SelectMysql(sqlbuf,1,lstRes);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    int gid = stoi(lstRes.front());lstRes.pop_front();
    //将该用户作为该群的群主插入到群成员表中
    sprintf(sqlbuf,"insert into group_members values(%d,%d,%d);",gid,rq->uid,1);
    m_sql->UpdataMysql(sqlbuf);
    //更新该用户群列表
    slot_addGroup(clientfd,rq->uid);
}

void CLogic::GroupAddRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_GROUP_GROUP_ADD_RQ * rq = (STRU_GROUP_GROUP_ADD_RQ *)szbuf;
    STRU_GROUP_GROUP_ADD_RS rs;
    //查群是否存在
    char sqlbuf[1024];
    sprintf(sqlbuf,"select id from groups where id = %d;",rq->gid);
    list<string> lstRes;
    bool res =m_sql->SelectMysql(sqlbuf,1,lstRes);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    //不存在该群，发送给客户端
    if(lstRes.size() == 0){
        rs.gid = rq->gid;
        rs.result = no_this_group;
        SendData(clientfd,(char *)&rs,sizeof(rs));
        return;
    }
    int gid = stoi(lstRes.front());lstRes.pop_front();
    //查群主，看是否在线
    sprintf(sqlbuf,"select u_id from group_members where g_id = %d and identity = 1;",rq->gid);
    res =m_sql->SelectMysql(sqlbuf,1,lstRes);
    if(!res){
        cout<<"select friend error:"<<sqlbuf<<endl;
        return;
    }
    //获取群主uid
    int uid = stoi(lstRes.front());lstRes.pop_front();
    if(m_mapUseridToSockfd.count(uid) == 0){
        rs.gid = rq->gid;
        rs.result = user_offline;
        SendData(clientfd,(char *)&rs,sizeof(rs));
        return;
    }

    //发送请求给群主，看是否同意
    SendData(m_mapUseridToSockfd[uid],szbuf,nlen);

}

void CLogic::GroupAddRs(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_GROUP_GROUP_ADD_RS * rs = (STRU_GROUP_GROUP_ADD_RS *)szbuf;
    //判断是否成功
    //成功，添加到群成员表中，以群员的身份
    if(rs->result == add_success){
        char sqlbuf[1024];
        sprintf(sqlbuf,"insert into group_members values(%d,%d,%d);",rs->gid,rs->senderid,0);
        m_sql->UpdataMysql(sqlbuf);
        if(m_mapUseridToSockfd.count(rs->senderid) == 0) return;
        //更新该群聊列表
        slot_addGroup(m_mapUseridToSockfd[rs->senderid],rs->senderid);
    }
    //无论成功与否，都有发送给对方
    if(m_mapUseridToSockfd.count(rs->senderid) == 0) return;
    SendData(m_mapUseridToSockfd[rs->senderid],szbuf,nlen);


}

void CLogic::GroupDelRq(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_DEL_GROUP_RQ * rq = (STRU_DEL_GROUP_RQ *)szbuf;
    char sqlbuf[1024];
    //是群主，将该群解散
    if(rq->identity == 1){
        //先通知其他成员，让他们删除对应群
        sprintf(sqlbuf,"select u_id from group_members where g_id = %d and u_id != %d;",rq->gid,rq->uid);
        list<string> lst;
        m_sql->SelectMysql(sqlbuf,1,lst);
        int uid = 0;
        while(lst.size() != 0){
            uid = stoi(lst.front());lst.pop_front();
            if(m_mapUseridToSockfd.count(uid) == 0) continue;
            SendData(m_mapUseridToSockfd[uid],szbuf,nlen);
        }
        //删除群中信息
        sprintf(sqlbuf,"delete from groups where id = %d;",rq->gid);
        m_sql->UpdataMysql(sqlbuf);
        //删除群成员中信息
        sprintf(sqlbuf,"delete from group_members where g_id = %d;",rq->gid);
        m_sql->UpdataMysql(sqlbuf);
        //删除群消息中信息
        sprintf(sqlbuf,"delete from group_messages where group_id = %d;",rq->gid);
        m_sql->UpdataMysql(sqlbuf);

    }else{
    //成员，删除对应的群成员信息

        sprintf(sqlbuf,"delete from group_members where g_id = %d and u_id = %d;",rq->gid,rq->uid);
        m_sql->UpdataMysql(sqlbuf);

    }
    STRU_DEL_GROUP_RS rs;
    rs.result = del_success;
    SendData(clientfd,(char *)&rs,sizeof(rs));

}

void CLogic::ImSendFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_IM_SEND_FILE_HEAD_RQ * rq = (STRU_IM_SEND_FILE_HEAD_RQ *)szbuf;
    STRU_IM_SEND_FILE_HEAD_RS rs;
    //不在线，回个提示
    if(m_mapUseridToSockfd.count(rq->recvid) == 0){
        rs.recvid = rq->recvid;
        rs.sendid = rq->sendid;
        rs.result = 0;
        SendData(clientfd,(char *)&rs,sizeof(rs));
        return;
    }
    //在线，直接发送给对端
    SendData(m_mapUseridToSockfd[rq->recvid],szbuf,nlen);

}

void CLogic::ImSendFileRs(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_IM_SEND_FILE_HEAD_RS * rs = (STRU_IM_SEND_FILE_HEAD_RS *)szbuf;
    if(m_mapUseridToSockfd.count(rs->sendid) == 0){
        return;
    }
    //在线，直接发送给对端
    SendData(m_mapUseridToSockfd[rs->sendid],szbuf,nlen);
}

void CLogic::ImSendFileContentRq(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_IM_SEND_FILE_CONTENT_RQ * rq =(STRU_IM_SEND_FILE_CONTENT_RQ *)szbuf;

    if(m_mapUseridToSockfd.count(rq->recvid) == 0) return;

    SendData(m_mapUseridToSockfd[rq->recvid],szbuf,nlen);
}

void CLogic::ImSendFileContentRs(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_IM_SEND_FILE_CONTENT_RS * rs =(STRU_IM_SEND_FILE_CONTENT_RS *)szbuf;

    if(m_mapUseridToSockfd.count(rs->sendid) == 0) return;

    SendData(m_mapUseridToSockfd[rs->sendid],szbuf,nlen);
}



void CLogic::FileHeadRs(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("FileHeadRs\n");
    //拆包
    STRU_FILE_HEAD_RS * rs = (STRU_FILE_HEAD_RS *)szbuf;

    //找到下载文件信息
    
    FileInfo *info = nullptr;
    cout<<rs->userid<<" "<<rs->fileid<<endl;
    if(!m_mapFileIdToFileInfo.find(make_pair(rs->userid,rs->fileid),info)){
        printf("file find fail\n");
        return;
    }

    //文件处理
    if(info->type !="dir"){
        //打开文件
        //info->filefd = open(info->absolutePath.c_str(),O_RDONLY);
        if((info->filefd= open(info->absolutePath.c_str(),O_RDONLY))==-1){
                perror("error:Sfile open fail...\n");
                exit(0);
        }
        //读取文件内容
        STRU_FILE_CONTENT_RQ rq;
        rq.fileid = rs->fileid;
        rq.userid = rs->userid;
        rq.len = read(info->filefd,rq.content,_DEF_BUFFER);
        cout<<"len:"<<rq.len<<"pos:"<<info->pos<<endl;

        //pos + len 读取位置向后移动
        //info->pos +=rq.len; 判断回复包成功的时候才可以+
        //发送
        SendData(clientfd,(char *)&rq,sizeof(rq));
    }else{
        //处理文件夹
    }

}

void CLogic::FileContentRs(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("FileContentRs\n");
    //拆包
    STRU_FILE_CONTENT_RS *rs = (STRU_FILE_CONTENT_RS *)szbuf;


    //找到下载文件信息
    char idbuf[100];
    sprintf(idbuf,"%10d%10d",rs->userid,rs->fileid);
    string strid = idbuf;
    FileInfo *info = nullptr;

    if(!m_mapFileIdToFileInfo.find(make_pair(rs->userid,rs->fileid),info)){
        printf("file find fail\n");
        return;
    }
    //判断客户端写入是否成功
    //不成功，回跳到之前的位置，重新读取发送 todo
    if(rs->result == 0){
        lseek(info->filefd,-1*rs->len,SEEK_CUR);
    }else{
        //成功，继续写
        //pos + len 读取位置向后移动
        info->pos +=rs->len;
        cout<<info->pos<<" "<<rs->len<<endl;
        //如果读到头了，关闭文件描述符，删除map对应映射，返回
        if(info->size == rs->len){
            close(info->filefd);
            m_mapFileIdToFileInfo.erase(make_pair(rs->userid,rs->fileid));
            delete info;
            return;
        }
    }
    //读取文件内容
    STRU_FILE_CONTENT_RQ rq;
    rq.fileid = rs->fileid;
    rq.userid = rs->userid;
    rq.len = read(info->filefd,rq.content,_DEF_BUFFER);
     cout<<"rq.len:"<<rq.len<<endl;
    usleep(100);

    //发送
    SendData(clientfd,(char *)&rq,sizeof(rq));
}

void CLogic::UploadFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("UploadFileRq\n");
    //拆包
    STRU_UPLOAD_FILE_HEAD_RQ *rq = (STRU_UPLOAD_FILE_HEAD_RQ *)szbuf;
    //查看信息
    //判断是否可以秒传，用文件名 + md5 + status 的形式进行判断
    {
        //先通过 文件名 + md5 + status 查表判断是否可以秒传
        char sqlbuf[1000] = "";
        sprintf(sqlbuf,"select f_id from t_file where f_name ='%s' and f_md5='%s' and f_state = 1; ",
                rq->fileName,rq->md5);
        list<string> lisRes;
        if(!m_sql->SelectMysql(sqlbuf,1,lisRes)){
            cout<<"select fail"<<sqlbuf<<endl;
            return ;
        }

        //可以秒传
        if(lisRes.size() > 0){
            int fileid = stoi(lisRes.front());
            lisRes.pop_back();
            //将用户和文件id信息插入到t_user_file表中
            sprintf(sqlbuf,"insert into t_user_file(u_id,f_id,f_dir) values(%d,%d,'%s');",rq->userid,fileid,rq->dir);
            if(!m_sql->UpdataMysql(sqlbuf)){
                cout<<"update t_user_file fail"<<endl;
                return;
            }
            STRU_QUICK_UPLOAD_RS rs;
            rs.result = 1;
            strcpy(rs.md5,rq->md5);
            //返回秒传包
            SendData(clientfd,(char *)&rs,sizeof(rs));
            return;
        }


    }
    //不是秒传
    //初始化个FileInfo对象
    FileInfo * info = new FileInfo;
    info->dir = rq->dir;
    info->md5 = rq->md5;
    info->name = rq->fileName;
    info->size = rq->size;
    info->time = rq->time;
    info->type = rq->fileType;
    //info->fileid ?
    //打开失败就是这里拼接有错误
    char pathbuf[1000] = "";
    sprintf(pathbuf,"%s%d%s%s",DEF_PATH,rq->userid,rq->dir,rq->fileName);
    info->absolutePath = pathbuf;

    //将文件信息插入到t_file表中
    char sqlbuf[1000] = "";
    //注意:这里的count值初始应为0,因为后面会插入到t_user_file表中一个数据，导致触发触发器使count值又+1
    sprintf(sqlbuf,"insert into t_file (f_name ,  f_uploadtime ,  f_size ,  f_path ,  f_count ,  f_MD5  , f_state , f_type ) "
            "values ( '%s' , '%s' , %d , '%s' , 0, '%s' , 0  , '%s' );",rq->fileName,rq->time,rq->size,pathbuf,rq->md5,rq->fileType);
    if(!m_sql->UpdataMysql(sqlbuf)){
        cout<<"update t_file fail"<<endl;
        return;
    }

    //根据md5和文件名查询到该文件的fileid
    list<string> lstRes;
    sprintf(sqlbuf,"select f_id from t_file where f_name = '%s' and f_md5 = '%s';",rq->fileName,rq->md5);
    if(!m_sql->SelectMysql(sqlbuf,1,lstRes)){
        cout<<"select t_file fail"<<endl;
        return;
    }
    info->fileid = stoi(lstRes.front());
    lstRes.pop_front();
    //将用户和文件id信息插入到t_user_file表中
    sprintf(sqlbuf,"insert into t_user_file(u_id,f_id,f_dir) values(%d,%d,'%s');",rq->userid,info->fileid,rq->dir);
    if(!m_sql->UpdataMysql(sqlbuf)){
        cout<<"update t_user_file fail"<<endl;
        return;
    }
    //根据绝对路径打开文件，获取fd，写入到FileInfo中
    info->filefd = open(pathbuf,O_CREAT|O_WRONLY|O_TRUNC,0776);

    //FileInfo添加到map中
    m_mapFileIdToFileInfo.insert(make_pair(rq->userid,info->fileid),info);
    //创建回复包并初始化
    STRU_UPLOAD_FILE_HEAD_RS rs;
    strcpy(rs.md5 , rq->md5);
    rs.fileid = info->fileid;
    rs.result = 1;//默认成功
    rs.userid = rq->userid;
    cout<<"end"<<endl;
    //发送
    SendData(clientfd,(char *)&rs,sizeof(rs));

}

void CLogic::FileContentRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("FileContentRq\n");
    //拆包

    STRU_FILE_CONTENT_RQ * rq = (STRU_FILE_CONTENT_RQ *)szbuf;

    //根据usid+fileid 获取FileInfo
    FileInfo * info = nullptr;
    if(!m_mapFileIdToFileInfo.find(make_pair(rq->userid,rq->fileid),info)){
        cout<<"find fail"<<endl;
        return;
    }
    //向文件中写入
    int len = write(info->filefd,rq->content,rq->len);

    STRU_FILE_CONTENT_RS rs;
    rs.len = rq->len;
    rs.userid = rq->userid;
    rs.fileid = rq->fileid;
    //写入不成功
    if(len != rq->len){
        lseek(info->filefd,-len,SEEK_CUR);
        rs.result = 0;
    //写入成功
    }else{
        rs.result = 1;
        info->pos += len;
        //如果写入完成，关闭fd，删除map中对应节点 ,更新下载状态
        if(info->pos >= info->size){
            close(info->filefd);
            char sqlbuf[1000] = "";
            sprintf(sqlbuf,"update t_file set f_state = 1 where f_id = %d;",info->fileid);
            if(!m_sql->UpdataMysql(sqlbuf)){
                cout<<"update f_status fail"<<endl;
            }
            m_mapFileIdToFileInfo.erase(make_pair(rq->userid,rq->fileid));

            delete info;
        }
    }
    //发送数据
    SendData(clientfd,(char *)&rs,sizeof(rs));
}

void CLogic::AddFolderRq(sock_fd clientfd, char *szbuf, int nlen)
{


    printf("AddFolderRq\n");
    //拆包
    STRU_ADD_FOLDER_RQ *rq = (STRU_ADD_FOLDER_RQ *)szbuf;
    //查看信息


    //打开失败就是这里拼接有错误
    char pathbuf[1000] = "";
    sprintf(pathbuf,"%s%d%s%s",DEF_PATH,rq->userid,rq->dir,rq->fileName);

    //判断文件夹是否插入过t_file ,插入过不需要再插入
    char sqlbuf[1000] = "";
    list<string> lstRes;
    sprintf(sqlbuf,"select f_id from t_file where f_name = '%s' and f_type = '%s';",rq->fileName,rq->fileType);
    if(!m_sql->SelectMysql(sqlbuf,1,lstRes)){
        cout<<"select t_file fail"<<endl;
        return;
    }
    cout<<"-------"<<endl;
    if(lstRes.size() == 0){
        //将文件信息插入到t_file表中

        sprintf(sqlbuf,"insert into t_file (f_name ,  f_uploadtime ,  f_size ,  f_path ,  f_count ,  f_MD5  , f_state , f_type ) "
                "values ( '%s' , '%s' , %d , '%s' , 0, '%s' , 1  , '%s' );",rq->fileName,rq->time,0,pathbuf,"",rq->fileType);
        if(!m_sql->UpdataMysql(sqlbuf)){
            cout<<"update t_file fail"<<endl;
            return;
        }
        cout<<"?????-"<<endl;
        //根据md5和文件名查询到该文件的fileid
        sprintf(sqlbuf,"select f_id from t_file where f_name = '%s' and f_type = '%s';",rq->fileName,rq->fileType);
        if(!m_sql->SelectMysql(sqlbuf,1,lstRes)){
            cout<<"select t_file fail"<<endl;
            return;
        }
        cout<<"::::::"<<endl;
    }

    int fileid = stoi(lstRes.front());
    lstRes.pop_front();
    //将用户和文件id信息插入到t_user_file表中
    sprintf(sqlbuf,"insert into t_user_file(u_id,f_id,f_dir) values(%d,%d,'%s');",rq->userid,fileid,rq->dir);
    if(!m_sql->UpdataMysql(sqlbuf)){
        cout<<"update t_user_file fail"<<endl;
        return;
    }
    //创建文件夹
    umask(0);
    mkdir(pathbuf,0777);

    //创建回复包并初始化
    STRU_ADD_FOLDER_RS rs;
    rs.result = 1;//默认成功
    //发送
    SendData(clientfd,(char *)&rs,sizeof(rs));
}

void CLogic::DeleteFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_DELETE_FILE_RQ * rq = (STRU_DELETE_FILE_RQ *)szbuf;
    //对每一项进行操作
    for(int i = 0 ;i<rq->fileCount;i++){
        //根据这三项数据能该用户该文件的获得所有信息
        DeleteItem(rq->userid,rq->fileidArray[i],rq->dir);
    }
    //发送回复包
    STRU_DELETE_FILE_RS rs;
    strcpy(rs.dir,rq->dir);
    SendData(clientfd,(char *)&rs,sizeof(rs));
}

void CLogic::ShareFileListRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_MY_SHARE_RQ * rq = (STRU_MY_SHARE_RQ *)szbuf;

    //查询文件信息
    char sqlstr[1000];
    list<string> lstRes;
    sprintf(sqlstr,"select f_name ,f_size,s_linkTime,s_link from share_file_info where u_id = %d;",rq->userid);
    bool res = m_sql->SelectMysql(sqlstr,4,lstRes);
    if(!res){
        printf("sql select error:%s\n",sqlstr);
        return;
    }
    if(lstRes.size() == 0) return ;
    int len = sizeof (STRU_MY_SHARE_RS ) + sizeof(MY_SHARE_FILE)*lstRes.size()/4 ;
    STRU_MY_SHARE_RS * rs = (STRU_MY_SHARE_RS *)malloc(len);
    rs->init();
    rs->itemCount =lstRes.size()/4;
    int i = 0;
    //循环获取文件信息，打包发送到客户端
    for(auto ite = lstRes.begin(); ite != lstRes.end();){
        string fileName = *ite;ite++;
        string size = *ite;ite++;
        string f_linkTime = *ite;ite++;
        string link = *ite;ite++;


        strcpy(rs->items[i].name,fileName.c_str());
        rs->items[i].size = atoi(size.c_str());
        strcpy(rs->items[i].time,f_linkTime.c_str());
        rs->items[i].shareLink = atoi(link.c_str());
        cout<<rs->items[i].name<<" "<<rs->items[i].size<<" "<<rs->items[i].time<<" "<<rs->items[i].shareLink<<endl;
        i++;   
    }
    //发送数据
    SendData(clientfd,(char *)rs,len);
}

void CLogic::ShareFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    cout<<"ShareFileRq"<<endl;
    //拆包
    STRU_SHARE_FILE_RQ * rq = (STRU_SHARE_FILE_RQ *)szbuf;

    //生成分享码
    int link=0;
    do{
        int tmp = random()%90000000 + 10000000;
        //去重
        char sqlbuf[1000];
        list<string> lstRes;
        sprintf(sqlbuf,"select *from share_file_info where s_link = %d;",tmp);
        m_sql->SelectMysql(sqlbuf,1,lstRes);
        if(lstRes.size() == 0){
            link = tmp;
            break;
        }

    }while(1);
    //单个处理每个文件或文件夹
    cout<<rq->itemCount<<endl;
    for(int i = 0;i<rq->itemCount;i++){

        ShareItem(rq->userid,rq->fileidArray[i],rq->shareTime,link,rq->dir);
        cout<<rq->fileidArray[i]<<endl;

    }
    //回复包
    STRU_SHARE_FILE_RS rs;
    rs.result = 1;

    SendData(clientfd,(char *)&rs,sizeof(rs));
}

void CLogic::GetShareFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_GET_SHARE_RQ * rq = (STRU_GET_SHARE_RQ *)szbuf;

    //根据分享码，查发送方的 f_name f_id u_id f_type f_dir
    char sqlbuf[1000];
    sprintf(sqlbuf,"select f_name,f_id,u_id,f_type,f_dir from share_file_info where s_link = %d;",
            rq->shareLink);
    list<string> lstRes;
    bool res = m_sql->SelectMysql(sqlbuf,5,lstRes);
    if(!res){
        cout<<"select fail:"<<sqlbuf<<endl;
    }
    while(lstRes.size() != 0){
        string name = lstRes.front();lstRes.pop_front();
        int fid = stoi(lstRes.front());lstRes.pop_front();
        int fromUid = stoi(lstRes.front());lstRes.pop_front();
        string type = lstRes.front();lstRes.pop_front();
        string fromDir = lstRes.front();lstRes.pop_front();

        if(type == "file"){
            GetShareFile(rq->userid,fid,rq->dir);
        }else{
            GetShareFolder(rq->userid,fid,rq->dir,name,fromUid,fromDir);
        }
    }

    STRU_GET_SHARE_RS rs;
    strcpy(rs.dir , rq->dir);
    rs.result = 1;
    SendData(clientfd,(char *)&rs,sizeof(rs));
}

void CLogic::DeleteItem(int uid, int fid, string dir)
{
    cout<<"DeleteItem"<<endl;
   //create view user_file_info as(
//    select t_file.f_id , f_name , f_size , f_MD5 , f_type , t_user_file.f_dir f_dir, f_path , f_count , t_user_file.u_id u_id from
//    t_file inner join t_user_file on t_file.f_id = t_user_file.f_id );
   char strSql[1000];
   list<string> lstRes;
   sprintf(strSql,"select f_name , f_count , f_path , f_type from user_file_info where u_id = %d and f_id = %d and f_dir = '%s';",
           uid,fid,dir.c_str());
   m_sql->SelectMysql(strSql,4,lstRes);
   if(lstRes.size() == 0) return;
   for(auto ite = lstRes.begin(); ite != lstRes.end();){
       string name = *ite; ite++; //带文件的下载需要dir
       int count = atoi((*ite).c_str());ite++;
       string path = *ite; ite++;
       string type = *ite; ite++;

       if(type == "file"){
           DeleteFile(uid,fid,dir,name,count,path);
       }else{
           DeleteFolder(uid,fid,dir,name,count,path);
       }

   }
}

void CLogic::DeleteFile(int uid, int fid, string dir, string name, int count, string path)
{
    cout<<"DeleteFile"<<endl;
    //count为1,删除对应的文件（执行下面的删除语句后对于的count值会-1 = 0,所以在这里直接删除了）
    if(count == 1){
        unlink(path.c_str());
    }
    char strSql[1000];
    //删除数据库中t_user_file中的用户文件映射关系
    sprintf(strSql,"delete from t_user_file where u_id = %d and f_id = %d and f_dir = '%s';",
            uid,fid,dir.c_str());
    m_sql->UpdataMysql(strSql);
    cout<<strSql<<endl;
}

void CLogic::DeleteFolder(int uid, int fid, string dir, string name, int count, string path)
{
    cout<<"DeleteFolder"<<endl;
    char strSql[1000];
    //删除数据库中t_user_file中的用户文件映射关系
    sprintf(strSql,"delete from t_user_file where u_id = %d and f_id = %d and f_dir = '%s';",
            uid,fid,dir.c_str());
    cout<<strSql<<endl;
    m_sql->UpdataMysql(strSql);

    //文件夹不能删除，不然对应路径就没了
    {
        char strSql[1000];
        dir += name +"/";
        list<string> lstRes;
        //注意：这里一定要查询f_id 因为是查当前目录下别的文件或者文件夹的
        sprintf(strSql,"select f_id , f_name , f_count , f_path , f_type from user_file_info where u_id = %d and f_dir = '%s';",
                uid,dir.c_str());
        m_sql->SelectMysql(strSql,5,lstRes);
        if(lstRes.size() == 0) return;
        for(auto ite = lstRes.begin(); ite != lstRes.end();){
            int fid = atoi((*ite).c_str());ite++;
            string name = *ite; ite++; //带文件的下载需要dir
            int count = atoi((*ite).c_str());ite++;
            string path = *ite; ite++;
            string type = *ite; ite++;
            cout<<name<<endl;
            if(type == "file"){
                DeleteFile(uid,fid,dir,name,count,path);
            }else{
                DeleteFolder(uid,fid,dir,name,count,path);

            }

        }
    }
}

void CLogic::ShareItem(int userid, int fid, string time, int link, string dir)
{
    cout<<"ShareItem"<<endl;
    char strSql[1000];
    list<string> lstRes;
    sprintf(strSql,"select f_type ,f_name from t_file where f_id = %d;",fid);
    m_sql->SelectMysql(strSql,2,lstRes);
    if(lstRes.size() == 0) return;
    for(auto ite = lstRes.begin(); ite != lstRes.end();){
        string name = *ite; ite++; //带文件的下载需要dir
        string type = *ite; ite++;

        if(type == "file"){
            ShareFile(userid,fid,dir,time,link);
        }else{
            ShareFolder(userid,fid,dir,time,link,name);
        }

    }
}

void CLogic::ShareFile(int userid, int fid, string dir, string time, int link)
{
    char strSql[1000];
    sprintf(strSql,"insert into t_shareFile values(%d,%d,'%s',%d,NULL,'%s');",
            userid,fid,dir.c_str(),link,time.c_str());
    bool res = m_sql->UpdataMysql(strSql);
    if(!res){
        cout<<"insert fail:"<<strSql<<endl;
    }
}

void CLogic::ShareFolder(int userid, int fid, string dir, string time, int link, string name)
{
    char strSql[1000];
    sprintf(strSql,"insert into t_shareFile values(%d,%d,'%s',%d,NULL,'%s');",
            userid,fid,dir.c_str(),link,time.c_str());
    bool res = m_sql->UpdataMysql(strSql);
    if(!res){
        cout<<"insert fail:"<<strSql<<endl;
    }

    //分享该文件夹下的文件和文件夹
    {
        char strSql[1000];
        dir += name +"/";
        list<string> lstRes;
        //注意：这里一定要查询f_id 因为是查当前目录下别的文件或者文件夹的
        sprintf(strSql,"select  f_id ,f_name,f_type from user_file_info where u_id = %d and f_dir = '%s';",userid,dir.c_str());
        m_sql->SelectMysql(strSql,3,lstRes);
        if(lstRes.size() == 0) return;
        for(auto ite = lstRes.begin(); ite != lstRes.end();){
            int fid = atoi((*ite).c_str());ite++;
            string name = *ite; ite++; //带文件的下载需要dir
            string type = *ite; ite++;
            cout<<name<<endl;
            if(type == "file"){
                ShareFile(userid,fid,dir,time,link);
            }else{
                ShareFolder(userid,fid,dir,time,link,name);
            }

        }
    }
}

void CLogic::GetShareFile(int userid, int fid, string dir)
{
    char strSql[1000];
    sprintf(strSql,"insert into t_user_file values(%d,%d,'%s');",
            userid,fid,dir.c_str());
    bool res = m_sql->UpdataMysql(strSql);
    if(!res){
        cout<<"insert fail:"<<strSql<<endl;
    }

}

void CLogic::GetShareFolder(int userid, int fid, string dir, string name, int fromUid, string fromDir)
{
    //先将当前文件夹添加到t_user_file对应的映射表中
    GetShareFile(userid,fid,dir);

    //拼路径
    dir += name + '/';
    fromDir += name + '/';
    //根据dir 和 userid，查发送方的 f_name f_id u_id f_type f_dir
    char sqlbuf[1000];
    sprintf(sqlbuf,"select f_name,f_id,u_id,f_type,f_dir from user_file_info where u_id = %d and f_dir = '%s';",
            fromUid,fromDir.c_str());
    list<string> lstRes;
    bool res = m_sql->SelectMysql(sqlbuf,5,lstRes);
    if(!res){
        cout<<"select fail:"<<sqlbuf<<endl;
    }
    while(lstRes.size() != 0){
        string name = lstRes.front();lstRes.pop_front();
        int fid = stoi(lstRes.front());lstRes.pop_front();
        int fromUid = stoi(lstRes.front());lstRes.pop_front();
        string type = lstRes.front();lstRes.pop_front();
        string fromDir = lstRes.front();lstRes.pop_front();

        if(type == "file"){
            GetShareFile(userid,fid,dir);
        }else{
            GetShareFolder(userid,fid,dir,name,fromUid,fromDir);
        }
    }
}












