#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*         Gruppo        P1G103
 *   
 *   	Nicolas Pietro Martignon    870034 
 *   	Thomas Visentin             869438
 *
 */

/**********************************************************************************/
/*DEFINE*/
/**********************************************************************************/
#define MAXREG 32
#define MAXSTACK 16384


/**********************************************************************************/
/*INIZIALIZZAZIONE E/O DICHIARAZIONE VARIABILI GLOBALI*/
/**********************************************************************************/
unsigned int SP = 0;
unsigned int IP = 0;
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
        registri = (int * ) calloc(MAXREG, sizeof(int)); 		/*alloco tutti i 32 registri a 0 */
        stack = (int * ) calloc(MAXSTACK, sizeof(int)); 		/*alloco lo stack di 64 KB (di interi) a 0*/
        memoria = (int * ) calloc(istruzioni, sizeof(int)); 		/*alloco la memoria che conterrà il programma a 0*/
}

void chiusura(){                                         		/*free e chiusura programma*/
        free(registri); 			
        free(stack); 			
        free(memoria); 
	fclose(stream);
        exit(0);		
}

void errore(char * c){                                    		/*errore virtual machine*/        
	printf("Errore Virtual Machine: %s\n", c);
	printf("Programma abortito !\n");
	chiusura();
}

void stack_push(int n){ 						/*stack push con i relativi controlli*/ 
	if(SP>=MAXSTACK)
		errore("Stack Overflow");
		
	stack[SP++] = n;
}

int stack_pop(){							/*stack pop con i relativi controlli*/
	if(SP<1)
		errore("Stack Underflow");
		
	return stack[--SP];
}

void registro_scrivi(int pos, int n){					/*scrittura su registro con i relativi controlli*/
	if(pos<0 || pos>=MAXREG)
		errore("Registro inesistente, registri disponibili 0->31");

	registri[pos] = n;
}

int registro_leggi(int pos){						/*lettura da registro con i relativi controlli*/		
	if(pos<0 || pos>=MAXREG)
		errore("Registro inesistente, registri disponibili 0->31");

	return registri[pos];
}


/**********************************************************************************/
/*FUNZIONI AUSILIARE*/
/**********************************************************************************/

