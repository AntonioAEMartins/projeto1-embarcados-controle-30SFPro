/************************************************************************
* 5 semestre - Eng. da Computao - Insper
*
* 2021 - Exemplo com HC05 com RTOS
*
*/

#include <asf.h>
#include "conf_board.h"
#include <string.h>

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botoes
	// Lado Direito
#define BUT1_PIO      PIOA
#define BUT1_PIO_ID   ID_PIOA
#define BUT1_IDX      13
#define BUT1_IDX_MASK (1 << BUT1_IDX)

#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX      19
#define BUT2_IDX_MASK (1 << BUT2_IDX)

#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   ID_PIOA
#define BUT3_IDX      6
#define BUT3_IDX_MASK (1 << BUT3_IDX)

#define BUT4_PIO      PIOD
#define BUT4_PIO_ID   ID_PIOD
#define BUT4_IDX      25
#define BUT4_IDX_MASK (1 << BUT4_IDX)
	// Lado Esquerdo
#define BUT5_PIO      PIOD
#define BUT5_PIO_ID   ID_PIOD
#define BUT5_IDX      24
#define BUT5_IDX_MASK (1 << BUT5_IDX)

#define BUT6_PIO      PIOA
#define BUT6_PIO_ID   ID_PIOA
#define BUT6_IDX      24
#define BUT6_IDX_MASK (1 << BUT6_IDX)

#define BUT7_PIO      PIOB
#define BUT7_PIO_ID   ID_PIOB
#define BUT7_IDX      3
#define BUT7_IDX_MASK (1 << BUT7_IDX)

#define BUT8_PIO      PIOA
#define BUT8_PIO_ID   ID_PIOA
#define BUT8_IDX      9
#define BUT8_IDX_MASK (1 << BUT8_IDX)
	// Botoes Pause 
#define BUT9_PIO      PIOA
#define BUT9_PIO_ID   ID_PIOA
#define BUT9_IDX      5
#define BUT9_IDX_MASK (1 << BUT9_IDX)

// usart (bluetooth ou serial)
// Descomente para enviar dados
// pela serial debug

#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define USART_COM USART1
#define USART_COM_ID ID_USART1
#else
#define USART_COM USART0
#define USART_COM_ID ID_USART0
#endif


/************************************************************************/

#define TASK_BLUETOOTH_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY        (tskIDLE_PRIORITY)



extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	* identify which task has overflowed its stack.
	*/
	for (;;) {
	}
}

/* This function is called by FreeRTOS idle task */
extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/* This function is called by FreeRTOS each tick */
extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}


void io_init(void) {

	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pmc_enable_periph_clk(BUT4_PIO_ID);
	pmc_enable_periph_clk(BUT5_PIO_ID);
	pmc_enable_periph_clk(BUT6_PIO_ID);
	pmc_enable_periph_clk(BUT7_PIO_ID);
	pmc_enable_periph_clk(BUT8_PIO_ID);

	// Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT4_PIO, PIO_INPUT, BUT4_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT5_PIO, PIO_INPUT, BUT5_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT6_PIO, PIO_INPUT, BUT6_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT7_PIO, PIO_INPUT, BUT7_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT8_PIO, PIO_INPUT, BUT8_IDX_MASK, PIO_PULLUP);
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	#if defined(__GNUC__)
	setbuf(stdout, NULL);
	#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	* emits one character at a time.
	*/
	#endif
}

uint32_t usart_puts(uint8_t *pstring) {
	uint32_t i ;

	while(*(pstring + i))
	if(uart_is_tx_empty(USART_COM))
	usart_serial_putchar(USART_COM, *(pstring+i++));
}

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms) {
	uint timecounter = timeout_ms;
	uint32_t rx;
	uint32_t counter = 0;

	while( (timecounter > 0) && (counter < bufferlen - 1)) {
		if(usart_read(usart, &rx) == 0) {
			buffer[counter++] = rx;
		}
		else{
			timecounter--;
			vTaskDelay(1);
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen,
char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void config_usart0(void) {
	sysclk_enable_peripheral_clock(ID_USART0);
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);

	// RX - PB0  TX - PB1
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

int hc05_init(void) {
	char buffer_rx[128];
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+NAMEagoravai", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+PIN0000", 100);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

void task_bluetooth(void) {
	printf("Task Bluetooth started \n");
	
	printf("Inicializando HC05 \n");
	config_usart0();
	hc05_init();

	// configura LEDs e Botões
	io_init();

	char button1 = '0';
	char button2 = '0';
	char button3 = '0';
	char button4 = '0';
	char button5 = '0';
	char button6 = '0';
	char button7 = '0';
	char button8 = '0';
	char button9 = '0';
	
	char eof = 'X';

	// Task não deve retornar.
	while(1) {
		// atualiza valor do botão
		if(pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK) == 0) {
			button1 = '1';
 		} else {
			 button1 = '0';
		}
		if(pio_get(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK) == 0) {
			button2 = '1';
		} else {
			button2 = '0';
		}
		if(pio_get(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK) == 0) {
			button3 = '1';
		} else {
			button3 = '0';
		}
		
		if(pio_get(BUT4_PIO, PIO_INPUT, BUT4_IDX_MASK) == 0) {
			button4 = '1';
		} else {
			button4 = '0';
		}
		
		if(pio_get(BUT5_PIO, PIO_INPUT, BUT5_IDX_MASK) == 0) {
			button5 = '1';
		} else {
			button5 = '0';
		}
		
		if(pio_get(BUT6_PIO, PIO_INPUT, BUT6_IDX_MASK) == 0) {
			button6 = '1';
		} else {
			button6 = '0';
		}
		
		if(pio_get(BUT7_PIO, PIO_INPUT, BUT7_IDX_MASK) == 0) {
			button7 = '1';
		} else {
			button7 = '0';
		}
		
		if(pio_get(BUT8_PIO, PIO_INPUT, BUT8_IDX_MASK) == 0) {
			button8 = '1';
		} else {
			button8 = '0';
		}
		
// 		printf("Button 1: %c\n", button1);
// 		printf("Button 2: %c\n", button2);
// 		printf("Button 3: %c\n", button3);
// 		printf("Button 4: %c\n", button4);
		printf("Button 5: %c\n", button5);
		printf("Button 6: %c\n", button6);
		printf("Button 7: %c\n", button7);
		printf("Button 8: %c\n", button8);
		

// 		// envia status botão
// 		while(!usart_is_tx_ready(USART_COM)) {
// 			vTaskDelay(10 / portTICK_PERIOD_MS);
// 		}
// 		usart_write(USART_COM, button1);
// 		
// 		// envia fim de pacote
// 		while(!usart_is_tx_ready(USART_COM)) {
// 			vTaskDelay(10 / portTICK_PERIOD_MS);
// 		}
// 		usart_write(USART_COM, eof);

		// dorme por 500 ms
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	configure_console();

	/* Create task to make led blink */
	xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
