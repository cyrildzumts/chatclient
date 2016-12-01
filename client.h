#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include "logger.h"
#include "serialization.h"
#include <fstream>
#include <limits>
#include "queue.h"

class Client
{
public:
    Client(const std::string &server_ip, const std::string &port);
    Client();
    /**
     * @brief init initialize the socket this client uses.
     */
    void init();
    /**
     * @brief start call this method to start Client.
     * make sure to first call init();
     * This method prompts the user to enter his/her
     * login registers the client to the server.
     *
     */
    void start(bool testMode = false);
    /**
     * @brief create_socket creates and connects this client to the
     * server
     * @return : the created socket this client uses to communicate.
     *         : -1 on error.
     *
     */
    int create_socket();
    /**
     * @brief send_data This method pushes data in the transfert
     * queue. data will then be sent to the server by the write_task.
     * @param data The data to be sent
     * @param size The number of Byte in data to be sent
     * @return : 0 --> success
     *          -1 --> Error ( connexion to the server were lost)
     */
    int send_data(void *data, int size);
    /**
     * @brief login prompts the user to enter a username and try to register
     * the client to the server.
     * @return : 0 --> on success
     *          -1 --> Error ( connexion to the server were lost)
     */
    int login();
    /**
     * @brief logout Deconnects this client from the server
     */
    void logout();
    /**
     * @brief decode on any received data this method decode the type of
     * request that it is and call the corresponding handler to precess
     * the request.
     * @param data The data received from the server.
     * @param size how byte are in this data
     * @return : -1 --> Error ( connexion to the server were lost)
     *
     */
    int decode(void *data, int size);
    /**
     * @brief shell An always running loop which
     * queries the user request through the standard Input.
     */
    void shell();
    /**
     * @brief read_task This task reads every data received from the server
     *
     */
    void read_task();
    /**
     * @brief write_task This task sends every users request to the server
     */
    void write_task();
private:
    /**
     * @brief print_raw_data print the raw data received from the server
     * @param data the received data
     * @param size How many byte we received
     */
    void print_raw_data(char *data, int size) const;
    /**
     * @brief process_loginout handler to the login request
     * @param log
     * @return
     */
    int process_loginout(const LogInOut &log);
    /**
     * @brief process_message handler for any received message
     * @param msg the received message
     * @return 0 on success
     */
    int process_message(const Message &msg);
    /**
     * @brief process_get_request handler for the GET request
     * @return 0 on success
     */
    int process_get_request(void *data);
    void send_test();
    void show_userlist()const;
private:

    int socket_fd;
    bool loggedIn;
    std::string username;
    bool quit;
    sockaddr *addr;
    socklen_t addrlen;
    addrinfo hints;
    addrinfo *result;
    int flags;
    std::string port;
    std::string usage;
    std::string head;
    Utils::Queue<std::pair<void*, int>> txd_data;
    std::vector<std::string> userlist;


};

#endif // CLIENT_H
