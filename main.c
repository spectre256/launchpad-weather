#include "msp.h"
#include "csHFXT.h"
#include "csLFXT.h"
#include "json.h"
#include "map.h"
#include "array.h"
#include "lcd.h"
#include <stdlib.h>
#include <stdio.h>

// #define TEST
#define BUFFER_SIZE 320
#define CLK_FREQUENCY 48000000 // MCLK using 48MHz HFXT
#define TIMER_TICKS 1250 // 5 s * (32kHz / 128)

/* Global Variables */
const char httpRequest[] = "GET /v1/current.json?key=921e078dd8a44054a06172330242501&q=47803 HTTP/1.1\nHost: api.weatherapi.com\nUser-Agent: Windows NT 10.0; +https://github.com/spectre256/forwarder Forwarder/0.0.1\nAccept: application/json\n\n";

JSONValue* json = NULL;
JSONValue* current = NULL;

volatile char* buffer;
volatile int buffer_i = 0;
volatile int nl_cnt = 0;
volatile bool responseReady = false;

/*
 * This function prints a (NUL-terminated) message over UART. Assumes configuration
 *  of eUSCI_A0 for UART transmit.
 *
 *  TODO: Move to uart.c
 */
void printMessage(const char* const message) {
    int i;
    for (i = 0; i == 0 || message[i - 1] != '\0'; i++) {
        // Check if the TX buffer is empty first
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

        // Send next character of message
        EUSCI_A0->TXBUF = message[i];
    }
}

void sendRequest(void) {
    if (!responseReady) {
        printMessage(httpRequest);
    }
}

void displayLCD(LCDField field){
    JSONValue* value;
    JSONValue* wind_dir;
    char* format;
    char charBuffer[] = "                ";

    switch (field) {
    case TEMP:
        value = JSONGet(current, "temp_f");
        format = "Temp: %8.3f F";
        break;
    case HUMIDITY:
        value = JSONGet(current, "humidity");
        format = "Humidity: %5.2f%%";
        break;
    case CONDITION:
        value = JSONGet(current, "condition");
        value = JSONGet(value, "text");
        format = "%-16.*s";
        break;
    case WIND:
        value = JSONGet(current, "wind_mph");
        wind_dir = JSONGet(current, "wind_dir");
        format = "Wind: %3.1f mph %-2.*s";
    }

    switch(value->type){
    case NUMBER:
        if (field == WIND) {
            snprintf(charBuffer, sizeof(charBuffer), format, value->value.number, wind_dir->value.str->length, wind_dir->value.str->str);
        } else {
            snprintf(charBuffer, sizeof(charBuffer), format, value->value.number);
        }
        break;
    case STRING:
        snprintf(charBuffer, sizeof(charBuffer), format, value->value.str->length, value->value.str->str);
        break;
    default:
        return;
    }

    int i;
    for(i = 0; i < (sizeof(charBuffer)/sizeof(char)) - 1; i++){
        printChar(charBuffer[i]);    // print format string plus value
    }
}

inline void updateLCD(void) {
    setCursorFirstLine();   // Set LCD cursor to start of first line
    displayLCD(field1);
    setCursorSecondLine();
    displayLCD(field2);
}

void handleResponse(void) {
    destroyJSON(json);
    json = parseJSON(buffer);
    if (!json) return;

    current = JSONGet(json, "current");

    updateLCD();

    responseReady = false;
}

void initSW(void){
    // set pin modes to GPIO
    // clear bit 4 of SEL0 and SEL1
    P1->SEL0 &= ~BIT4;
    P1->SEL1 &= ~BIT4;

    // set pin directions to input
    P1->DIR & ~BIT4;

    // set internal resistors for pull-up and enable them
    P1->OUT |= BIT4;
    P1->REN |= BIT4;

    // Enable port interrupts with high-to-low transition
//    P1->IES |= BIT4;    // set high to low transition
//    P1->IE |= BIT4;     // enable interrupt
}

