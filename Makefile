##############################################################################
# Product: BLINKY/C example application
# Date of the Last Update:  Jun 14, 2007
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
#
# Contact information:
# Quantum Leaps Web site:  http://www.quantum-leaps.com
# e-mail:                  info@quantum-leaps.com
##############################################################################
# general utilities
RM = erase

# GNU-ARM tools directory
GNU_ARM = C:\tools\CodeSourcery\arm-eabi

# The gcc compiler and linker
CC      = $(GNU_ARM)\bin\arm-none-eabi-gcc
ASM     = $(GNU_ARM)\bin\arm-none-eabi-as
LINK    = $(GNU_ARM)\bin\arm-none-eabi-gcc
OBJCOPY = $(GNU_ARM)\bin\arm-none-eabi-objcopy

BLDDIR  = .

CCINC   = -I$(BLDDIR)

APP_DEP = $(BLDDIR)\bsp.h \
	$(BLDDIR)\arm_exc.h \
	$(BLDDIR)\isr.h

APP_NAME = blinky
ARM_CPU  = arm7tdmi


# dbg (default target) .......................................................

BINDIR   = dbg

# gcc options for ARM and THUMB
# -c    : compile only
# -g    : source level debugging
CCFLAGS = -gdwarf-2 -c -mcpu=$(ARM_CPU) -mthumb-interwork -O \
	-mlong-calls -ffunction-sections -Wall -o$@

ASMFLAGS = -gdwarf2 -mcpu=$(ARM_CPU) -mthumb-interwork -o$@

LINKFLAGS = -T .\$(APP_NAME).ld -o $(BINDIR)\$(APP_NAME).elf \
	-Wl,-Map,$(BINDIR)\$(APP_NAME).map,--cref -lm


# release ....................................................................
ifeq ($(MAKECMDGOALS), rel)

BINDIR   = rel

# gcc options for ARM and THUMB
# -c    : compile only
# -g    : source level debugging
CCFLAGS = -c -mcpu=$(ARM_CPU) -mthumb-interwork -O2 \
	-mlong-calls -ffunction-sections -Wall -DNDBEBUG -o$@

ASMFLAGS = -gdwarf2 -mcpu=$(ARM_CPU) -mthumb-interwork -o$@

LINKFLAGS = -T .\$(APP_NAME).ld -o $(BINDIR)\$(APP_NAME).elf \
	-Wl,-Map,$(BINDIR)\$(APP_NAME).map,--cref -lm

endif
ifeq ($(MAKECMDGOALS), rel_clean)

BINDIR   = rel

endif

#.............................................................................

all: $(BINDIR)\$(APP_NAME).bin

$(BINDIR)\$(APP_NAME).bin : $(BINDIR)\$(APP_NAME).elf
	$(OBJCOPY) -O binary $(BINDIR)\$(APP_NAME).elf $(BINDIR)\$(APP_NAME).bin

$(BINDIR)\$(APP_NAME).elf : \
	.\$(APP_NAME).ld \
	$(BINDIR)\startup.o \
	$(BINDIR)\arm_exc.o \
	$(BINDIR)\low_level_init.o \
	$(BINDIR)\bsp.o \
	$(BINDIR)\isr.o \
	$(BINDIR)\blinky.o
	$(LINK) \
	$(BINDIR)\startup.o \
	$(BINDIR)\arm_exc.o \
	$(BINDIR)\low_level_init.o \
	$(BINDIR)\bsp.o \
	$(BINDIR)\isr.o \
	$(BINDIR)\blinky.o \
	$(LINKFLAGS)


$(BINDIR)\startup.o: $(BLDDIR)\startup.s
	$(ASM) $(ASMFLAGS) $<

$(BINDIR)\arm_exc.o: $(BLDDIR)\arm_exc.s
	$(ASM) $(ASMFLAGS) $<

$(BINDIR)\low_level_init.o: $(BLDDIR)\low_level_init.c $(APP_DEP)
	$(CC) -marm $(CCFLAGS) $(CCINC) $<

$(BINDIR)\bsp.o: $(BLDDIR)\bsp.c $(APP_DEP)
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<

$(BINDIR)\isr.o: $(BLDDIR)\isr.c $(APP_DEP)
	$(CC) -marm $(CCFLAGS) $(CCINC) $<

$(BINDIR)\blinky.o: $(BLDDIR)\blinky.c $(APP_DEP)
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<


dbg: all

rel: all


# clean targets...

.PHONY: clean rel_clean

rel_clean: clean

dbg_clean: clean

clean:
	-$(RM) $(BINDIR)\*.o
	-$(RM) $(BINDIR)\*.elf
	-$(RM) $(BINDIR)\*.map
