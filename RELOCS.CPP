/************************************************************************
*                 relocs.cpp                                            *
* This class maintains a list of relocation items from the exe file.    *
* Borg keeps a list of any items which have been or should be relocated *
* when an executable is loaded. The class includes functions to         *
* maintain the list including loading and saving to a database file,    *
* and including relocating the image in memory when the database and    *
* the file are reloaded.                                                *
* The main reason for doing this is that during disassembly we can gain *
* additional information by knowing that a certain location is          *
* relocated on file load. For example with an instruction like          *
* mov eax,400000h, if the location containing 400000h is a relocation   *
* item then we know that 400000h is an offset, and so we can            *
* immediately rewrite it as mov eax, offset 400000h                     *
************************************************************************/

#include <windows.h>

#include "relocs.h"
#include "data.h"
#include "debug.h"
#include "dasm.h"


/************************************************************************
* constructor function                                                  *
************************************************************************/
relocs::relocs()
{ sizeofitem=sizeof(relocitem);
}

/************************************************************************
* destructor function                                                   *
* - currently null                                                      *
************************************************************************/
relocs::~relocs()
{
}

/************************************************************************
* addreloc                                                              *
* - adds a reloc item to the list of relocs                             *
* - a reloc item consists of only a location which is in the reloc      *
*   table, and a type for the relocation                                *
************************************************************************/
void relocs::addreloc(lptr loc,reloctype type)
{ relocitem *newname;
  newname=new relocitem;
  newname->addr=loc;
  newname->type=type;
  addto(newname);
#ifdef DEBUG
  DebugMessage("Added Reloc : %04lx:%04lx",loc.segm,loc.offs);
#endif
}

/************************************************************************
* isreloc                                                               *
* - this is the function used throughout the disassembly engine to see  *
*   if somewhere was relocated. It returns true if the location is in   *
*   the table                                                           *
************************************************************************/
bool relocs::isreloc(lptr loc)
{ relocitem checkit,*findit;
  checkit.addr=loc;
  findit=find(&checkit);
  if(findit!=NULL)
    if(findit->addr==loc)
      return true;
  return false;
}

/************************************************************************
* the compare function for the reloc items                              *
* - relocs are sorted by location                                       *
************************************************************************/
int relocs::compare(relocitem *i,relocitem *j)
{ if(i->addr==j->addr)
	 return 0;
  if(i->addr>j->addr)
	 return 1;
  return -1;
}

/************************************************************************
* relocfile                                                             *
* - this should be called after loading a database file. It goes        *
*   through all of the relocs and relocates the file again since when   *
*   we load a database we simply load the file image and do not work    *
*   our way through the whole file format again.                        *
************************************************************************/
bool relocs::relocfile(void)
{ dsegitem *ds;
  relocitem *ri;
  resetiterator();
  ri=nextiterator();
  while(ri!=NULL)
  { // relocate item.
	 ds=dta.findseg(ri->addr);
	 if(ds!=NULL)   // changed in build 14, used to return false if not found
	 { switch(ri->type)
		{ case RELOC_NONE:
			 break;
		  case RELOC_SEG:
			 ((word *)(&(ds->data[ri->addr.offs-ds->addr.offs])))[0]+=options.loadaddr.segm;
			 break;
		  case RELOC_OFFS16:
			 break;
		  case RELOC_OFFS32:
			 break;
		  case RELOC_SEGOFFS16:
			 break;
		  case RELOC_SEGOFFS32:
			 break;
		  default:
			 return false;
		}
	 }
	 ri=nextiterator();
  }
  return true;
}

/************************************************************************
* newitem                                                               *
* - returns a pointer to a new relocitem, only used by database load    *
************************************************************************/
relocitem *relocs::newitem(void)
{ return (new relocitem);
}

/************************************************************************
* write_item                                                            *
* - writes a reloc item to the savefile specified                       *
*   uses the current item, and moves the iterator on                    *
************************************************************************/
bool relocs::write_item(savefile *sf)
{ relocitem *currdec;
  currdec=nextiterator();
  if(!sf->swrite(currdec,sizeof(relocitem)))
    return false;
  return true;
}

/************************************************************************
* read_item                                                             *
* - reads a reloc item from the savefile specified                      *
************************************************************************/
bool relocs::read_item(savefile *sf)
{ dword num;
  relocitem *currdec;
  currdec=new relocitem;
  if(!sf->sread(currdec,sizeof(relocitem),&num))
    return false;
  addto(currdec);
  return true;
}

