//
// Created by ball on 9/12/22.
//

#include "../TcpServer.h"
#include "../Logger.h"

class EchoServer {
public:
    EchoServer(EventLoop *loop,
               const InetAddress &address,
               const std::string &name):
            server_(loop,address,name),
            loop_(loop){
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection,this,std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,
                                             std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        server_.setThreadNum(3);
    }
    void start(){
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn){
        if (conn->connected()){
            LOG_INFO("conn up:%s",conn->peerAddress().toIpPort().c_str());
        }else {
            LOG_INFO("conn down:%s",conn->peerAddress().toIpPort().c_str());
        }
    }
    void onMessage(const TcpConnectionPtr &conn,Buffer *buffer,TimeStamp time) {
        std::string msg = buffer->retrieveAllAsString();
        conn->send(msg);
//        conn->shutdown();
    }
    EventLoop *loop_;
    TcpServer server_;
};


int main(){
    EventLoop loop;
    InetAddress address(8000);
    EchoServer server(&loop,address,"EchoServer");
    server.start();
    loop.loop();
    return 0;
}