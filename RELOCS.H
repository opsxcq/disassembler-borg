//					relocs.h
//

#ifndef relocs_h
#define relocs_h

#include "list.h"
#include "common.h"
#include "savefile.h"

// relocation types, should be self explanatory. Most are not generally used by programs,
// mainly reloc_offs32 by the PE file format.
enum reloctype {RELOC_NONE=1,RELOC_SEG,RELOC_OFFS16,RELOC_OFFS32,RELOC_SEGOFFS16,RELOC_SEGOFFS32};

// we will keep track of relocation addresses and the type.
#pragma pack(push,pack_save,1)
struct relocitem
{ lptr addr;
  reloctype type;
};
#pragma pack(pop,pack_save)

// class definition.
class relocs: public slist <relocitem *>
{ public:
    int sizeofitem;

  public:
	 relocs();
	 ~relocs();
	 void addreloc(lptr loc,reloctype type);
	 bool isreloc(lptr loc);
	 bool relocfile(void);
    relocitem *newitem(void);
    bool write_item(savefile *sf);
    bool read_item(savefile *sf);
    // virtual functions from slist
    int compare(relocitem *i,relocitem *j);
};

extern relocs reloc;

#endif