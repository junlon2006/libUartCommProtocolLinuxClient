/**********************************************************************
 * Copyright (C) 2017-2017  junlon2006
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
 **********************************************************************
 *
 * Description : uni_event_list.c
 * Author      : junlon2006@163.com
 * Date        : 2017.9.19
 *
 **********************************************************************/

#include "uni_event_list.h"
#include "list_head.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>

#define _consume_higest_list               _consume_one_list
#define _consume_medium_list               _consume_one_list
#define _consume_lowest_list               _consume_one_list
#define THREAD_TRY_COND_TIMROUT_MSC        (1000)

typedef int (*_interruptable_handler)(void *args);

typedef struct {
  list_head link;
  int       priority;
  void      *event;
} EventListItem;

typedef struct {
  list_head                 highest_list;
  list_head                 medium_list;
  list_head                 lowest_list;
  int                       highest_cnt;
  int                       medium_cnt;
  int                       lowest_cnt;
  EventListEventHandler     event_handler;
  EventListEventFreeHandler free_handler;
  _interruptable_handler    highest_interrupt_handler;
  _interruptable_handler    medium_interrupt_handler;
  _interruptable_handler    lowest_interrupt_handler;
  pthread_t                 pid;
  pthread_mutex_t           mutex;
  pthread_cond_t            cond;
  int                       running;
} EventList;

static void _consume_one_list(list_head *header,
                              EventListEventHandler event_handler,
                              EventListEventFreeHandler free_handler,
                              _interruptable_handler interrupt,
                              void *interrupt_args,
                              pthread_mutex_t *mutex,
                              int *total_cnt, int *running) {
  list_head *p;
  EventListItem *item;
  do {
    pthread_mutex_lock(mutex);
    if (NULL != interrupt && interrupt(interrupt_args)) {
      pthread_mutex_unlock(mutex);
      break;
    }
    if (NULL == (p = list_get_head(header))) {
      pthread_mutex_unlock(mutex);
      break;
    }
    item = list_entry(p, EventListItem, link);
    list_del(&item->link);
    *total_cnt = *total_cnt - 1;
    pthread_mutex_unlock(mutex);
    if (NULL != event_handler && *running) {
      event_handler(item->event);
    }
    if (NULL != free_handler) {
      free_handler(item->event);
    }
    free(item);
  } while (p != NULL);
}

static int _highest_interruptable_handler(void *args) {
  return 0;
}

static int _medium_interruptable_handler(void *args) {
  EventList *event_list = (EventList*)args;
  return (0 < event_list->highest_cnt);
}

static int _lowest_interruptable_handler(void *args) {
  EventList *event_list = (EventList*)args;
  return (0 < event_list->highest_cnt || 0 < event_list->medium_cnt);
}

static void _consume_event_list(EventList *event_list) {
  _consume_higest_list(&event_list->highest_list, event_list->event_handler,
                       event_list->free_handler,
                       event_list->highest_interrupt_handler, event_list,
                       &event_list->mutex,
                       &event_list->highest_cnt, &event_list->running);
  _consume_medium_list(&event_list->medium_list, event_list->event_handler,
                       event_list->free_handler,
                       event_list->medium_interrupt_handler, event_list,
                       &event_list->mutex,
                       &event_list->medium_cnt, &event_list->running);
  _consume_lowest_list(&event_list->lowest_list, event_list->event_handler,
                       event_list->free_handler,
                       event_list->lowest_interrupt_handler, event_list,
                       &event_list->mutex,
                       &event_list->lowest_cnt, &event_list->running);
}

static void _reset_event_handler(EventList *event_list) {
  event_list->event_handler = NULL;
}

static void _reset_interrupt_handler(EventList *event_list) {
  event_list->highest_interrupt_handler = NULL;
  event_list->medium_interrupt_handler = NULL;
  event_list->lowest_interrupt_handler = NULL;
}

static void _free_all(EventList *event_list) {
  _reset_event_handler(event_list);
  _reset_interrupt_handler(event_list);
  _consume_event_list(event_list);
  pthread_mutex_destroy(&event_list->mutex);
  pthread_cond_destroy(&event_list->cond);
  free(event_list);
}

static void _set_ts(struct timespec *ts) {
  int64_t timeout;
  clock_gettime(CLOCK_MONOTONIC, ts);
  timeout = (int64_t)ts->tv_sec * (int64_t)1000000000 + (int64_t)ts->tv_nsec;
  timeout += (int64_t)(THREAD_TRY_COND_TIMROUT_MSC * (int64_t)1000000);
  ts->tv_sec = (timeout / 1000000000);
  ts->tv_nsec = (timeout % 1000000000);
}

