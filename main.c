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

int display_loading_bar(int p)
{
        char l_bar[33];
        memset(l_bar, ' ', 33);
        l_bar[0] = '[';
        l_bar[31] = ']';
        l_bar[32] = '\0';
        static int old_p = 0;
        // print loading bar
        if (p != old_p) {

                // -1 = file done downloading
                if (p == -1) {
                        for (int i = 0; i < 30; i++)
                                l_bar[i + 1] = '#';
                        printf("\r%s DONE.\n", l_bar);
                        return 0;
                }

                for (int i = 0; i < (int) (((double) p / 100.0) * 30.0); i++)
                        l_bar[i + 1] = '#';

                printf("\r%s %d%%", l_bar, p);
                fflush(stdout);
                old_p = p;
        }
        return 0;
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
	if (argc < 2) {
		puts("Invalid use try \"elft --help\"");
		return -1;
	}
	if (strcmp(argv[1], "-g") == 0 && argc == 4)
		recv_file_mode(argc, argv);
	else if (strcmp(argv[1], "-h") == 0 && argc == 3)
		host_file_mode(argc, argv);	
	else if (strcmp(argv[1], "--help") == 0) {
		puts("Options:");
		puts("-g [IPv4 addres / hostname] [port] [name for new file] -> downloads file from elft host");
		puts("-h [path to file] [port] -> host file on specified port (port defaults to 8000)");
	} else {
		puts("Invalid use try \"elft --help\"");
		return -1;
	}
	return 0;
}
