/*! \file */
/*!
 * csLFXT.h
 *
 * Description: Basic driver for configuring LFXT clock source on MSP432P4111
 *               Launchpad.
 *
 *  Created on:
 *      Author:
 */

#ifndef CSLFXT_H_
#define CSLFXT_H_

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

/*!
 * \brief This function configures LFXT as clock source for ACLK
 *
 * This function configures PJ.0 and PJ.1 for external oscillator and configures
 *  LFXT as clock source for ACLK with frequency set to 32kHz.
 *
 * Modified bits 0 and 1 of \b PJSEL register. Modified CS peripheral registers.
 *
 * \return None
 */
extern void configLFXT(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif /* CSLFXT_H_ */
