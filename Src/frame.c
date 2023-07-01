/*
* @author			: Dhananjay Khairnar (Nashik, Maharashtra, India)
* @email			: khairnardm@gmail.com
* @github			: https://github.com/8-DK
 */

#include "lmic.h"
#include "modem.h"

FRAME rxframe;
FRAME txframe;

static osjob_t txjob;
static osjob_t rxjob;

void frame_init (FRAME* f, uint8_t* buf, uint16_t max) {
    f->state = 0;
    f->lrc = 0;
    f->len = 0;
    f->buf = buf;
    f->max = max;
}

// called by usart irq handler (return next char to send 0x00XX, or 0x0100)
uint16_t frame_tx (uint8_t next) {
    if(next) {
	return (txframe.len < txframe.max) ? txframe.buf[txframe.len++] : 0x100;
    } else { // complete
	os_setCallback(&txjob, modem_txdone); // run job
	return 0;
    }
}

// called by usart irq handler (pass received char, return 1 to continue rx, 0 to stop)
// ASCII format:  ATxxxxxxx\r
// Binary format: B%lxxxxxxxx%c
uint8_t frame_rx (uint8_t c) {
    switch(rxframe.state) {
    case FRAME_INIT:
	if(c == 'a' || c == 'A') {
	    rxframe.state = FRAME_A_A;
	} else if(c == 'B') {
	    rxframe.state = FRAME_B_B;
	    rxframe.lrc = c;
	} else {
	    // keep state, wait for sync
	}
	break;
    case FRAME_A_A:
	if(c == 't' || c == 'T') {
	    rxframe.state = FRAME_A_T;
	}
	break;
    case FRAME_A_T:
	if(c == '\r') {
	    rxframe.state = FRAME_A_OK;
	    os_setCallback(&rxjob, modem_rxdone); // run job
	    return 0; // stop reception
	} else {
	    if(rxframe.len == rxframe.max) { // overflow
		rxframe.state = FRAME_A_ERR;
		os_setCallback(&rxjob, modem_rxdone); // run job
		return 0; // stop reception
	    }
	    rxframe.buf[rxframe.len++] = c; // store cmd
	}
	break;
    case FRAME_B_B:
	if(c > rxframe.max) {
	    rxframe.state = FRAME_B_ERR;
	    os_setCallback(&rxjob, modem_rxdone); // run job
	    return 0; // stop reception
	}
	rxframe.max = c; // store len
	rxframe.lrc ^= c;
	rxframe.state = FRAME_B_LEN;
	break;
    case FRAME_B_LEN:
	rxframe.buf[rxframe.len++] = c; // store cmd
	rxframe.lrc ^= c;
	if(rxframe.len == rxframe.max) {
	    rxframe.state = FRAME_B_LRC;
	}
	break;
    case FRAME_B_LRC:
	if(rxframe.lrc ^ c) {
	    rxframe.state = FRAME_B_ERR;
	} else {
	    rxframe.state = FRAME_B_OK;
	}
	os_setCallback(&rxjob, modem_rxdone); // run job
	return 0; // stop reception
    }
    return 1; // continue to receive
}
