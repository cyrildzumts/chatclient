#include <iostream>
#include "client.h"

using namespace std;

int main(int argc, char *argv[])
{
    Client client;
    client.init();
    client.start();
    client.logout();
    return 0;
}
