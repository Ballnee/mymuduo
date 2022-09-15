//
// Created by ball on 9/14/22.
//
#include "mymuduo/Logger.h"
#include "mymuduo/TcpServer.h"
#include<fstream>
#include <sstream>
#include "iostream"
#include <stdio.h>
#include "cassert"


std::string readFile(const std::string &filename) {
   std::ifstream infile;
   infile.open(filename.data());

    assert(infile.is_open());

    std::string str;
    std::string res;
    while(getline(infile,str)) {
        res.append(str+"\n");
    }
    return res;
}

void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("fileServer - %s ->  %s is up",conn->peerAddress().toIpPort().c_str(),
             conn->localAddress().toIpPort().c_str());
    if(conn->connected()) {
        std::string fileContent = readFile("./file.txt");

        conn->send(fileContent);
//        conn->shutdown();
    }
}

void onMessage(const TcpConnectionPtr &conn,Buffer *buffer,TimeStamp time) {
    std::string msg = buffer->retrieveAllAsString();
    conn->send(msg);
//    conn->shutdown();
}

int main(int argc,char* argv[]){
    LOG_INFO("pid = %d\n",getpid());

    EventLoop loop;
    InetAddress  listenAddr(8080);
    TcpServer server(&loop,listenAddr,"fileServer");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}
