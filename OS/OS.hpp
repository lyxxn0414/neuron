#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <thread>

#include "../Utils/types.hpp"

using namespace std;

class OS{
public:
    int cfd = 0;
    OS(){
        connect_socket();
    }
    ~OS(){
        printf("cfd closed\n");
        close(cfd);
    }
    bool post_info(Info info){
        char buf[1024];
        memcpy(buf,&info,sizeof(info));  
        DataPackage *dp = new DataPackage();
        dp->func_name = POST_INFO;
        memcpy(dp->params, buf, sizeof(info));
        send(cfd, (const char*)dp, sizeof(info)+sizeof(dp->func_name)+1, 0);
        char b[1024];
        int len = recv(cfd, b, sizeof(b), 0);
        if(len == -1) {
            perror("recv");
            exit(-1);
        } else if(len == 0) {
            printf("server closed....\n");
        } else if(len > 0) {
            DataPackage *ret = (DataPackage *)b;
            bool *res = (bool *) ret->params;
            printf("%d\n",*res);
        }
    }
    bool post_ckp(Checkpoint_My ckp[], int num){
        DataPackage *dp = new DataPackage();
        dp->func_name = POST_CKP;
        for(int i=0;i<num;i++){
            memcpy(dp->params+i*sizeof(Checkpoint_My)/sizeof(char), &ckp[i],sizeof(Checkpoint_My));
        }
        send(cfd, (const char*)dp, num*sizeof(Checkpoint_My)+sizeof(dp->func_name)+1, 0);
        char b[1024];
        int len = recv(cfd, b, sizeof(b), 0);
        if(len == -1) {
            perror("recv");
            exit(-1);
        } else if(len == 0) {
            printf("server closed....\n");
        } else if(len > 0) {
            DataPackage *ret = (DataPackage *)b;
            bool *res = (bool *) ret->params;
            printf("%d\n",*res);
        }
    }
    bool load_ckp(char id[][CKP_ID_LENGTH], int num){
        DataPackage *dp = new DataPackage();
        dp->func_name = GET_CKP;
        for(int i=0;i<num;i++){
            memcpy(dp->params+i*CKP_ID_LENGTH, id[i],CKP_ID_LENGTH);
        }
        send(cfd, (const char*)dp, num*CKP_ID_LENGTH+sizeof(dp->func_name)+1, 0);
        char buf[1024];
        int len = recv(cfd, buf, sizeof(buf), 0);
        if(len == -1) {
            perror("recv");
            exit(-1);
        } else if(len == 0) {
            printf("server closed....\n");
        } else if(len > 0) {
            DataPackage *dp = (DataPackage *)buf;
            int num2 = len / sizeof(Checkpoint_My);
            Checkpoint_My temp2[num2];
            memcpy(temp2, dp->params, num2*sizeof(Checkpoint_My));
            for(int i = 0;i<num2;i++){
                cout << "The CKP id:" << temp2[i].id <<"info:" <<temp2[i].other_info << "time:" << *(uint32_t*)(temp2[i].time)<<endl;
            }
        }
    }
    bool post_heartbeat(Checkpoint_Heartbeat* hb){
        cout<< hb->id <<"," <<hb->time << "," <<hb->expectedId[0]<<","<<hb->expectedId[1]<<endl;
        Checkpoint ckp;
        memcpy(ckp.data,hb,sizeof(Checkpoint_Heartbeat)); 
        ckp.type = HEART_BEAT;
        char buf[PACKAGE_SIZE];
        DataPackage *dp = new DataPackage();
        dp->func_name = POST_CKP;
        memcpy(dp->params, &ckp, sizeof(Checkpoint_Heartbeat)+sizeof(Checkpoint_Type));
        send(cfd, (const char*)dp, sizeof(Checkpoint_Heartbeat)+sizeof(Checkpoint_Type), 0);
        char b[1024];
        int len = recv(cfd, b, sizeof(b), 0);
        if(len == -1) {
            perror("recv");
            exit(-1);
        } else if(len == 0) {
            printf("server closed....\n");
        } else if(len > 0) {
            DataPackage *ret = (DataPackage *)b;
            bool *res = (bool *) ret->params;
            printf("%d\n",*res);
        }
    }
    //建立和Agent套接字连接
    void connect_socket(){
        unlink(os_socket);

        // 1.创建套接字
        cfd = socket(AF_LOCAL, SOCK_STREAM, 0);

        // 2.绑定本地套接字文件
        struct sockaddr_un addr;
        addr.sun_family = AF_LOCAL;
        strcpy(addr.sun_path, os_socket);
        int ret = bind(cfd, (struct sockaddr *)&addr, sizeof(addr));
        if(ret == -1) {
           perror("bind");
            exit(-1);
        }

        // 3.连接服务器
        struct sockaddr_un seraddr;
        seraddr.sun_family = AF_LOCAL;
        strcpy(seraddr.sun_path, agent_socket);
        ret = connect(cfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
        if(ret == -1) {
            perror("connect");
            exit(-1);
        }
    }

    //监听发送的错误信息
    void listen_for_error(){
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "无法创建socket" << std::endl;
        }

        // 准备地址结构体
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(PORT);

        // 绑定socket到地址
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "绑定失败" << std::endl;
        }

