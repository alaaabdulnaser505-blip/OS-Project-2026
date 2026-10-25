// Search for and parse the multiprocessor configuration table
// See http://developer.intel.com/design/pentium/datashts/24201606.pdf

#include <inc/types.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/x86.h>
#include <inc/mmu.h>
#include <inc/environment_definitions.h>
#include <kern/cpu/cpu.h>
#include <kern/cpu/lapic.h>
#include <kern/mem/memory_manager.h>

//struct CpuInfo cpus[NCPUS];
//struct CpuInfo *bootcpu;
int ismp;
//int ncpu;

// Per-CPU kernel stacks
//unsigned char percpu_kstacks[NCPUS][KERN_STACK_SIZE]
//__attribute__ ((aligned(PAGE_SIZE)));


// See MultiProcessor Specification Version 1.[14]

struct mp {             // floating pointer [MP 4.1]
	uint8 signature[4];           // "_MP_"
	uint32 uint32;            // phys addr of MP config table
	uint8 length;                 // 1
	uint8 specrev;                // [14]
	uint8 checksum;               // all bytes must add up to 0
	uint8 type;                   // MP system config type
	uint8 imcrp;
	uint8 reserved[3];
} __attribute__((__packed__));

struct mpconf {         // configuration table header [MP 4.2]
	uint8 signature[4];           // "PCMP"
	uint16 length;                // total table length
	uint8 version;                // [14]
	uint8 checksum;               // all bytes must add up to 0
	uint8 product[20];            // product id
	uint32 oemtable;            // OEM table pointer
	uint16 oemlength;             // OEM table length
	uint16 entry;                 // entry count
	uint32 lapicaddr;           // address of local APIC
	uint16 xlength;               // extended table length
	uint8 xchecksum;              // extended table checksum
	uint8 reserved;
	uint8 entries[0];             // table entries
} __attribute__((__packed__));

struct mpproc {         // processor table entry [MP 4.3.1]
	uint8 type;                   // entry type (0)
	uint8 apicid;                 // local APIC id
	uint8 version;                // local APIC version
	uint8 flags;                  // CPU flags
	uint8 signature[4];           // CPU signature
	uint32 feature;               // feature flags from CPUID instruction
	uint8 reserved[8];
} __attribute__((__packed__));

// mpproc flags
#define MPPROC_BOOT 0x02                // This mpproc is the bootstrap processor

// Table entry types
#define MPPROC    0x00  // One per processor
#define MPBUS     0x01  // One per bus
#define MPIOAPIC  0x02  // One per I/O APIC
#define MPIOINTR  0x03  // One per bus interrupt source
#define MPLINTR   0x04  // One per system interrupt source

static uint8
sum(void *addr, int len)
{
	int i, sum;

	sum = 0;
	for (i = 0; i < len; i++)
		sum += ((uint8 *)addr)[i];
	return sum;
}

// Look for an MP structure in the len bytes at physical address addr.
static struct mp *
mpsearch1(uint32 a, int len)
{
	struct mp *mp = STATIC_KERNEL_VIRTUAL_ADDRESS(a), *end = STATIC_KERNEL_VIRTUAL_ADDRESS(a + len);
	cprintf("start va = %x, end va = %x\n", mp, end);
	for (; mp < end; mp++)
	{
		//cprintf("signature of mp %x = %s\n", mp, mp->signature);
		if (memcmp(mp->signature, "_MP_", 4) == 0)
		{
			cprintf("mpsearch1: signature is found\n");
			if (sum(mp, sizeof(*mp)) == 0)
				return mp;
		}
	}
	return NULL;
}

