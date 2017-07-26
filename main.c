#include <stdio.h>
#include <string.h>

#include "recv.h"
#include "host.h"

unsigned char check_sum(char *chunk, size_t len)
{
	unsigned char chk = 0;
	while(len-- != 0) {
		chk -= *chunk++;
	}
	return chk;
}

// get a file from server
int recv_file_mode(int argc, char *argv[]) {
	int port = atoi(argv[3]);
	get_file((const char *) argv[2], port, argv[4]);
	return 0;
}

// host file
int host_file_mode(int argc, char *argv[]) {
	puts("Loading file in mem...");	
	load_file(argv[2]);	
	int port = 8000;
	if (argv[3] != NULL) 
		port = atoi(argv[3]);
	printf("Starting server...\t->\tON PORT %d\n", port);	
	send_file(port);
	return 0;
}

int main(int argc, char *argv[])
{
	printf("%d\n", argc);
	if (argc < 2) {
		puts("*Error* -> run elfs -help for help");
		return 0;
	}

	if (strcmp(argv[1], "-g") == 0)
		recv_file_mode(argc, argv);
	else if (strcmp(argv[1], "-h") == 0)
		host_file_mode(argc, argv);	
	else if (strcmp(argv[1], "-help") == 0) {
		puts("Options:");
		puts("-g [IPv4 addres] [port] [name for file] downloads file from elft host");
		puts("-h [path to file] [port] host file on specified port (port defaults to 8000)");
	} else {
		puts("Invalid mode.");
		return -1;
	}
	return 0;
}
