//			disio.h
//
// - added version 2.11

#ifndef disio_h
#define disio_h

#include "list.h"
#include "common.h"
#include "proctab.h"
#include "stacks.h"
#include "disfuncs.h"
#include "savefile.h"

// display flags
#define DISPFLAG_NEGATE 1

class disio
{ public:
	 lptr curraddr,outend;
	 dsmitemtype subitem;  // subindex to top line of output
	 stack retstack;

  public:
  	disio();
   ~disio();
	void dumptofile(char *fname,bool printaddrs);
	void dumpblocktofile(char *fname,bool printaddrs);
	void outcomment(dsmitem *inst);
   void outargs(dsmitem *inst,argtype a);
	void updatewindow(void);
	void scroller(dword amount);
	void updatewindowifinrange(lptr loc);
	void savecuraddr(void);
	void findcurrentaddr(lptr *loc);
	void vertsetpos(int pos);
	void jumpback(void);
	void jumpto(bool arg1);
	dsmitem *findcurrentline(void);
	void setcuraddr(lptr loc);
	void setpos(int ypos);
	void outinst(dsmitem *inst,bool printaddrs);
	void outdb(lptr *lp,bool printaddrs);
	void outprefix(byte prefixbyte);
	void argoverdec(void);
	void argoversingle(void);
	void arg_negate(void);
	void argoverhex(void);
	void argoveroffsetdseg(void);
	void argoverchar(void);
	void makecode(void);
	void makeword(void);
	void makedword(void);
	void makesingle(void);
	void makedouble(void);
	void makelongdouble(void);
	void makestring(void);
	void pascalstring(void);
	void ucstring(void);
	void upstring(void);
	void dosstring(void);
	void generalstring(void);
   void updatewindowifwithinrange(lptr loc_start,lptr loc_end);

 private:
   void printlineheader(lptr loc,bool printaddrs);
   bool issegprefix(byte byt);
   bool isprefix(byte byt);
};

extern disio dio;

#endif