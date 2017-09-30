#include <stdio.h>
#include "44b.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

#define N 4

/* Variables para la gesti�n de la ISR del teclado
 * 
 * Keybuffer: puntero que apuntar� al buffer en el que la ISR del teclado debe
 *            almacenar las teclas pulsadas
 * keyCount: variable en el que la ISR del teclado almacenar� el n�mero de teclas pulsadas
 */
volatile static char *keyBuffer = NULL;
volatile static int keyCount = 0;

/* Variables para la gestion de la ISR del timer
 * 
 * tmrbuffer: puntero que apuntar� al buffer que contendr� los d�gitos que la ISR del
 *            timer debe mostrar en el display de 8 segmentos
 * tmrBuffSize: usado por printD8Led para indicar el tama�o del buffer a mostrar
 */
volatile static char *tmrBuffer = NULL;
volatile static int tmrBuffSize = 0;

//Variables globales para la gesti�n del juego
static char passwd[N];  //Buffer para guardar la clave inicial
static char guess[N];   //Buffer para guardar la segunda clave

enum state {
	INIT = 0,     //Init:       Inicio del juego
	SPWD = 1,     //Show Pwd:   Mostrar password
	DOGUESS = 2,  //Do guess:   Adivinar contrase�a
	SGUESS = 3,   //Show guess: Mostrar el intento
	GOVER = 4     //Game Over:  Mostrar el resultado
};
enum state gstate; //estado/fase del juego 

//COMPLETAR: Declaraci�n adelantada de las ISRs de timer y teclado (las marca como ISRs)
void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));

// Funci�n que va guardando las teclas pulsadas
static void push_buffer(char *buffer, int key)
{
	int i;
	for (i=0; i < N-1; i++)
		buffer[i] = buffer[i+1];
	buffer[N-1] = (char) key;
}

void timer_ISR(void)
{
	static int pos = 0; //contador para llevar la cuenta del d�gito del buffer que toca mostrar

    //COMPLETAR: Visualizar el d�gito en la posici�n pos del buffer tmrBuffer en el display
	D8Led_digit(tmrBuffer[pos]);

	if(pos == tmrBuffSize-1){// Si es el �ltimo d�gito:
		pos = 0;//      Poner pos a cero,
		tmr_stop(0);//      Parar timer
		tmrBuffer = NULL;//      Dar tmrBuffer valor NULL
		rI_ISPC =~ 0x0;
	}

	else{
		*tmrBuffer = tmrBuffer[pos];// Si no, se apunta al siguiente d�gito a visualizar (pos)
		pos++;
	}

	// COMPLETAR: Finalizar correctamente la ISR
	 rI_ISPC =~ 0x0;
}

void printD8Led(char *buffer, int size)
{
	//Esta rutina prepara el buffer que debe usar timer_ISR (tmrBuffer)
	tmrBuffer = buffer;
	tmrBuffSize = size;
	
	//COMPLETAR: Arrancar el TIMER0 para que interrumpa SIEMPRE cada segundo
	tmr_start(TIMER0);
	tmr_update(TIMER0);

	//COMPLETAR: Esperar a que timer_ISR termine (tmrBuffer)
	while(tmrBuffer != NULL){

	}
}

void keyboard_ISR(void)
{
	int key;

	/* Eliminar rebotes de presi�n */
	Delay(200);

	/* Escaneo de tecla */
	// COMPLETAR
	key = kb_scan();


	if (key != -1) {
		//COMPLETAR:
		//Si la tecla pulsada es F deshabilitar interrupciones por teclado y 
		//poner keyBuffer a NULL
		if(key == 0xF){
			rI_ISPC =~ 0x0;
			//ic_disable(24);
			keyBuffer = NULL;
		}
		// Si la tecla no es F guardamos la tecla pulsada en el buffer apuntado
		// por keybuffer mediante la llamada a la rutina push_buffer
		else{
			push_buffer(keyBuffer, key);
			keyCount++;
		}
		
		// Actualizamos la cuenta del n�mero de teclas pulsadas

		/* Esperar a que la tecla se suelte, consultando el registro de datos rPDATG */		
		while (!(rPDATG & 0x02));
	}

	/* Eliminar rebotes de depresión */
	Delay(200);

	//COMPLETAR: Finalizar correctamente la ISR
	rINTMSK &=~ (0x0 << 24); //eint1 del tclado
	//ic_disable(24);
	 rI_ISPC =~ 0x0;
	//ic_cleanflag(24);


}

int read_kbd(char *buffer)
{
	//Esta rutina prepara el buffer en el que keyboard_ISR almacenar� las teclas 
	//pulsadas (keyBuffer) y pone a 0 el contador de teclas pulsadas
	keyBuffer = buffer;
	keyCount = 0;

	//COMPLETAR: Habilitar interrupciones por teclado

	//COMPLETAR: Esperar a que keyboard_ISR indique que se ha terminado de
	//introducir la clave (keyBuffer)
	while(keyBuffer != NULL){

	}

	//COMPLETAR: Devolver n�mero de teclas pulsadas
	return keyCount;
}

