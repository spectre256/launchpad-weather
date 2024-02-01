/*! \file */
/*!
 * csHFXT.h
 *
 * Description: Basic driver for configuring HFXT clock source on MSP432P4111
 *               Launchpad. Assumes LED1 configured for output.
 *
 *  Created on: 12/18/2023
 *      Author: Conner Tavares
 */

#ifndef CSHFXT_H_
#define CSHFXT_H_

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include "msp.h"

/*!
 * \brief This function configures HFXT as clock source for MCLK and SMCLK
 *
 * This function configures PJ.2 and PJ.3 for external oscillator and configures
 *  HFXT as clock source for MCLK and SMCLK with frequency set to 48MHz.
 *
 * Modified bits 2 and 3 of \b PJSEL register. Modified CS, PCM, and FLCTL
 *  peripheral registers.
 *
 * \return None
 */
extern void configHFXT(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif /* CSHFXT_H_ */
