#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**********************************************************************************/
/*INIZIALIZZAZIONE DICHIARAZIONE VARIABILI GLOBALI*/
/**********************************************************************************/

FILE * stream;
int * stack;
int * memoria;
int * registri;
char * buffer = NULL;
size_t len;


/**********************************************************************************/
/*FUNZIONI AUSILIARE ESEGUI*/
/**********************************************************************************/

void inizializza(int istruzioni) {
        registri = (int * ) calloc(32, sizeof(int)); 			/*alloco tutti i 32 registri a 0 */
        stack = (int * ) calloc(16384, sizeof(int)); 			/*alloco lo stack di 64 KB (di interi) a 0*/
        memoria = (int * ) calloc(istruzioni, sizeof(int)); 		/*alloco la memoria che conterrà il programma a 0*/
}

void chiusura() {                                         		/*free*/
        free(registri); 			
        free(stack); 			
        free(memoria); 		
}



/**********************************************************************************/
/*FUNZIONI AUSILIARE STAMPA*/
/**********************************************************************************/

void getline_sscanf(int* parametro1, int* parametro2){        		/*legge dal file e memorizza i numeri*/
        getline(&buffer, &len, stream);
        sscanf(buffer, "%d", parametro1);
	
	if(parametro2){
		getline(&buffer, &len, stream);
		sscanf(buffer, "%d", parametro2);
	}
}


/**********************************************************************************/
/*CODICE PRINCIPALE MAIN*/
/**********************************************************************************/

