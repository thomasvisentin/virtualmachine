#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
	FILE *stream;
	int registri[32] = {0};                   /*inizializzo tutti i registri a 0 */
    int stack[16184] = {0};	                  /*inizializzo lo stack di 64 KB (di interi) a 0*/
	int* memoria;
	ssize_t nread;                            /*usato per raccogliere la lunghezza della linea letta da getline() */
	int primo_ciclo_fetch = 0;
	int numero_istruzioni_programma;
	int numero_istruzione = 0;
	int parametro1 = 0;
	int parametro2 = 0;
	ssize_t len;
	char* buffer;
	char* parametro_1 = ("%s",argv[1]);       /* salvo il primo parametro */
	char* parametro_2 = ("%s",argv[2]);       /* salvo il secondo parametro*/
	if(argc > 3)                              /* controllo che il programma non venga invocato con più di 2 parametri in caso contrario termino il programma*/                              
	{
		printf("Too many parameters\n");
		return 1;
	}
	if(argc < 3)                              /* controllo che il programma non venga invocato con meno di 2 parametri in caso contrario termino il programma*/ 
	{
		printf("Too few parameters\n");
		return 1;
	}
	if((strcmp(parametro_1,"esegui") != 0) && (strcmp(parametro_1,"stampa") != 0))
	{
		printf("First parameter is invalid\n");
		return 1;
	}
	
	stream = fopen(parametro_2, "r");        
	if(stream == NULL)                       /* se il file non esiste termino il programma */
	{
		printf("File \"%s\" doesn't exist\n",parametro_2);
		return 1;
	}
	
	if(strcmp(parametro_1,"esegui") == 0)
	{
	while ((nread = getline(&buffer, &len, stream)) != -1)
	{
		if(primo_ciclo_fetch == 0)
		{
			sscanf(buffer,"%d",&numero_istruzioni_programma);
			printf("%d",numero_istruzioni_programma);
		}
		primo_ciclo_fetch++;
	}
	}
	if(strcmp(parametro_1,"stampa") == 0)
	{
	
	while ((nread = getline(&buffer, &len, stream)) != -1)
	{
		if(primo_ciclo_fetch == 0)
		{
			sscanf(buffer,"%d",&numero_istruzioni_programma);
		}
		else
		{
			sscanf(buffer,"%d",&numero_istruzioni_programma);
			switch (numero_istruzioni_programma) {
				case 0: printf("[%d]  HALT\n",numero_istruzione); break;
 	            case 21: printf("[%d]  RET\n",numero_istruzione); break;
				case 1: {nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
				printf("[%d]  DISPLAY R%d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 2: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  PRINT_STACK %d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 10: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  PUSH R%d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 11: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  POP R%d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 12: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
				nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro2);
			    printf("[%d]  MOV R%d %d \n",numero_istruzione,parametro1, parametro2);
				numero_istruzione = numero_istruzione + 2;
				}; break;
				case 20: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  CALL %d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 22: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  JMP %d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 23: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  JZ %d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 24: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  JPOS %d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 25: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
			    printf("[%d]  JNEG %d \n",numero_istruzione,parametro1);
				numero_istruzione++;
				}; break;
				case 30: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
				nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro2);
			    printf("[%d]  ADD R%d R%d \n",numero_istruzione,parametro1, parametro2);
				numero_istruzione = numero_istruzione + 2;
				}; break;
				case 31: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
				nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro2);
			    printf("[%d]  SUB R%d R%d \n",numero_istruzione,parametro1, parametro2);
				numero_istruzione = numero_istruzione + 2;
				}; break;
				case 32: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
				nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro2);
			    printf("[%d]  MUL R%d R%d \n",numero_istruzione,parametro1, parametro2);
				numero_istruzione = numero_istruzione + 2;
				}; break;
				case 33: { nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro1);
				nread = getline(&buffer, &len, stream);
				sscanf(buffer,"%d",&parametro2);
			    printf("[%d]  DIV R%d R%d \n",numero_istruzione,parametro1, parametro2);
				numero_istruzione = numero_istruzione + 2;
				}; break;
				default: printf("error"); break;
	    }
		numero_istruzione++;
		}
		primo_ciclo_fetch++;
	}
	}
	fclose(stream);
	return 0;
}
