#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "sync_list.h"
#include "riscv.h"
#include "defs.h"

// thread safe double-linked, circular list. double-linked makes remove
// fast. circular simplifies code, because don't have to check for
// empty list in insert and remove.

void
s_lst_init(struct sync_list *lst)
{
  lst_init(&lst->lst);

  lst->ref_cnt = 2;
  initlock(&lst->list_lock, "sync_list_lock");
}

// lst->list_lock must be acquired
void s_lst_release_element(struct sync_list *lst, struct sync_list *e) {
  if (lst == e) {
    return;
  }
  acquire(&e->list_lock);
  e->ref_cnt--;
  CHECK(e->ref_cnt >= 0);
  
  if (e->ref_cnt == 0) {
    s_lst_release_element(lst, (struct sync_list*)e->lst.next);
    s_lst_release_element(lst, (struct sync_list*)e->lst.prev);
    pop_off();
    bd_free(e);
  } else {
    release(&e->list_lock);
  }
}

// lst->list_lock must be acquired
struct sync_list *s_lst_next(struct sync_list *e) {
  struct sync_list *next = (struct sync_list *)e->lst.next;
  if (next == e) {
    return e;
  }
  acquire(&next->list_lock);
  next->ref_cnt++;
  release(&next->list_lock);
  return next;
}

// lst->list_lock must be acquired
void s_lst_move_next(struct sync_list *lst, struct sync_list **e) {
  struct sync_list *next;
  
  if ((*e)->lst.next == &lst->lst) {
    next = (struct sync_list *)(*e)->lst.next;
  } else {
    next = s_lst_next(*e);
  }

  if (*e != lst) {
    s_lst_release_element(lst, *e);
  }

  *e = next;
}

// lst->list_lock must be acquired
int
s_lst_empty(struct sync_list *lst) {
  return lst_empty(&lst->lst);
}

// lst->list_lock must be acquired
void
s_lst_remove(struct sync_list *lst, struct sync_list *e) {
  lst_remove(&e->lst);

  struct sync_list *next = (struct sync_list*)e->lst.next;
  struct sync_list *prev = (struct sync_list*)e->lst.prev;

  if (next != lst) {
    acquire(&next->list_lock);
    next->ref_cnt++;
    release(&next->list_lock);
  }
  if (prev != lst) {
    acquire(&prev->list_lock);
    prev->ref_cnt++;
    release(&prev->list_lock);
  }

  acquire(&e->list_lock);
  e->ref_cnt -= 2;

  // freed before ref_count++
  CHECK(e->ref_cnt > 0);
  
  if (e->ref_cnt == 0) {
    pop_off();
    bd_free(e);
  } else {
    release(&e->list_lock);
  }
}

// lst->list_lock must be acquired
void
s_lst_push(struct sync_list *lst, struct sync_list *p) {
  *p = (struct sync_list){.ref_cnt = 3};
  initlock(&p->list_lock, "sync_list_element_lock");
  lst_push(&lst->lst, &p->lst);
}