#include "client.h"

Client::Client(const std::string &server_ip, const std::string &port)
{
    Logger::log("Not implemented yet");
}

Client::Client(): quit{false},loggedIn{false}, socket_fd{-1}
{
    usage =std::string
                (
                     " Usage : type in the message you want to send\n"
                     " and validate your action with the Enter key.\n"
                     " To log in, please enter /login + your username\n"
                     " Please notice that the username must be maximal\n"
                     " 16 character long.\n"
                     " To logout  please enter /logout and validate with \n"
                     " with Enter key."
                     " To see the list of available users to chat with, please \n"
                     " enter /users and validate with the Enter key\n"
                     " To quit this application, please enter /quit and then\n"
                     " validate with the Enter key\n"
                );

    head =                  "-------------------------------------------\n"
                            "-- Welcome to Haw Chat Application "
                            "-- Copyright 2016 "
                            "-------------------------------------------\n";
}

void Client::init()
{
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        std::cerr << "signal" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    port = SERVER_PORT;
    // getaddrinfo() to get a list of usable addresses
    std::string host = "localhost";
    char service[NI_MAXSERV];
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    // Work with IPV4/6
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_protocol = 0;
    hints.ai_flags =  AI_NUMERICSERV ;
    // we could provide a host instead of nullptr
    if(getaddrinfo(host.c_str(),
                   port.c_str(),
                   &hints,
                   &result) != 0)
    {
        perror("getaddrinfo()");
        std::exit(EXIT_FAILURE);
    }
}

int Client::create_socket()
{
    addrinfo *rp;
    for( rp = result; rp != nullptr; rp = rp->ai_next)
    {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(socket_fd == SOCKET_ERROR)
        {
            // on error we try the next address
            continue;
        }
        if(connect(socket_fd,
                   rp->ai_addr,
                   rp->ai_addrlen) != SOCKET_ERROR)
        {
            break; // success
        }
        close(socket_fd);
    }
    if(rp == nullptr) // could not bind socket to any address of the list
    {
        std::cerr << "Fatal Error : couldn't find a suitable address" << std::endl;
        socket_fd = SOCKET_ERROR;
    }

    freeaddrinfo(rp);
    return socket_fd;
}
void Client::logout()
{
    quit = true;
    LogInOut log = create_loginout(username, false);
    void *data = Serialization::Serialize<LogInOut>::serialize(log);
    int size = STR_LEN + sizeof(Header);
    write(socket_fd, data, size);
}

int Client::login()
{
    bool username_invalid = true;
    //std::string username = "cyrildz";
    std::string username;
    Logger::log(head);
    Logger::log(usage);

    int ret = 0;
    LogInOut log;
    while(username_invalid && !loggedIn)
    {
        Logger::log( " enter a username : ");
        std::cin >> username;
        if(username.size() > STR_LEN)
        {
            Logger::log("\n username too long ");

        }
        else if(username.size() < 3)
        {
            Logger::log("\n username too short ");

        }
        else
        {
            Logger::log("username : " + username);
            log = create_loginout(username);
            void *data = Serialization::Serialize<LogInOut>::serialize(log);
            int len = log.header.length + sizeof(Header) + username.size();
            ret = send_data(data, len);
            if(ret < 0)
            {
                Logger::log("Connexion error with server");
                exit(EXIT_FAILURE);
            }
            Logger::log("Client : login sent ..");
            //std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ret = read(socket_fd, data, len);
            if(ret < 0)
            {
                Logger::log("Connexion error with server: unable to read from socket");
                exit(EXIT_FAILURE);
            }

            if(ret > 1)
            {
                Logger::log("Client : login received ..");
                decode(data, ret);
            }
            if(ret == 1)
            {
                Logger::log("Client received heartbeat signal ...");
            }

            if(loggedIn)
            {
                Logger::log("User logged in");
                this->username = username;
                break;
            }
        }

    }
}

void Client::start(bool testMode)
{
   create_socket();
   std::thread w_worker{&Client::write_task, this};
   w_worker.detach();
   login();
   if(testMode)
   {
       send_test();
   }
   else
   {
       shell();
   }


}

int Client::send_data(void *data, int size)
{
    print_raw_data((char*)data, size);
    std::pair<void*, int> entry;
    entry.first = data;
    entry.second = size;
    txd_data.push(entry);
}

int Client::decode(void *data, int size)
{

    Logger::log("Client decoding...");
    int ret = -1;
    char *ptr = (char*)data;
    LogInOut log;
    Message msg;
    flat_header header;
    memcpy(&header.value, ptr, sizeof(Header));
    switch(header.header.type)
    {
      case LOGINOUT:
        log = Serialization::Serialize<LogInOut>::deserialize(data);
        ret = process_loginout(log);
        break;
     case MSG:
        msg = Serialization::Serialize<Message>::deserialize(data);
        ret = process_message(msg);
        break;
    case CONTROLINFO:
        ret = process_get_request(data);
    }
    ptr = nullptr;
    memset(data, 0, size);
}

