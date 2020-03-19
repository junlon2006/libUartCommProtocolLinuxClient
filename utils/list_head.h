/**************************************************************************
 * Copyright (C) 2018-2019 Junlon2006
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **************************************************************************
 *
 * Description : list_head.h
 * Author      : junlon2006@163.com
 * Date        : 2019.03.23
 *
 **************************************************************************/
#ifndef LIST_HEAD_INC_LIST_HEAD_H_
#define LIST_HEAD_INC_LIST_HEAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_head {
  struct list_head *next, *prev;
} list_head;

#define __LIST_ADD(entry,before,after) {list_head *new_= (entry), \
                                        *prev = (before), *next = (after); (next)->prev = (new_); \
                                        (new_)->next = (next); (new_)->prev = (prev); (prev)->next = (new_);}
#define list_init(entry)               do {(entry)->next = (entry); (entry)->prev = (entry);}  while(0)
#define list_add(entry,base)           do {__LIST_ADD((entry), (base), (base)->next);} while(0)
#define list_add_after(entry,base)     do {__LIST_ADD((entry), (base), (base)->next);} while(0)
#define list_add_before(entry,base)    do {__LIST_ADD((entry), (base)->prev, (base));} while(0)
#define list_add_head(entry,head)      list_add_after(entry,head)
#define list_add_tail(entry,head)      list_add_before(entry,head)
#define list_del(entry)                do {(entry)->prev->next = (entry)->next; \
                                           (entry)->next->prev = (entry)->prev; \
                                           (entry)->next = (entry)->prev = (entry);} while(0)
#define list_empty(head)               ((head)->next == (head))
#define list_get_head(head)            (list_empty(head) ? (list_head*)NULL : (head)->next)
#define list_get_tail(head)            (list_empty(head) ? (list_head*)NULL : (head)->prev)
#define list_is_head(entry, head)      ((entry)->prev == head)
#define list_is_tail(entry, head)      ((entry)->next == head)
#define list_entry(ptr, type, member)  ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#define list_for_each(h, head)         for (h = (head)->next; h != (head); h = h->next)
#define list_for_each_safe(h, n, head) for (h = (head)->next, n = h->next; h != (head);  h = n, n = h->next)
#define list_for_each_entry(p, head, type, member) for (p = list_entry((head)->next, type, member); \
                                                        &p->member != (head); p = list_entry(p->member.next, type, member))
#define list_for_each_entry_safe(p, t, head, type, member) for (p = list_entry((head)->next, type, member), \
                                                                t = list_entry(p->member.next, type, member); \
                                                                &p->member != (head); p = t, t = list_entry(t->member.next, type, member))
#define list_get_head_entry(head, type, member) (list_empty(head) ? (type*)NULL : list_entry(((head)->next), type, member))
#define list_get_tail_entry(head, type, member) (list_empty(head) ? (type*)NULL : list_entry(((head)->prev), type, member))

#ifdef __cplusplus
}
#endif
#endif