/**
 * main.c
 */
int main(void) {
    // Stop Watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    buffer = malloc(BUFFER_SIZE * sizeof(char));

    // Config stuff
    configHFXT();
    configLFXT();
    initSW();
    configLCD(CLK_FREQUENCY);
    initLCD();

    // Configure UART pins
    P1->SEL0 |= BIT2 | BIT3;
    P1->SEL1 &= ~(BIT2 | BIT3);

    /* Configure UART
     *  Asynchronous UART mode, 8O1 (8-bit data, even parity, 1 stop bit),
     *  LSB first, SMCLK clock source
     */
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST;     // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SSEL__SMCLK // SMCLK source
                // | EUSCI_A_CTLW0_PEN             // Parity enable
                | EUSCI_A_CTLW0_SWRST;          // Remain in reset

    /* Baud Rate calculation
     * Refer to Section 24.3.10 of Technical Reference manual
     * BRCLK = 48000000, BR = 38400
     * N = 1250
     */
    EUSCI_A0->BRW = 78;

    // Configure baud clock modulation in eUSCI_A0 modulation control register
    // EUSCI_A0->MCTLW |= 0x0021 & 0x00FF;
    EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS)
            | (0 << EUSCI_A_MCTLW_BRS_OFS)
            | EUSCI_A_MCTLW_OS16;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;    // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;        // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;            // Enable USCI_A0 RX interrupt

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCIA0 interrupt in NVIC module
    NVIC->ISER[0] = (1 << EUSCIA0_IRQn);

    // Optional tests
    #ifdef TEST

    // Array tests
    testArray();

    // Map tests
    testMap();

    // Test JSON parser
    testParser();

    #endif

    sendRequest();

    // Configure timer to send another request every 5 seconds
    TIMER_A0->CCR[0] = TIMER_TICKS;
    TIMER_A0->CTL = TIMER_A_CTL_MC__UP
                | TIMER_A_CTL_SSEL__ACLK
                | TIMER_A_CTL_IE
                | TIMER_A_CTL_CLR;

    NVIC->ISER[0] |=  1 << TA0_N_IRQn;

    __enable_irq(); // Enable global interrupt

    int delay;

    sendRequest();

    while (true) {
        if (responseReady) {
            handleResponse();
        }

        // Handle button press
        if (!(P1->IN & BIT4)) {
            // create function in lcd.c to cycle info on LCD screen
            cycleLCD();
            updateLCD();
            // lazy debounce for now
            for (delay = 0; delay < 5000; delay++);
            // wait for S2 released
            while (((P1->IN & 0x0010) >> 4) == 0);
            // debounce
            for (delay = 0; delay < 5000; delay++);
        }
    }
}

// UART interrupt service routine
void EUSCIA0_IRQHandler(void) {
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) {
        // Note that reading RX buffer clears the flag and removes value from buffer
        char input = EUSCI_A0->RXBUF;

        // If the buffer is full or there is already a response pending, don't do anything
        if (responseReady || buffer_i >= BUFFER_SIZE) return;

        // Set flag if input is a NUL character
        if (input == '\0') {
            buffer[buffer_i] = '\0';
            buffer_i = 0;
            nl_cnt = 0;
            responseReady = true;
            return;
        }

        // Drop HTTP response headers and only save body
        if (nl_cnt < 2) {
            switch (input) {
            case '\n':
                nl_cnt++;
            case '\r':
                break;
            default:
                nl_cnt = 0;
            }
        } else {
            buffer[buffer_i] = input;
            buffer_i++;
        }
    }
}

// Timer interrupt to send request every 5 s
void TA0_N_IRQHandler(void) {
    // Not necessary to check which flag is set because only one IRQ mapped to this interrupt vector
    sendRequest();

    // Clear timer compare flag in TA3CCTL0
    TIMER_A0->CTL &= ~TIMER_A_CTL_IFG;

}
