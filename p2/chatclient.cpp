/* Linux */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 3100

int main(int argc, char *argv[])
{
    	size_t numbytes;  
    	char buf[5120];

	fd_set master;
    	fd_set read_fds;

	FD_ZERO(&master);
    	FD_ZERO(&read_fds);

	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    	if (sockfd == -1) {
		printf("%s\n", strerror(errno));
	        return 1;
	}

	struct sockaddr_in their_addr;
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(PORT);
	their_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	memset(&(their_addr.sin_zero), '\0', 8);
	
	if ( connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1 ) {
		printf("%s\n", strerror(errno));
	        return 1;
	}

	FD_SET(sockfd, &master);
	FD_SET(STDIN_FILENO, &master);
	
	int fdmax = sockfd;

	while(true) {
		read_fds = master;
        	if ( select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1 ) {
            		printf("%s\n", strerror(errno));
        		return 1;
        	}
		for(int i = 0; i <= fdmax; ++i)
            		if (FD_ISSET(i, &read_fds)) {
				if (i == sockfd) {
					if ( (numbytes = recv(i, buf, sizeof(buf), MSG_NOSIGNAL)) > 0 ) {
						buf[numbytes] = '\0';
						printf("%s\n", buf);
					}	
				}
				else {
					numbytes = 0;
					while( (buf[numbytes] = getchar()) != '\n' )
						++numbytes;
					send(sockfd, buf, numbytes, MSG_NOSIGNAL);
				}	
			}
	}

	return 0;
}
