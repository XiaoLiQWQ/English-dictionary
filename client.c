/*===============================================
*   文件名称：client.c
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
#include <string.h>

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

int do_register(int sockfd,MSG *msg){
    
    msg->type = R;
    printf("Input name:");
    scanf("%s",msg->name);
    getchar();
    
    printf("Input password:");
    scanf("%s",msg->data);

    if(send(sockfd,msg, sizeof(MSG),0) < 0){
        
        printf("fail to send...\n");
        return -1;
    }
    if(recv(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to recv..\n");
        
        return -1;
    }
    printf("%s\n",msg->data);
    return 0;
}
int do_login(int sockfd,MSG *msg){

    msg->type = L;
    printf("Input name:");
    scanf("%s",msg->name);
    getchar();

    printf("Input passwd:");
    scanf("%s",msg->data);
    
    if(send(sockfd,msg,sizeof(MSG),0) < 0){
    
        printf("faile to send...\n");
        return -1;

    }

    if(recv(sockfd,msg,sizeof(MSG),0) < 0){
    
        printf("fail to recv...\n");
        return -1;
    }
    if(strncmp(msg->data,"OK",3) == 0){
        printf("登陆成功\n");
        return 1;
    }else{
    
        printf("%s\n",msg->data);

    }

    return 0;

}
int do_history(int sockfd,MSG*msg){
    msg->type = H;
    send(sockfd,msg,sizeof(MSG),0);
    
    while(1){
    recv(sockfd,msg,sizeof(MSG),0);
    
    if(msg->data[0] = '\0'){
        
        break;

    }
    printf("%s\n",msg->data);
    }
    return 0;

}
int do_query(int sockfd,MSG *msg){

    msg->type = Q;
    puts("------------");
    while(1){
    
        printf("Input word:");
        scanf("%s",msg->data);
        getchar();
        //输入 # 结束，返回上一级
        if(strncmp(msg->data,"#",1) == 0){
            break;
        }
        //将单词发送给服务器
        if(send(sockfd,msg,sizeof(MSG),0) < 0){
            
            printf("faile to send..\n");
            return -1;

        }
        //等待接受服务器传递，传递回来的信息
        if(recv(sockfd,msg,sizeof(MSG),0) < 0){
            
            printf("faile to recv..\n");
            return -1;

        }
        printf("%s\n",msg->data);
    
    }
    return 0;

}

int main(int argc, char *argv[])
{ 
    int sockfd;
    struct sockaddr_in serveraddr;
    int n;
    MSG msg;
    if(argc != 3){
        printf("Usage:%s serverip port.\n",argv[0] );
            return -1;
    }
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){

        perror("socket\n");
        return -1;

    }
    memset(&serveraddr,0,sizeof(serveraddr));//将结构体清零
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));
    
    if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0){
        
        perror("fail to connent\n");
        return -1;

    }

while(1){
    
    printf("*****************************\n");
    printf("1.register\n");
    printf("2.login\n");
    printf("3.quit\n");
    printf("*****************************\n");

    printf("选择一个命令:");
    scanf("%d",&n);
    getchar();

    switch(n){
    
        case 1:
            do_register(sockfd,&msg);
            break;
        case 2:
            if(do_login(sockfd,&msg) == 1){
                goto next;
            }
            break;
        case 3:
            close(sockfd);
            exit(1);
            break;
        default:
            printf("重新选择:\n");
        

    }




}


next:
    while(1){
        
        printf("***************************\n");
        printf("1.查询单词\n");
        printf("2.历史查询(坏了)\n");
        printf("3.退出\n");
        printf("***************************\n");
    
        printf("选择:");
        scanf("%d",&n);
        getchar();

        switch(n){
        
            case 1:
                do_query(sockfd,&msg);
                break;
            case 2:
                do_history(sockfd,&msg);
                break;
            case 3:
                close(sockfd);
                exit(0);
                break;
            default:
                printf("Invalid data cmd...\n");
    
        }
    }



    return 0;
} 
