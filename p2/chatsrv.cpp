/* Используемая архитектура: Linux */

#include <iostream>
#include <set>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>

#define MAX_EVENTS 100
#define PORT 3100

using std::cout;

int set_nonblock(int fd)
{
	int flags;
	
	#if defined(O_NONBLOCK)
		if ( (flags = fcntl(fd, F_GETFL, 0)) == -1 )
			flags = 0;
		return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	#else
		flags = 1;
		return ioctl(fd, FIOBIO, &flags);
	#endif
} 


int main(int argc, char **argv)
{
	char Buffer[1024];
	std::set<int> fileDescriptors;

	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( MasterSocket == -1 ) {
		cout << strerror(errno) << "\n";
		return 1;
	}
	
	int optval = 1;
	if ( setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1 ) {
        	cout << strerror(errno) << "\n";
        	return 1;
    	}
	
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(PORT);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(SockAddr.sin_zero), '\0', 8);
	
	if( bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr)) == -1 ) {
		cout << strerror(errno) << "\n";
		return 1;
	}
	
	set_nonblock(MasterSocket);
	
	if( listen(MasterSocket, SOMAXCONN) == -1 ) {
		cout << strerror(errno) << "\n";
		return 1;
	}
	
	struct epoll_event Event;
	Event.data.fd = MasterSocket;
	Event.events = EPOLLIN | EPOLLET;
	
	struct epoll_event *Events = (struct epoll_event *) calloc(MAX_EVENTS, sizeof(struct epoll_event));
	
	int EPoll = epoll_create1(0);
	epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);

	while(true) {
		int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1); 
		
		for(int i = 0; i < N; ++i) {
			if(Events[i].data.fd == MasterSocket) {
				int SlaveSocket = accept(MasterSocket, nullptr, nullptr);
				if( SlaveSocket == -1 ) {
					cout << strerror(errno) << "\n";
				}
				else {
					cout << "accepted connection\n";	

					send(SlaveSocket, "Welcome!\n", 10, MSG_NOSIGNAL);	
					
					set_nonblock(SlaveSocket);
					
					Event.data.fd = SlaveSocket;
					Event.events = EPOLLIN | EPOLLET;
					
					epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);

					fileDescriptors.insert(SlaveSocket);
				}
			}
			else {
				if( (Events[i].events & EPOLLERR)||(Events[i].events & EPOLLHUP) ) {
					cout << "connection terminated\n";
				
					shutdown(Events[i].data.fd, SHUT_RDWR);
					close(Events[i].data.fd);

					fileDescriptors.erase(Events[i].data.fd);
				}
				else {
					int RecvSize;
					while( (RecvSize = recv(Events[i].data.fd, Buffer, sizeof(Buffer), MSG_NOSIGNAL)) > 0 )
						for(int val : fileDescriptors)
							send(val, Buffer, RecvSize, MSG_NOSIGNAL);
				}
			}
		}
	}

	return 0;
}
