/* See COPYRIGHT for copyright information. */

#ifndef FOS_KERN_KCLOCK_H
#define FOS_KERN_KCLOCK_H
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif
#include <inc/types.h>
#define	IO_RTC		0x070		/* RTC port */

#define	MC_NVRAM_START	0xe	/* start of NVRAM: offset 14 */
#define	MC_NVRAM_SIZE	50	/* 50 bytes of NVRAM */

/* NVRAM bytes 7 & 8: base memory size */
#define NVRAM_BASELO	(MC_NVRAM_START + 7)	/* low byte; RTC off. 0x15 */
#define NVRAM_BASEHI	(MC_NVRAM_START + 8)	/* high byte; RTC off. 0x16 */

/* NVRAM bytes 9 & 10: extended memory size */
#define NVRAM_EXTLO	(MC_NVRAM_START + 9)	/* low byte; RTC off. 0x17 */
#define NVRAM_EXTHI	(MC_NVRAM_START + 10)	/* high byte; RTC off. 0x18 */

/* NVRAM bytes 34 and 35: extended memory POSTed size */
#define NVRAM_PEXTLO	(MC_NVRAM_START + 34)	/* low byte; RTC off. 0x30 */
#define NVRAM_PEXTHI	(MC_NVRAM_START + 35)	/* high byte; RTC off. 0x31 */

/* NVRAM byte 36: current century.  (please increment in Dec99!) */
#define NVRAM_CENTURY	(MC_NVRAM_START + 36)	/* RTC offset 0x32 */

unsigned mc146818_read(unsigned reg);
void mc146818_write(unsigned reg, unsigned datum);

/* Every time the mode/command register is written to, all internal logic in the selected
 * PIT channel is reset, and the output immediately goes to its initial state
 * (which depends on the mode).
 */

void kclock_init();
void kclock_start(uint8 quantum_in_ms);
void kclock_stop(void);
void kclock_resume(void);
void kclock_start_counter(uint8 cnt0);

uint16 kclock_read_cnt0(void);
uint16 kclock_read_cnt0_latch(void);

//2017
void kclock_write_cnt0_LSB_first(uint16 val);

//2018
void kclock_set_quantum(uint8 quantum_in_ms);


extern uint32 virtualTime;

#endif	// !FOS_KERN_KCLOCK_H
