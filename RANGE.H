//			range.h
//

#ifndef range_h
#define range_h

#include "common.h"

class range
{ public:
	 lptr top,bottom;

  public:
	range();
	~range();
   bool checkblock(void);
   void undefine(void);
   void settop(void);
   void setbottom(void);
};

extern range blk;

#endif
