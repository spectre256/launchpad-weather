#include "msp.h"
#include "json.h"
#include "map.h"
#include "array.h"
#include "csHFXT.h"
#include <stdlib.h>

#define TEST
#define BUFFER_SIZE 900

/* Global Variables */
const char httpRequest[] = "GET /v1/current.json?key=921e078dd8a44054a06172330242501&q=47803 HTTP/1.1\nHost: api.weatherapi.com\nUser-Agent: Windows NT 10.0; +https://github.com/spectre256/forwarder Forwarder/0.0.1\nAccept: application/json\n\n";
char* buffer;
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

void handleResponse(void) {
    JSONValue* json = parseJSON(buffer);
    if (json == NULL) return; // TODO: Properly handle error

    // TODO: Update LCD with new values

    responseReady = false;
}

/**
 * main.c
 */
int main(void) {
    // Stop Watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    // TODO: Replace with dynamically resizable buffer
    buffer = malloc(BUFFER_SIZE * sizeof(char));

    configHFXT();

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
    Array* array = newArray();
    int len = array->length;

    // Allocate just enough members to cause the buffer to resize
    while (len <= DEFAULT_CAPACITY) {
        int* val = malloc(sizeof(int));
        *val = len;
        arrayAppend(array, val);
        len++;
    } // Check capacity/length here to ensure buffer gets resized

    int* first = (int*)arrayGet(array, 0);
    int* last = (int*)arrayGet(array, len - 1);

    arrayDelete(array, len - 2);
    arrayDelete(array, len - 1); // Check capacity/length here to ensure buffer gets resized

    destroyArray(array);

    // Map tests
    Map* map = newMap();
    int* a = malloc(sizeof(int));
    *a = 1;
    mapInsert(map, "romane", 6, a);
    int* a_new = (int*)mapGet(map, "romane", 6);

    int* b = malloc(sizeof(int));
    *b = 2;
    mapInsert(map, "romanus", 7, b);
    int* b_new = (int*)mapGet(map, "romanus", 7);

    int* c = malloc(sizeof(int));
    *c = 3;
    mapInsert(map, "romulus", 7, c);
    int* c_new = (int*)mapGet(map, "romulus", 7);

    int* d = malloc(sizeof(int));
    *d = 4;
    mapInsert(map, "rubens", 6, d);
    int* d_new = (int*)mapGet(map, "rubens", 6);

    int* e = malloc(sizeof(int));
    *e = 5;
    mapInsert(map, "ruber", 5, e);
    int* e_new = (int*)mapGet(map, "ruber", 5);

    int* f = malloc(sizeof(int));
    *f = 6;
    mapInsert(map, "rubicon", 7, f);
    int* f_new = (int*)mapGet(map, "rubicon", 7);

    int* g = malloc(sizeof(int));
    *g = 7;
    mapInsert(map, "rubicundus", 10, g);
    int* g_new = (int*)mapGet(map, "rubicundus", 10);

    int* h = malloc(sizeof(int));
    *h = 8;
    mapInsert(map, "roman", 5, h);
    int* h_new = (int*)mapGet(map, "roman", 5);

    destroyMap(map);

    // Test JSON parser
    char* rawjson = "{}";
    JSONValue* value = parseJSON(rawjson);
    destroyJSON(value);

    rawjson = "{  \"location\" : \"Terre Haute\"}";
    value = parseJSON(rawjson);
    JSONValue* location = JSONGet(value, "location");
    destroyJSON(value);

    #endif

    sendRequest();

    while(true) {
        if (responseReady) {
            handleResponse();
        }

        // Handle button press
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
