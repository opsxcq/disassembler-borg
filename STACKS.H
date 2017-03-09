//			stacks.h
//
// generic stack class -
// - added version 2.11

#ifndef stacks_h
#define stacks_h

#include "common.h"
#include "savefile.h"

#define CALLSTACKSIZE 100

class stack
{ public:
	lptr callstack[CALLSTACKSIZE];
	int stacktop;

  public:
    stack();
    ~stack();
    void push(lptr loc);
    lptr pop(void);
};

#endif