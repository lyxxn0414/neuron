#include <stdio.h>
#include <iostream>
#include <string.h>

#define ID_LENGTH 16
#define AGENT_ID_LENGTH 16
#define CKP_ID_LENGTH 16
//多久发送一次监控信息给监控存储。单位：s
#define SEND_INFO_PERIOD 1
//用于故障恢复策略的端口
#define PORT 8081
//本地IP
#define IP_LENGTH 16
#define PORT_LENGTH 8

struct Info{
public:
    char time[32] = "";
    char agent_id[AGENT_ID_LENGTH] = "";
    char id[ID_LENGTH] = "";
    char ip[IP_LENGTH] = "127.0.0.1";
    char port[PORT_LENGTH] = "8081";
    char hw_info[128] = "";
};

struct Checkpoint{
public:
    char time[32] = "";
    char agent_id[AGENT_ID_LENGTH] = "";
    char id[ID_LENGTH] = "";
    char ckp_id[CKP_ID_LENGTH] = "";
    char other_info[128] = "";
};

enum Func_Name{RETURN, POST_INFO, POST_CKP, GET_CKP};

class DataPackage{
public:
    Func_Name func_name;
    char params[1024];
};

const char* os_socket = "client.sock";
const char* agent_socket = "../Agent/server.sock";
const char local_id[ID_LENGTH] = "123456";