// Search for the MP Floating Pointer Structure, which according to
// [MP 4] is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) if there is no EBDA, in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
static struct mp *
mpsearch(void)
{
	uint8 *bda;
	uint32 p;
	struct mp *mp;

	static_assert(sizeof(*mp) == 16);

	// The BIOS data area lives in 16-bit segment 0x40.
	bda = (uint8 *) STATIC_KERNEL_VIRTUAL_ADDRESS(0x40 << 4);

	// [MP 4] The 16-bit segment of the EBDA is in the two bytes
	// starting at byte 0x0E of the BDA.  0 if not present.
	if ((p = *(uint16 *) (bda + 0x0E)))
	{
		p <<= 4;	// Translate from segment to PA
		if ((mp = mpsearch1(p, 1024)))
			return mp;
	}
	else
	{
		// The size of base memory, in KB is in the two bytes
		// starting at 0x13 of the BDA.
		p = *(uint16 *) (bda + 0x13) * 1024;
		if ((mp = mpsearch1(p - 1024, 1024)))
			return mp;
	}

	return mpsearch1(0xF0000, 0x10000);
}

// Search for an MP configuration table.  For now, don't accept the
// default configurations (uint32 == 0).
// Check for the correct signature, checksum, and version.
static struct mpconf *
mpconfig(struct mp **pmp)
{
	struct mpconf *conf;
	struct mp *mp;

	if ((mp = mpsearch()) == 0)
		return NULL;
	if (mp->uint32 == 0 || mp->type != 0) {
		cprintf("SMP: Default configurations not implemented\n");
		return NULL;
	}
	cprintf("mpconfig3\n");

	conf = (struct mpconf *) STATIC_KERNEL_VIRTUAL_ADDRESS(mp->uint32);
	if (memcmp(conf, "PCMP", 4) != 0) {
		cprintf("SMP: Incorrect MP configuration table signature\n");
		return NULL;
	}
	if (sum(conf, conf->length) != 0) {
		cprintf("SMP: Bad MP configuration checksum\n");
		return NULL;
	}
	if (conf->version != 1 && conf->version != 4) {
		cprintf("SMP: Unsupported MP version %d\n", conf->version);
		return NULL;
	}
	if ((sum((uint8 *)conf + conf->length, conf->xlength) + conf->xchecksum) & 0xff) {
		cprintf("SMP: Bad MP configuration extended checksum\n");
		return NULL;
	}
	*pmp = mp;
	return conf;
}

void
mp_init(void)
{
	struct mp *mp;
	struct mpconf *conf;
	struct mpproc *proc;
	uint8 *p;
	unsigned int i;

	bootcpu = &cpus[0];
	if ((conf = mpconfig(&mp)) == 0)
		return;
	ismp = 1;
	lapicaddr = conf->lapicaddr;

	for (p = conf->entries, i = 0; i < conf->entry; i++) {
		switch (*p) {
		case MPPROC:
			proc = (struct mpproc *)p;
			if (proc->flags & MPPROC_BOOT)
				bootcpu = &cpus[ncpu];
			if (ncpu < NCPUS) {
				cpus[ncpu].cpu_id = ncpu;
				ncpu++;
			} else {
				cprintf("SMP: too many CPUs, CPU %d disabled\n", proc->apicid);
			}
			p += sizeof(struct mpproc);
			continue;
		case MPBUS:
		case MPIOAPIC:
		case MPIOINTR:
		case MPLINTR:
			p += 8;
			continue;
		default:
			cprintf("mpinit: unknown config type %x\n", *p);
			ismp = 0;
			i = conf->entry;
		}
	}
	cprintf(">>>>>> mp_init(): NUMBER OF CPUS = %d\n", ncpu);
	bootcpu->cpu_status = CPU_STARTED;
	if (!ismp) {
		// Didn't like what we found; fall back to no MP.
		ncpu = 1;
		lapicaddr = 0;
		cprintf("SMP: configuration not found, SMP disabled\n");
		return;
	}
	cprintf("SMP: CPU %d found %d CPU(s)\n", bootcpu->cpu_id,  ncpu);

	if (mp->imcrp) {
		// [MP 3.2.6.1] If the hardware implements PIC mode,
		// switch to getting interrupts from the LAPIC.
		cprintf("SMP: Setting IMCR to switch from PIC mode to symmetric I/O mode\n");
		outb(0x22, 0x70);   // Select IMCR
		outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
	}
}
