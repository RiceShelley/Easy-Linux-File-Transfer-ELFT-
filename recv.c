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

int resolve_ip(char *hostname, char *ip) 
{
	struct hostent *he;
	struct in_addr *addr;

	if ((he = gethostbyname(hostname)) == NULL) 
		return 1;

	if ((addr = (struct in_addr *) he->h_addr_list[0]) == NULL)
		return 1;

	strcpy(ip, inet_ntoa(*addr));
	return 0;
}

int get_file(const char *ip, int port, char *name) 
{
	
	if (name == NULL) {
		puts("No file name specified (Defaulting to \"unnamed\")");
		name = "unnamed";	
	}
	
	struct sockaddr_in s_addr;
	int fd, slen = sizeof(s_addr);

	char buff[SECT_SIZE];
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Unable to create socket\n");
	}
	
	memset((char *) &s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);

	char ip_from_hname[100];
	if (resolve_ip((char *) ip, ip_from_hname) != 0)
		printf("failed to resolve hostname\n");

	printf("Connected to\t->\t%s:%d\n", ip_from_hname, port);	

	if (inet_aton(ip_from_hname, &s_addr.sin_addr) == 0)
		printf("inet_aton() failed\n");

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

	puts("Downloading file...");

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

		if (strncmp(buff, "END", 3) == 0)
			break;
		
		if (check_sum(buff, SECT_SIZE) != 0) {
			printf("FAILED CHECK SUM TEST!!! ON -> %d\n", i);
			i--;
			continue;
		}
		
		display_loading_bar((int) (((double) i / (double) sect) * 100.0));
		
		// write chunk to file
		write_chunk(out_f, buff);
	}
	display_loading_bar(-1);
	close(out_f);
	strcpy(buff, "CLOSE");
	if (sendto(fd, buff, strlen(buff), 0, (struct sockaddr *) &s_addr, slen) == -1) {
		printf("failed to close chunk\n");
		return -1;
	}
	close(fd);
	return 0;	
}
