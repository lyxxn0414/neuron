#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "Agent.hpp"

using namespace std;

int main() {
    Agent *agent = new Agent();
    std::thread t1(&Agent::handle_request, agent); 
    std::thread t2(&Agent::schedule_upload_info, agent); 
    t1.join();
    t2.join();
}
