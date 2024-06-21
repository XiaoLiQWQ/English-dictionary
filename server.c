/*===============================================
*   文件名称：server.c
*   创 建 者：     
*   创建日期：2024年05月09日
*   描    述：
================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

#define DATABASE "my.db"
#define N 32
#define R 1 //用户注册
#define L 2//登陆
#define Q 3//查询
#define H 4//记录

//定义通信双方结构体信息
typedef struct {
    int type;
    char name[N];
    char data[256];
    
}MSG;

int do_client(int accept,sqlite3 *db);

int history_callback(void *arg,int f_num,char** f_value,char** f_name);

void do_register(int acceptfd,MSG *msg,sqlite3 *db){
    
    char *errmsg;
    char sql[1024];
    sprintf(sql,"insert into usr values('%s','%s');",msg->name,msg->data); 
    printf("%s\n",sql);
    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
    
        printf("%s\n",errmsg);
        strcpy(msg->data,"usr name alread exit");

    }else{
        printf("client register ok...\n");
        strcpy(msg->data,"ok");
    }
    if(send(acceptfd,msg,sizeof(MSG),0) < 0){
        perror("send");
        return;
    }
        return;
}
int do_login(int acceptfd,MSG *msg,sqlite3 *db){

    char sql[1024] = {};
    char *errmsg;
    int nrow;
    int ncloumn;
    char **resultp;

    sprintf(sql,"select * from usr where name = '%s' and pass = '%s';",msg->name,msg->data);
    if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg) != SQLITE_OK){
        
    printf("%s\n",errmsg);
    return -1;

    }else{
    
        printf("查询成功..\n");
        
    }
    //查询成功数据库中存在
    if(nrow == 1){
        strcpy(msg->data,"OK");
        send(acceptfd,msg,sizeof(MSG),0);
        return 1;
    }

    if(nrow == 0){ //密码或者用户名错误

        strcpy(msg->data,"usr/passwd wrong");
        send(acceptfd,msg,sizeof(MSG),0);
        
    }

    return 0;

}
int do_history(int acceptfd,MSG *msg,sqlite3 *db){
  /*  char sql[512];
    MSG *msg;
    char *errmsg;
    sprintf(sql,"select * from record where name = '%s'",msg->name);

    //查询数据库
    if(sqlite3_exec(db,sql,history_callback,(void*)&acceptfd,&errmsg)!=SQLITE_OK){
        
        printf("%s\n",errmsg);

    }else{
    
        printf("Query record done..\n");

    }
    //查询之后 返回结束信息
    msg->date[0] = '\0';
    send(acceptfd,msg,sizeof(MSG),0);
    return 0;
*/
}

//得到查询结果,并且需要将历史记录发送给客户端

int history_callback(void *arg,int f_num,char** f_value,char** f_name){
        
 /*   //record,name ,date,word
    int acceptfd;
    MSG msg;
    acceptfd =*((int *)arg);
    sprintf(msg.data,"%s,%s",f_value[1],f_value[2]);
    send(acceptfd,&msg,sizeof(MSG),0);
*/
    return 0;
}

int do_searchword(int acceptfd,MSG *msg,char word[]){

    //打开文件 读取，进行查找
    FILE *fp;
    int len = 0;
    char temp[512] = {};
    int result;
    char *p;
    if((fp = fopen("dict.txt","r")) == NULL){
    
        perror("fopen");
        strcpy(msg->data,"fail open dict.txt");
        send(acceptfd,msg,sizeof(MSG),0);
        return -1;
    }
    //打印出查询的单词
    len = strlen(word);
    printf("%s,len = %d\n",word,len);

    //获取文件，查询单词
    while(fgets(temp,512,fp)!=NULL){
         
        result = strncmp(temp,word,len);
        printf("result:%d,%s\n",result,temp);
        if(result < 0){
            continue;
        }
    
        if(result > 0 || ((result == 0) && (temp[len]!=' '))){
            break; 

        }
        
        //表示找到了单词
        p = temp + len;
        while(*p == ' '){
    
            p++;
        
        }
        //找到了注释
        strcpy(msg->data,p);
        printf("found word:%s\n",msg->data); 
        //关闭文件
        fclose(fp);
        return 1;
    }
    return 0;
    fclose(fp);
}

int get_date(char *date){

    time_t t;
    struct tm *tp;
    time(&t);
    //进行转化
    localtime(&t);
    
    sprintf(date,"%d-%d-%d %d:%d:%d",tp->tm_year +1900,tp->tm_mon+1,tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);


    return 0;
}   

int do_query(int acceptfd,MSG *msg,sqlite3 *db){
    
    char word[64];
    int found = 0;
    char date[128];
    char sql[1024];
    char *errmsg;

    //拿出结构体中 查询的单词
    strcpy(word,msg->data);
    found = do_searchword(acceptfd,msg,word);
    
    //找到单词,插入到历史记录中
    if(found == 1){
        //获取时间
        get_date(date);
        sprintf(sql,"insert into record values('%s','%s','%s')",msg->name,date,word);
        printf("%s\n",sql);

        if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
        printf("%s\n",errmsg);
        return -1;
        
        }

    }else{
    //没有找到
        strcpy(msg->data,"Not found");

    }

    //将查询的结果，发送给客户端
    send(acceptfd,msg,sizeof(MSG),0);


    return 0;
}
int main(int argc, char *argv[])
{ 
    int sockfd;
    struct sockaddr_in serveraddr;
    int n;
    MSG msg;
    sqlite3 *db;
    int acceptfd;
    pid_t pid;

    if(argc != 3){
        printf("Usage:%s serverip port.\n",argv[0] );
            return -1;
    }
    //数据库
    if(sqlite3_open(DATABASE,&db) != SQLITE_OK){
    
        printf("%s\n",sqlite3_errmsg(db));
        return -1;

    } else{
    
        printf("Open Database success..\n");

    }
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){

        perror("socket\n");
        return -1;

    }
    memset(&serveraddr,0,sizeof(serveraddr));//将结构体清零
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));
    
    if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0){
        perror("bind");
        return -1;


    }
    //将套接子设为监听模式
    if(listen(sockfd,5) < 0){
    
        printf("fail to listen\n");
        return -1;

    }
    
    //处理僵尸进程
    signal(SIGCHLD,SIG_IGN);
    

while(1){
    
    if((acceptfd = accept(sockfd,NULL,NULL)) < 0){
    
        perror("accept");
        return -1;
    
    }
    if((pid = fork()) < 0){

        perror("fork");
        return -1;
        
    }else if(pid == 0){
    
        //处理客户端具体的消息
        close(sockfd);
        do_client(acceptfd,db);
    
    }else{
        //处理接受客户端的请求
        close(acceptfd);
    }

    
}


next:
    while(1){
        
    
        
    }



    return 0;
}
int do_client(int acceptfd,sqlite3 *db){
    
    MSG msg;

    while(recv(acceptfd,&msg,sizeof(msg),0) > 0){
        printf("type:%d\n",msg.type);
        switch(msg.type){
            
            case R:
                do_register(acceptfd,&msg,db);
                break;
            case L:
                do_login(acceptfd,&msg,db);
                break;
            case Q:
                do_query(acceptfd,&msg,db);
                break;
            case H:
                do_history(acceptfd,&msg,db);
                break;
            default:
                printf("Invalid error...\n");


        }
    


    }
    printf("客户端退出..\n");
    close(acceptfd);
    exit(0);
    return 0;

}



