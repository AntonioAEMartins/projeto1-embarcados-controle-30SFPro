/************************************************************************
 * 5 semestre - Eng. da Computao - Insper
 *
 * 2021 - Exemplo com HC05 com RTOS
 *
 */

#include "conf_board.h"
#include <asf.h>
#include <string.h>

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// analogico
#define AFEC_VX AFEC0
#define AFEC_VX_ID ID_AFEC0
#define AFEC_VX_CHANNEL 0 // Canal do pino PD30

#define AFEC_VY AFEC0
#define AFEC_VY_ID ID_AFEC0
#define AFEC_VY_CHANNEL 5 // Canal do pino PB2

/* 
MAIS PINOS PARA USAR ANALOGICO 
AFEC1 Canal 1 PC13
AFEC1 Canal 1 pc31
*/

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
/* RTOS                                                                 */
/************************************************************************/
/* FILA DO ADC */
QueueHandle_t xQueueA1VX;
QueueHandle_t xQueueA1VY;
// QueueHandle_t xQueueA1VXdigital;
// QueueHandle_t xQueueA1VYdigital;
SemaphoreHandle_t xSemaphoreLeitura;

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
static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
static void AFEC_a1vx_callback(void);
static void AFEC_a1vy_callback(void);
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

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
}

void RTT_Handler(void){
	uint32_t ul_status;
	ul_status = rtt_get_status(RTT);
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS){
        xSemaphoreGiveFromISR(xSemaphoreLeitura,&xHigherPriorityTaskWoken);
	}
}

static void AFEC_a1vx_callback(void) {

    uint32_t a1_vx;
    a1_vx = afec_channel_get_value(AFEC_VX, AFEC_VX_CHANNEL);
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    xQueueSendFromISR(xQueueA1VX, &a1_vx, &xHigherPriorityTaskWoken);
}

static void AFEC_a1vy_callback(void) {

    uint32_t a1_vy;
    a1_vy = afec_channel_get_value(AFEC_VY, AFEC_VY_CHANNEL);
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    xQueueSendFromISR(xQueueA1VY, &a1_vy, &xHigherPriorityTaskWoken);
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void io_init(void) {
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

static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback) {
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
    char eof = 'X';

    // Task não deve retornar.
    for (;;) {
        // atualiza valor do botão
 /*       if (pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK) == 0) {
            button1 = '1';
        } else {
            button1 = '0';
        }
*/
        // envia status botão
        while (!usart_is_tx_ready(USART_COM)) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        usart_write(USART_COM, button1);

        // envia fim de pacote
        while (!usart_is_tx_ready(USART_COM)) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        usart_write(USART_COM, eof);

        // dorme por 500 ms
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void task_a1vx(void){

    /* configurar adc e tc para controlar a leitura */
    config_AFEC_pot(AFEC_VX, AFEC_VX_ID, AFEC_VX_CHANNEL, AFEC_a1vx_callback);
    // afec_enable_interrupt(AFEC_VX, AFEC_INTERRUPT_EOC_0);

    uint32_t a1vx;
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    for(;;){

        afec_channel_enable(AFEC_VX, AFEC_VX_CHANNEL);
        afec_start_software_conversion(AFEC_VX);

        printf("oi\n");

        if (xSemaphoreTake(xSemaphoreLeitura,5)){

            if (xQueueReceive(xQueueA1VX, &a1vx, 10)){

            uint32_t vx = a1vx;
            // char estado_vx;

            printf("vx = %d\n",vx);

            // if (vx > 2300){
            //     estado_vx = 'd';
            // } else {
            //     estado_vx = 'e';
            // }

            // printf("estado vx: %c\n",estado_vx);

            // xQueueSend(xQueueA1VXdigital)
            // printf("vx = %d\n",vx);
            }
        }
    }
}

void task_a1vy(void){

    /* configurar adc e tc para controlar a leitura */
    config_AFEC_pot(AFEC_VY, AFEC_VY_ID, AFEC_VY_CHANNEL, AFEC_a1vy_callback);

    uint32_t a1vy;
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    for(;;){

        // afec_channel_enable(AFEC_VY, AFEC_VY_CHANNEL);
        // afec_start_software_conversion(AFEC_VY);

        // if (xQueueReceive(xQueueA1VY, &a1vy, 100)){

        //     int vy = a1vy;
        //     // printf("vy = %d\n",vy);
        // }

        vTaskDelay(5);
    }
}

// void task_a1vx_digital(void){
// }

// void task_a1vy_digital(void){
// }

/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {
    /* Initialize the SAM system */
    sysclk_init();
    board_init();

    configure_console();

    /* Create task to make led blink */
    xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    /* task analogico a1vx*/
    xTaskCreate(task_a1vx, "analogico a1vx", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    /* task analogico a1vy*/
    xTaskCreate(task_a1vy, "analogico a1vy", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    // /* task analogico a1vx*/
    // xTaskCreate(task_a1vx_digital, "analogico a1vx digital", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    // /* task analogico a1vy*/
    // xTaskCreate(task_a1vy_digital, "analogico a1vy digital", TASK_BLUETOOTH_STACK_SIZE, NULL, TASK_BLUETOOTH_STACK_PRIORITY, NULL);

    RTT_init(5, 1000, RTT_MR_ALMIEN);

    /* criando semaforo */
    xSemaphoreLeitura = xSemaphoreCreateBinary();

    /* fila leitura analogico */
    xQueueA1VX = xQueueCreate(100, sizeof(uint32_t));
    xQueueA1VY = xQueueCreate(100, sizeof(uint32_t));   
    /* fila de tratamento da leitura analogica */
    // xQueueA1VXdigital = xQueueCreate(100, sizeof(uint32_t));
    // xQueueA1VYdigital = xQueueCreate(100, sizeof(uint32_t));

    if (xQueueA1VX == NULL)
        printf("falha em criar a queue xQueueA1VX \n");

    if (xQueueA1VY == NULL)
        printf("falha em criar a queue xQueueA1VY \n");

    // if (xQueueA1VXdigital == NULL)
    //     printf("falha em criar a queue xQueueA1VX \n");

    // if (xQueueA1VYdigital == NULL)
    //     printf("falha em criar a queue xQueueA1VY \n");
    
    if (xSemaphoreLeitura == NULL){
		printf("Failed to create semaphore\n");
	}

    /* Start the scheduler. */
    vTaskStartScheduler();

    while (1) {

    }

    /* Will only get here if there was insufficient memory to create the idle task. */
    return 0;
}
