//						debug.h
//

#ifndef debug_h
#define debug_h

#include "common.h"

//remove comment on line below for debug mode.
//#define DEBUG
#define DEBUGFILE "i:\\borgnew\\debug.txt"

void DebugMessage(char *szFormat,...);
void ShowLastError(void);

#endif