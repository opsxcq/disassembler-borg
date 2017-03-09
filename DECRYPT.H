//			decrypt.h
//

#ifndef decrypt_h
#define decrypt_h

#include "common.h"
#include "list.h"
#include "savefile.h"

// decrypt types supported - add, sub, mul, xor
enum dectype {decnull=0,decadd,decsub,decmul,decrot,decxor,decxadd};

// decrypt lengths - byte,word,dword,array
enum ditemtype {decbyte=1,decword,decdword,decarray};

// decrypters are held in an array - this array is saved when a database is saved
// and the list is reapplied on loading the file.
// decrypters are held in an array - this array is saved when a database is saved
// and the list is reapplied on loading the file. structure now internal to the class
#pragma pack(push,pack_save,1)
struct declist
{  lptr dec_start;      // start address
   lptr dec_end;        // end address
   dectype typ;         // type of patch
   ditemtype dlength;   // size of patch value byte/word/dword/array
   dword value;         // patch value
   lptr arrayaddr;      // address of array if present
   bool patch;          // was the exe patched ?
   dword uid;           // unique id for ordering
};
#pragma pack(pop,pack_save)

class decrypt:public slist <declist *>
{ public:
    dword nextitemnum;
    bool loading_db;

  public:
    decrypt();
    ~decrypt();
    dword add_decrypted(lptr dstart,lptr dend,dectype t,ditemtype ditem,dword val,lptr adr,bool patchedexe);
    void process_dec(dword dec_id);
    void process_reload(dword dec_id);
    void exepatch(dword dec_id);
    bool read_item(savefile *sf);
    bool write_item(savefile *sf);
    // virtual functions from slist
    int compare(declist *i,declist *j);
};

extern decrypt decrypter;

#endif