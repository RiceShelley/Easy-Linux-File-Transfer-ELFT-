#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>

#define BUFFLEN 512
#define PORT 8000
#define SERV_ADDR "127.0.0.1"

int main()
{
	struct sockaddr_in s_addr;
	int fd, i, slen = sizeof(s_addr);
	char buff[BUFFLEN];
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Unable to create socket\n");
	}
	
	memset((char *) &s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	
	if (inet_aton(SERV_ADDR, &s_addr.sin_addr) == 0) {
		printf("inet_aton() failed\n");
	}
	
	while(true) {
		/*char *message = "welp welp welp\0";
		if(sendto(fd, message, strlen(message), 0, (struct sockaddr *) &s_addr, slen) == -1) {
			printf("failed to send msg");
		}*/

		memset(buff, '\0', BUFFLEN);	
		if(recvfrom(fd, buff, BUFFLEN, 0, (struct sockaddr *) &s_addr, &slen) == -1) {
			printf("failed to recv data");
		}
		
		puts(buff);
	}
}
