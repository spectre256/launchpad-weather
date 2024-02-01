/*! \file */
/*!
 * csHFXT.c
 *
 * Description: Basic driver for configuring HFXT clock source on MSP432P4111
 *               Launchpad. Assumes LED1 configured for output.
 *
 *  Created on: 12/18/2023
 *      Author: Conner Tavares
 */

#include "msp.h"
#include "csHFXT.h"

void error(void);

void configHFXT(void)
{
    /*Set Vcore = 1, and flash wait state = 3 for operation up to 48MHz
     * Refer to device datasheet for execution frequency Vs flash wait-state
     * and for minimum supply voltage at Vcore = 1
     */

    /* Step 1: Transition to VCORE Level 1: AM_LDO_VCORE0 --> AM_LDO_VCORE1
     *
     * See <u>Chapter 8 Power Control Manager</u> in Technical Reference Manual
     *   for details of power modes. Per Section 8.4.1:
     *     For AM_LDO_VCORE0 and AM_DCDC_VCORE0 modes, the maximum CPU operating
     *     frequency is 24 MHz, and the maximum input clock frequency for
     *     peripherals is 12 MHz. For AM_LDO_VCORE1 and AM_DCDC_VCORE1 modes,
     *     the maximum CPU operating frequency is 48 MHz, and the maximum input
     *     clock frequency for peripherals is 24 MHz
     *
     * See Section 8.26 of Technical Reference Manual for details on PCM registers
     */
    PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
    while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
    //check errors
    if (PCM->IFG & PCM_IFG_AM_INVALID_TR_IFG)
        error();                        // Error if transition was not successful
    if ((PCM->CTL0 & PCM_CTL0_CPM_MASK) != PCM_CTL0_CPM_1)
        error();                        // Error if device is not in AM_LDO_VCORE1 mode

    /* Step 2: Configure Flash wait-state to 3 for both banks 0 & 1
     *
     * Supports frequencies up to 24 MHz for VCORE0 and 48MHz for VCORE1
     * See table in <u>Section 5.8 Operating Mode Execution Frequency vs
     *   Flash Wait-State Requirements</u> of Data Sheet for summary
     * See Section 9.4 of Technical Reference Manual for details on FLCTL registers
     */
    FLCTL_A->BANK0_RDCTL = (FLCTL_A->BANK0_RDCTL & ~(FLCTL_A_BANK0_RDCTL_WAIT_MASK)) |
            FLCTL_A_BANK0_RDCTL_WAIT_3;
    FLCTL_A->BANK1_RDCTL  = (FLCTL_A->BANK0_RDCTL & ~(FLCTL_A_BANK1_RDCTL_WAIT_MASK)) |
            FLCTL_A_BANK1_RDCTL_WAIT_3;

    /* Step 3: Configure HFXT to use 48MHz crystal, source to MCLK & HSMCLK */


    /* See Table 6-85 in Section 6.12.23 of Data Sheet for summary of PJ.2 and
     *   PJ.3 functions.
     * Configure for HFXIN/HFXOUT crystal mode operation
     */
    PJ->SEL0 |= BIT2 | BIT3;            // Configure PJ.2/3 for HFXT function
    PJ->SEL1 &= ~(BIT2 | BIT3);

    /* See Section 6.3 CS Registers of Technical Reference Manual for summary of
     *   clock system peripheral registers.
     */
    CS->KEY = CS_KEY_VAL ;              // Unlock CS module for register access
    /* Enable HFXT oscillator, set frequency to 48 MHz, and set HFXT drive selection */
    CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
    /* If fault triggered, clear flag and check again. If fault persist, will
     *   remain in this loop. */
    while(CS->IFG & CS_IFG_HFXTIFG)
        CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;

    /* Select clock source for MCLK & HSMCLK = HFXT, no divider */
    CS->CTL1 = (CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK
                | CS_CTL1_SELS_MASK | CS_CTL1_DIVHS_MASK))      // clear fields
                | CS_CTL1_SELM__HFXTCLK         // select MCLK source HFXTCLK
                | CS_CTL1_SELS__HFXTCLK         // select SMCLK source HFXTCLK
                | CS_CTL1_DIVM__1               // set MCLK divider /1
                | CS_CTL1_DIVS__1;              // set SMCLK divider /1

    CS->KEY = 0;                        // Lock CS module from unintended accesses

}

void error(void)
{
    //leave on LED to indicate error
    P1->OUT |= BIT0;
    while (1);
}
