//*********************************************************************
// Universidad del Valle de Guatemala
// IE2023: Programación de Microcontroladores
// Proyecto: LAB6 6
// Descripción: UART con menú, lectura ADC en tiempo real y visualización en LEDs
// Hardware: ATmega328p
//*********************************************************************

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>


void UART_init9600(void);
void UART_writeChar(char c);
void UART_writeText(char *texto);
uint16_t leerADC(uint8_t canal);
void mostrarMenu(void);

// Variables 
volatile char seleccion = 0;
volatile char bufferTX = 0;

void UART_writeChar(char c) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

void UART_writeText(char *texto) {
	while (*texto) {
		UART_writeChar(*texto++);
	}
}

void UART_init9600(void) {
	DDRD &= ~(1 << DDD0); // RX
	DDRD |=  (1 << DDD1); // TX

	UCSR0A = 0;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	UBRR0 = 103; // 9600 baud
}

uint16_t leerADC(uint8_t canal) {
	ADMUX = (1 << REFS0) | (canal & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

void mostrarMenu(void) {
	UART_writeText("\r\n=== MENÚ ===\r\n");
	UART_writeText("1. Leer potenciómetro en tiempo real (ADC5)\r\n");
	UART_writeText("2. Enviar ASCII y mostrar en LEDs (PORTB)\r\n");
	UART_writeText("Seleccione una opción: ");
}

void setup(void) {
	DDRB = 0xFF;
	PORTB = 0x00;

	UART_init9600();

	// ADC prescaler 128
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	sei();
}

int main(void) {
	setup();

	UART_writeText("Microcontrolador listo.\r\n");
	mostrarMenu();

	while (1) {
		if (seleccion == '1') {
			UART_writeText("\r\nModo lectura de potenciómetro activo (ADC5).\r\n");
			UART_writeText("Presiona 'x' para regresar al menú.\r\n");

			while (1) {
				uint16_t valor = leerADC(5);
				char buffer[32];
				sprintf(buffer, "ADC5: %4u\r\n", valor);
				UART_writeText(buffer);

				_delay_ms(300);

				if (bufferTX == 'x') {
					bufferTX = 0;
					seleccion = 0;
					mostrarMenu();
					break;
				}
			}
		}
		else if (seleccion == '2') {
			UART_writeText("\r\nModo ASCII activo. Escribe caracteres para PORTB.\r\n");
			UART_writeText("Presiona 'x' para regresar al menú.\r\n");

			while (1) {
				if (bufferTX != 0) {
					if (bufferTX == 'x') {
						bufferTX = 0;
						seleccion = 0;
						mostrarMenu();
						break;
					} else {
						PORTB = bufferTX;
						UART_writeChar(bufferTX);
						UART_writeText("\r\nCaracter mostrado en PORTB.\r\n");
						bufferTX = 0;
					}
				}
			}
		}
	}
}

// Interrupción UART RX 
ISR(USART_RX_vect) {
	bufferTX = UDR0;

	if (bufferTX == '1' || bufferTX == '2') {
		seleccion = bufferTX;
	}
}
