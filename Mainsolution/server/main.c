#include <sys/types.h>
#include <winsock2.h>
#include <stdio.h>
#include <errno.h>
#pragma comment(lib, "Ws2_32.lib")

int main( void )
{
	struct sockaddr_in local;
	int s;
	int s1;
	int rc;
	char buf[ 1 ];
	char str[10];
	int err;
	WSADATA wsa_data;
	err = WSAStartup (MAKEWORD(2,2), &wsa_data);

	local.sin_family = AF_INET;
	local.sin_port = htons( 5555 );
	local.sin_addr.s_addr = htonl( INADDR_ANY );
	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( s < 0 ){
		perror( "socket call failed" );
		exit( 1 );
	}

	rc = bind( s, ( struct sockaddr * )&local, sizeof( local ) );
	if ( rc < 0 ){
		perror( "bind call failure" );
		exit( 1 );
	}
	
	while(1){
		rc = listen( s, 5 );
		if ( rc ){
			perror( "listen call failed" );
			exit( 1 );
		}

		s1 = accept( s, NULL, NULL );
		if ( s1 < 0 ){
			perror( "accept call failed" );
			exit( 1 );
		}
		else{
			rc = send( s1, "OK!", 3, 0 );
			printf("Connect!\n");
		}
		while(1){

			rc = recv( s1, buf, 1, 0 );
			if ( rc <= 0 ){
				perror( "recv call failed" );
				break;
			}

			printf( "%c", buf[ 0 ] );
		
			strcpy(str, "hello from server");
			rc = send(s1, str, strlen(str), 0);
			if (rc <= 0){
				printf("error send");
				exit(3);
			}

		}
	}
	exit( 0 );
}
