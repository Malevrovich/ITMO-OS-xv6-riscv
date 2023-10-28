#include "list.h"
#include "spinlock.h"

struct sync_list {
    struct list lst;
    
    int ref_cnt;
    struct spinlock list_lock;
};