static int show_result()
{
	int error = 0;
	int i = 0;
	int j;
	char buffer[2] = {0};

	// COMPLETAR: poner error a 1 si las contrase�as son distintas
	for(j = 0; j < N; j++){
		if(passwd[j] == guess[j]){
			i++;
		}
	}
	if(i == N){
		error = 0;
	}
	else{
		error = 1;
	}
	// COMPLETAR
	// Hay que visualizar el resultado durante 2s.
	if(error == 0){ // Si se ha acertado tenemos que mostrar una A y si no una E
		buffer[0] = 0xA;
		buffer[1] = 0xA;
	}
	else{
		buffer[0] = 0xE;
		buffer[1] = 0xE;
	}
	// Como en printD8Led haremos que la ISR del timer muestre un buffer con dos
	// caracteres A o dos caracteres E (eso durar� 2s)
	printD8Led(buffer, 2);
	// COMPLETAR: esperar a que la ISR del timer indique que se ha terminado
	while(tmrBuffer != NULL){

	}

	return error;// COMPLETAR: Devolver el valor de error para indicar si se ha acertado o no
}

int setup(void)
{

	D8Led_init();

	/* COMPLETAR: Configuraci�n del timer0 para interrumpir cada segundo */
	//printD8Led(tmrBuffer, tmrBuffSize);
	tmr_set_prescaler(0, 255);		//      valor de prescalado a 255
	tmr_set_divider(0, 1);			//      valor del divisor 1/4
	tmr_set_count(0, 62500, 62495);	//      valor de cuenta 62500 y cualquier valor de comparacion entre 1 y 62499
	tmr_update(0);					//      actualizar el contador con estos valores (update)
	tmr_set_mode(0,1);				//      poner el contador en modo RELOAD
	tmr_stop(0);					//      dejar el contador parado
	/***************************/
	/* COMPLETAR: Port G-configuraci�n para generaci�n de interrupciones externas
	 *         por teclado
	 **/
	portG_conf(1,EINT);
	portG_eint_trig(1,FALLING);
	portG_conf_pup(1,ENABLE);


	/********************************************************************/

	// COMPLETAR: Registramos las ISRs
	pISR_TIMER0   = (unsigned) timer_ISR;
	pISR_EINT1    = (unsigned) keyboard_ISR;

	/* Configuraci�n del controlador de interrupciones*/


	ic_init();

	 /* Habilitamos la l�nea IRQ, en modo vectorizado y registramos una ISR para
		 * la l�nea IRQ
		 * Configuramos el timer 0 en modo IRQ y habilitamos esta l�nea
		 * Configuramos la l�nea EINT1 en modo IRQ y la habilitamos
		 */
	ic_conf_irq(1, 0); //Habilitamos la l�nea IRQ, en modo vectorizado
	ic_conf_fiq(0);				//		deshabilitar la l�nea FIQ
	ic_conf_line(13, 0); //Configuramos el timer 0 en modo IRQ
	ic_enable(13); //habilitamos esta l�nea
	ic_conf_line(24, 0); //Configuramos la l�nea EINT1 en modo IRQ
	ic_enable(24); //la habilitamos

	/***************************************************/

	Delay(0);

	/* Inicio del juego */
	gstate = INIT;
	D8Led_digit(12);

	return 0;
}


int loop(void)
{
	int count; //n�mero de teclas pulsadas
	int error;

	//M�quina de estados

	switch (gstate) {
		case INIT:
			do {
				//COMPLETAR:
				D8Led_digit(12);//Visualizar una C en el display
				//Llamar a la rutina read_kbd para guardar los 4 d�gitos en el buffer passwd
				count = read_kbd(passwd);
     			//Esta rutina devuelve el n�mero de teclas pulsadas.
				//Dibujar una E en el display si el n�mero de teclas pulsadas es menor que 4, en este caso hacer un Delay(10000)
				if(count < 4){
					D8Led_digit(14);
					Delay(10000);
				}
			//} while (/*permanecer en el while mientras se hayan pulsado menos de 4 teclas*/);
			}while(count < 4);

			//COMPLETAR: Pasar al estado siguiente
			gstate = SPWD;
			break;

		case SPWD:

			// COMPLETAR:
			// Visualizar en el display los 4 d�gitos del buffer passwd, para
			// ello llamar a la rutina printD8Led
			printD8Led(passwd, N);
			Delay(10000);
			//COMPLETAR: Pasar al estado siguiente
			gstate = DOGUESS;
			break;

		case DOGUESS:
			Delay(10000);

			do {
				//COMPLETAR:
				D8Led_digit(15);//Visualizar en el display una F
				count = read_kbd(guess);//Llamar a la rutina read_kbd para guardar los 4 d�gitos en el buffer guess
				//Esta rutina devuelve el n�mero de teclas pulsadas.
				if(count < 4){//Dibujar una E en el display si el n�mero de teclas pulsadas es menor que 4, en este caso hacer un Delay(10000)
					D8Led_digit(14);
					Delay(10000);
				}
			//} while (/*permanecer en el while mientras se hayan pulsado menos de 4 teclas*/);
			}while(count < 4);
			//COMPLETAR: Pasar al estado siguiente
			gstate = SGUESS;
			break;

		case SGUESS:
			//COMPLETAR:
			//Visualizar en el display los 4 d�gitos del buffer guess, 
			//para ello llamar a la rutina printD8Led
			printD8Led(guess, 4);
			Delay(10000);
			//COMPLETAR: Pasar al estado siguiente
			gstate = GOVER;

			break;

		case GOVER:
			//COMPLETAR:
			//Mostrar el mensaje de acierto o error con
			error = show_result();
			Delay(10000);
			//Si he acertado el estado siguiente es INIT sino DOGUESS
			if(error == 0){
				gstate = INIT;
			}
			else if(error == 1){
				gstate = DOGUESS;
			}

			break;
	}
	return 0;
}

int main(void)
{
	setup();

	while (1) {
		loop();
	}
}
