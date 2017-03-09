/************************************************************************
*				   gname.cpp                                                *
* generic name class                                                    *
* the basic gname class consists of a name and a location, and these    *
* are the basic management functions for the class. The class is        *
* inherited by names, exports and imports which are all treated very    *
* slightly differently. These are essentially the common routines       *
* The gname class itself inherits the list class for management of the  *
* array of named locations.                                             *
************************************************************************/

#include <windows.h>

#include "gname.h"
#include "disasm.h"
#include "data.h"
#include "mainwind.h"
#include "debug.h"

/************************************************************************
* gname constructor function                                            *
************************************************************************/
gname::gname()
{
}

/************************************************************************
* gname destructor function                                             *
* - empty. the deletion of the list is carried out by the list class    *
*   and by the deletion function                                        *
************************************************************************/
gname::~gname()
{
}

/************************************************************************
* gname compare function                                                *
* - the compare function for ordering the list of names                 *
* - the names are kept in location order                                *
************************************************************************/
int gname::compare(gnameitem *i,gnameitem *j)
{ if(i->addr==j->addr)
	 return 0;
  if(i->addr>j->addr)
	 return 1;
  return -1;
}

/************************************************************************
* gname delete function                                                 *
* - overrides the stnadard gnamefunc delete function                    *
************************************************************************/
void gname::delfunc(gnameitem *i)
{ // delete name.
  delete i->name;
  // delete gnameitem.
  delete i;
}

/************************************************************************
* addname                                                               *
* - this is to add a name to the list of names.                         *
* - if the address is not covered in our list of segments then we       *
*   ignore the request                                                  *
* - we check that it is ok to name the location before naming it. This  *
*   basically ensures that names cannot be added in the middle of a     *
*   disassembled instruction, etc. It should not affect imports/exports *
*   since these will be named prior to disassembly                      *
* - the name is copied and a new string created for it, so the calling  *
*   function must delete any memory created to hold the name            *
* - if the same location is named twice then the first name is deleted  *
* - the name is added to the disassembly so that it appears in the      *
*   listing.                                                            *
* - naming a location with "" results in any name being deleted         *
************************************************************************/
void gname::addname(lptr loc,char *nm)
{ gnameitem *newname,*checkdup;
  dsegitem *l_ds;
  // check for non-existant address added v2.20
  l_ds=dta.findseg(loc);
  if(l_ds==NULL)
    return;
  if(!dsm.oktoname(loc))
    return; // check not in the middle of an instruction.
  newname=new gnameitem;
  newname->name=new char[strlen(nm)+1];
  strcpy(newname->name,nm);
  demangle(&newname->name);
  newname->addr=loc;
  checkdup=find(newname);
  // just add it once.
  if(checkdup!=NULL)
	 if(checkdup->addr==loc)
	 { delete checkdup->name;
		checkdup->name=newname->name;
		delete newname;
		dsm.delcomment(loc,dsmnameloc);
		if(strlen(checkdup->name))
		  dsm.discomment(loc,dsmnameloc,(byte *)checkdup->name);
		else
		{ delfrom(checkdup);
		}
		return;
	 }
  if(!strlen(newname->name))
  { // bugfix by Mark Ogden
	 delete newname->name;
    delete newname;
    return;
  }
  addto(newname);
  dsm.discomment(loc,dsmnameloc,(byte *)newname->name);
  // NB - no need for this anymore. Need to ensure that
  // printing of names is done as :
  // if names.isname() ...
  // else if imports.isname()...
  // else if export.isexport()...
  // so names aren't printed twice, and import names,etc are retained always
  //if(dta.findseg(loc)!=NULL)name.addname(loc,nm);
}

/************************************************************************
* isname                                                                *
* - returns true if loc has a name                                      *
************************************************************************/
bool gname::isname(lptr loc)
{ gnameitem checkit,*findit;
  checkit.addr=loc;
  findit=find(&checkit);
  if(findit!=NULL)
    if(findit->addr==loc)
      return true;
  return false;
}

/************************************************************************
* isname                                                                *
* - prints name to last buffer location in mainwindow buffering array   *
* - use with isname, for example:                                       *
*   if(name.isname(loc))name.printname(loc); etc                        *
************************************************************************/
void gname::printname(lptr loc)
{ gnameitem checkit,*findit;
  checkit.addr=loc;
  findit=find(&checkit);
  if(findit!=NULL)
    LastPrintBuff("%s",findit->name);
}

/************************************************************************
* delname                                                               *
* - used as a simple name deleter for a given location                  *
* - also deletes the name from the disassembly listing                  *
************************************************************************/
void gname::delname(lptr loc)
{ gnameitem dname,*checkdup;
  dname.addr=loc;
  checkdup=find(&dname);
  if(checkdup!=NULL)
	 if(checkdup->addr==loc)
		delfrom(checkdup);
  dsm.delcomment(loc,dsmnameloc);
}

/************************************************************************
* getoffsfromname                                                       *
* - this checks to see if a name is in the list, and if it is then it   *
*   returns the offset of its loc otherwise it returns 0. This function *
*   is used in generating the segment for imports in an NE file.        *
************************************************************************/
dword gname::getoffsfromname(char *nm)
{ gnameitem *t;
  resetiterator();
  t=nextiterator();
  while(t!=NULL)
  { if(!strcmp(t->name,nm))
      return t->addr.offs;
    t=nextiterator();
  }
  return 0;
}



