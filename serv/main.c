#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <fcntl.h>

#define BUFFLEN 512
#define PORT 8000
#define IMG_PATH "t.txt"

char **file = NULL;
int file_len = 0;

int send_file()
{
	struct sockaddr_in s_addr, o_addr;
	int fd, i, slen = sizeof(o_addr), recv_len;
	char buff[BUFFLEN];
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error");
	}
	
	memset((char *) &s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(fd, (struct sockaddr*)&s_addr, sizeof(s_addr)) == -1) {
		printf("failed to bind\n");
	}

	while (true) {
		printf("Waiting for data...\n");
		memset(buff, 0, BUFFLEN);
		if ((recv_len = recvfrom(fd, buff, BUFFLEN, 0, (struct sockaddr *) &o_addr, &slen)) == -1) {
			printf("failed to recv\n");
		}
		printf("Received packet from %s:%d\n", inet_ntoa(o_addr.sin_addr), ntohs(o_addr.sin_port));
		flush(fd);
		printf("buff: %s\n", buff);
		usleep(100000);
		if (strncmp(buff, "FILE_INFO", 9) == 0) {
			memset(buff, 0, BUFFLEN);
			sprintf(buff, "SECTIONS: %d\n", file_len);
			sendto(fd, buff, strlen(buff), 0, (struct sockaddr*) &s_addr, slen);
		}
		printf("msg: %s\n", buff);
		usleep(10000);

		if (sendto(fd, buff, recv_len, 0, (struct sockaddr*) &s_addr, slen) == -1) {
			printf("failed to send packet\n");
		}
		memset(buff, 0, sizeof(buff));
	}
	close(fd);
	return 0;
}

int load_file()
{
	int fd = open(IMG_PATH, O_RDONLY);
	int pos = 0;
	while (true) {
		// allacate buff on heap
		char *buff = malloc(512 * sizeof(char));
		memset(buff, 0, 512);
		// create section header
		sprintf(buff, "%s:%d\n\n", "Section", pos);
		// read file into temp_buff
		char temp_buff[512 - 15];
		memset(temp_buff, 0, (512 - 15));
		int bytes_read = read(fd, temp_buff, (512 - 15));
		// append data to header
		strcat(buff, temp_buff);
		// store in file buffer
		file[pos] = buff;
		pos++;
		if (bytes_read < 1) {
			char *end = malloc(10 * sizeof(char));
			strcpy(end, "END");
			file[pos] = end;
			break;
		}
	}
	
	printf("file len %d\n", pos);
	file_len = pos;	
	printf("%s\n", file[pos]);
	close(fd);
}

int main()
{
	file = (char **) malloc(1024 * sizeof(char **));
	printf("starting serv\n");
	load_file();
	send_file();
	return 0;
}
