//			gname.h
//
// generic name class - class of items comprising names and locations
// - used for exports,imports and names classes
// - common parts separated into this class Build 16.

#ifndef gname_h
#define gname_h

#include "list.h"
#include "common.h"
#include "savefile.h"

#define GNAME_MAXLEN 40

#pragma pack(push,pack_save,1)
struct gnameitem
{ lptr addr;
  char *name;
};
#pragma pack(pop,pack_save)

class gname:public slist <gnameitem *>
{ public:
    gname();
    ~gname();
    void addname(lptr loc,char *nm);
    bool isname(lptr loc);
    void printname(lptr loc);
    void delname(lptr loc);
    dword getoffsfromname(char *nm);
    // virtual functions from slist
    int compare(gnameitem *i,gnameitem *j);
    void delfunc(gnameitem *i);
};

extern gname expt;     // exports table
extern gname import;   // imports table
extern gname name;     // user defined names

#endif