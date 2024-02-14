/*!
 * sysTickDelays.c
 *      Description: Helper file for delay functions using SysTick timer. Must be
 *                   initialized with system clock frequency using initDelayTimer.
 *
 *      Author: ece230
 */

#include <msp.h>
#include "sysTickDelays.h"

#define USEC_DIVISOR    1000000
#define MSEC_DIVISOR    1000
#define SYSTICK_LIMIT   0x00FFFFFF

/* Holds frequency of system clock, must be set in initDelayTimer */
uint64_t sysClkFreq = 0;

void initDelayTimer(uint32_t clkFreq) {
    // store value of system clock (MCLK) frequency
    //   used for tick count calculations
    sysClkFreq = clkFreq;
}

int delayMicroSec(uint32_t micros) {
    // calculate timer ticks needed for \b micros microseconds
    uint64_t ticks = sysClkFreq * micros / USEC_DIVISOR;
    // if requested delay is too short or exceeds max period of SysTick timer,
    //  return error state
    if (ticks < 2) {
        return UNDERFLOW;
    }
    if (ticks > SYSTICK_LIMIT) {
        return OVERFLOW;
    }

    // Set the period of the SysTick counter
    SysTick->LOAD = ticks - 1;
    // Write any value to reset timer counter
    SysTick->VAL = 1;
    // Enable SysTick timer
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    // Wait for SysTick COUNT flag
    while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    // Disable SysTick timer
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    return SUCCESS;
}

int delayMilliSec(uint32_t millis) {
    return delayMicroSec(1000 * millis);
}
