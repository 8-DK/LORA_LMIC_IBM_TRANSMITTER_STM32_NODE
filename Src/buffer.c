/*
* @author			: Dhananjay Khairnar (Nashik, Maharashtra, India)
* @email			: khairnardm@gmail.com
* @github			: https://github.com/8-DK
 */



#include "lmic.h"
#include "modem.h"

// ring buffer - append at end, free from beg

static uint8_t buffer[1024];
static uint8_t* beg;
static uint8_t* end;
static uint8_t* max;

void buffer_init () {
    beg = end = max = buffer;
}

uint8_t* buffer_alloc (uint16_t len) {
    uint8_t* buf = NULL;
    hal_disableIRQs();
    if(beg <= end) { // .......******...
	if(end + len < buffer + sizeof(buffer)) { // append
	    buf = end;
	    end += len;
	    max = end;
	} else { // wrap
	    if(buffer + len < beg) {
		buf = buffer;
		end = buffer + len;
	    }
	}
    } else { // ***.......*****.
	if(end + len < beg) {
	    buf = end;
	    end += len;
	}
    }
    hal_enableIRQs();
    return buf;
}

void buffer_free (uint8_t* buf, uint16_t len) {
    if(buf >= buffer && buf+len < buffer+sizeof(buffer)) {
	hal_disableIRQs();
	while(buf != beg); // halt if trying to free not from beginning
	if(beg <= end) { // .......******...
	    beg += len;
	    if(beg == end) { // empty
		beg = end = max = buffer; // reset
	    }
	} else { // ***.......*****.
	    beg += len;
	    if(beg == max) {
		beg = buffer;
		max = end;
	    }
	}
	hal_enableIRQs();
    }
}
