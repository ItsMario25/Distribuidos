#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


bool compareTo(char v1, char v2){
	printf("Comparo : %c y %c \n", v1, v2);
	bool igual = true;
	if (strcmp(&v1, &v2) != 0 ){
		igual = false;
		printf("ERROR. Numero con valores repetidos, digite otro \n");
	}
	return igual;
}


main() { 
      char a = '3'; 
      char b = '3'; 
      if (!compareTo(a, b)) { 
      		printf("They are similar.\n"); 
      } 
} 
