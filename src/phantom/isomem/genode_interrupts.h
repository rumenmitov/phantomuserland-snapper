// Required by intrdisp.c (machine independent interrupts interface)
// Values taken from interrupts.h

#define MAX_IRQ_COUNT        32
#define SOFT_IRQ_NOT_PENDING 0x80000000
#define SOFT_IRQ_DISABLED    0x40000000
#define SOFT_IRQ_COUNT       32


#define STAT_CNT_INTERRUPT 47
#define STAT_CNT_SOFTINT   48
