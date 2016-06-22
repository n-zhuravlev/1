/* Linux */
/* Формат конфигурационного файла: 3000,127.0.0.1:3001,127.0.0.1:3002,127.0.0.0:3003,... */

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define MAX_SIZE_CONFIG_DATA 1024*1024


void read_config_data(const char *filename, int *port, char *config_data)
{
	FILE *f = fopen(filename, "r");
	
  	int i = 0;
  	while( ( config_data[i] = fgetc(f) ) != ',' ) {
  		++i;
  	}
  	config_data[i] = '\0';
  	*port = atoi( config_data );  			
  			
  	config_data[0] = ',';
  	i = 1;
  	while( ( config_data[i] = fgetc(f) ) != EOF ) {
  		++i;
  	}
  	config_data[i] = '\0';
  		
  	fclose(f);
}


void read_cb( struct bufferevent *buf_ev, void *arg )
{
  	struct evbuffer *input = bufferevent_get_input( buf_ev );

  	if(arg != NULL) {
  		struct evbuffer *output = bufferevent_get_output( (struct bufferevent *)arg );
  		evbuffer_add_buffer( output, input );
  	}	
}


void event_cb( struct bufferevent *buf_ev, short events, void *arg )
{
  	if( events & BEV_EVENT_ERROR ) {
    		fprintf(stderr, "Ошибка объекта bufferevent.\n" );
    		bufferevent_free( buf_ev );
    	}
  	if( events & BEV_EVENT_EOF ) {
  		read_cb( buf_ev, arg );
    		bufferevent_free( buf_ev );
    	}
}


void accept_connection_cb( struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int sock_len, void *arg )
{
	char *config_data = (char *)arg;
	
	int srv_number = 0;
	int i = 0;
	for( ; config_data[i] != '\0'; ++i) {
		if(config_data[i] == ',') {
			++srv_number;
		}
	}
	int n = rand() % srv_number;
	
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    	if (sockfd == -1) {
		fprintf(stderr, "Ошибка при создании сокета.\n");
	}
	
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
  		
	for(i = 0; config_data[i] != '\0'; ++i) {
		if(config_data[i] == ',') {
			if(n == 0) {
				int j = i + 1;
				for( ; config_data[j] != '\0' && config_data[j] != ','; ++j) {
					if(config_data[j] == ':') {
						config_data[j] = '\0';
						SockAddr.sin_addr.s_addr = inet_addr( config_data + i + 1 );
						config_data[j] = ':';
						i = j + 1;
					}
				}
				int t = 0;
				if(config_data[j] == ',') {
					t = 1;
				}			
				config_data[j] = '\0';
				SockAddr.sin_port = htons( atoi(config_data + i) );		
				if(t == 1) {
					config_data[j] = ',';
				}
				
				break;
			}
			else {
				--n;
			}
		}
	}
	
	memset(&(SockAddr.sin_zero), '\0', 8);
			
	if ( connect(sockfd, (struct sockaddr *)&SockAddr, sizeof(struct sockaddr)) == -1 ) {
		fprintf(stderr, "Не получилось подсоединиться к серверу.\n");
	}		
			
	struct event_base *base = evconnlistener_get_base( listener );
  
  	struct bufferevent *buf_ev_client = bufferevent_socket_new( base, fd, BEV_OPT_CLOSE_ON_FREE ),
  				*buf_ev_server = bufferevent_socket_new( base, sockfd, BEV_OPT_CLOSE_ON_FREE );

  	bufferevent_setcb( buf_ev_client, read_cb, NULL, event_cb, buf_ev_server );
  	bufferevent_setcb( buf_ev_server, read_cb, NULL, event_cb, buf_ev_client );
  
  	bufferevent_enable( buf_ev_client, (EV_READ | EV_WRITE) );		
	bufferevent_enable( buf_ev_server, (EV_READ | EV_WRITE) );
}


void accept_error_cb( struct evconnlistener *listener, void *arg )
{
  	struct event_base *base = evconnlistener_get_base( listener );
  	int error = EVUTIL_SOCKET_ERROR();
  
  	fprintf( stderr, "Ошибка %d (%s) в объекте evconnlistener. Завершение работы.\n", error, evutil_socket_error_to_string( error ) );
  
  	event_base_loopexit( base, NULL );
}


int main( int argc, char **argv )
{
  	int port;
  	char *config_data = (char *)malloc(MAX_SIZE_CONFIG_DATA * sizeof(char));
  	
  	if(argc == 2) {
  		read_config_data(argv[1], &port, config_data);
  	}
  	else {
  		fprintf(stderr, "Не найден конфигурационный файл.\n");	
  		return 1;
  	}
	
	struct event_base *base = event_base_new();	
  	if( !base ) {
    		fprintf( stderr, "Ошибка при создании объекта event_base.\n" );
    		return 1;
	}
	
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(SockAddr.sin_zero), '\0', 8);
	
	struct evconnlistener *listener = evconnlistener_new_bind( base, accept_connection_cb, config_data, (LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE), -1, (struct sockaddr *)&SockAddr, sizeof(SockAddr) );
  	if( !listener ) {
    		fprintf(stderr, "Ошибка при создании объекта evconnlistener.\n" );
    		return 1;
  	}  
  	evconnlistener_set_error_cb( listener, accept_error_cb );

  	event_base_dispatch( base );
  	
  	free(config_data);
  
	return 0;
}
