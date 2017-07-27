#ifndef __HOST_H
#define __HOST_H

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define CHUNK_LEN 1024

// defined in main.c
unsigned char check_sum(char *chunk, size_t len);

char **file;
int file_len;

int send_file(int port);
int load_file(char *path);

#endif
