#include "stdafx.h"
#include "FileTimer.h"

#include <stdint.h>
#include <time.h>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


using namespace boost;


#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)


struct timer_node {
	struct timer_node *prev;
	struct timer_node *next;
	uint32_t delay;
	uint32_t expire;
	void * data;
	void * list;
	void(*timer_execute_func)(void *data);
};



struct link_list {
	struct timer_node head;
	struct timer_node *tail;
};


struct timer {
	struct link_list neart[TIME_NEAR];
	struct link_list t[4][TIME_LEVEL];
	shared_mutex  * sharedMutex;
	uint32_t time;
	uint32_t starttime;
	uint64_t current;
	uint64_t current_point;
};



static struct timer * TI = NULL;


static inline struct timer_node *
link_clear(struct link_list *list) {
	struct timer_node * ret = list->head.next;
	list->head.next = 0;
	list->tail = &(list->head);
	return ret;
}


static inline void
link(struct link_list *list, struct timer_node *node) {
	node->list = list;
	list->tail->next = node;
	node->prev = list->tail;
	list->tail = node;
	node->next = 0;
}


static void
add_node(struct timer *T, struct timer_node *node) {
	uint32_t time = node->expire;
	uint32_t current_time = T->time;
	if ((time | TIME_NEAR_MASK) == (current_time | TIME_NEAR_MASK)) {
		link(&T->neart[time&TIME_NEAR_MASK], node);
	}
	else {
		int i;
		uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
		for (i = 0; i < 3; i++) {
			if ((time | (mask - 1)) == (current_time | (mask - 1))) {
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
		}
		link(&T->t[i][((time >> (TIME_NEAR_SHIFT + i * TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)], node);
	}
}



static timer_node *
timer_add(struct timer *T, void *data, timer_execute_func cb, int time) {

	struct timer_node *node = (struct timer_node *)malloc(sizeof(timer_node));

	node->data = data;
	node->timer_execute_func = cb;
	node->delay = time;
	node->expire = time + T->time;

	T->sharedMutex->lock();
	add_node(T, node);
	T->sharedMutex->unlock();

	return node;

}



static void
move_list(struct timer *T, int level, int idx) {
	struct timer_node *current = link_clear(&T->t[level][idx]);
	while (current) {
		struct timer_node *temp = current->next;
		add_node(T, current);
		current = temp;
	}
}



static void
timer_shift(struct timer *T) {
	int mask = TIME_NEAR;
	uint32_t ct = ++T->time;
	if (ct == 0) {
		move_list(T, 3, 0);
	}
	else {
		uint32_t time = ct >> TIME_NEAR_SHIFT;
		int i = 0;
		while ((ct & (mask - 1)) == 0) {
			int idx = time & TIME_LEVEL_MASK;
			if (idx != 0) {
				move_list(T, i, idx);
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
			time >>= TIME_LEVEL_SHIFT;
			++i;
		}
	}
}


static inline void
dispatch_list(struct timer_node *current) {
	do {
		struct timer_node * temp = current;
		current->timer_execute_func(current->data);
		current = current->next;
		free(temp);
	} while (current);
}


static inline void
timer_execute(struct timer *T) {
	int idx = T->time & TIME_NEAR_MASK;
	while (T->neart[idx].head.next) {
		struct timer_node *current = link_clear(&T->neart[idx]);
		T->sharedMutex->unlock();
		// dispatch_list don't need lock T
		dispatch_list(current);
		T->sharedMutex->lock();
	}
}


static void
timer_update(struct timer *T) {

	T->sharedMutex->lock();
	// try to dispatch timeout 0 (rare condition)
	timer_execute(T);
	// shift time first, and then dispatch timer message
	timer_shift(T);
	timer_execute(T);
	T->sharedMutex->unlock();
}



static struct timer *
timer_create_timer() {
	struct timer *r = (struct timer *)malloc(sizeof(struct timer));
	memset(r, 0, sizeof(*r));
	int i, j;
	for (i = 0; i < TIME_NEAR; i++) {
		link_clear(&r->neart[i]);
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < TIME_LEVEL; j++) {
			link_clear(&r->t[i][j]);
		}
	}

	r->sharedMutex = new shared_mutex();
	r->current = 0;
	return r;

}



void *
file_timeout(void *data, int time, timer_execute_func cb) {
	if (time <= 0) {
		cb(data);
		return nullptr;
	}
	else {
		return timer_add(TI, data, cb, time);
	}
}


// centisecond: 1/100 second
static void
systime(uint32_t *sec, uint32_t *ms) {

	const posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	const posix_time::time_duration td = now.time_of_day();

	*sec = (uint32_t)td.seconds();
	
	*ms = (uint32_t)(td.total_milliseconds() / 10);
}


static uint64_t
gettime() {
	uint64_t t;
	const posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	const posix_time::time_duration td = now.time_of_day();
	t = (uint32_t)(td.total_milliseconds() / 10);
	return t;
}


static void *
thread_timer(void) {
	for (;;) {
		file_updatetimer();
		boost::this_thread::sleep(boost::posix_time::microseconds(2500));
	}
	//std::cout << "end timer";
}

void
file_updatetime(void *time) {
	struct timer_node * node = (timer_node *)time;
	struct link_list *list = (link_list *)(node->list);
	TI->sharedMutex->lock();
	if (node == list->tail){
		list->tail = node->prev;
	}
	node->prev->next = node->next;
	node->expire = node->delay + TI->time;
	add_node(TI, node);
	TI->sharedMutex->unlock();

}

void
file_updatetimer(void) {

	uint64_t cp = gettime();
	if (cp < TI->current_point) {
		TI->current_point = cp;
	}
	else if (cp != TI->current_point) {
		uint32_t diff = (uint32_t)(cp - TI->current_point);
		TI->current_point = cp;
		TI->current += diff;
		uint32_t i;

		for (i = 0; i < diff; i++) {
			timer_update(TI);
		}
	}
	// std::cout << "time: " << cp << "\n";
}


uint32_t
file_starttime(void) {
	return TI->starttime;
}


uint64_t
file_now(void) {
	return TI->current;
}


void
file_timer_init(void) {
	TI = timer_create_timer();
	uint32_t current = 0;
	systime(&TI->starttime, &current);
	TI->current = current;
	TI->current_point = gettime();
}

void 
file_timer_run(void) {
	thread thrd(thread_timer);
	thrd.detach();
}

uint64_t
file_thread_time(void) {
	uint64_t t;
	const posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	const posix_time::time_duration td = now.time_of_day();
	t = td.total_microseconds();
	return t;
}
