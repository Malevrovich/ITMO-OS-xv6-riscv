// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "atomic.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
 
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  
  void *pa_start;
  void *pa_end;

  struct atomic_uint* ref_cnts;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  kmem.ref_cnts = (struct atomic_uint*)end;
  
  kmem.pa_end = (void*)PGROUNDDOWN(PHYSTOP);
  kmem.pa_start = (char*)end + (((char *)kmem.pa_end - (char*)end) / PGSIZE + 1) * sizeof(uint64);
  kmem.pa_start = (void*)PGROUNDUP((uint64)kmem.pa_start);

  for (struct atomic_uint *it = kmem.ref_cnts; (it - kmem.ref_cnts) < (kmem.pa_end - kmem.pa_start) / PGSIZE; it++) {
    atomic_uint_init(it, 1);
  }

  freerange(kmem.pa_start, kmem.pa_end);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (void*)pa < kmem.pa_start || pa >= kmem.pa_end)
    panic("kfree");

  int res = atomic_uint_decrement_and_fetch(&kmem.ref_cnts[((char*)pa - (char*)kmem.pa_start) / PGSIZE]);
  if (res != 0) {
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

unsigned int 
krefcnt(void *pa)
{
  return atomic_uint_fetch(&kmem.ref_cnts[((char*)pa - (char*)kmem.pa_start) / PGSIZE]);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if (r) {
    if (atomic_uint_fetch(&kmem.ref_cnts[((char*)r - (char*)kmem.pa_start) / PGSIZE]) != 0) {
      panic("kalloc");
    }
    kdup(r);
  }
  
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
kdup(void *pa)
{
  atomic_uint_increment(&kmem.ref_cnts[((char*)pa - (char*)kmem.pa_start) / PGSIZE]);
}
