/************************************************************************
*				xref.cpp                                                    *
*                                                                       *
* class to maintain xref list - xref items consist simply of an address *
* which is the location being referenced and a ref_by address which is  *
* where the reference is from. It is ordered by location and then by    *
* ref_by.                                                               *
************************************************************************/

// include files
#include <windows.h>

#include "xref.h"
#include "schedule.h"
#include "dasm.h"
#include "disasm.h"
#include "data.h"
#include "debug.h"
#include "mainwind.h"

/************************************************************************
* constructor - now empty                                               *
************************************************************************/
xref::xref()
{
}

/************************************************************************
* destructor - nothing is done here, the item can simply be freed       *
************************************************************************/
xref::~xref()
{
}

/************************************************************************
* basic function which adds an xref to the current list.                *
* - just need loc=location for which to create an xref                  *
* - and ref_by=where it is being referenced from                        *
* after the xref has been added we still need to add a line             *
* to the disassembly and so a task to do this is added using            *
* the scheduler.                                                        *
************************************************************************/
void xref::addxref(lptr loc,lptr ref_by)
{ xrefitem *newxref,*chk;
  dsegitem *chkseg;
  chkseg=dta.findseg(loc);
  if(chkseg==NULL)
    return;
  newxref=new xrefitem;
  newxref->addr=loc;
  newxref->refby=ref_by;
  chk=find(newxref);
  if(chk!=NULL)
	 if(!compare(newxref,chk))
	 { delete newxref;
		return;
	 }
  addto(newxref);
  scheduler.addtask(dis_xref,priority_xref,loc,NULL);
}

/************************************************************************
* compare function for list - uses address/referencing address          *
* to sort list                                                          *
************************************************************************/
int xref::compare(xrefitem *i,xrefitem *j)
{ if(i->addr < j->addr)
    return -1;
  if(i->addr > j->addr)
	 return 1;
  if(i->refby < j->refby)
	 return -1;
  if(i->refby > j->refby)
	 return 1;
  return 0;
}

/************************************************************************
* function to delete an xref from the list                              *
* after the xref has been deleted we check if we need to                *
* delete anything from the disassembly as well if there are             *
* no xrefs left for that loc                                            *
* does a window update if deleting one, but not the comment, which      *
* ensures that the number of xrefs is changed                           *
************************************************************************/
void xref::delxref(lptr loc,lptr ref_by)
{ xrefitem xtmp,*xfind;
  xtmp.addr=loc;
  xtmp.refby=ref_by;
  xfind=find(&xtmp);
  if(xfind!=NULL)
	 if((xfind->addr==loc)&&(xfind->refby==ref_by))
	 {	delfrom(xfind);
		xtmp.refby=nlptr;
		xfind=findnext(&xtmp);
		if(xfind!=NULL)
		  if(xfind->addr==loc)
        { dio.updatewindowifinrange(loc);
          return;
        }
		dsm.delcomment(loc,dsmxref);
	 }
}

/************************************************************************
* prints the first xref for a given loc                                 *
************************************************************************/
void xref::printfirst(lptr loc)
{ dword numents=0;
  xrefitem findit,*chk;
  findit.addr=loc;
  findit.refby=nlptr;
  findnext(&findit);
  chk=nextiterator();
  if(chk==NULL)
    return;
  if(options.mode32)
    LastPrintBuff("%04lx:%08lx",chk->refby.segm,chk->refby.offs);
  else
    LastPrintBuff("%04lx:%04lx",chk->refby.segm,chk->refby.offs);
  while(chk!=NULL)
  { if(chk->addr==loc)
	 { chk=nextiterator();
		numents++;
	 }
	 else
      break;
  }
  LastPrintBuff(" Number : %ld",numents);
}

/************************************************************************
* userdel                                                               *
* - deletes an xref, using the users current line and the refby passed  *
*   from the scheduler, which is from the xref viewer dialog            *
************************************************************************/
void xref::userdel(lptr loc)
{ lptr xcur;
  dio.findcurrentaddr(&xcur);
  delxref(xcur,loc);
}