        // 监听连接
        if (listen(serverSocket, 5) == -1) {
            std::cerr << "监听失败" << std::endl;
        }

        std::cout << "等待客户端连接..." << std::endl;

        while (true) {
            // 接受连接
            sockaddr_in clientAddress{};
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
            if (clientSocket == -1) {
                std::cerr << "接受连接失败" << std::endl;
            }

            // 接收消息
            char buffer[4096];
            memset(buffer, 0, sizeof(buffer));
            int len = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (len == -1) {
                std::cerr << "接收消息失败" << std::endl;
            }

            // int num2 = len / sizeof(Checkpoint_My);
            // Checkpoint_My temp2[num2];
            // memcpy(temp2, buffer, num2*sizeof(Checkpoint_My));
            // cout<< "recv:" << len <<endl;
            // for(int i = 0;i<num2;i++){
            //     cout << i<<":"<<"The CKP id:" << temp2[i].ckp_id <<"info:" <<temp2[i].other_info << "time:" << *(uint32_t*)(temp2[i].time)<<endl;
            // }
            cout<< "recv:" << len <<endl;
            ErrorPackage *err = (ErrorPackage *)buffer;
            switch (err->type)
            {
            case BOARD_DEAD:{
                cout << "Err id:"<<err->id<<endl;
                break;
            }
            
            default:
                break;
            }

            // 关闭客户端socket
            close(clientSocket);
        }
        close(serverSocket);
    }

    void test_post_info(char* id){
        Info info;
        // 基于当前系统的当前日期/时间
        time_t now = time(0);
   
        // 把 now 转换为字符串形式
        char* dt = ctime(&now);
        const char* temp_char = std::to_string(now).c_str();
        char* hw_info = "This is info.";
        memcpy(info.time,temp_char,strlen(temp_char));
        memcpy(info.id,id,strlen(id));
        memcpy(info.agent_id,local_id,strlen(local_id));
        memcpy(info.hw_info,hw_info,strlen(hw_info));
        post_info(info);
    }

    void test_post_ckp(){
    Checkpoint_My ckp;
    // 基于当前系统的当前日期/时间
    time_t now = time(0);
    const char* temp_char = std::to_string(now).c_str();
    char* id = "111";
    char* ckp_id = "666";
    char* other_info = "This is info.";
    memcpy(ckp.time,temp_char,strlen(temp_char));
    memcpy(ckp.id,id,strlen(id));
    memcpy(ckp.agent_id,local_id,strlen(local_id));
    memcpy(ckp.ckp_id,ckp_id,strlen(ckp_id));
    memcpy(ckp.other_info,other_info,strlen(other_info));
    Checkpoint_My ckp2;
    char* id2 = "111";
    char* ckp_id2 = "888";
    char* other_info2 = "This is info.";
    memcpy(ckp2.time,temp_char,strlen(temp_char));
    memcpy(ckp2.id,id2,strlen(id2));
    memcpy(ckp2.agent_id,local_id,strlen(local_id));
    memcpy(ckp2.ckp_id,ckp_id2,strlen(ckp_id2));
    memcpy(ckp2.other_info,other_info2,strlen(other_info2));
    Checkpoint_My arr[2] = {ckp,ckp2};
    post_ckp(arr,2);
}


    void test_1(){
        char* id1 = "111";
        char* id2 = "222";
        char* id3 = "333";
        //前十秒每2秒发送一次心跳信号
        for(int i=0;i<5;i++){
            test_post_info(id1);
            test_post_info(id2);
            test_post_info(id3);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        //板卡1发送checkpoint
        test_post_ckp();
        //第十秒起板卡111阵亡
        for(int i=0;i<20;i++){
            test_post_info(id2);
            test_post_info(id3);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    void test_post_heartbeat(){
        Checkpoint_Heartbeat *heartbeat = new Checkpoint_Heartbeat();
        // 基于当前系统的当前日期/时间
        time_t now = time(0);
        const char* temp_char = std::to_string(now).c_str();
        memcpy(heartbeat->time,temp_char,strlen(temp_char));
        char* id = "123456";
        char expected[MAX_HEARBEAT_LENGTH][HEARTBEAT_ID_LENGTH] = {"111","222","333","444","555","000"};
        char actual[MAX_HEARBEAT_LENGTH][HEARTBEAT_ID_LENGTH] = {"111","222","333","444","000"};
        memcpy(heartbeat->id,id,strlen(id)); 
        memcpy(heartbeat->heartbeatId,actual,sizeof(actual));
        memcpy(heartbeat->expectedId,expected,sizeof(expected));
        post_heartbeat(heartbeat);
    }
};