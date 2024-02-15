/*! \file */
/*!
 * csLFXT.c
 *
 * Description: Basic driver for configuring LFXT clock source on MSP432P4111
 *               Launchpad.
 *
 *  Created on:
 *      Author:
 */


#include "msp.h"
#include "csLFXT.h"

void configLFXT(void)
{
    /* Configure LFXT to use 32kHz crystal, source to ACLK */

    /* See Table 6-84 in Section 6.12.22 of Data Sheet for summary of PJ.0 and
     *   PJ.1 functions.
     * Configure for LFXIN/LFXOUT crystal mode operation
     */
    PJ->SEL0 |= BIT0 | BIT1;            // Configure PJ.0/1 for LFXT function
    PJ->SEL1 &= ~(BIT0 | BIT1);

    /* See Section 6.3 CS Registers of Technical Reference Manual for summary of
     *   clock system peripheral registers.
     */
    CS->KEY = CS_KEY_VAL ;              // Unlock CS module for register access
    /* Enable LFXT oscillator */
    CS->CTL2 |= CS_CTL2_LFXT_EN;
    /* If fault triggered, clear flag and check again. If fault persist, will
     *   remain in this loop. */
    do          // Loop until LFXT fault flag is cleared
    {
       // Clear XT2,XT1,DCO fault flags
       CS->CLRIFG |= CS_CLRIFG_CLR_DCOR_OPNIFG | CS_CLRIFG_CLR_HFXTIFG |
               CS_CLRIFG_CLR_LFXTIFG | CS_CLRIFG_CLR_FCNTLFIFG;
       SYSCTL_A->NMI_CTLSTAT &= ~ SYSCTL_A_NMI_CTLSTAT_CS_SRC;

    } while ((SYSCTL_A->NMI_CTLSTAT & SYSCTL_A_NMI_CTLSTAT_CS_FLG)
            || (CS->IFG & CS_IFG_LFXTIFG)); // Test LFXT oscillator fault flag

    /* Select clock source for ACLK = LFXT, no divider */
    CS->CTL1 = (CS->CTL1 & ~(CS_CTL1_SELA_MASK | CS_CTL1_DIVA_MASK))
                            | CS_CTL1_SELA__LFXTCLK     // select source LFXTCLK
                            | CS_CTL1_DIVA__128;          // set divider /128

    CS->KEY = 0;                        // Lock CS module from unintended accesses

}