void Client::print_raw_data(char *data, int size) const
{
    Logger::log("Printing raw data");
    std::ofstream file;
    file.open("raw_data.log", std::ios::app);

    if(file.is_open())
    {
        if(data)
        {
            for(int i = 0; i < size; i++)
            {
                if(data[i] != 0)
                {
                    file << data[i];
                }
            }
            file << '\n';
        }
    }
    file.close();
}
void Client::shell()
{
    std::thread r_worker{&Client::read_task, this};

    r_worker.detach();
    ControlInfo info;
    LogInOut loginout;
    std::string receiver = "Jacob";
    int count = 0;
    std::string line = "";
    Message msg;
    char *ptr = nullptr;
    char buffer[BUFFER_SIZE] = {0};
    int size = 0;

    std::cin.tie(&std::cout);
    while(!quit)
    {

        Logger::log(username + " : " + line);

        //std::cin.ignore();
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(std::cin,line);
        Logger::log("input : " + line);
        if(line == "/quit" )
        {
            loginout = create_loginout(username, false);
            ptr = (char*)Serialization::Serialize<LogInOut>::serialize(loginout);
            size = sizeof(Header) + STR_LEN;
            break;
        }
        if(line == "/GET")
        {
            info = create_controlInfo(
                        std::vector<std::string>());
            ptr = (char*)Serialization::Serialize<ControlInfo>::serialize(info);
            size = sizeof(Header)
                    + (info.header.length * sizeof(Entry));
        }
        else
        {
            Logger::log("message entered");
            msg = create_message(username,receiver,line.c_str(), line.size());
            ptr = (char*)Serialization::Serialize<Message>::serialize(msg);
            size = (2 * STR_LEN) + line.size() + sizeof(Header);
        }

        send_data(ptr, size);
        size = 0;


    }
    Logger::log("Quitting the shell ...");
}

int Client::process_loginout(const LogInOut &log)
{
    int ret = -1;
    if(log.header.flags == (SYN | ACK | DUP))
    {
        Logger::log("username already in use");
    }
    else if(log.header.flags == (SYN | ACK ))
    {
        loggedIn = true;
        ret = 0;
        Logger::log("you successfuly logged in");
    }
    else if(log.header.flags == (FIN | ACK))
    {
        Logger::log("you successfuly logged out");
        quit = true;
        ret = 0;
    }

    return ret;
}


int Client::process_message(const Message &msg)
{
    std::string str = std::string(msg.sender)
            + " : "
            + std::string(msg.data);
    Logger::log(str);
}

int Client::process_get_request(void *data)
{
    ControlInfo info = Serialization::Serialize<ControlInfo>::deserialize(data);
    std::string user;
    userlist.clear();
    for(int i = 0; i < info.header.length; i++)
    {
        user = info.entries[i].username;
        userlist.push_back(user);
    }
    show_userlist();
}

void Client::send_test()
{
    Logger::log("Starting send test ...");
    std::string receiver = "Jacob";
    std::thread r_worker{&Client::read_task, this};

    r_worker.detach();
    std::vector<std::string> strings;
    int size;
    for(int i = 0; i < 10; i++)
    {
        strings.push_back(std::string("Test " + std::to_string(i)));
    }
    void *data = nullptr;
    Message msg;
    for(std::string str : strings)
    {
        msg = create_message(username, receiver, str.c_str(), str.size());

        data = Serialization::Serialize<Message>::serialize(msg);
        size = sizeof(Header) + (2*STR_LEN) + str.size();
        send_data(data, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }
    strings.clear();
    close(socket_fd);
    Logger::log("send test terminated ...");
}

void Client::show_userlist() const
{
    Logger::log("Available Users : ");
    for(std::string user : userlist)
    {
        Logger::log("* " + user);
    }
}

void Client::read_task()
{
    int count = 0;
    char buffer[BUFFER_SIZE] = {0};
    char *ptr = nullptr;
    while(!quit)
    {
        count = read(socket_fd,buffer, BUFFER_SIZE );
        if(count < 0)
        {
            perror("Read Task: ");
            quit = true;
            break;
        }
        else if(count == 1)
        {
            Logger::log("heartbeat signal received ...");
        }
        else if(count > 1)
        {
            Logger::log("new data received ...");
            ptr = new char[count];
            memcpy(ptr, buffer, count);
            decode(ptr, count);
            delete[] ptr;
        }
    }
}

void Client::write_task()
{
    std::pair<void*, int> entry;

    while(!quit)
    {
        txd_data.wait_and_pop(entry);
        Logger::log("new data to sent ...");
        if(write(socket_fd, entry.first, entry.second) < 0)
        {
            perror("Write Task: ");
            quit = true;
            break;
        }
        Logger::log("new data sent to server ...");
    }
}
