#include "protocol.h"

Message create_message(const std::string &sender,
                      const std::string &receiver,
                      const char *data, int len)
{
    //std::cout << "inside  "<< __FUNCTION__ << std::endl;
    Header header;
    header.length =  len;
    header.type = MSG;
    header.version = VERSION;
    header.flags = SYN | ACK;
    Message msg;
    msg.data = new char[len+1];


    memset(msg.data, 0, len+1);
    memset(msg.sender, 0, STR_LEN);
    memset(msg.receiver, 0, STR_LEN);
    memcpy(msg.receiver, receiver.c_str(), receiver.size());
    memcpy(msg.sender, sender.c_str(), sender.size());
    msg.header = header;

    memcpy(msg.data, data, len);
    //std::cout << "leaving  "<< __FUNCTION__ << std::endl;
    return msg;
}

LogInOut create_loginout(const std::string &username, bool IN)
{
    LogInOut log;
    log.header.type     = LOGINOUT;
    log.header.flags    = (IN == true) ? SYN : SYN | FIN;
    log.header.version  = VERSION;
    log.header.length   = username.size();
    memset(log.username, 0, STR_LEN);
    strcpy(log.username, username.c_str());
    return log;
}

ControlInfo create_controlInfo(const std::vector<std::string> &users)
{
    ControlInfo info;
    info.header.type = CONTROLINFO;
    info.header.version = VERSION;
    if(users.empty())
    {
        info.header.flags = GET;
        info.header.length = 0;
    }
    else
    {
        info.entries = new Entry[users.size()];
        info.header.flags = 0;
        for(int i = 0; i < users.size(); i++)
        {
            memset(info.entries[i].username,
                   0,
                   STR_LEN);

            info.entries[i].hops = 1;
            memcpy(info.entries[i].username,
                   users.at(i).c_str(),
                   users.at(i).size());
        }
        info.header.length = users.size();

    }
    return info;
}
