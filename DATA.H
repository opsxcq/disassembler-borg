//					data.h
//

#ifndef data_h
#define data_h

#include "list.h"
#include "common.h"
#include "savefile.h"

enum segtype {code16=1,code32,data16,data32,uninitdata,debugdata,resourcedata};

//essentially the data-mapping to the file.
//divides file into code/data segments or objects
//each set of data is then referenced through these sorted headers
#pragma pack(push,pack_save,1)
struct dsegitem
{ lptr addr;
  dword size;
  byte *data;
  segtype typ;
  char *name;
};
#pragma pack(pop,pack_save)

class dataseg: public slist <dsegitem *>
{ public:
	 dataseg();
	 ~dataseg();
	 void addseg(lptr loc,dword size,byte *dataptr,segtype t,char *name);
	 dsegitem *findseg(lptr loc);
	 bool beyondseg(lptr loc);
	 void nextseg(lptr *loc);
	 void lastseg(lptr *loc);
    unsigned long datagetpos(lptr loc);
    lptr getlocpos(unsigned long pos);
	 void possibleentrycode(lptr loc);
	 void segheader(lptr loc);
    bool read_item(savefile *sf,byte *filebuff);
    bool write_item(savefile *sf,byte *filebuff);
    // virtual functions from slist
    int compare(dsegitem *i,dsegitem *j);
    void delfunc(dsegitem *i);

  private:
    bool insegloc(dsegitem *ds,lptr loc);
};

extern dataseg dta;

// for vertical scrollbar only
extern unsigned long total_data_size;
extern unsigned long current_data_pos;

#endif
