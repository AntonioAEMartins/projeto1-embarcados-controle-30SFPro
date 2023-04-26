#include "conf_board.h"
#include <asf.h>
#include <string.h>


#define AFEC_VX1 AFEC0
#define AFEC_VX1_ID ID_AFEC0
#define AFEC_VX1_CHANNEL 0 // Canal do pino PD30
#define AFEC_VY1 AFEC0
#define AFEC_VY1_ID ID_AFEC0
#define AFEC_VY1_CHANNEL 5 // Canal do pino PB2
#define AFEC_VX2 AFEC1
#define AFEC_VX2_ID ID_AFEC1
#define AFEC_VX2_CHANNEL 1 // Canal do pino Pc13
#define AFEC_VY2 AFEC1
#define AFEC_VY2_ID ID_AFEC1
#define AFEC_VY2_CHANNEL 6 // Canal do pino Pc31

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

#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define USART_COM USART1
#define USART_COM_ID ID_USART1
#else
#define USART_COM USART0
#define USART_COM_ID ID_USART0
#endif

QueueHandle_t xQueueA1VX;
QueueHandle_t xQueueA1VY;
QueueHandle_t xQueueA1VXdigital;
QueueHandle_t xQueueA1VYdigital;

QueueHandle_t xQueueA2VX;
QueueHandle_t xQueueA2VY;
QueueHandle_t xQueueA2VXdigital;
QueueHandle_t xQueueA2VYdigital;

TimerHandle_t xTimerA1VX;
TimerHandle_t xTimerA1VY;

#define TASK_BLUETOOTH_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY (tskIDLE_PRIORITY)

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                          signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);
static void config_AFEC_pot(Afec *afec);
static void config_AFEC_channel(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                                afec_callback_t callback);
// static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
static void AFEC_a1vx_callback(void);
static void AFEC_a1vy_callback(void);
static void AFEC_a2vx_callback(void);
static void AFEC_a2vy_callback(void);
void xTimerA1VXCallback(TimerHandle_t xTimerA1VX);
void xTimerA1VYCallback(TimerHandle_t xTimerA1VY);
/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* RTOS application HOOK                                                */
/************************************************************************/

