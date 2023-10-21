#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>

#include "../Utils/types.hpp"

using namespace std;

class Agent{
public:
    int lfd;
    int cfd;
    int clientSocket;
    vector<Info> buffer_info;
    vector<Checkpoint_My*> buffer_ckp;
    Agent(){
        connect_storage();
        connect_os();
        // handle_request();
        // schedule_upload_info();
    }
    ~Agent(){
        close(cfd);
        close(lfd);
        close(clientSocket);
    }

    void connect_os(){
        unlink(agent_socket);

        // 1.创建监听的套接字
        lfd = socket(AF_LOCAL, SOCK_STREAM, 0);

        // 2.绑定本地套接字文件
        struct sockaddr_un addr;
        addr.sun_family = AF_LOCAL;
        strcpy(addr.sun_path, agent_socket);
        int ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr)); // 绑定会创建一个本地套接字文件serve.sock
        if(ret == -1) {
            perror("bind");
            exit(-1);
        }

        // 3.监听
        listen(lfd, 100);
        printf("Listening\n");

        // 4.等待客户端连接
        struct sockaddr_un cliaddr;
        unsigned int len = sizeof(cliaddr);
        printf("%d\n",len);
        cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len); // 连接后, 就能从cliaddr中得到客户端的本地socket文件信息
        printf("client socket filename: %s\n", cliaddr.sun_path);
    }

    void reconnect(){
        // // 3.监听
        // listen(lfd, 100);
        // printf("Listening\n");

        // 4.等待客户端连接
        struct sockaddr_un cliaddr;
        unsigned int len = sizeof(cliaddr);
        printf("%d\n",len);
        cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len); // 连接后, 就能从cliaddr中得到客户端的本地socket文件信息
        printf("client socket filename: %s\n", cliaddr.sun_path);
    }

    void handle_request(){
        while(1) {
            DataPackage *ret_buf = new DataPackage();
            char buf[1024];
            char recv_buf[1024];
            int recv_len = -1;
            int len = recv(cfd, buf, sizeof(buf), 0);

            if(len == -1) {
                perror("recv");
                exit(-1);
            } else if(len == 0) {
                printf("client closed....\n");
                reconnect();
            } else if(len > 0) {
                printf("recv %d\n",len);
                DataPackage *dp = (DataPackage *)buf;
                switch(dp->func_name){
                    case POST_INFO:{
                        // bool res = save_info(buf,len);
                        bool res = save_info(dp);
                        memcpy(ret_buf->params,&res,sizeof(res));
                        cout<< res << endl;
                        recv_len = sizeof(res)+sizeof(ret_buf->func_name)+1;
                        break;
                    }
                    case POST_CKP:{
                        // int num = len/sizeof(Checkpoint_My);
                        // bool res = save_ckp(buf,len);
                        cout << "post heartbeat"<<endl;
                        // Checkpoint ckp;
                        // memcpy(&ckp, dp->params, sizeof(ckp)); 
                        bool res = post_ckp(dp);
                        memcpy(ret_buf->params,&res,sizeof(res));
                        recv_len = sizeof(res)+sizeof(ret_buf->func_name)+1;
                        break;
                    }
                    case GET_CKP:{
                        int num = len/ID_LENGTH;
                        cout << "The num is" << num <<endl;
                        char* res = get_ckp(dp,num);
                        int arrlen = num*sizeof(Checkpoint_My);
                        memcpy(ret_buf->params,res,arrlen);
                        recv_len = arrlen+sizeof(ret_buf->func_name)+1;
                        break;
                    }
                    default:{
                        cout << "Func Type Error!" << endl;
                    }
                }
                send(cfd, (const char*)ret_buf, recv_len, 0);
            }
        }
        connect_os();
    }

    bool save_info(DataPackage *dp){
        Info info;
        memcpy(&info, dp->params, sizeof(info)); 
        cout << "The Info is:" << "{ " << "time:" << info.time << ",id:" << info.id << ",hw_info:" << info.hw_info  <<endl;
        buffer_info.push_back(info);
        return true;
    }
    bool upload_info(){
        int num = buffer_info.size();
        if(num>0){
            DataPackage *dp = new DataPackage();
            dp->func_name = POST_INFO;
            for(int i=0;i<num;i++){
                cout << "The Info is:" << "{ " << "time:" << buffer_info[i].time << ",id:" << buffer_info[i].id << ",hw_info:" << buffer_info[i].hw_info  <<endl;
                memcpy(dp->params+i*sizeof(Info)/sizeof(char), &buffer_info[i],sizeof(Info));
            }

            // 发送数据
            if (send(clientSocket, (const char*)dp, num*sizeof(Info)+sizeof(dp->func_name)+1, 0) == -1) {
                std::cerr << "发送数据失败" << std::endl;
            }

            // 接收服务器响应
            char buffer[1024];
            int len = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (len == -1) {
                std::cerr << "接收服务器响应失败" << std::endl;
            }
            buffer_info.erase(buffer_info.begin(),buffer_info.begin()+num);
            DataPackage *dp2 = (DataPackage *)buffer;
            bool *res = (bool *) dp2->params;
            printf("upload_info res: %d\n",*res);
            return *res;
        }
    }
    void schedule_upload_info(){
        while(true){
            upload_info();
            std::this_thread::sleep_for(std::chrono::seconds(SEND_INFO_PERIOD));
        }
    }
    bool save_ckp_test(DataPackage *dp, int num){
        char temp[num][sizeof(Checkpoint_My)];
        memcpy(temp, dp->params, num*sizeof(Checkpoint_My));
        for(int i = 0;i<num;i++){
            Checkpoint_My *ckp = (Checkpoint_My*) temp[i];
            cout << "The ckp is:" << "{ " << "time:" << ckp->time << ",id:" << ckp->id << ",hw_info:" << ckp->other_info  <<endl;
            buffer_ckp.push_back(ckp);
        }

        // 发送数据
        if (send(clientSocket, (const char*)dp, num*sizeof(Checkpoint_My)+sizeof(dp->func_name)+1, 0) == -1) {
            std::cerr << "发送数据失败" << std::endl;
        }

        // 接收服务器响应
        char buffer[1024];
        int len = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (len == -1) {
            std::cerr << "接收服务器响应失败" << std::endl;
        }
        printf("recv2 %d\n",len);
        bool *res = (bool *) buffer;
        printf("save_ckp res: %d\n",*res);
        return *res;
    }
    bool post_ckp(DataPackage *dp){
        // 发送数据
            if (send(clientSocket, (const char*)dp, PACKAGE_SIZE, 0) == -1) {
                std::cerr << "发送数据失败" << std::endl;
            }

            // 接收服务器响应
            char buffer[1024];
            int len = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (len == -1) {
                std::cerr << "接收服务器响应失败" << std::endl;
            }
            DataPackage *dp2 = (DataPackage *)buffer;
            bool *res = (bool *) dp2->params;
            printf("upload_ckp res: %d\n",*res);
            return *res;
        // switch (ckp.type)
        // {
        //     case HEART_BEAT:{
        //     Checkpoint_Heartbeat *heartbeat = (Checkpoint_Heartbeat*)ckp.data;
        //     cout << heartbeat->id<<","<<heartbeat->time<<","<<heartbeat->expectedId[0]  <<endl;
        //     break;
        //     }
        
        //     default:{
        //     cout<<"Type Err"<<endl;
        //     break;
        //     }
        // }
        // return true;
    }
    bool save_ckp(char buf[], int num){
        // 发送数据
        if (send(clientSocket, (const char*)buf, num, 0) == -1) {
            std::cerr << "发送数据失败" << std::endl;
        }

        // 接收服务器响应
        char buffer[1024];
        int len = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (len == -1) {
            std::cerr << "接收服务器响应失败" << std::endl;
        }
        printf("recv2 %d\n",len);
        bool *res = (bool *) buffer;
        printf("save_ckp res: %d\n",*res);
        return *res;
    }
    char* get_ckp(DataPackage *dp, int num){
        char temp[num][CKP_ID_LENGTH];
        memcpy(temp, dp->params, num*CKP_ID_LENGTH);
        for(int i = 0;i<num;i++){
            cout << "The id is:" << temp[i]<<endl;
        }
        DataPackage* a_dp = new DataPackage();
        a_dp->func_name = dp->func_name;
        memcpy(a_dp->params,local_id,ID_LENGTH);
        memcpy(a_dp->params+ID_LENGTH,dp->params,num*CKP_ID_LENGTH);
        
        // 发送数据
        int test = send(clientSocket, (const char*)a_dp, ID_LENGTH+num*CKP_ID_LENGTH+sizeof(dp->func_name)+1, 0);
        cout<< test <<endl;
        if ( test == -1) {
            std::cerr << "发送数据失败" << std::endl;
        }

        // 接收服务器响应
        char buffer[1024];
        int len = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (len == -1) {
            std::cerr << "接收服务器响应失败" << std::endl;
        }
        // printf("recv2 %d\n",len);

        int num2 = len/sizeof(Checkpoint_My);
        cout<< num2 << endl;
        Checkpoint_My temp2[num2];
        memcpy(temp2, buffer, num2*sizeof(Checkpoint_My));
        for(int i = 0;i<num2;i++){
            cout << "The CKP id:" << temp2[i].ckp_id <<"info:" <<temp2[i].other_info << "time:" << *(uint32_t*)(temp2[i].time)<<endl;
        }
        char* res = new char[num2*sizeof(Checkpoint_My)];
        memcpy(res, buffer, num2*sizeof(Checkpoint_My));
        return res;
    }
    int connect_storage(){
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "无法创建socket" << std::endl;
            return -1;
        }

        // 设置服务器地址和端口
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(8080); // 替换为实际的服务端端口
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // 替换为实际的服务端IP地址

        // 连接到服务器
        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "连接到服务器失败" << std::endl;
            close(clientSocket);
            return -1;
        }
        cout<< "connect success" << endl;
    }
};