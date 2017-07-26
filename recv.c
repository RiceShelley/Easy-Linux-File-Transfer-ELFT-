#include "recv.h"

int write_chunk(int fd, char *chunk) 
{
	char *d_start = (char *) (memmem((const void *) chunk, (size_t) SECT_SIZE, (const void *) &"<SOT>", (size_t) 5) + 5);
	char *d_end = (char *) memmem((const void *) chunk, (size_t) SECT_SIZE, (const void *) &"<EOT>", (size_t) 5);
	int d_len = 0;
	while (true) {
		if (&d_start[d_len] == &d_end[0])
			break;
		d_len++;
	}
	write(fd, d_start, d_len);		
	return 0;
}

int get_file(const char *ip, int port, char *name) 
{
	printf("port %d\n", port);

	if (name == NULL)
		name = "unnamed";	
	
	struct sockaddr_in s_addr;
	int fd, slen = sizeof(s_addr);

	char buff[SECT_SIZE];
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Unable to create socket\n");
	}
	
	memset((char *) &s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);

	struct hostent *hPtr;
	hPtr = gethostbyname(ip);

	printf("addr %s\n", hPtr->h_addr);

	if (inet_aton(ip, &s_addr.sin_addr) == 0) {
		printf("inet_aton() failed\n");
	}

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		puts("Error setting timeout");
	}

	// request info about file
	char *message = "FILE_INFO\0";
	if(sendto(fd, message, strlen(message), 0, (struct sockaddr *) &s_addr, slen) == -1) {
		printf("failed to send msg");
		return -1;
	}

	memset(buff, '\0', SECT_SIZE);	
	if(recvfrom(fd, buff, SECT_SIZE, 0, (struct sockaddr *) &s_addr, &slen) == -1) {
		printf("failed to recv data");		
		return -1;
	}

	char *info[2];
	// get amt of sections
	info[0] = strtok(buff, " ");
	info[1] = strtok(NULL, " \n");
	int sect = atoi(info[1]);
	printf("sect %d\n", sect);

	puts("Downloading from server...");

	char l_bar[103];
	l_bar[0] = '[';
	l_bar[101] = ']';
	l_bar[102] = '\0';
	int old_lbar = 0;

	int out_f = open(name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	for (int i = 0; i < sect + 1; i++) {

		// request a chunk
		sprintf(buff, "GET %d\n", i);
		if (sendto(fd, buff, strlen(buff), 0, (struct sockaddr *) &s_addr, slen) == -1) {
			printf("failed to request chunk\n");
			i--;
			continue;
		}

		// recv chunk	
		if (recvfrom(fd, buff, SECT_SIZE, 0, (struct sockaddr *) &s_addr, &slen) == -1) {
			printf("failed to recv chunk\n");
			i--;
			continue;
		}	

		// make sure chunk isnt corrupt
		char *d_start = (char *) memmem((const void *) buff, (size_t) SECT_SIZE, (const void *) &"<SOT>", (size_t) 5);
		char *d_end = (char *) memmem((const void *) buff, (size_t) SECT_SIZE, (const void *) &"<EOT>", (size_t) 5);	
		if (d_start == NULL || d_end == NULL) {
			for (int i = 0; i < 100; i++) {
				l_bar[i + 1] = '#';
			}
			printf("\r%s Done.\n", l_bar);
			if (strncmp(buff, "END", 3) == 0) {
				puts("found end");
				break;
			}
			i--;
			continue;
		}

		if (check_sum(buff, SECT_SIZE) != 0) {
			printf("FAILED CHECK SUM TEST!!! ON -> %d\n", i);
			i--;
			continue;
		}
		
		// print loading bar
		int lb_l = (int) (((double) i / (double) sect) * 100.0);
		if (lb_l != old_lbar) {
			for (int i = 0; i < 100; i++) {
				if (i < lb_l + 1) {
					l_bar[i + 1] = '#';
				} else {
					l_bar[i + 1] = ' ';
				}
			}
			printf("\r%s %d%%", l_bar, lb_l);
			fflush(stdout);
			old_lbar = lb_l;
		}
		
		// write chunk to file
		write_chunk(out_f, buff);
	}

	close(out_f);
	strcpy(buff, "CLOSE");
	if (sendto(fd, buff, strlen(buff), 0, (struct sockaddr *) &s_addr, slen) == -1) {
		printf("failed to close chunk\n");
		return -1;
	}
	close(fd);
	return 0;	
}
