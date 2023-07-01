/*
* @author			: Dhananjay Khairnar (Nashik, Maharashtra, India)
* @email			: khairnardm@gmail.com
* @github			: https://github.com/8-DK
 */


#include "lmic.h"
#include "modem.h"

#define NUMQ 5

static struct {
	uint8_t* buf;
	uint16_t len;
} queue[NUMQ];

static uint8_t qhead;

void queue_init () {
	memset(queue, 0, sizeof(queue));
	qhead = 0;
}

void queue_add (uint8_t* buf, uint16_t len) {
	uint8_t i, j;
	hal_disableIRQs();
	for(i=0, j=qhead; i<NUMQ; i++, j++) {
		if(j == NUMQ) {
			j = 0;
		}
		if(queue[j].buf == NULL) {
			queue[j].buf = buf;
			queue[j].len = len;
			break;
		}
	}
	if(i == NUMQ) {

		hal_failed(0,0);
	}
	hal_enableIRQs();
}

uint8_t queue_shift (FRAME* f) {
	uint8_t r = 0;
	hal_disableIRQs();
	if(queue[qhead].buf) {
		frame_init(f, queue[qhead].buf, queue[qhead].len);
		queue[qhead].buf = NULL;
		if(++qhead == NUMQ) {
			qhead = 0;
		}
		r = 1;
	}
	hal_enableIRQs();
	return r;
}
