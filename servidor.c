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
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define SIZE 256
#define PORT 2222
#define MAXLINE 4096
#define TRUE 1
char line[MAXLINE];
int alt[4], clientes[9], ganador; char numm[4];
bool bandera = false;



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


bool compareTo(char v1, char v2){
	//printf("Comparo : %c y %c \n", v1, v2);
	bool igual = false;
	if (v1 == v2){
		igual = true;
	}
	return igual;
}

bool validar_rpt(char arr[]){
	bool repetido;
	for (int i=0; i<3; i++){
		for (int j=i+1; j<4; j++){
			repetido = compareTo(arr[i], arr[j]);
			if (repetido)
				break;
		}
		if (repetido)
			break;
	}
	return repetido;
}

bool esEntero(char arr[], int t){
	bool eSentero = true;
	int c = 0;
	for (int i=0; i<4; i++){
		char dato = arr[i];
		if (isdigit(dato) != 0){
			c++;
		} 
	}
	
	if (t == 4 && c == 4){
		if(validar_rpt(arr))
			eSentero = false;
	} else {
		eSentero = false;
	} 
	return eSentero;
}

void generar_alt(){
	srand(time(NULL));
	do{
		for (int i=0; i < 4; i++){
			alt[i] = rand () % 10;
			numm[i] = alt[i] + '0';
		}
	}while(validar_rpt(numm));
}

void servicio( int sock){
	int n ;
	for (;;) {
		n = read( sock, line, MAXLINE );
		if(n <= 0){
			return;
		}
		printf("Mensaje : %s", line);
		if (bandera == false){
			if (esEntero(line, n-1)){
				int c = 0;
				for (int i = 0; i<4; i++){
					if (compareTo(numm[i], line[i]))
						c++;
				}
				int c2 = 4-c;
				char value1 = c + '0', value2 = c2 + '0';
				if (c == 4){
					bandera = true;
					char congr[] = "FELICITACIONES\n";
					int tr = sizeof(congr);
					write(sock, congr, tr);
					exit(0);
				} else {
					char cadena[] = {value1,' ','F','I','J','A','S','\n',value2,' ','P','I','C','A','S','\n','\0'};
					int m = sizeof(cadena);
					write(sock, cadena, m);
				}
			} else {
				char cadena[] = "ERROR, Digite un numero de 4 digitos\n";
				int m = sizeof(cadena);
				write(sock, cadena, m);
			}
		} else {
			char congr[] = "Lo siento, numero ya encontrado\n";
			int ct = sizeof(congr);
			write(sock, congr, ct);
			if (fork() == 0){
				exit(-1);
			}
		}
		printf("---------------------------------------------------------\n");
		memset(line, 0, MAXLINE); 
	}
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

	port = atoi(argv[1]);

	if(( sock_escucha = crearsocket( &port, SOCK_STREAM ))== -1){
		fprintf( stderr, "Error: No se pudo crear/conectar socket\n");
		exit(2);
	}

	signal( SIGCHLD, sigchld);
	listen( sock_escucha, 1024);
	
	fprintf( stdout, "Inicio servidor en el puerto %d\n", port);
	generar_alt();
	printf("NUMERO ALEATORIO GENERADO :  %i%i%i%i \n----------------------------\n", alt[0], alt[1], alt[2], alt[3]);
	
	
	while(TRUE){

		lgadr = sizeof(adr);
		sock_servicio = accept(sock_escucha,(struct sockaddr*)&adr,&lgadr);
		
		printf("Servicio aceptado \n");
		
		if(fork() == 0){
			close(sock_escucha);
			servicio( sock_servicio );
			exit(0);
		}
		close(sock_servicio);
	}
}



