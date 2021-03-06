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
#include "bsp.h"
#include "isr.h"

/* local event flags shared between foreground (ISRs) and the background ...*/
static uint32_t volatile l_eventFlags; /* uninitialized data (.bss section) */

/*..........................................................................*/
void eventFlagSet(uint8_t flag) {
    ARM_INT_KEY_TYPE int_lock_key;
    ARM_INT_LOCK(int_lock_key);
    l_eventFlags |= (1 << flag);
    ARM_INT_UNLOCK(int_lock_key);
}
/*..........................................................................*/
uint8_t eventFlagCheck(uint8_t flag) {
    register uint8_t isSet;
    ARM_INT_KEY_TYPE int_lock_key;
    ARM_INT_LOCK(int_lock_key);
    isSet = ((l_eventFlags & (1 << flag)) != 0);
    if (isSet) {
        l_eventFlags &= ~(1 << flag);                     /* clear the flag */
    }
    ARM_INT_UNLOCK(int_lock_key);
    return isSet;
}
/*..........................................................................*/
void ISR_pit(void) {               /* Programmable Interval Timer (PIT) ISR */
    uint32_t volatile dummy = AT91C_BASE_PITC->PITC_PIVR;/*clear int source */
    eventFlagSet(PIT_FLAG);                       /* set the PIT event flag */

    (void)dummy;         /* suppress warning "dummy" was set but never used */
}
/*..........................................................................*/
void ISR_timer0(void) {                                       /* Timer0 ISR */
    uint32_t volatile dummy = AT91C_BASE_TC0->TC_SR;    /* clear int soruce */
    eventFlagSet(TIMER0_FLAG);                 /* set the TIMER0 event flag */

    (void)dummy;         /* suppress warning "dummy" was set but never used */
}
/*..........................................................................*/
void ISR_spur(void) {                                       /* spurious ISR */
}

/*..........................................................................*/
__attribute__ ((section (".text.fastcode")))
void BSP_irq(void) {
    typedef void (*IntVector)(void);
    IntVector vect = (IntVector)AT91C_BASE_AIC->AIC_IVR;    /* read the IVR */
                                      /* write IVR if AIC in protected mode */
    AT91C_BASE_AIC->AIC_IVR = (AT91_REG)vect;

    asm("MSR cpsr_c,#(0x1F)");                  /* allow nesting interrupts */
    (*vect)();          /* call the IRQ handler via the pointer to function */
    asm("MSR cpsr_c,#(0x1F | 0x80)");             /* lock IRQ before return */

                                      /* write AIC_EOICR to clear interrupt */
    AT91C_BASE_AIC->AIC_EOICR = (AT91_REG)vect;
}
/*..........................................................................*/
__attribute__ ((section (".text.fastcode")))
void BSP_fiq(void) {                                              /* FIQ ISR*/
    /* Handle the FIQ directly. No AIC vectoring overhead necessary */
    uint32_t volatile dummy = AT91C_BASE_TC1->TC_SR;    /* clear int soruce */
    eventFlagSet(TIMER1_FLAG);            /* for example, set an event flag */

    (void)dummy;         /* suppress warning "dummy" was set but never used */
}
