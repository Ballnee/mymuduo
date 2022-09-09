//
// Created by ball on 9/9/22.
//

#include "Poller.h"
#include "Channel.h"


Poller::Poller(EventLoop *loop):ownerLoop_(loop) {}

bool Poller::hasChannel(Channel *channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

int main(){return 0;}