int main(int argc, char ** argv) {

 	/**********************************************************************************/
        /*INIZIALIZZAZIONE E O DICHIARAZIONE VARIABILI*/
        /**********************************************************************************/

        int numero_istruzioni_programma, appo;
        unsigned int SP = 0;
        unsigned int IP = 0;
        int guardia = 1;
        int numero_istruzione = 0;
        int parametro1 = 0;
        int parametro2 = 0;
        int primo_ciclo_fetch = 0;


        /**********************************************************************************/
        /*CODICE*/
        /**********************************************************************************/

        /* controllo che il programma non venga invocato con più di 2 parametri in caso contrario termino il programma*/
        if (argc > 3) {
                printf("Too many parameters\n");
                return 1;
        }
        /* controllo che il programma non venga invocato con meno di 2 parametri in caso contrario termino il programma*/
        if (argc < 3) {
                printf("Too few parameters\n");
                return 1;
        }
        /* primo parametro errato */
        if ((strcmp(argv[1], "esegui") != 0) && (strcmp(argv[1], "stampa") != 0)) {
                printf("First parameter is invalid\n");
                return 1;
        }

        stream = fopen(argv[2], "r");

        /* se il file non esiste termino il programma */
        if (stream == NULL) {
                printf("File \"%s\" doesn't exist\n", argv[2]);
                return 1;
        }

	/* operazione esegui */
        if (strcmp(argv[1], "esegui") == 0) {
                getline_sscanf( &numero_istruzioni_programma, NULL);                    

                inizializza(numero_istruzioni_programma);                               /*chiamata ad inizializza*/

                for (appo = 0; appo < numero_istruzioni_programma; ++appo)		/*prefetch istruzioni*/
			getline_sscanf( &memoria[appo], NULL);

                while (guardia) {
                        switch (memoria[IP++]) {

                        case 0: /*halt*/
                                guardia = 0;
                                break;

                        case 1: /*display*/	   /*IP++ come era prima dentro il printf da problemi*/
                                printf("R%d:%d\n", memoria[IP], registri[memoria[IP]]);		IP++; 
                                break;

                        case 2: /*print stack*/
                                for (appo = 0; appo < memoria[IP]; appo++)
                                        printf("STACK[%d]:%d\n", appo, stack[appo]);
                                IP++;
                                break;

                        case 10: /*push*/
                                stack[SP++] = registri[memoria[IP++]];
                                break;

                        case 11: /*pop*/
                                registri[memoria[IP++]] = stack[--SP];
                                break;

                        case 12: /*mov*/
                                registri[memoria[IP]] = memoria[IP + 1]; 	  IP += 2;
                                break;

                        case 20: /*call*/
                                stack[SP++] = IP + 1;	   IP = memoria[IP];
                                break;

                        case 21: /*ret*/
                                IP = stack[--SP];
                                break;

                        case 22: /*jmp*/
                                IP = memoria[IP];                         
                                break;

                        case 23: /*jz*/
                                (stack[--SP])      ?	IP = memoria[IP] : IP++;	
                                break;

                        case 24: /*jpos*/
                                (0 < stack[--SP])  ?	IP = memoria[IP] : IP++;   
                                break;

                        case 25: /*jneg*/
                                (0 > stack[--SP])  ?	IP = memoria[IP] : IP++;      
                                break;

                        case 30: /*add*/
                                stack[SP++] = registri[memoria[IP]] + registri[memoria[IP + 1]];
				 /*controllo overflow*/
                                if ((stack[SP - 1] < 0  &&  registri[memoria[IP]] > 0  &&  registri[memoria[IP + 1]] > 0)  ||
				    (stack[SP - 1] > 0  &&  registri[memoria[IP]] < 0  &&  registri[memoria[IP + 1]] < 0)) {
                                 	printf("Overflow\n");
                                        guardia = 0;
                                }
                                IP += 2;
                                break;

                        case 31: /*sub*/
                                stack[SP++] = registri[memoria[IP]] - registri[memoria[IP + 1]];
				 /*controllo overflow*/
                                if ((stack[SP - 1] < registri[memoria[IP]]) != (registri[memoria[IP + 1]] > 0)){ 
                                      	printf("Overflow\n");
                                        guardia = 0;
                                }
                                IP += 2;
                                break;

                        case 32: /*mult*/
                                stack[SP++] = registri[memoria[IP]] * registri[memoria[IP + 1]];
				 /*controllo overflow*/
                                if (stack[SP - 1] != 0 && stack[SP - 1] / registri[memoria[IP]] != registri[memoria[IP + 1]]){
                                       	printf("Overflow\n");
                                        guardia = 0;
                                }
                                IP += 2;
                                break;

                        case 33: /*div*/
				 /*controllo divisione per 0*/
                                if (registri[memoria[IP + 1]] == 0) {
                                       	printf("Divisione per 0!\n");
                                        guardia = 0;
                                }
				else{
                                	stack[SP++] = registri[memoria[IP]] / registri[memoria[IP + 1]];
                                	IP += 2;
				}
                                break;

                        default: /*error*/
                                printf("Comando non riconosciuto");
                                break;
                        
			} /*parentesi switch*/


                        /****************FOR DEBUG*********************** 
				
			for(appo=0; appo< 32; appo++){
                            printf("REG[%d]: %d\n", appo, registri[appo]);
                        }
                        printf("\n");

                        for(appo=0; appo< 32; appo++){
                            printf("STACK[%d]: %d\n", appo, stack[appo]);
                        }
                        scanf("%d", &appo);*/


		} /*parentesi while*/

                printf("Esecuzione Terminata!\n");
		chiusura();

        } /*parentesi funzione*/

	/* operazione stampa */
        if (strcmp(argv[1], "stampa") == 0) {

                while ((getline( & buffer, & len, stream)) != -1) {

                        if (primo_ciclo_fetch == 0)
                                sscanf(buffer, "%d", & numero_istruzioni_programma);

                        else {
                                sscanf(buffer, "%d", & numero_istruzioni_programma);
                                switch (numero_istruzioni_programma) {

                                case 0:
                                        printf("[%d]  HALT\n", numero_istruzione);
                                        break;

                                case 21:
                                        printf("[%d]  RET\n", numero_istruzione);
                                        break;
                                case 1:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  DISPLAY R%d \n", numero_istruzione, parametro1);	
					numero_istruzione++;
                                        break;

                                case 2:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  PRINT_STACK %d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;

                                case 10:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  PUSH R%d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;

                                case 11:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  POP R%d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;
                                case 12:
                                        getline_sscanf( &parametro1, &parametro2);
                                        printf("[%d]  MOV R%d %d \n", numero_istruzione, parametro1, parametro2);
                                        numero_istruzione += 2;
                                        break;

                                case 20:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  CALL %d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;
                                case 22:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  JMP %d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;

                                case 23:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  JZ %d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;

                                case 24:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  JPOS %d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;

                                case 25:
                                        getline_sscanf( &parametro1, NULL);
                                        printf("[%d]  JNEG %d \n", numero_istruzione, parametro1);
                                        numero_istruzione++;
                                        break;

                                case 30:
                                        getline_sscanf( &parametro1, &parametro2);
                                        printf("[%d]  ADD R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                        numero_istruzione += 2;
                                        break;

                                case 31:
                                        getline_sscanf( &parametro1, &parametro2);
                                        printf("[%d]  SUB R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                        numero_istruzione += 2;
                                        break;

                                case 32:
                                        getline_sscanf( &parametro1, &parametro2);
                                        printf("[%d]  MUL R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                        numero_istruzione += 2;
                                        break;

                                case 33:
                                        getline_sscanf( &parametro1, &parametro2);
                                        printf("[%d]  DIV R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                        numero_istruzione += 2;
                                        break;

                                default:
                                        printf("error");
                                        break;
                                }
                                numero_istruzione++;
                        }
                        primo_ciclo_fetch++;
                }
        }

        fclose(stream);
        return 0;
}
