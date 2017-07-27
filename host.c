#include "host.h"

int send_file(int port)
{
	struct sockaddr_in s_addr, o_addr;
	int fd, slen = sizeof(o_addr), recv_len;
	char chunk[CHUNK_LEN];
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		puts("Error on sock creation.");
		return -1;
	}
	
	memset((char *) &s_addr, 0, sizeof(s_addr));

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(fd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) {
		puts("Failed to bind socket.");
		return -1;
	}

	puts("Waiting for client connection...");
	memset(chunk, '\0', CHUNK_LEN);

	if ((recv_len = recvfrom(fd, chunk, CHUNK_LEN, 0, (struct sockaddr *) &o_addr, &slen)) == -1) {
		puts("Failed to recv packet from client.");
		return -1;
	}

	puts("Client connected.");

	if (strncmp(chunk, "FILE_INFO", 9) == 0) {
		sprintf(chunk, "SECTIONS: %d\n", file_len);
		sendto(fd, chunk, strlen(chunk), 0, (struct sockaddr *) &o_addr, slen);
	} else {
		puts("Error requesting file info.");
		return -1;
	}

	// set 1 sec timeout
	struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                puts("Error setting timeout.");
		return -1;
        }

	puts("Sending file...");

	while (true) {
		memset(chunk, '\0', CHUNK_LEN);

		if ((recv_len = recvfrom(fd, chunk, CHUNK_LEN, 0, (struct sockaddr *) &o_addr, &slen)) == -1)
			continue;

		int sect = -1;

		if (strncmp(chunk, "GET ", 4) == 0) {
			sect = atoi(strtok(&chunk[4], "\n"));
			if (sect < 0 || sect > file_len + 1) {
				continue;
			}
		} else if (strncmp(chunk, "CLOSE", 5) == 0)
			break;
		
		sendto(fd, &(*file[sect]), CHUNK_LEN, 0, (struct sockaddr *) &o_addr, slen);
		display_loading_bar((int) (((double) sect / (double) file_len) * 100.0));
	}
	display_loading_bar(-1);
	close(fd);
	return 0;
}

int load_file(char *path)
{
	if (path == NULL)
		return 1;

	int fd = open(path, O_RDONLY);

	// Figure out how many chunks we need for file to be stored in RAM
	struct stat st;
	stat(path, &st);
	/*
	 * Hey what's this magic number???
	 * Could it be the answer to the universe???
	 * Well no ermmm yes? but no.
	 * just subtracting 42 bytes from CHUNK_LEN so when 
	 * the file size is / against CHUNK_LEN there are a few
	 * extra chunks in memmory so that the program can have a 
	 * "END" chunk with no data and some room to avoid segfaults
	 */
	const int chunks = st.st_size / (CHUNK_LEN - 42);

	file = (char **) malloc(chunks * sizeof(char **));

	int pos = 0;

	while (true) {
		char *chunk = malloc(CHUNK_LEN);
		memset(chunk, '\0', CHUNK_LEN);

		// Chunk format -> "Section:(Sect number)<SOT>(File Data)<EOT>"
		sprintf(chunk, "%s:%d<SOT>", "Section", pos);
		// NOTE: check sum value is going to be stored after <EOT>
		const char *eot = "<EOT> \0";

		const int h_len = strlen(chunk);
		const int e_len = strlen(eot);
		const int d_len = (CHUNK_LEN - (h_len + e_len));
	
		// Read data into chunk without over writing section header
		int bytes_read = read(fd, &chunk[h_len], d_len);

		// append <EOT> to data chunk
		strncpy(&chunk[bytes_read + h_len], eot, e_len);

		if (bytes_read < 1) {
			memset(chunk, '\0', CHUNK_LEN);
			strcpy(chunk, "END\0");
			file[pos] = chunk;
			break;
		}
		// calc check sum and append in last byte of chunk
		chunk[CHUNK_LEN - 1] = (char) check_sum(chunk, CHUNK_LEN - 1);
		// store in chunk "file" or array of pointers to chunks
		file[pos] = chunk;
		pos++;
	}
	file_len = chunks;
	close(fd);
	return 0;
}