static int _all_list_empty(EventList *event_list) {
  return (0 == event_list->highest_cnt &&
          0 == event_list->medium_cnt &&
          0 == event_list->lowest_cnt);
}

static void _try_cond_timedwait(EventList *event_list) {
  struct timespec ts;
  if (_all_list_empty(event_list)) {
    pthread_mutex_lock(&event_list->mutex);
    if (_all_list_empty(event_list)) {
      _set_ts(&ts);
      pthread_cond_timedwait(&event_list->cond, &event_list->mutex, &ts);
    }
    pthread_mutex_unlock(&event_list->mutex);
  }
}

static void *_event_handler(void *args) {
  EventList *event_list = (EventList*)args;
  pthread_detach(pthread_self());
  while (event_list->running) {
    _consume_event_list(event_list);
    _try_cond_timedwait(event_list);
  }
  _free_all(event_list);
  return NULL;
}

static void _mzero_event_list(EventList *event_list) {
  memset(event_list, 0, sizeof(EventList));
}

static void _list_init(EventList *event_list) {
  list_init(&event_list->highest_list);
  list_init(&event_list->medium_list);
  list_init(&event_list->lowest_list);
}

static void _register_event_handler(EventList *event_list,
                                    EventListEventHandler event_handler) {
  event_list->event_handler = event_handler;
}

static void _register_free_handler(EventList *event_list,
                                   EventListEventFreeHandler free_handler) {
  event_list->free_handler = free_handler;
}

static void _register_interuppt_handler(EventList *event_list) {
  event_list->highest_interrupt_handler = _highest_interruptable_handler;
  event_list->medium_interrupt_handler = _medium_interruptable_handler;
  event_list->lowest_interrupt_handler = _lowest_interruptable_handler;
}

static void _worker_thread_create(EventList *event_list) {
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  pthread_cond_init(&event_list->cond, &attr);
  pthread_mutex_init(&event_list->mutex, NULL);
  event_list->running = 1;
  pthread_create(&event_list->pid, NULL, _event_handler, event_list);
}

EventListHandle EventListCreate(EventListEventHandler event_handler,
                                EventListEventFreeHandler free_handler) {
  EventList *event_list = NULL;
  if (NULL == (event_list = (EventList*)malloc(sizeof(EventList)))) {
    return NULL;
  }
  _mzero_event_list(event_list);
  _list_init(event_list);
  _register_event_handler(event_list, event_handler);
  _register_free_handler(event_list, free_handler);
  _register_interuppt_handler(event_list);
  _worker_thread_create(event_list);
  return event_list;
}

int EventListDestroy(EventListHandle handle) {
  EventList *event_list = (EventList*)handle;
  event_list->running = 0;
  return 0;
}

int EventListAdd(EventListHandle handle, void *event, int priority) {
  EventListItem *item = NULL;
  EventList *event_list = (EventList *)handle;
  if (NULL == (item = (EventListItem*)malloc(sizeof(EventListItem)))) {
    return -1;
  }
  item->priority = priority;
  item->event = event;
  pthread_mutex_lock(&event_list->mutex);
  if (EVENT_LIST_PRIORITY_HIGHEST == priority) {
    list_add_tail(&item->link, &event_list->highest_list);
    event_list->highest_cnt++;
  } else if (EVENT_LIST_PRIORITY_MEDIUM == priority) {
    list_add_tail(&item->link, &event_list->medium_list);
    event_list->medium_cnt++;
  } else if (EVENT_LIST_PRIORITY_LOWEST == priority) {
    list_add_tail(&item->link, &event_list->lowest_list);
    event_list->lowest_cnt++;
  }
  pthread_cond_signal(&event_list->cond);
  pthread_mutex_unlock(&event_list->mutex);
  return 0;
}

int EventListClear(EventListHandle handle) {
  EventList *event_list = (EventList *)handle;
  _consume_higest_list(&event_list->highest_list, NULL,
                       event_list->free_handler, NULL, NULL,
                       &event_list->mutex, &event_list->highest_cnt,
                       &event_list->running);
  _consume_medium_list(&event_list->medium_list, NULL,
                       event_list->free_handler, NULL, NULL,
                       &event_list->mutex, &event_list->medium_cnt,
                       &event_list->running);
  _consume_lowest_list(&event_list->lowest_list, NULL,
                       event_list->free_handler, NULL, NULL,
                       &event_list->mutex, &event_list->lowest_cnt,
                       &event_list->running);
  return 0;
}
