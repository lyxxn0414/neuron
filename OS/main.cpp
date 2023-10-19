#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <thread>

#include "OS.hpp"

using namespace std;

void test_post_info(OS *os, char* id){
    Info info;
    // 基于当前系统的当前日期/时间
    time_t now = time(0);
   
//    // 把 now 转换为字符串形式
    char* dt = ctime(&now);
    cout << dt <<endl;
    const char* temp_char = std::to_string(now).c_str();
    char* hw_info = "This is info.";
    memcpy(info.time,temp_char,strlen(temp_char));
    memcpy(info.id,id,strlen(id));
    memcpy(info.agent_id,local_id,strlen(local_id));
    memcpy(info.hw_info,hw_info,strlen(hw_info));
    os->post_info(info);
}

void test_post_ckp(OS *os){
    Checkpoint ckp;
    // 基于当前系统的当前日期/时间
    time_t now = time(0);
   const char* temp_char = std::to_string(now).c_str();
    char* id = "123456";
    char* ckp_id = "666";
    char* other_info = "This is info.";
    memcpy(ckp.time,temp_char,strlen(temp_char));
    memcpy(ckp.id,id,strlen(id));
    memcpy(ckp.agent_id,local_id,strlen(local_id));
    memcpy(ckp.ckp_id,ckp_id,strlen(ckp_id));
    memcpy(ckp.other_info,other_info,strlen(other_info));
    Checkpoint ckp2;
    char* id2 = "123456";
    char* ckp_id2 = "123";
    char* other_info2 = "This is info.";
    memcpy(ckp2.time,temp_char,strlen(temp_char));
    memcpy(ckp2.id,id2,strlen(id2));
    memcpy(ckp2.agent_id,local_id,strlen(local_id));
    memcpy(ckp2.ckp_id,ckp_id2,strlen(ckp_id2));
    memcpy(ckp2.other_info,other_info2,strlen(other_info2));
    Checkpoint arr[2] = {ckp,ckp2};
    os->post_ckp(arr,2);
}

void test_1(OS *os){
    char* id1 = "111";
    char* id2 = "222";
    char* id3 = "333";
    //前十秒每2秒发送一次心跳信号
    for(int i=0;i<5;i++){
        test_post_info(os,id1);
        test_post_info(os,id2);
        test_post_info(os,id3);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    //第十秒起板卡111阵亡
    for(int i=0;i<20;i++){
        test_post_info(os,id2);
        test_post_info(os,id3);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
int main() {
    OS *os = new OS();
    std::thread t1(&OS::listen_for_error, os); 
    std::thread t2(&OS::test_1, os); 
    t1.join();
    t2.join();
    // char* id1 = "111";
    // char* id2 = "222";
    // char* id3 = "333";
    // test_post_info(os,id1);
    // char b[3][ID_LENGTH] = {"123","456","789"};
    // os->load_ckp(b, 3);
    // test_post_ckp(os);
    // char a[2][ID_LENGTH] = {"123","666"};
    // os->load_ckp(a, 2);
}
