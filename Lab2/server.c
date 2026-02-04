#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "common.h"

char **theArray;                    // array of strings
pthread_rwlock_t *array_locks;      // one RW lock per array index
int ARRAY_SIZE;




void *HandleClient(void *arg) {
    int client_fd = (int)(long)arg;
    char buf[COM_BUFF_SIZE];
    char reply[COM_BUFF_SIZE];
    ClientRequest rq;

    if (read(client_fd, buf, COM_BUFF_SIZE) <= 0) {
        close(client_fd);
        return NULL;
    }

    ParseMsg(buf, &rq);

    double start, end;

    if (rq.is_read == 1) {
        // read
        pthread_rwlock_rdlock(&array_locks[rq.pos]);
        getContent(reply, rq.pos, theArray);
        pthread_rwlock_unlock(&array_locks[rq.pos]);

    } else {
        // write
        pthread_rwlock_wrlock(&array_locks[rq.pos]);
        setContent(rq.msg, rq.pos, theArray);
        getContent(reply, rq.pos, theArray);
        pthread_rwlock_unlock(&array_locks[rq.pos]);
    }

    /* Send response */
    write(client_fd, reply, COM_BUFF_SIZE);
    close(client_fd);

    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr,
            "usage: %s <array size> <server ip> <server port>\n", argv[0]);
        exit(1);
    }

    ARRAY_SIZE = atoi(argv[1]);
    char *server_ip = argv[2];
    int server_port = atoi(argv[3]);


    theArray = malloc(ARRAY_SIZE * sizeof(char *));
    array_locks = malloc(ARRAY_SIZE * sizeof(pthread_rwlock_t));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        theArray[i] = malloc(COM_BUFF_SIZE);
        snprintf(theArray[i], COM_BUFF_SIZE,
                 "String %d: the initial value", i);
        pthread_rwlock_init(&array_locks[i], NULL);
    }



    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = server_port;
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(server_fd, 2000);


    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        pthread_t tid;
        pthread_create(&tid, NULL, HandleClient, (void *)(long)client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        pthread_rwlock_destroy(&array_locks[i]);
        free(theArray[i]);
    }
    
    free(theArray);
    free(array_locks);

    return 0;
}