/************************************************************************
*                  stack.cpp                                            *
* This is a simple stack class for a stack of lptr's. It is used in     *
* keeping track of jump locations, so that when a jump is followed it   *
* can be reversed. The stack is of a set size, and when it becomes too  *
* large the bottom of the stack is lost. I did have some plans on using *
* this class in a front end unpacker-emulator but my plans have changed *
* and any unpacker-emulator will use a different method more akin to    *
* single step tracing.                                                  *
* The stack was added in Version 2.11                                   *
************************************************************************/

#include <windows.h>
#include "stacks.h"
#include "debug.h"

/************************************************************************
* constructor function                                                  *
* - simply reset the top of the stack                                   *
************************************************************************/
stack::stack()
{	stacktop=0;
}

/************************************************************************
* destructor function                                                   *
* - nothing to do since the stack is a fixed array                      *
************************************************************************/
stack::~stack()
{
}

/************************************************************************
* push                                                                  *
* - places an item on top of the stack. If there is no room then we     *
*   lose an item from the bottom and move the others down               *
************************************************************************/
void stack::push(lptr loc)
{ int i;
  if(stacktop==CALLSTACKSIZE)  // need to remove bottom item from stack
  { for(i=0;i<CALLSTACKSIZE-1;i++)
	 { callstack[i]=callstack[i+1];
		stacktop--;
	 }
  }
  callstack[stacktop]=loc;
  stacktop++;
}

/************************************************************************
* pop                                                                   *
* - gets an item from the top of the stack, or returns nlptr if the     *
*   stack is empty                                                      *
************************************************************************/
lptr stack::pop(void)
{ if(!stacktop)
    return nlptr;
  stacktop--;
  return callstack[stacktop];
}


