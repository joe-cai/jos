// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	int r;
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR) || !(uvpt[(unsigned)addr / PGSIZE] & PTE_COW))
		panic("pgfault: faulting access is not a write or at least not on a copy-on-write page");
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	int perm = (uvpt[(unsigned)addr / PGSIZE] 
		    & PTE_SYSCALL & ~PTE_COW) | PTE_W;
	int envid = sys_getenvid();
	if ((r = sys_page_alloc(envid, PFTEMP, perm)) < 0)
		panic("sys_page_alloc: %e with perm %d", r, perm);
	memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE); 
        // ROUNDDOWN is critical here. Debugging for a day before recognizing.
	if ((r = sys_page_map(envid, PFTEMP, envid, 
			      ROUNDDOWN(addr, PGSIZE), perm)) < 0)
		panic("sys_page_map: %e", r);
	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn * PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function? **I don't know...**)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	int curenv_id = sys_getenvid();
	// LAB 4: Your code here.
	if ((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW)) {
		r = sys_page_map(curenv_id, (void *)(pn * PGSIZE),
				 envid, (void *)(pn * PGSIZE),
				 (uvpt[pn] & PTE_SYSCALL & ~PTE_W) | PTE_COW);
		if (r < 0) panic("sys_page_map: %e", r);
		// have no idea why we need to mark the same page again...
		r = sys_page_map(curenv_id, (void *)(pn * PGSIZE),
				 curenv_id, (void *)(pn * PGSIZE),
				 (uvpt[pn] & PTE_SYSCALL & ~PTE_W) | PTE_COW);
		if (r < 0) panic("sys_page_map: %e", r);
	} else {
		r = sys_page_map(curenv_id, (void *)(pn * PGSIZE),
				 envid, (void *)(pn * PGSIZE),
				 uvpt[pn] & PTE_SYSCALL);
		if (r < 0) panic("sys_page_map: %e", r);
	}
	// panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);

	int r;
	unsigned addr;
	envid_t envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	else if (envid == 0) { // in the child process
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	// envid > 0: in the parent process
	// copy address space from parent to child
	for (addr = UTEXT; addr < USTACKTOP; addr += PGSIZE)
		// if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PTX(addr)] & PTE_P)) {
		if ((uvpd[addr / PTSIZE] & PTE_P) && 
		    (uvpt[addr / PGSIZE] & PTE_P))
			duppage(envid, addr / PGSIZE);

	// set up exception stack for child process	
	if ((r = sys_page_alloc(envid, (void *)UXSTACKTOP - PGSIZE, 
				PTE_W | PTE_U | PTE_P)) < 0)
		panic("sys_page_alloc: %e", r);
	if ((r = sys_env_set_pgfault_upcall(
		     envid, thisenv->env_pgfault_upcall)) < 0)
		panic("sys_env_set_pgfault_upcall: %e", r);
	    
	// set_pgfault_handler(pgfault);
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);
	// panic("fork not implemented");
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
