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
#include <pthread.h>


#define SIZE 256
#define PORT 2222
#define MAXLINE 4096
#define TRUE 1
#define cantidad 3

char line[MAXLINE], numm[4];
int alt[4], ganador, clientes[cantidad], conectados = 0; 
bool bandera = true;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


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


bool compareTo(char v1, char v2){
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



int disponible(int dato){
	int ds;
	for(ds = 0; ds<cantidad; ds++){
		if (clientes[ds] == 0 || clientes[ds] == dato)
			break;
	}
	return ds;
}

void *servicio(void *arg){
	int sock = *((int*)arg);
	int n ;
	int num = disponible(sock)+1;
	char cadena[MAXLINE] ;
	snprintf (cadena, MAXLINE, "BIENVENIDO AL JUEGO DE PICAS, ERES EL JUGADOR %i \n", num);
	int tam = sizeof(cadena);
	write(sock, cadena, tam);
	for (;;) {
		pthread_mutex_lock(&lock);
		
		int jugador = disponible(sock);
		n = read( sock, line, MAXLINE );
		if (n <= 0){
			clientes[jugador] = 0;
			conectados--;
			if (bandera)
				printf("Jugador %i desconectado prematuramente \n", jugador+1);
			if (conectados == 0 && bandera == false){
				printf("-------------JUEGO TERMINADO-----------\n");
				exit(0);
			}
			break;
		}
		
		printf("Mensaje del jugador %i : %s", jugador+1,line);
		if (bandera){
			if (esEntero(line, n-1)){
				int c = 0;
				for (int i = 0; i<4; i++){
					if (compareTo(numm[i], line[i]))
						c++;
				}
				int c2 = 4-c;
				if (c == 4){
					bandera = false;
					clientes[jugador] = 0;
					ganador = jugador+1;
					printf("NUMERO ENCONTRADO POR EL JUGADOR %i \n", ganador);
					char cadena[] =  "FELICITACIONES ERES EL GANADOR\n";
					int tam = sizeof(cadena);
					write(sock, cadena, tam);
					printf("Jugador %i terminado \n", jugador+1);
					close(sock);
				} else {
					char cadena[MAXLINE];
					snprintf (cadena, MAXLINE, "-- %i FIJAS --\n-- %i PICAS --\n", c, c2);
					int m = sizeof(cadena);
					write(sock, cadena, m);
				}
			} else {
				char cadena[] = "ERROR, Digite un numero de 4 digitos\n"; 
				int tam = sizeof(cadena);
				write(sock, cadena, tam);
			}
		} else {
			char cadena[MAXLINE] ;
			snprintf (cadena, MAXLINE, "UPS, NUMERO HALLADO POR EL JUGADOR %i \n", ganador);
			int tam = sizeof(cadena);
			write(sock, cadena, tam);
			printf("Jugador %i terminado \n", jugador+1);
			close(sock);
			
		}
		printf("---------------------------------------\n");
		memset(line, 0, MAXLINE); 
		pthread_mutex_unlock(&lock);
	}
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

int main( int argc, char*argv[]){

	pthread_t thr[cantidad];
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

	listen(sock_escucha, 1024);
	
	fprintf( stdout, "Inicio servidor en el puerto %d\n", port);
	generar_alt();
	printf("NUMERO ALEATORIO GENERADO :  %i%i%i%i \n---------------------------------------\n", 
	alt[0], alt[1], alt[2], alt[3]);
	
	while(TRUE){
		lgadr = sizeof(adr);
		sock_servicio = accept(sock_escucha,(struct sockaddr*)&adr,&lgadr);
		if (conectados < cantidad && bandera == true ){
			int conectese = disponible(0);
			clientes[conectese] = sock_servicio;
			conectados++;
			printf("Servicio aceptado \n");
			printf("Jugador %i conectado \n", conectese+1);
			pthread_mutex_init(&lock, NULL);
			pthread_create(&thr[conectese], NULL, servicio, &sock_servicio);
		} else {
			char cadena[] = "Lo siento, servidor lleno o numero ya encontrado\n";
			int tam = sizeof(cadena);
			write(sock_servicio, cadena, tam);
			close(sock_servicio);
		} 
	}
}



