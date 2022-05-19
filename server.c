/*
** A very simple http server.
**
** Just want to learn some socket programming and http protocol.
** use althttpd by D. Richard Hipp as my tutorial.
** https://sqlite.org/althttpd/file?name=althttpd.c&ci=tip
**
** TODO:
** use cmd argument to specify port
** GET request
** transfer other file
** use fork() to handle connections
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <string.h>

#ifndef DEFAULT_PORT
#define DEFAULT_PORT "80"
#endif
#ifndef MAX_CONTENT_LENGTH
#define MAX_CONTENT_LENGTH 20000000
#endif

void sigHandler (int);
int http_server ();

int main ()
{
    int listener;
    int connection;
    int rc;
    int opt = 1;
    struct sockaddr_in listen_addr;
    struct sockaddr_in cli_addr;
    socklen_t len;
    char buffer[BUFSIZ + 1];

    signal(SIGINT, sigHandler);

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    listen_addr.sin_port = htons(DEFAULT_PORT);
    listener = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    /*
    ** TODO: use getaddrinfo to rewrite         */

    if (bind(listener, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("Error while binding: ");
        exit(3);
    }

    if (listen(listener, 20) < 0) {
        perror("Error while listening: ");
        exit(3);
    }

    len = sizeof(cli_addr);

    while (1) {
        int i;
        int file_fd;
        connection = accept(listener, (struct sockaddr *)&cli_addr, &len);

        rc = read(connection, buffer, BUFSIZ);
        if (rc < 0) {
            perror("Error while reading:");
            exit(3);
        }
        buffer[rc] = '\0';

        if (strncmp(buffer, "GET ", 4)) {
            close(connection);
            continue;
        }

        for (i = 4; i < rc; i++) {
            if (buffer[i] == ' ') {
                buffer[i] = '\0';
                break;
            }
        }

        printf("%s\n", buffer);

        if (!strncmp(&buffer[4], "/\0", 2)) {
            sprintf(&buffer[4], "/index.html");
        }

        if ((file_fd = open(&buffer[5], O_RDONLY)) == -1) {
            sprintf(buffer, "HTTP/1.1 400 Not found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Not found</h1></body></html>\r\n");
            write(connection, buffer, strlen(buffer));
            close(connection);
            continue;
        }
        else {
            sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
            write(connection, buffer, strlen(buffer));
        }
        while ((rc = read(file_fd, buffer, BUFSIZ)) > 0) {
            write(connection, buffer, rc);
        }
        close(file_fd);
        close(connection);
    }
    close(listener);

    return 0;
}

void sigHandler (int sig)
{
    printf("\rExiting...\n");
    exit(0);
}
