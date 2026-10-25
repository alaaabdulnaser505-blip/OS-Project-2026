// The local APIC manages internal (non-I/O) interrupts.
// See Chapter 8 & Appendix C of Intel processor manual volume 3.

#include <inc/types.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <inc/mmu.h>
#include <inc/stdio.h>
#include <inc/x86.h>
#include <kern/mem/memory_manager.h>
#include <kern/cpu/cpu.h>
#include <kern/cpu/picirq.h>
#include <kern/cpu/lapic.h>
#include <kern/cpu/sched.h>

uint32 cpu_init_cnt[NCPUS];
uint32 cpu_last_cnt[NCPUS];

static void
lapicw(int index, int value)
{
	lapic[index] = value;
	lapic[LAPICID];  // wait for write to finish, by reading
}

static uint32
lapicr(int index)
{
	return lapic[index];
}

void
lapic_init(void)
{
	if (!lapicaddr)
		return;

	// lapicaddr is the physical address of the LAPIC's 4K MMIO
	// region.  Map it in to virtual memory so we can access it.
	lapic = mmio_map_region(lapicaddr, 4096);

	// Enable local APIC; set spurious interrupt vector.
	lapicw(SVR, ENABLE | (IRQ_OFFSET + IRQ_SPURIOUS));

	//start the timer
	if (quantums)
	{
		cpuclk_set_counter(quantums[0]);
		cpuclk_start();
	}
	else
	{
		panic("quantums array is not created! quantums[0] must be set");
	}
	// Leave LINT0 of the BSP enabled so that it can get
	// interrupts from the 8259A chip.
	//
	// According to Intel MP Specification, the BIOS should initialize
	// BSP's local APIC in Virtual Wire Mode, in which 8259A's
	// INTR is virtually connected to BSP's LINTIN0. In this mode,
	// we do not need to program the IOAPIC.
	if (mycpu() != bootcpu)
		lapicw(LINT0, MASKED);

	// Disable NMI (LINT1) on all CPUs
	lapicw(LINT1, MASKED);

	// Disable performance counter overflow interrupts
	// on machines that provide that interrupt entry.
	if (((lapic[VER]>>16) & 0xFF) >= 4)
		lapicw(PCINT, MASKED);

	// Map error interrupt to IRQ_ERROR.
	lapicw(ERROR, IRQ_OFFSET + IRQ_ERROR);

	// Clear error status register (requires back-to-back writes).
	lapicw(ESR, 0);
	lapicw(ESR, 0);

	// Ack any outstanding interrupts.
	lapicw(EOI, 0);

	// Send an Init Level De-Assert to synchronize arbitration ID's.
	lapicw(ICRHI, 0);
	lapicw(ICRLO, BCAST | INIT | LEVEL);
	while(lapic[ICRLO] & DELIVS)
		;

	// Enable interrupts on the APIC (but not on the processor).
	lapicw(TPR, 0);
}

int cpunum(void)
{
	if (lapic)
		return lapic[LAPICID] >> 24;
	return 0;
}

void cpuclk_set_counter(uint8 quantum_in_ms)
{
	if (quantum_in_ms > 0 && (uint32)quantum_in_ms < 1<<(sizeof(uint8)*8))
	{
		//TODO: NEED ACCURATE WAY TO MEASURE THE COUNTER FROM THE QUANTUM! 100000 IS JUST A ROUGH VALUE
		//REFER TO https://wiki.osdev.org/APIC_Timer#Example_code_in_C FOR EXAMPLE CODE IN C
		int counter = quantum_in_ms * 100000;
		cpu_init_cnt[cpunum()] = counter;
		cpu_last_cnt[cpunum()] = counter;
		//lapicw(TIMER, PERIODIC | (IRQ_OFFSET + IRQ_TIMER));
		lapicw(TIMER, MASKED);
		lapicw(TICR, counter);
	}
	else
	{
		panic("attempt to set the CPU quantum by too large value. Quantum should be between 1 ms and %d ms", QUANTUM_LIMIT - 1);
	}
}
void cpuclk_start()
{
	// The timer repeatedly counts down at bus frequency
	// from lapic[TICR] and then issues an interrupt.
	// If we cared more about precise timekeeping,
	// TICR would be calibrated using an external time source.
	//lapicw(TDCR, X1);
	lapicw(TDCR, X2);
	//lapicw(TICR, 10000000);
	lapicw(TIMER, PERIODIC | (IRQ_OFFSET + IRQ_TIMER));
	lapicw(TICR, cpu_init_cnt[cpunum()]);
}

