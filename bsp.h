/*****************************************************************************
* Product: GNU toolset for ARM, BSP for AT91SAM7S-EK
* Date of the Last Update:  Jun 29, 2007
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
*
* Contact information:
* Quantum Leaps Web site:  http://www.quantum-leaps.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#ifndef bsp_h
#define bsp_h

#include <at91sam7s64.h>                        /* AT91SAMT7S64 definitions */

#include <stdint.h>    /* Exact-width integer types. WG14/N843 C99 Standard */

                                         /* External Oscillator MAINCK [Hz] */
#define MAINCK               18432000

                                     /* Maseter Clock (PLLRC div by 2) [Hz] */
#define MCK                  47923200

                                             /* System clock tick rate [Hz] */
#define BSP_TICKS_PER_SEC    100

void BSP_init(void);         /* initialization of the board support package */

                                                        /* user LED control */
#define LED_ON(num_)       (AT91C_BASE_PIOA->PIO_CODR = BSP_led[num_])
#define LED_OFF(num_)      (AT91C_BASE_PIOA->PIO_SODR = BSP_led[num_])

extern uint32_t const BSP_led[];         /* constant data (.rodata section) */


#endif                                                             /* bsp_h */

