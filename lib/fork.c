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
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (err != FEC_WR || (uvpt[PTX(addr)] & PTE_COW) != PTE_COW)
		panic("pgfault: faulting access is not a write or not on a copy-on-write page");
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	int perm = (uvpt[PTX(addr)] & 0xFFF & ~PTE_COW) | PTE_W;
	if ((r = sys_page_alloc(sys_getenvid(), PFTEMP, perm)) < 0)
		panic("sys_page_alloc: %e", r);
	memmove(PFTEMP, addr, PGSIZE);
	if ((r = sys_page_map(sys_getenvid(), PFTEMP, sys_getenvid(), addr,
			      perm)) < 0)
		panic("sys_page_map: %e", r);
	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn * PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	// LAB 4: Your code here.
	if ((uvpt[PTX(pn * PGSIZE)] & PTE_W) == PTE_W ||
	    (uvpt[PTX(pn * PGSIZE)] & PTE_COW) == PTE_COW) {
		r = sys_page_map(sys_getenvid(), (void *)(pn * PGSIZE),
				 envid, (void *)(pn * PGSIZE),
				 (uvpt[PTX(pn * PGSIZE)] & 0xFFF) | PTE_COW);
		if (r < 0) 
			return r;
		/* have no idea why we need to mark the same page again... */
		r = sys_page_map(sys_getenvid(), (void *)(pn * PGSIZE),
				 sys_getenvid(), (void *)(pn * PGSIZE),
				 (uvpt[PTX(pn * PGSIZE)] & 0xFFF) | PTE_COW);
	} else
		r = sys_page_map(sys_getenvid(), (void *)(pn * PGSIZE),
				 envid, (void *)(pn * PGSIZE),
				 uvpt[PTX(pn * PGSIZE)] & 0xFFF);
	// panic("duppage not implemented");
	return r;
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
		// set up exception stack for child process	
		set_pgfault_handler(pgfault);
		return 0;
	}
	// envid > 0: in the parent process
	// copy address space from parent to child
	for (addr = UTEXT; addr < USTACKTOP; addr += PGSIZE)
		if ((uvpd[PDX(addr)] & PTE_P) == PTE_P &&
		    (uvpt[PTX(addr)] & PTE_P) == PTE_P)
			duppage(envid, addr / PGSIZE);

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