void cpuclk_pause()
{
	//Save current counter value
	cpu_last_cnt[cpunum()] = lapicr(TCCR);
	if (cpu_last_cnt[cpunum()] == 0)
		cpu_last_cnt[cpunum()] = cpu_init_cnt[cpunum()];
	//Stop the counter
	lapicw(TIMER, MASKED);
	//lapicw(TICR, 0);
}

void cpuclk_resume()
{
	lapicw(TIMER, PERIODIC | (IRQ_OFFSET + IRQ_TIMER));
	lapicw(TICR, cpu_last_cnt[cpunum()]);
}

void cpuclk_stop()
{
	lapicw(TIMER, MASKED);
	lapicw(TICR, 0);
}

void cpuclk_restart()
{
	lapicw(TIMER, PERIODIC | (IRQ_OFFSET + IRQ_TIMER));
	lapicw(TICR, cpu_init_cnt[cpunum()]);
}
// Acknowledge interrupt.
void
lapic_eoi(void)
{
	if (lapic)
		lapicw(EOI, 0);
}

// Spin for a given number of microseconds.
// On real hardware would want to tune this dynamically.
int __s__md__=0;
static void
microdelay(int us)
{
	__s__md__ = 0;
	for (int i = 0; i < us*1000; ++i) {
		__s__md__+=1;
	}
}

// Start additional processor running entry code at addr.
void
lapic_startap(uint8 apicid, uint32 addr)
{
	//cprintf("lapic_startap: 1 - apicid = %d, addr = %x\n", apicid, addr);
	int i;
	uint16 *wrv;

	// "The BSP must initialize CMOS shutdown code to 0AH
	// and the warm reset vector (DWORD based at 40:67) to point at
	// the AP startup code prior to the [universal startup algorithm]."
	outb(IO_RTC, 0xF);  // offset 0xF is shutdown code
	outb(IO_RTC+1, 0x0A);

	wrv = (uint16 *)STATIC_KERNEL_VIRTUAL_ADDRESS((0x40 << 4 | 0x67));  // Warm reset vector
	wrv[0] = 0;
	wrv[1] = addr >> 4;

	// "Universal startup algorithm."
	// Send INIT (level-triggered) interrupt to reset other CPU.
	lapicw(ICRHI, apicid << 24);
	lapicw(ICRLO, INIT | LEVEL | ASSERT);
	microdelay(200);
	lapicw(ICRLO, INIT | LEVEL);
	microdelay(100);    // should be 10ms, but too slow in Bochs!

	// Send startup IPI (twice!) to enter code.
	// Regular hardware is supposed to only accept a STARTUP
	// when it is in the halted state due to an INIT.  So the second
	// should be ignored, but it is part of the official Intel algorithm.
	// TODO: {IMP} Bochs complains about the second one.  Too bad for Bochs.
	for (i = 0; i < 1/*2*/; i++) {
		lapicw(ICRHI, apicid << 24);
		lapicw(ICRLO, STARTUP | (addr >> 12));
		microdelay(200);
	}
}

void
lapic_ipi(int vector)
{
	lapicw(ICRLO, OTHERS | FIXED | vector);
	while (lapic[ICRLO] & DELIVS)
		;
}
