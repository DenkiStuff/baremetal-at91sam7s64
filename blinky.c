/*****************************************************************************
* Product: BLINKY example application
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

#include "bsp.h"                                   /* Board Support Package */
#include "isr.h"                      /* interface to the ISRs (foreground) */

/* "class" Blinky ..........................................................*/
typedef struct BlinkyTag Blinky;
struct BlinkyTag {
    uint8_t state;
    uint8_t id;
    uint32_t offDelay;
    uint32_t onDelay;
    uint32_t ctr;
};

enum BlinkyStates {
    BLINKY_OFF,
    BLINKY_ON
};

void Blinky_ctor(Blinky *me, uint8_t id, uint32_t offDelay, uint32_t onDelay);
void Blinky_dispatch(Blinky *me);

/*..........................................................................*/
void Blinky_ctor(Blinky *me, uint8_t id, uint32_t offDelay, uint32_t onDelay)
{
    me->id       = id;
    me->offDelay = offDelay;
    me->onDelay  = onDelay;

    me->state    = BLINKY_OFF;          /* initial transition to BLINKY_OFF */
    me->ctr      = me->offDelay;
    LED_OFF(me->id);
}
/*..........................................................................*/
void Blinky_dispatch(Blinky *me) {
    switch (me->state) {
        case BLINKY_OFF: {
           --me->ctr;
           if (me->ctr == 0) {
               me->state = BLINKY_ON;             /* transition to ON state */
               me->ctr = me->onDelay;
               LED_ON(me->id);
           }
           break;
        }
        case BLINKY_ON: {
           --me->ctr;
           if (me->ctr == 0) {
               me->state = BLINKY_OFF;           /* transition to OFF state */
               me->ctr = me->offDelay;
               LED_OFF(me->id);
           }
           break;
        }
    }
}

/* test harness ------------------------------------------------------------*/
static Blinky blinky_pit;      /* instance of "class" Blinky (.bss section) */
static Blinky blinky_timer0;   /* instance of "class" Blinky (.bss section) */
static Blinky blinky_timer1;   /* instance of "class" Blinky (.bss section) */
static Blinky blinky_idle;     /* instance of "class" Blinky (.bss section) */

static Blinky *pBlinky[] = {          /* pointers to Blinky (.data section) */
    &blinky_pit,
    &blinky_timer0,
    &blinky_timer1
};

int main (void) {
                                            /* explicit "constructor" calls */
    Blinky_ctor(&blinky_pit,    1,     9,     1);
    Blinky_ctor(&blinky_timer0, 2,  9000,  1000);
    Blinky_ctor(&blinky_timer1, 3,  9000,  1000);
    Blinky_ctor(&blinky_idle,   0, 18000,  2000);

    BSP_init();                     /* initialize the board support package */

    for (;;) {                                                  /* for-ever */
        if (eventFlagCheck(PIT_FLAG)) {
            Blinky_dispatch(pBlinky[PIT_FLAG]);
        }

        if (eventFlagCheck(TIMER0_FLAG)) {
            Blinky_dispatch(pBlinky[TIMER0_FLAG]);
        }

        if (eventFlagCheck(TIMER1_FLAG)) {
            Blinky_dispatch(pBlinky[TIMER1_FLAG]);
        }

        Blinky_dispatch(&blinky_idle);
    }

    return 0; /* unreachable; this return is only to avoid compiler warning */
}
