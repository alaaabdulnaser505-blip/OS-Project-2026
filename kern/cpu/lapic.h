/* See COPYRIGHT for copyright information. */

#ifndef FOS_KERN_LAPIC_H
#define FOS_KERN_LAPIC_H
#ifndef FOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

// Local APIC registers, divided by 4 for use as uint32_t[] indices.
#define LAPICID   (0x0020/4)   // LAPIC ID
#define VER     (0x0030/4)   // Version
#define TPR     (0x0080/4)   // Task Priority
#define EOI     (0x00B0/4)   // EOI
#define SVR     (0x00F0/4)   // Spurious Interrupt Vector
	#define ENABLE     0x00000100   // Unit Enable
#define ESR     (0x0280/4)   // Error Status
#define ICRLO   (0x0300/4)   // Interrupt Command
	#define INIT       0x00000500   // INIT/RESET
	#define STARTUP    0x00000600   // Startup IPI
	#define DELIVS     0x00001000   // Delivery status
	#define ASSERT     0x00004000   // Assert interrupt (vs deassert)
	#define DEASSERT   0x00000000
	#define LEVEL      0x00008000   // Level triggered
	#define BCAST      0x00080000   // Send to all APICs, including self.
	#define OTHERS     0x000C0000   // Send to all APICs, excluding self.
	#define BUSY       0x00001000
	#define FIXED      0x00000000
#define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
#define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
	#define X1         0x0000000B   // divide counts by 1
	#define X2         0x00000000   // divide counts by 2
	#define X4         0x00000001   // divide counts by 4
	#define PERIODIC   0x00020000   // Periodic
#define PCINT   (0x0340/4)   // Performance Counter LVT
#define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
#define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
#define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
	#define MASKED     0x00010000   // Interrupt masked
#define TICR    (0x0380/4)   // Timer Initial Count
#define TCCR    (0x0390/4)   // Timer Current Count
#define TDCR    (0x03E0/4)   // Timer Divide Configuration

/*
 * Bits 0-3	Divide Configuration Register Value
	0000	2
	0001	4
	0010	8
	0011	16
	1000	32
	1001	64
	1010	128
	1011	1
 */
uint32 lapicaddr;        //Physical MMIO address of the local APIC. Initialized in mpconfig.c
volatile uint32 *lapic;

void lapic_init(void);
void lapic_startap(uint8 apicid, uint32 addr);
void lapic_eoi(void);
void lapic_ipi(int vector);
void cpuclk_set_counter(uint8 counter);
void cpuclk_start();
void cpuclk_pause();
void cpuclk_resume();
void cpuclk_stop();
void cpuclk_restart();

#endif // FOS_KERN_LAPIC_H
