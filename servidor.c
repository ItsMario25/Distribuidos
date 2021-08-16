#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
//#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#define SIZE 256
#define PORT 2222
#define MAXLINE 4096
#define TRUE 1
char line[MAXLINE];

servicio( int sock);

int crearsocket( int *port, int type){
	int sockfd;
	struct sockaddr_in adr;;
	int longitud;

	if(( sockfd = socket(PF_INET, type, 0)) == -1){
		perror("Error: Imposible crear socket");
		exit(2);
	}

	bzero((char*)&adr, sizeof(adr));
	adr.sin_port = htons(*port);
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_family = PF_INET;
	
	if(bind( sockfd,(struct sockaddr*)&adr, sizeof(adr))== -1){
		perror("Error: bind");
		exit(3);
	}

	longitud = sizeof(adr);

	if( getsockname( sockfd,(struct sockaddr*)&adr,&longitud )){
		perror( "Error: Obtencion del nombre del sock");
		exit(4);
	}

	*port = ntohs( adr.sin_port );
	return( sockfd );
}

void sigchld(){
	pid_t pid;
	int stat;

	pid=wait( &stat);
	fprintf( stderr, "proceso hijo: %d terminado\n",pid );
	return;
}

int main( int argc, char*argv[]){

	int sock_escucha, sock_servicio;
	struct sockaddr_in adr;
	int lgadr = sizeof(adr);
	int port = PORT;

	if (argc !=2){
		fprintf( stderr, "uso: %s [port]\n", argv[0]);
		exit(1);
	}

	port = atoi( argv[1]);

	if(( sock_escucha = crearsocket( &port, SOCK_STREAM ))== -1){
		fprintf( stderr, "Error: No se pudo crear/conectar socket\n");
		exit(2);
	}

	signal( SIGCHLD, sigchld);
	listen( sock_escucha, 1024);
	fprintf( stdout, "Inicio servidor en el puerto %d\n", port);

	while(TRUE){

		lgadr = sizeof(adr);
		sock_servicio = accept(sock_escucha,(struct sockaddr*)&adr,&lgadr);
		fprintf( stdout, "Servicio aceptado\n");

		if(fork() == 0){
			close(sock_escucha);
			servicio( sock_servicio );
			exit(0);
		}
		close(sock_servicio);
	}
}


bool esEntero(char *arr, int t){
	bool eSentero = true;
	int c = 0;
	char tempo[4];
	for (int i=0; i<4; i++){
		if (isdigit(arr[i]) != 0){
			c++;
			tempo[i] = arr[i];
		} 
	}
	if (t == 4 && c == 4){
		for (int i=0; i<3; i++){
			for (int j=i+1; j<4; j++){
				printf("Comparo : %c y %c \n", tempo[i], tempo[j]);
				char* tem1 = &tempo[i];
				char* tem2 = &tempo[j];
				if (strcmp (tem1, tem2) == 0){
					eSentero = false;
					printf("ERROR. Numero con valores repetidos, digite otro \n");
				}
			}	
		}
	} else {
		printf("ERROR. Digite un numero de 4 digitos\n");
		eSentero = false;
	} 
	return eSentero;
}


void servicio( int sock){
	int n ;
	
	int alt[4];
	for (int i=0; i < 4; i++){
		alt[i] = rand () % 11;
	}
	int num = (alt[0]*1000) + (alt[1]*100) + (alt[2]*10) + alt[3];
	
	printf("NUMERO ALEATORIO GENERADO :  %d \n----------------------------\n", num);
	
	for (;;) {
		n = read( sock, line, MAXLINE );
		if(n <= 0){
			return;
		}
		printf("Mensaje : %s", line);
		bool v = esEntero(line, n-1);
		if (v){
			char cadena[] = "Es un numero de 4 digitos";
			int m = sizeof(cadena);
			write(sock, cadena, m);
		} else {
			char cadena[] = "ERROR,Digite un numero de 4 digitos";
			int m = sizeof(cadena);
			write(sock, cadena, m);
		}
		printf("---------------------------------------------------------\n");
		//write(sock, line, n);
		memset(line, 0, MAXLINE); 
	}
}

