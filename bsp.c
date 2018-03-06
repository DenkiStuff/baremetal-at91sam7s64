/*****************************************************************************
* Product: GNU toolset for ARM, BSP for AT91SAM7S-EK
* Date of the Last Update:  Jun 30, 2007
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
#include "bsp.h"                                   /* Board Support Package */
#include "isr.h"                      /* interface to the ISRs (foreground) */

/* NOTE: uncomment the macro MANUAL_TEST to perform manual testing
*/
#define MANUAL_TEST


/* BSP objects -------------------------------------------------------------*/
uint32_t const BSP_led[] = {             /* constant data (.rodata section) */
    (1 << 0),                                     /* LED D1 on AT91SAM7S-EK */
    (1 << 1),                                     /* LED D2 on AT91SAM7S-EK */
    (1 << 2),                                     /* LED D3 on AT91SAM7S-EK */
    (1 << 3)                                      /* LED D4 on AT91SAM7S-EK */
};
                                               /* Timer0,1 clock selections */
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4
                                                          /* IRQ priorities */
#define ISR_PIT_PRIO     (AT91C_AIC_PRIOR_LOWEST + 0)
#define ISR_TIMER0_PRIO  (AT91C_AIC_PRIOR_LOWEST + 1)

/*..........................................................................*/
static void AT91F_TC_open(AT91S_TC *me, uint32_t mode, uint32_t timerId) {
    uint32_t volatile dummy;

    AT91C_BASE_PMC->PMC_PCER = (1 << timerId); /* enable the clock of TIMER */
    me->TC_CCR = AT91C_TC_CLKDIS;   /* disable the clock and the interrupts */
    me->TC_IDR = ~0U;
    dummy = me->TC_SR;                                  /* clear status bit */
    (void)dummy;         /* suppress warning "dummy" was set but never used */
    me->TC_CMR = mode;                 /* set the Mode of the Timer Counter */
    me->TC_CCR = AT91C_TC_CLKEN;                        /* enable the clock */
}
/*..........................................................................*/
void BSP_init(void) {
    uint32_t i;

                                                  /* initialize the LEDs... */
    for (i = 0; i < sizeof(BSP_led)/sizeof(BSP_led[0]); ++i) {
        AT91C_BASE_PIOA->PIO_PER = BSP_led[i];                /* enable pin */
        AT91C_BASE_PIOA->PIO_OER = BSP_led[i];   /* configure as output pin */
        LED_OFF(i);                                   /* extinguish the LED */
    }

                                             /* hook the exception handlers */
    *(uint32_t volatile *)0x24 = (uint32_t)&ARM_undef;
    *(uint32_t volatile *)0x28 = (uint32_t)&ARM_swi;
    *(uint32_t volatile *)0x2C = (uint32_t)&ARM_pAbort;
    *(uint32_t volatile *)0x30 = (uint32_t)&ARM_dAbort;
    *(uint32_t volatile *)0x34 = (uint32_t)&ARM_reserved;
    *(uint32_t volatile *)0x38 = (uint32_t)&ARM_irq;
    *(uint32_t volatile *)0x3C = (uint32_t)&ARM_fiq;

                /* configure Advanced Interrupt Controller (AIC) of AT91... */
    AT91C_BASE_AIC->AIC_IDCR = ~0;                /* disable all interrupts */
    AT91C_BASE_AIC->AIC_ICCR = ~0;                  /* clear all interrupts */
    for (i = 0; i < 8; ++i) {
        AT91C_BASE_AIC->AIC_EOICR = 0;           /* write AIC_EOICR 8 times */
    }

    AT91C_BASE_AIC->AIC_SPU  = (AT91_REG)&ISR_spur;         /* spurious ISR */

    /* configure the PIT interrupt */
    i = (MCK / 16 / BSP_TICKS_PER_SEC) - 1;
    AT91C_BASE_PITC->PITC_PIMR = (AT91C_PITC_PITEN | AT91C_PITC_PITIEN | i);

    AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (AT91_REG)&ISR_pit;  /* PIT ISR */
    AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] =
        (AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | ISR_PIT_PRIO);
    AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_SYS);
    AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_SYS);

    /* configure Timer0 interrupt as IRQ */
    AT91F_TC_open(AT91C_BASE_TC0,
                  (AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO | TC_CLKS_MCK2),
                  AT91C_ID_TC0);
    AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_TC0);
    AT91C_BASE_AIC->AIC_SVR[AT91C_ID_TC0] = (AT91_REG)&ISR_timer0;
    AT91C_BASE_AIC->AIC_SMR[AT91C_ID_TC0] =
        (AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | ISR_TIMER0_PRIO);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;              /*  IRQ enable CPC */
    AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_TC0);
    AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_TC0);

    /* configure Timer1 interrupt as FIQ */
    AT91F_TC_open(AT91C_BASE_TC1,
                  (AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO | TC_CLKS_MCK2),
                  AT91C_ID_TC1);
    AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_TC1);
    AT91C_BASE_AIC->AIC_SMR[AT91C_ID_TC1] =
        (AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 0);  /* ignore priority for FIQ */
    AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_TC1);
    AT91C_BASE_TC1->TC_IER = AT91C_TC_CPCS;                  /*  enable CPC */
    AT91C_BASE_AIC->AIC_FFER = (1 << AT91C_ID_TC1);   /* Fast Forcing (FIQ) */
    AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_TC1);

#ifndef MANUAL_TEST
    AT91C_BASE_TC0->TC_RC = 1000;                 /* Timer0 reset compare C */
    AT91C_BASE_TC1->TC_RC = 1000 - 1;             /* Timer1 reset compare C */

    AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;                /* start Timer0 */
    AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG;                /* start Timer1 */
#else
    AT91C_BASE_TC0->TC_RC = 1;    /* Timer0 reset compare C (just one tick) */
    AT91C_BASE_TC1->TC_RC = 1;    /* Timer1 reset compare C (just one tick) */
#endif

    ARM_INT_UNLOCK(0x1F);           /* unlock IRQ/FIQ at the ARM core level */
}
/*..........................................................................*/
void BSP_abort(char const *msg) {
    /* this function is called when an exception occurs.
    * For production code you need to log the message and go to fail-safe
    * state. You might also want to reset the CPU.
    */
    for (;;) {
    }
}
