#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>

#define SIZE 256
#define MAXLINE 4096
char sendline[MAXLINE], recvline[MAXLINE];

void str_echo(FILE *fp, int sock);

int main( int argc, char *argv[]){
	int sock;
	char com[SIZE];
	struct sockaddr_in adr;
	struct hostent *hp,*gethostbyname();

	if( argc != 3 ){
		fprintf( stderr, " uso: %s <host> <port>\n", argv[0]);
		exit(1);
	}

	if ((sock = socket ( PF_INET, SOCK_STREAM,0))==-1){
		perror ("imposible creacion del socket");
		exit(2);
	}

	if ((hp=gethostbyname(argv[1]))==NULL){
		perror ("Error: Nombre de la maquina desconocido");
		exit(3);
	}

	adr.sin_family = PF_INET;
	adr.sin_addr.s_addr = htonl( INADDR_ANY );
	adr.sin_port = htons ( atoi ( argv[2]) );
	bcopy( hp->h_addr,&adr.sin_addr, hp->h_length);

	if ( connect( sock,(struct sockaddr*)&adr, sizeof(adr)) ==-1){
		perror( "connect" );
		exit(4);
	}
	str_echo( stdin, sock);
}

void *hilo_escucha(void *arg){
	int n;
	int sock = *((int*)arg);
	while(true){
		n = read(sock, recvline, MAXLINE);
		
		if(n  == 0) {
			fprintf ( stderr, "Servidor terminado\n" );
			exit(5);
		}
		
		fputs(recvline, stdout);
		memset(recvline, 0, MAXLINE);			
	}
}

void str_echo(FILE*fp, int sock ){
	
	pthread_t tr1;
	
	pthread_create(&tr1, NULL, hilo_escucha, (void *)&sock);
	while(fgets(sendline, MAXLINE, fp) != NULL){
		write(sock, sendline, strlen(sendline));
	}
	
}

