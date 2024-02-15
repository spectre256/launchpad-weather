#include "msp.h"
#include "csHFXT.h"
#include "json.h"
#include "map.h"
#include "array.h"
#include "lcd.h"
#include <stdlib.h>
#include <stdio.h>

// #define TEST
#define BUFFER_SIZE 280
#define CLK_FREQUENCY       48000000    // MCLK using 48MHz HFXT

/* Global Variables */
const char httpRequest[] = "GET /v1/current.json?key=921e078dd8a44054a06172330242501&q=47803 HTTP/1.1\nHost: api.weatherapi.com\nUser-Agent: Windows NT 10.0; +https://github.com/spectre256/forwarder Forwarder/0.0.1\nAccept: application/json\n\n";
const char tempText[] = "Temp(F): ";
const char humidText[] = "Humidity: ";
const char condText[] = "Condition: ";

volatile char* buffer;
volatile int buffer_i = 0;
volatile int nl_cnt = 0;
volatile bool responseReady = false;
char numBuffer[4];

/*
 * This function prints a (NUL-terminated) message over UART. Assumes configuration
 *  of eUSCI_A0 for UART transmit.
 *
 *  TODO: Move to uart.c
 */
void printMessage(const char* const message) {
    int i;
    for (i = 0; message[i - 1] != '\0'; i++) {
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

void switchData(int data){
    switch(data){
    case 0:{
        JSONValue* temp_f = JSONGet(current, "temp_f");
        //Display temp
        for(i = 0; i < sizeof(tempText)/sizeof(char) - 1; i++){
            printChar(tempText[i]); // Print "Temp(F): "
        }
        //Convert value in temp_f to char array
        snprintf(numBuffer, sizeof numBuffer, "%f", temp_f->value.number);
        for(i = 0; i < (sizeof(numBuffer)/sizeof(char)) - 1; i++){
            printChar(numBuffer[i]);    // print value of temp_f
        }
    }
    case 1:{
        JSONValue* humidity = JSONGet(current, "humidity");
        //Display humidity
        for(i = 0; i < sizeof(humidText)/sizeof(char) - 1; i++){
            printChar(humidText[i]);    // Print "Humidity: "
        }
        // Ditto above
        snprintf(numBuffer, sizeof numBuffer, "%f", humidity->value.number);
        for(i = 0; i < (sizeof(numBuffer)/sizeof(char)) - 1; i++){
            printChar(numBuffer[i]);    // Print value of humidity
        }
    }
    case 2:{
        JSONValue* condition = JSONGet(current, "condition");
        JSONValue* conditionText = JSONGet(condition, "text");
        for(i = 0; i < sizeof(humidText)/sizeof(char) - 1; i++){
            printChar(condText[i]);    // Print "Humidity: "
        }
        for(i = 0; i < conditionText->value.str.length; i++){
            printChar(*(conditionText->value.str.str + i));
        }
    }
    }

}

void handleResponse(void) {
    int i;
    JSONValue* json = parseJSON(buffer);
    if (json == NULL) return; // TODO: Properly handle error

    JSONValue* current = JSONGet(json, "current");
    //TODO: Modify to only get the two currently displayed values on LCD
//    JSONValue* temp_f = JSONGet(current, "temp_f");
//    JSONValue* condition = JSONGet(current, "condition");
//    JSONValue* conditionText = JSONGet(condition, "text");
//    JSONValue* humidity = JSONGet(current, "humidity");
//
//    setCursorFirstLine();   // Set LCD cursor to start of first line
//    //Display temp
//    for(i = 0; i < sizeof(tempText)/sizeof(char) - 1; i++){
//        printChar(tempText[i]); // Print "Temp(F): "
//    }
//    //Convert value in temp_f to char array
//    snprintf(numBuffer, sizeof numBuffer, "%f", temp_f->value.number);
//    for(i = 0; i < (sizeof(numBuffer)/sizeof(char)) - 1; i++){
//        printChar(numBuffer[i]);    // print value of temp_f
//    }
//
//    setCursorSecondLine();  // Set LCD cursor to start of second line
//    //Display humidity
//    for(i = 0; i < sizeof(humidText)/sizeof(char) - 1; i++){
//        printChar(humidText[i]);    // Print "Humidity: "
//    }
//    // Ditto above
//    snprintf(numBuffer, sizeof numBuffer, "%f", humidity->value.number);
//    for(i = 0; i < (sizeof(numBuffer)/sizeof(char)) - 1; i++){
//        printChar(numBuffer[i]);    // Print value of humidity
//    }

    /************************************************************************/
    setCursorFirstLine();   // Set LCD cursor to start of first line
    switchData(data1);

    setCursorSecondLine();
    switchData(data2);

    destroyJSON(json);

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

    int delay;

    // TODO: Replace with dynamically resizable buffer
    buffer = malloc(BUFFER_SIZE * sizeof(char));

    // Config stuff
    configHFXT();

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
    NVIC->ISER[0] = (1 << EUSCIA0_IRQn );

    // Optional tests
    #ifdef TEST

    // Array tests
//    testArray();

    // Map tests
//    testMap();

    // Test JSON parser
    testParser();

    #endif

    sendRequest();

    while(true) {
        if (responseReady) {
            handleResponse();
        }

        // Handle button press
        if(((P1->IN & 0x0010) >> 4) == 0){
            //create function in lcd.c to cycle info on LCD screen
            cycleLCD();
            //lazy debounce for now
            for(delay = 0; delay < 5000; delay++);
                // wait for S2 released
            while(((P1->IN & 0x0010) >> 4) == 0);
            for(delay = 0; delay < 5000; delay++);
        }
    }
}

// UART interrupt service routine
void EUSCIA0_IRQHandler(void) {
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) {
        // Note that reading RX buffer clears the flag and removes value from buffer
        char input = EUSCI_A0->RXBUF;

        // Set flag if input is a NUL character
        if (input == '\0') {
            buffer[buffer_i] = '\0';
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

// TODO: Timer interrupt to send request again

// TODO: Button interrupt to send request again and reset timer
//void DIO_PORT_IRQHandler(void){
//    if(P1->IFG & DIO_PORT_IV__IFG4){
//            cycleLCD();
//            P1->IE &= ~BIT4;
//    }
//}
