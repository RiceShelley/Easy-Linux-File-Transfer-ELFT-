#ifndef __RECV_H
#define __RECV_H

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>

#define SECT_SIZE 1024

// defined in main.c
unsigned char check_sum(char *chunk, size_t len);

int write_file(const char *name, char **file, int sect);
int get_file(const char *ip, int port, char *name);

#endif
