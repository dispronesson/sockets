#include "server.h"

int main(int argc, char *argv[]){
    int sfd = start_server(argc, argv);
    main_worker(sfd);
    return 0;
}