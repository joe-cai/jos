/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TRAP_H
#define JOS_KERN_TRAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/trap.h>
#include <inc/mmu.h>

/* The kernel's interrupt descriptor table */
extern struct Gatedesc idt[];
extern struct Pseudodesc idt_pd;

/* exception handler defined in trapentry.S*/
// processor interrupts
extern void t_divide(void);
extern void t_debug(void);
extern void t_nmi(void);
extern void t_brkpt(void);
extern void t_oflow(void);
extern void t_bound(void);
extern void t_illop(void);
extern void t_device(void);
extern void t_dblflt(void);
extern void t_tss(void);
extern void t_segnp(void);
extern void t_stack(void);
extern void t_gpflt(void);
extern void t_pgflt(void);
extern void t_fperr(void);
extern void t_align(void);
extern void t_mchk(void);
extern void t_simderr(void);
extern void t_syscall(void);

// hardware interrupts
extern void irq_timer(void);
extern void irq_kbd(void);
extern void irq_serial(void);
extern void irq_spurious(void);
extern void irq_ide(void);
extern void irq_error(void);

void trap_init(void);
void trap_init_percpu(void);
void print_regs(struct PushRegs *regs);
void print_trapframe(struct Trapframe *tf);
void page_fault_handler(struct Trapframe *);
void backtrace(struct Trapframe *);

#endif /* JOS_KERN_TRAP_H */
