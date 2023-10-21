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
#define HEARTBEAT_ID_LENGTH 16
#define MAX_HEARBEAT_LENGTH 16
#define PACKAGE_SIZE 1024
#define CKP_DATA_SIZE 1024

struct Info{
public:
    char time[32] = "";
    char agent_id[AGENT_ID_LENGTH] = "";
    char id[ID_LENGTH] = "";
    char ip[IP_LENGTH] = "127.0.0.1";
    char port[PORT_LENGTH] = "8081";
    char hw_info[128] = "";
};

struct Checkpoint_Heartbeat{
public:
    char id[ID_LENGTH] = "";
    char time[32] = "";
    char actural_heartbeats[MAX_HEARBEAT_LENGTH][HEARTBEAT_ID_LENGTH];
    char expected_heartbeats[MAX_HEARBEAT_LENGTH][HEARTBEAT_ID_LENGTH];
    char ip[IP_LENGTH] = "127.0.0.1";
    char port[PORT_LENGTH] = "8081";
};

enum Func_Name{RETURN, POST_INFO, POST_CKP, GET_CKP};
enum Checkpoint_Type{HEART_BEAT};
enum Rule_Type{BOARD_LIVENESS_INTEGRITY_CHECK_FAILED};

class DataPackage{
public:
    Func_Name func_name;
    char params[1024];
};

struct Checkpoint{
public:
    Checkpoint_Type type;
    char data[CKP_DATA_SIZE];
};

struct Checkpoint_My{
public:
    char time[32] = "";
    char agent_id[AGENT_ID_LENGTH] = "";
    char id[ID_LENGTH] = "";
    char ckp_id[CKP_ID_LENGTH] = "";
    char other_info[128] = "";
};

struct ErrorPackage{
    Rule_Type type;
    char id[ID_LENGTH];
};
const char* os_socket = "client.sock";
const char* agent_socket = "../Agent/server.sock";
const char local_id[ID_LENGTH] = "123456";
const char* end_id = "000";