void getline_sscanf(int* parametro1, int* parametro2){        		/*legge dal file e memorizza i numeri*/
        getline(&buffer, &len, stream);
	while(buffer[0] == ';')
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
        /*INIZIALIZZAZIONE E/O DICHIARAZIONE VARIABILI*/
        /**********************************************************************************/

        int numero_istruzioni_programma, appo, REG1, REG2, REG1_ABS, REG2_ABS;
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
                printf("Troppi parametri\n");
                return 1;
        }
        /* controllo che il programma non venga invocato con meno di 2 parametri in caso contrario termino il programma*/
        if (argc < 3) {
                printf("Pochi parametri\n");
                return 1;
        }
        /* primo parametro errato */
        if ((strcmp(argv[1], "esegui") != 0) && (strcmp(argv[1], "stampa") != 0)) {
                printf("Operazioni invalida\n");
		printf("Operazioni ammesse: stampa esegui\n");
                return 1;
        }

        stream = fopen(argv[2], "r");

        /* se il file non esiste termino il programma */
        if (stream == NULL) {
                printf("File \"%s\" non esistente\n", argv[2]);
                return 1;
        }

	/* operazione esegui */
        if (strcmp(argv[1], "esegui") == 0) {
                getline_sscanf( &numero_istruzioni_programma, NULL);                    

                inizializza(numero_istruzioni_programma);                               /*chiamata ad inizializza*/

                for (appo = 0; appo < numero_istruzioni_programma; ++appo)		/*prefetch istruzioni*/
			getline_sscanf( &memoria[appo], NULL);

                while (guardia) {
			
			/* se instruction pointer supera il numero di istruzioni del programma */
			if(IP >= (unsigned int)numero_istruzioni_programma){           
				errore("ERRORE DI SEGMENTAZIONE");
				break;
			}

                        switch (memoria[IP++]) {     /* fetch istruzione */

                        case 0: /*halt*/
                                guardia = 0;
                                break;

                        case 1: /*display*/	   
                                printf("R%d:%d\n", memoria[IP], registro_leggi(memoria[IP]));		IP++; 
                                break;

                        case 2: /*print stack*/
                                for (appo = 0; appo < memoria[IP] && appo < numero_istruzioni_programma; appo++)
                                        printf("STACK[%d]:%d\n", appo, stack[appo]);
                                IP++;
                                break;

                        case 10: /*push*/                                               
				stack_push(registro_leggi(memoria[IP++]));
                                break;

                        case 11: /*pop*/					       
				registro_scrivi(memoria[IP++], stack_pop());
                                break;

                        case 12: /*mov*/
				registro_scrivi(memoria[IP], memoria[IP + 1]);		IP += 2;
                                break;

                        case 20: /*call*/						
				stack_push(IP + 1);	IP = memoria[IP];
                                break;

                        case 21: /*ret*/					       
				IP = stack_pop();
                                break;

                        case 22: /*jmp*/			
                                IP = memoria[IP];                         
                                break;

                        case 23: /*jz*/							
				(!stack_pop())     ?	IP = memoria[IP] : IP++; 		
                                break;

                        case 24: /*jpos*/						
				(0 < stack_pop())  ?	IP = memoria[IP] : IP++;
                                break;

                        case 25: /*jneg*/						
				(0 > stack_pop())  ?	IP = memoria[IP] : IP++;      
                                break;

                        case 30: /*add*/
						/*controllo overflow*/
                                if (registro_leggi(memoria[IP]) > 0 && registro_leggi(memoria[IP + 1]) > 0 && 
				   ((INT_MAX - registro_leggi(memoria[IP])) < registro_leggi(memoria[IP + 1])))
                                 	errore("Add Overflow");
                                        
				else
				{
					if(registro_leggi(memoria[IP]) < 0 && registro_leggi(memoria[IP + 1]) < 0 && 
					  ((INT_MIN - registro_leggi(memoria[IP])) > registro_leggi(memoria[IP + 1])))
                                                errore("Add Underflow");
                                                 
					else
					        stack_push(registro_leggi(memoria[IP]) + registro_leggi(memoria[IP + 1]));	
				}

				IP += 2;
                                break;

                        case 31: /*sub*/
						/*controllo overflow*/
                                if (registro_leggi(memoria[IP]) > 0 && registro_leggi(memoria[IP + 1]) < 0 && 
				   ((INT_MAX - registro_leggi(memoria[IP])) > (registro_leggi(memoria[IP + 1]))*-1)) 
                                 	errore("Sub Overflow");
                                        
				else
				{
				        if(registro_leggi(memoria[IP]) < 0 && registro_leggi(memoria[IP + 1]) > 0 && 
					  ((INT_MIN - registro_leggi(memoria[IP]))*-1 < registro_leggi(memoria[IP + 1])))
                                           	errore("Sub Underflow");
                                          
					else
                                                stack_push(registro_leggi(memoria[IP]) - registro_leggi(memoria[IP + 1]));
				        
				}
                                IP += 2;
                                break;

                        case 32: /*mult*/
				REG1 = registro_leggi(memoria[IP]);
				REG2 = registro_leggi(memoria[IP + 1]);
				REG1_ABS = abs(registro_leggi(memoria[IP]));
				REG2_ABS = abs(registro_leggi(memoria[IP + 1]));
					
				/*controllo overflow*/
				if(REG1 && REG2){
					if((INT_MAX / REG1_ABS) < REG2_ABS){
						if((REG1<0 && REG2>0) || (REG1>0 && REG2<0))
							errore("Mult Underflow");
						else
							errore("Mult Overflow");	
					}	
				}
				
				stack_push(REG1 * REG2);
				IP += 2;

                                break;

                        case 33: /*div*/
				 /*controllo divisione per 0*/
                                if (registro_leggi(memoria[IP + 1]) == 0) 
                                       	errore("Divisione per 0!");
                                        
				else{
                                	stack_push(registro_leggi(memoria[IP]) / registro_leggi(memoria[IP + 1]));
                                	IP += 2;
				}
                                break;

                        default: /*error*/
                                errore("Comando non riconosciuto");
                                break;
                        
			} /*parentesi switch*/


                        /****************FOR DEBUG***********************/
			/*
			for(appo=0; appo< 32; appo++){
                            printf("REG[%d]: %d\n", appo, registri[appo]);
                        }
                        printf("\n");
                        for(appo=0; appo< 32; appo++){
                            printf("STACK[%d]: %d\n", appo, stack[appo]);
                        }
                        scanf("%d", &appo);
			*/


		} /*parentesi while*/

                printf("Programma terminato correttamente!\n");
		chiusura();

        } /*parentesi funzione*/

	/* operazione stampa */
        if (strcmp(argv[1], "stampa") == 0) {

                while ((getline( & buffer, & len, stream)) != -1) {   /* fetch istruzione */
                	if(buffer[0] != ';'){
                        	if (primo_ciclo_fetch == 0)
                                	sscanf(buffer, "%d", & numero_istruzioni_programma);

                        	else {
                                	sscanf(buffer, "%d", & numero_istruzioni_programma);
                                	switch (numero_istruzioni_programma) {

                                	case 0: /*halt*/
                                	        printf("[%d]  HALT\n", numero_istruzione);
                                	        break;

                                	case 21: /*ret*/ 
                                	        printf("[%d]  RET\n", numero_istruzione);
                                	        break;
                                	
					case 1: /*display*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  DISPLAY R%d \n", numero_istruzione, parametro1);	
						numero_istruzione++;
                                	        break;
	
                                	case 2: /*print stack*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  PRINT_STACK %d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;

                                	case 10: /*push*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  PUSH R%d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;

                                	case 11: /*pop*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  POP R%d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;
                                	
					case 12: /*mov*/
                                	        getline_sscanf( &parametro1, &parametro2);
                                	        printf("[%d]  MOV R%d %d \n", numero_istruzione, parametro1, parametro2);
                                	        numero_istruzione += 2;
                                	        break;
	
                                	case 20: /*call*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  CALL %d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;
                                	
					case 22: /*jmp*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  JMP %d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;
	
                                	case 23: /*jz*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  JZ %d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;
	
                                	case 24: /*jpos*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  JPOS %d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;
	
                                	case 25: /*jneg*/
                                	        getline_sscanf( &parametro1, NULL);
                                	        printf("[%d]  JNEG %d \n", numero_istruzione, parametro1);
                                	        numero_istruzione++;
                                	        break;
	
                                	case 30: /*add*/
                                	        getline_sscanf( &parametro1, &parametro2);
                                	        printf("[%d]  ADD R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                	        numero_istruzione += 2;
                                	        break;
	
                                	case 31: /*sub*/
                                	        getline_sscanf( &parametro1, &parametro2);
                                	        printf("[%d]  SUB R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                	        numero_istruzione += 2;
                                	        break;
	
                                	case 32: /*mult*/
                                	        getline_sscanf( &parametro1, &parametro2);
                                	        printf("[%d]  MUL R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                	        numero_istruzione += 2;
                                	        break;
	
                                	case 33: /*div*/
                                	        getline_sscanf( &parametro1, &parametro2);
                                	        printf("[%d]  DIV R%d R%d \n", numero_istruzione, parametro1, parametro2);
                                	        numero_istruzione += 2;
                                	        break;
	
                                	default: /*error*/
                                	        printf("Errore Virtual Machine: Comando non riconosciuto\n");
                                       		break;
                                	} /*parentesi switch*/
                                	
					numero_istruzione++;
                        	} /*parentesi else*/

                        	primo_ciclo_fetch++;
			} /*parentesi if*/
                } /*parentesi while*/
		
		fclose(stream);
		return 0;
        } /*parentesi stampa*/

	return 1;
}
