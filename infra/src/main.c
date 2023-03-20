#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)

#define BUT1_PIO PIOD
#define BUT1_ID ID_PIOD
#define BUT1_IDX 28
#define BUT1_MASK (1 << BUT1_IDX)

#define BUT2_PIO PIOC
#define BUT2_ID ID_PIOC
#define BUT2_IDX 31
#define BUT2_MASK (1 << BUT2_IDX)

#define BUT3_PIO PIOA
#define BUT3_ID ID_PIOA
#define BUT3_IDX 19
#define BUT3_MASK (1 << BUT3_IDX)

#define LED1_PIO PIOA
#define LED1_ID ID_PIOA
#define LED1_IDX 0
#define LED1_MASK (1 << LED1_IDX)

#define LED2_PIO PIOC
#define LED2_ID ID_PIOC
#define LED2_IDX 30
#define LED2_MASK (1 << LED2_IDX)

#define LED3_PIO PIOB
#define LED3_ID ID_PIOB
#define LED3_IDX 2
#define LED3_MASK (1 << LED3_IDX)

volatile char but1_flag = 0;
volatile char but1_flagPressed = 0;
volatile char but2_flag = 0;
volatile char but3_flag = 0;
volatile char but3_flagPressed = 0;
volatile int frequencia = 100;
volatile int indexLed = 0;
volatile char estadoLed = 0;
volatile char freq[100];

void atualiza_barra(){
	for (int i = 0; i<indexLed*4; i+=2){
		gfx_mono_draw_rect(i, 30, 2, 2, GFX_PIXEL_SET);
	}
}

void limpa_barra(){
	for (int i = 0; i<130; i+=2){
		gfx_mono_draw_rect(i, 30, 2, 2, GFX_PIXEL_CLR);
	}
}

void atualiza_texto(){
	gfx_mono_draw_string("     " , 50,0, &sysfont);
	gfx_mono_draw_string(freq , 50,0, &sysfont);
}

void pisca_led(int n, int t, int led){
	if (estadoLed){
		pio_clear(LED2_PIO, LED2_MASK);
		delay_ms(t);
		pio_set(LED2_PIO, LED2_MASK);
		atualiza_barra();
	}
	if (indexLed >= 30){
		estadoLed = 0;
		indexLed = 0;
		limpa_barra();
	} else{
		indexLed += 1;
	}
}

void but_callback1(void){
	but1_flagPressed = !pio_get(BUT1_PIO, PIO_INPUT, BUT1_MASK);
	but1_flag = 1;
}


void but_callback2(void){
	but2_flag = 1;
}


void but_callback3(void){
	but3_flagPressed = !pio_get(BUT3_PIO, PIO_INPUT, BUT3_MASK);
	but3_flag = 1;
}

char int_str(){
	sprintf(freq,"%d", frequencia);
}

void aumenta_freq(){
	frequencia += 2;
	int_str();
	atualiza_texto();
}

void reset_indexLed(){
	indexLed = 0;
}

void reset_freq(){
	frequencia = 100;
	int_str();
	atualiza_texto();
	limpa_barra();
	reset_indexLed();
}

void diminui_freq(){
	if (frequencia !=0){
		frequencia -= 2;
		int_str();
		atualiza_texto();
	}
}

void init(void)
{
	sysclk_init();
	
	// Initializing OLED-display
	
	// INIT Board Clock
	WDT->WDT_MR = WDT_MR_WDDIS;								// Desativa WatchDog Timer

	//Led 1
	pmc_enable_periph_clk(LED1_ID);						// Ativa o PIO utilizando ID, Modo de Entrada
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_MASK, PIO_DEFAULT);
	//Led 2
	pmc_enable_periph_clk(LED2_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_MASK, PIO_DEFAULT);
	//Led 3
	pmc_enable_periph_clk(LED3_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_MASK, PIO_DEFAULT);
	//Button 1
	pmc_enable_periph_clk(BUT1_ID);								// Ativando Botao 1
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	// Button 2
	pmc_enable_periph_clk(BUT2_ID);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	// Button 3
	pmc_enable_periph_clk(BUT3_ID);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	// Adding Interruptions - Button 1
	pio_handler_set(BUT1_PIO, BUT1_ID, BUT1_MASK, PIO_IT_FALL_EDGE, but_callback1);
	pio_enable_interrupt(BUT1_PIO, BUT1_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	NVIC_EnableIRQ(BUT1_ID);
	NVIC_SetPriority(BUT1_ID, 4);
	
	// Adding Interruptions - Button 2
	pio_handler_set(BUT2_PIO, BUT2_ID, BUT2_MASK, PIO_IT_FALL_EDGE, but_callback2);
	pio_enable_interrupt(BUT2_PIO, BUT2_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	NVIC_EnableIRQ(BUT2_ID);
	NVIC_SetPriority(BUT2_ID, 4);
	// Adding Interruptions - Button 3
	pio_handler_set(BUT3_PIO, BUT3_ID, BUT3_MASK, PIO_IT_FALL_EDGE, but_callback3);
	pio_enable_interrupt(BUT3_PIO, BUT3_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	NVIC_EnableIRQ(BUT3_ID);
	NVIC_SetPriority(BUT3_ID, 4);
	
	
}


int main(void)
{
	board_init();
	init();
	delay_init();
	gfx_mono_ssd1306_init();
	pio_set(LED1_PIO, LED1_MASK);
	pio_set(LED3_PIO, LED3_MASK);
	while (1){
		if (but1_flag ||but1_flagPressed){
			aumenta_freq();
			but1_flagPressed = !pio_get(BUT1_PIO, PIO_INPUT, BUT1_MASK);
			estadoLed = 1;
			but1_flag = 0;
		}
		
		if (but2_flag){
			reset_freq();
			estadoLed = 0;
			but2_flag = 0;
		}
		
		if (but3_flag ||but3_flagPressed){
			diminui_freq();
			but3_flagPressed = !pio_get(BUT3_PIO, PIO_INPUT, BUT3_MASK);
			estadoLed = 1;
			but3_flag = 0;
		}
		
		pisca_led(1,frequencia,2);
		delay_ms(10);
		if (!estadoLed){
			pmc_sleep(SLEEPMGR_SLEEP_WFI);
		}
	}
	return 0;
}