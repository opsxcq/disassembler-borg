//					xref.h
//

#ifndef xref_h
#define xref_h

#include "list.h"
#include "common.h"
#include "savefile.h"

#pragma pack(push,pack_save,1)
struct xrefitem
{ lptr addr;
  lptr refby;
};
#pragma pack(pop,pack_save)

class xref: public slist <xrefitem *>
{ public:
	 xref();
	 ~xref();
	 void addxref(lptr loc,lptr ref_by);
	 void printfirst(lptr loc);
    void delxref(lptr loc,lptr ref_by);
    void userdel(lptr loc);
    // virtual functions from slist
    int compare(xrefitem *i,xrefitem *j);
};

extern xref xrefs;

#endif