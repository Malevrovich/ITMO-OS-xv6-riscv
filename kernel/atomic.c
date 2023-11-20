#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "atomic.h"

void
atomic_uint_init(struct atomic_uint *x, unsigned int val) {
    x->val = val;
    __sync_synchronize();
}

void  
atomic_uint_increment(struct atomic_uint *x) {
    __sync_add_and_fetch(&x->val, 1);
}

unsigned int             
atomic_uint_decrement_and_fetch(struct atomic_uint *x) {
    return __sync_sub_and_fetch(&x->val, 1);
}

unsigned int   
atomic_uint_fetch(struct atomic_uint *x) {
    return x->val;
}
