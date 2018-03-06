/*****************************************************************************
* Product: Low-level initialization for AT91SAM7S-EK
* Date of the Last Update:  May 16, 2007
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
#include "bsp.h"

/*..........................................................................*/
/* low_level_init() is invoked by the startup sequence after initializing
* the C stack, but before initializing the segments in RAM.
*
* low_level_init() is invoked in the ARM state. The function gives the
* application a chance to perform early initializations of the hardware.
* This function cannot rely on initialization of any static variables,
* because these have not yet been initialized in RAM.
*/
void low_level_init(void (*reset_addr)(), void (*return_addr)()) {
    extern uint8_t __ram_start;
    static uint32_t const LDR_PC_PC = 0xE59FF000U;
    static uint32_t const MAGIC = 0xDEADBEEFU;
    AT91PS_PMC pPMC;

    /* Set flash wait sate FWS and FMCN */
    AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN) & ((MCK + 500000)/1000000 << 16))
                             | AT91C_MC_FWS_1FWS;

    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;  /* Disable the watchdog */

    /* Enable the Main Oscillator:
    * set OSCOUNT to 6, which gives Start up time = 8 * 6 / SCK = 1.4ms
    * (SCK = 32768Hz)
    */
    pPMC = AT91C_BASE_PMC;
    pPMC->PMC_MOR = ((6 << 8) & AT91C_CKGR_OSCOUNT) | AT91C_CKGR_MOSCEN;
    while ((pPMC->PMC_SR & AT91C_PMC_MOSCS) == 0) {/* Wait the startup time */
    }

    /* Set the PLL and Divider:
    * - div by 5 Fin = 3,6864 =(18,432 / 5)
    * - Mul 25+1: Fout = 95,8464 =(3,6864 *26)
    * for 96 MHz the error is 0.16%
    * Field out NOT USED = 0
    * PLLCOUNT pll startup time estimate at : 0.844 ms
    * PLLCOUNT 28 = 0.000844 /(1/32768)
    */
    pPMC->PMC_PLLR = ((AT91C_CKGR_DIV & 0x05)
                      | (AT91C_CKGR_PLLCOUNT & (28 << 8))
                      | (AT91C_CKGR_MUL & (25 << 16)));
    while ((pPMC->PMC_SR & AT91C_PMC_LOCK) == 0) { /* Wait the startup time */
    }
    while ((pPMC->PMC_SR & AT91C_PMC_MCKRDY) == 0) {
    }

    /* Select Master Clock and CPU Clock select the PLL clock / 2 */
    pPMC->PMC_MCKR =  AT91C_PMC_PRES_CLK_2;
    while ((pPMC->PMC_SR & AT91C_PMC_MCKRDY) == 0) {
    }

    pPMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK;
    while ((pPMC->PMC_SR & AT91C_PMC_MCKRDY) == 0) {
    }

    /* Setup the exception vectors to RAM
    *
    * NOTE: the exception vectors must be in RAM *before* the remap
    * in order to guarantee that the ARM core is provided with valid vectors
    * during the remap operation.
    */
    /* setup the primary vector table in RAM */
    *(uint32_t volatile *)(&__ram_start + 0x00) = LDR_PC_PC | 0x18;
    *(uint32_t volatile *)(&__ram_start + 0x04) = LDR_PC_PC | 0x18;
    *(uint32_t volatile *)(&__ram_start + 0x08) = LDR_PC_PC | 0x18;
    *(uint32_t volatile *)(&__ram_start + 0x0C) = LDR_PC_PC | 0x18;
    *(uint32_t volatile *)(&__ram_start + 0x10) = LDR_PC_PC | 0x18;
    *(uint32_t volatile *)(&__ram_start + 0x14) = MAGIC;
    *(uint32_t volatile *)(&__ram_start + 0x18) = LDR_PC_PC | 0x18;
    *(uint32_t volatile *)(&__ram_start + 0x1C) = LDR_PC_PC | 0x18;

    /* setup the secondary vector table in RAM */
    *(uint32_t volatile *)(&__ram_start + 0x20) = (uint32_t)reset_addr;
    *(uint32_t volatile *)(&__ram_start + 0x24) = 0x04U;
    *(uint32_t volatile *)(&__ram_start + 0x28) = 0x08U;
    *(uint32_t volatile *)(&__ram_start + 0x2C) = 0x0CU;
    *(uint32_t volatile *)(&__ram_start + 0x30) = 0x10U;
    *(uint32_t volatile *)(&__ram_start + 0x34) = 0x14U;
    *(uint32_t volatile *)(&__ram_start + 0x38) = 0x18U;
    *(uint32_t volatile *)(&__ram_start + 0x3C) = 0x1CU;

    /* check if the Memory Controller has been remapped already */
    if (MAGIC != (*(uint32_t volatile *)0x14)) {
        AT91C_BASE_MC->MC_RCR = 1;   /* perform Memory Controller remapping */
    }
}