/* Called if stack overflow during execution */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                          signed char *pcTaskName) {
    //printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
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
extern void vApplicationTickHook(void) {}

extern void vApplicationMallocFailedHook(void) {
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

    /* Force an assert. */
    configASSERT((volatile void *)NULL);
}

void but_init(void) {

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

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

static void AFEC_a1vx_callback(void) {

    uint32_t a1_vx;
    a1_vx = afec_channel_get_value(AFEC_VX1, AFEC_VX1_CHANNEL);
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    xQueueSendFromISR(xQueueA1VX, &a1_vx, &xHigherPriorityTaskWoken);
}

static void AFEC_a1vy_callback(void) {

    uint32_t a1_vy;
    a1_vy = afec_channel_get_value(AFEC_VY1, AFEC_VY1_CHANNEL);
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    xQueueSendFromISR(xQueueA1VY, &a1_vy, &xHigherPriorityTaskWoken);
}

static void AFEC_a2vx_callback(void) {

	uint32_t a2_vx;
	a2_vx = afec_channel_get_value(AFEC_VX2, AFEC_VX2_CHANNEL);
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueA2VX, &a2_vx, &xHigherPriorityTaskWoken);
}

static void AFEC_a2vy_callback(void) {

	uint32_t a2_vy;
	a2_vy = afec_channel_get_value(AFEC_VY2, AFEC_VY2_CHANNEL);
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueA2VY, &a2_vy, &xHigherPriorityTaskWoken);
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void io_init(void) {
    config_AFEC_pot(AFEC_VX1);
    //config_AFEC_pot(AFEC_VY1);
	
	config_AFEC_pot(AFEC_VX2);
	//config_AFEC_pot(AFEC_VY2);
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
    uint32_t i;

    while (*(pstring + i))
        if (uart_is_tx_empty(USART_COM))
            usart_serial_putchar(USART_COM, *(pstring + i++));
}

void usart_put_string(Usart *usart, char str[]) {
    usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms) {
    uint timecounter = timeout_ms;
    uint32_t rx;
    uint32_t counter = 0;

    while ((timecounter > 0) && (counter < bufferlen - 1)) {
        if (usart_read(usart, &rx) == 0) {
            buffer[counter++] = rx;
        } else {
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
    vTaskDelay(500 / portTICK_PERIOD_MS);
    usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    usart_send_command(USART_COM, buffer_rx, 1000, "AT+NAMEagoravai", 100);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    usart_send_command(USART_COM, buffer_rx, 1000, "AT+PIN0000", 100);
}

static void config_AFEC_channel(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                                afec_callback_t callback) {
    /*** Configuracao específica do canal AFEC ***/
    struct afec_ch_config afec_ch_cfg;
    afec_ch_get_config_defaults(&afec_ch_cfg);
    afec_ch_cfg.gain = AFEC_GAINVALUE_0;
    afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);

    /*
    * Calibracao:
    * Because the internal ADC offset is 0x200, it should cancel it and shift
    down to 0.
    */
    afec_channel_set_analog_offset(afec, afec_channel, 0x200);

    /***  Configura sensor de temperatura ***/
    struct afec_temp_sensor_config afec_temp_sensor_cfg;

    afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
    afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);

    /* configura IRQ */
    afec_set_callback(afec, afec_channel, callback, 1);
    NVIC_SetPriority(afec_id, 4);
    NVIC_EnableIRQ(afec_id);
}

static void config_AFEC_pot(Afec *afec) {
    /*************************************
     * Ativa e configura AFEC
     *************************************/
    /* Ativa AFEC - 0 */
    afec_enable(afec);

    /* struct de configuracao do AFEC */
    struct afec_config afec_cfg;

    /* Carrega parametros padrao */
    afec_get_config_defaults(&afec_cfg);

    /* Configura AFEC */
    afec_init(afec, &afec_cfg);

    /* Configura trigger por software */
    afec_set_trigger(afec, AFEC_TRIG_SW);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

void task_bluetooth(void) {
    //printf("Task Bluetooth started \n");

    //printf("Inicializando HC05 \n");
    config_usart0();
    hc05_init();

    // configura LEDs e Botões
    io_init();
    /* ordem: B,A,X,Y,BAIXO,DIREITA,CIMA,ESQUERDA,DESLIGA */
    char infos[] = {'0', '0', '0', '0', '0', '0', '0', '0', '0'};
	char buttons[10] = "000000000";
    char eof = 'X';
	
	printf("%c", eof);

    // Task não deve retornar.
    for (;;) {
        int32_t estado_ana_x_axis,estado_ana_y_axis;
        while (xQueueReceive(xQueueA1VXdigital, &estado_ana_x_axis, 100)) {
			
            if (estado_ana_x_axis == 1) {
				/* direita */
                infos[5] = '1';
                infos[7] = '0';
            } else if (estado_ana_x_axis == 0) {
				/* esquerda */
                infos[5] = '0';
                infos[7] = '1';
            } else {
				/* parado */
                infos[5] = '0';
                infos[7] = '0';
            }
        }
		
		while (xQueueReceive(xQueueA1VYdigital, &estado_ana_y_axis, 100)) {
			if (estado_ana_y_axis == 1) {
				/* BAIXO */
				infos[4] = '1';
				infos[6] = '0';
			} else if (estado_ana_y_axis == 0) {
				/* CIMA */
				infos[4] = '0';
				infos[6] = '1';	
			} else {
				/* parado */
				infos[4] = '0';
				infos[6] = '0';
			}
		}
		
		/* ordem: B,A,X,Y,BAIXO,DIREITA,CIMA,ESQUERDA,DESLIGA */
		int32_t estado_ana_x2_axis, estado_ana_y2_axis;
		while (xQueueReceive(xQueueA2VXdigital, &estado_ana_x2_axis, 100)) {
			        
			if (estado_ana_x2_axis == 1) {
				/* A */
				infos[1] = '1';
				infos[3] = '0';
			} else if (estado_ana_x2_axis == 0) {
				/* Y */
				infos[1] = '0';
				infos[3] = '1';
			} else {
				/* NADA */
				infos[1] = '0';
				infos[3] = '0';
			}
		}
		        
		while (xQueueReceive(xQueueA2VYdigital, &estado_ana_y2_axis, 100)) {
			if (estado_ana_y2_axis == 1) {
				/* b */
				infos[0] = '1';
				infos[2] = '0';
			} else if (estado_ana_y2_axis == 0) {
				/* x */
				infos[0] = '0';
				infos[2] = '1';
			} else {
				/* nada */
				infos[0] = '0';
				infos[2] = '0';
			}
		}
		
		printf("A");
		for (int i = 0; i < 9; i++){
			printf("%c", infos[i]);
		}
		printf("A");
		
		if(pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK) == 0) {
			buttons[0] = '1';
			} else {
			buttons[0] = '0';
		}
		if(pio_get(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK) == 0) {
			buttons[1] = '1';
			} else {
			buttons[1] = '0';
		}
		if(pio_get(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK) == 0) {
			buttons[2] = '1';
			} else {
			buttons[2] = '0';
		}
		
		if(pio_get(BUT4_PIO, PIO_INPUT, BUT4_IDX_MASK) == 0) {
			buttons[3] = '1';
			} else {
			buttons[3] = '0';
		}
		
		if(pio_get(BUT5_PIO, PIO_INPUT, BUT5_IDX_MASK) == 0) {
			buttons[4] = '1';
			} else {
			buttons[4] = '0';
		}
		
		if(pio_get(BUT6_PIO, PIO_INPUT, BUT6_IDX_MASK) == 0) {
			buttons[5] = '1';
			} else {
			buttons[5] = '0';
		}
		
		if(pio_get(BUT7_PIO, PIO_INPUT, BUT7_IDX_MASK) == 0) {
			buttons[6] = '1';
			} else {
			buttons[6] = '0';
		}
		
		if(pio_get(BUT8_PIO, PIO_INPUT, BUT8_IDX_MASK) == 0) {
			buttons[7] = '1';
			} else {
			buttons[7] = '0';
		}
		
		if(pio_get(BUT9_PIO, PIO_INPUT, BUT9_IDX_MASK) == 0) {
			buttons[8] = '1';
			} else {
			buttons[8] = '0';
		}

		printf("B");
		for (int i = 0; i < 9; i++){
			printf("%c", buttons[i]);
		}
		printf("B");
		printf("\n");
		printf("%c", eof);
        // dorme por 500 ms
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

void xTimerA1VXCallback(TimerHandle_t xTimerA1VX) {
    /* Selecina canal e inicializa conversão */
    afec_channel_enable(AFEC_VX1, AFEC_VX1_CHANNEL);
	afec_channel_enable(AFEC_VX2, AFEC_VX2_CHANNEL);
	
    afec_start_software_conversion(AFEC_VX1);
	afec_start_software_conversion(AFEC_VX2);
}

void xTimerA1VYCallback(TimerHandle_t xTimerA1VY) {
    /* Selecina canal e inicializa conversão */
	
    afec_channel_enable(AFEC_VY1, AFEC_VY1_CHANNEL);
	afec_channel_enable(AFEC_VY2, AFEC_VY2_CHANNEL);
	
	afec_start_software_conversion(AFEC_VY1);
	afec_start_software_conversion(AFEC_VY2);
}

void task_a1vx(void){

    config_AFEC_channel(AFEC_VX1, AFEC_VX1_ID, AFEC_VX1_CHANNEL, AFEC_a1vx_callback);

    xTimerA1VX = xTimerCreate(/* Just a text name, not used by the RTOS
                          kernel. */
                          "Timer",
                          /* The timer period in ticks, must be
                          greater than 0. */
                          400,
                          /* The timers will auto-reload themselves
                          when they expire. */
                          pdTRUE,
                          /* The ID is used to store a count of the
                          number of times the timer has expired, which
                          is initialised to 0. */
                          (void *)0,
                          /* Timer callback */
                          xTimerA1VXCallback);
						  
	xTimerStart(xTimerA1VX, 0);

    uint32_t a1vx;
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    int32_t estado; /* se 1, para direita, se 0 para esquerda */

    for(;;){

        if (xQueueReceive(xQueueA1VX, &a1vx, 100)){

            uint32_t vx = a1vx;

            if (vx < 3000) {
                estado = 0;
            } else if (vx > 3300){
                estado = 1;
            } else {
                estado = -1;
            }

            xQueueSend(xQueueA1VXdigital, &estado, 0);
        }
    }
}

void task_a1vy(void){
	
	config_AFEC_channel(AFEC_VY1, AFEC_VY1_ID, AFEC_VY1_CHANNEL, AFEC_a1vy_callback);
	
   xTimerA1VY = xTimerCreate(/* Just a text name, not used by the RTOS
                          kernel. */
                          "Timer",
                          /* The timer period in ticks, must be
                          greater than 0. */
                          400,
                          /* The timers will auto-reload themselves
                          when they expire. */
                          pdTRUE,
                          /* The ID is used to store a count of the
                          number of times the timer has expired, which
                          is initialised to 0. */
                          (void *)0,
                          /* Timer callback */
                          xTimerA1VYCallback);
						  
	xTimerStart(xTimerA1VY, 0);

    uint32_t a1vy;
    int32_t estado; /* se 1, para BAIXO, se 0 para CIMA */

    for (;;) {

        if (xQueueReceive(xQueueA1VY, &a1vy, 100)) {

            uint32_t vy = a1vy;
            
            if (vy < 3000) {
                estado = 0;
            } else if (vy > 3300){
                estado = 1;
            } else {
                estado = -1;
            }
            
            xQueueSend(xQueueA1VYdigital, &estado, 0);
        }
    }
}

void task_a2vx(void){

    config_AFEC_channel(AFEC_VX2, AFEC_VX2_ID, AFEC_VX2_CHANNEL, AFEC_a2vx_callback);

    uint32_t a2vx;
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    int32_t estado; /* se 1, para direita, se 0 para esquerda */

    for(;;){

        if (xQueueReceive(xQueueA2VX, &a2vx, 100)){

            uint32_t vx = a2vx;

            if (vx < 3000) {
                estado = 0;
            } else if (vx > 3300){
                estado = 1;
            } else {
                estado = -1;
            }

            xQueueSend(xQueueA2VXdigital, &estado, 0);
        }
    }
}

void task_a2vy(void){
	
	config_AFEC_channel(AFEC_VY2, AFEC_VY2_ID, AFEC_VY2_CHANNEL, AFEC_a2vy_callback);
	
    uint32_t a2vy;
    int32_t estado; /* se 1, para BAIXO, se 0 para CIMA */

    for (;;) {

        if (xQueueReceive(xQueueA2VY, &a2vy, 100)) {

            uint32_t vy = a2vy;
            
            if (vy < 3000) {
                estado = 0;
            } else if (vy > 3300){
                estado = 1;
            } else {
                estado = -1;
            }
            //printf("vy= %d\n",vy);
            xQueueSend(xQueueA2VYdigital, &estado, 0);
        }
    }
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {
    /* Initialize the SAM system */
    sysclk_init();
    board_init();
    io_init();
	but_init();

    configure_console();

    /* Create task to make led blink */
    xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    /* task analogico a1vx*/
    xTaskCreate(task_a1vx, "analogico a1vx", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    /* task analogico a1vy*/
    xTaskCreate(task_a1vy, "analogico a1vy", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);
	
	/* task analogico a1vx*/
	xTaskCreate(task_a2vx, "analogico a2vx", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

	/* task analogico a1vy*/
	xTaskCreate(task_a2vy, "analogico a2vy", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    /* fila leitura analogico */
    xQueueA1VX = xQueueCreate(100, sizeof(uint32_t));
    xQueueA1VY = xQueueCreate(100, sizeof(uint32_t));   
    /* fila de tratamento da leitura analogica */
    xQueueA1VXdigital = xQueueCreate(100, sizeof(int32_t));
    xQueueA1VYdigital = xQueueCreate(100, sizeof(int32_t));
	
	/* fila leitura analogico */
	xQueueA2VX = xQueueCreate(100, sizeof(uint32_t));
	xQueueA2VY = xQueueCreate(100, sizeof(uint32_t));
	/* fila de tratamento da leitura analogica */
	xQueueA2VXdigital = xQueueCreate(100, sizeof(int32_t));
	xQueueA2VYdigital = xQueueCreate(100, sizeof(int32_t));

    if (xQueueA1VX == NULL)
        printf("falha em criar a queue xQueueA1VX \n");

    if (xQueueA1VY == NULL)
        printf("falha em criar a queue xQueueA1VY \n");

    if (xQueueA1VXdigital == NULL)
        printf("falha em criar a queue xQueueA1VX \n");

    if (xQueueA1VYdigital == NULL)
        printf("falha em criar a queue xQueueA1VY \n");
		
	if (xQueueA2VX == NULL)
		printf("falha em criar a queue xQueueA1VX \n");

	if (xQueueA2VY == NULL)
		printf("falha em criar a queue xQueueA1VY \n");

	if (xQueueA2VXdigital == NULL)
		printf("falha em criar a queue xQueueA1VX \n");

	if (xQueueA2VYdigital == NULL)
		printf("falha em criar a queue xQueueA1VY \n");

    /* Start the scheduler. */
    vTaskStartScheduler();

    while (1) {

    }

    /* Will only get here if there was insufficient memory to create the idle task. */
    return 0;
}
