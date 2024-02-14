/*!
 * sysTickDelays.h
 *      Description: Helper file for delay functions using SysTick timer. Must be
 *                   initialized with system clock frequency using initDelayTimer.
 *
 *      Author: ece230
 */

#ifndef SYSTICKDELAYS_H_
#define SYSTICKDELAYS_H_

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

#define UNDERFLOW       2
#define OVERFLOW        1
#define SUCCESS         0


/*!
 *
 * \brief This function initializes sysTick based delay module
 *
 * This function initializes sysTick based delay module to set clock frequency.
 *
 * \param clkFreq is the frequency of the system clock in Hz
 *
 * \return None
 */
extern void initDelayTimer(uint32_t clkFreq);

/*!
 * \brief This function delays for specified time
 *
 * This function delays for specified microseconds using sysTick.
 *
 * \param micros is the number of microseconds to delay
 *
 * \return 0 on success, 1 if microsecond count is too large,
 *              2 if microsecond count is too small
 */
extern int delayMicroSec(uint32_t micros);

/*!
 * \brief This function delays for specified time
 *
 * This function delays for specified milliseconds using sysTick.
 *
 * \param millis is the number of milliseconds to delay
 *
 * \return 0 on success, 1 if millisecond count is too large,
 *              2 if millisecond count is too small
 */
extern int delayMilliSec(uint32_t millis);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* SYSTICKDELAYS_H_ */
