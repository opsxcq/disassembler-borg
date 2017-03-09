/************************************************************************
*                disasm.cpp                                             *
* This is a fairly big class, even having split off the disio class.    *
* The class is the class responsible for maintaining the main           *
* disassembly structures of Borg, and for performing the disassembly    *
* analysis. The class has some very old code in it, and hasn't          *
* undergone much improvement for some time, so parts could do with      *
* rewriting.                                                            *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "disasm.h"
#include "dasm.h"
#include "xref.h"
#include "data.h"
#include "gname.h"
#include "schedule.h"
#include "relocs.h"
#include "debug.h"

/************************************************************************
* constructor function                                                  *
* - resets a few class globals                                          *
************************************************************************/
disasm::disasm()
{ lastdis=NULL;
  itables=0;
  jtables=0;
  irefs=0;
}

/************************************************************************
* destructor function is currently null                                 *
************************************************************************/
disasm::~disasm()
{
}

/************************************************************************
* initnewdsm                                                            *
* - this is some common code i pulled out and put into a function on    *
*   its own. when a new disassembly item is created it initialises some *
*   of the stucture.                                                    *
************************************************************************/
void disasm::initnewdsm(dsmitem *newdsm,lptr loc,dsmitemtype typ)
{ newdsm->addr=loc;
  newdsm->type=typ;
  newdsm->override=over_null;
  newdsm->modrm=0;
  newdsm->mode32=options.mode32;
  newdsm->flags=0;
  newdsm->displayflags=0;
}

/************************************************************************
* dissettable                                                           *
* - this function sets up the processor table. It should be called once *
*   when the processor has been selected and before disassembly begins  *
************************************************************************/
void disasm::dissettable(void)
{ int i=0;
  while((procnames[i].num!=0)&&(procnames[i].num!=options.processor))
    i++;
  itable=procnames[i].tab;
}

/************************************************************************
* nextiter_code                                                         *
* - a basic building block piece of code which skips to the next        *
*   code/data item in the disassembly (identified by having a length),  *
*   or returns null if it gets to the end                               *
************************************************************************/
dsmitem *disasm::nextiter_code(dsmitem *tblock)
{ if(tblock!=NULL)
  { while(!tblock->length)
    { tblock=nextiterator();
      if(tblock==NULL)
        break;
    }
  }
  return tblock;
}

/************************************************************************
* dsmitem_contains_loc                                                  *
* - a basic building block piece of code which returns true if the      *
*   dsmitem straddles loc                                               *
* - added in v2.22                                                      *
************************************************************************/
bool disasm::dsmitem_contains_loc(dsmitem *d,lptr loc)
{ if((d->addr<loc)&&(d->addr+d->length>loc))
    return true;
  return false;
}

/************************************************************************
* dsmfindaddrtype                                                       *
* - a basic building block piece of code which returns a dsmitem        *
*   pointer using the list class find to find it by loc and type        *
* - added in v2.22                                                      *
************************************************************************/
dsmitem *disasm::dsmfindaddrtype(lptr loc,dsmitemtype typ)
{ dsmitem fnd;
  fnd.addr=loc;
  fnd.type=typ;
  return find(&fnd);
}

/************************************************************************
* oktoname                                                              *
* - this checks that we arent trying to name a location which is in the *
*   middle of an instruction and returns true if it is ok to assign a   *
*   name here                                                           *
************************************************************************/
bool disasm::oktoname(lptr loc)
{ dsmitem *checker;
  // check table for the given address
  checker=dsmfindaddrtype(loc,dsmcode);
  // NULL returned - must be ok.
  if(checker==NULL)
    return true;
  // check bounds.
  if(dsmitem_contains_loc(checker,loc))
	 return false;
  return true;
}

/************************************************************************
* checkvalid                                                            *
* - this is called when adding an instruction disassembly to the list   *
* - it checks that an instruction isnt being added which would overlap  *
*   with instructions already in the database                           *
* - it will delete any names and xrefs or comments which get in the way *
************************************************************************/
bool disasm::checkvalid(dsmitem *newdsm)
{ dsmitem *lstdsm,*deldsm;
  lstdsm=dsmfindaddrtype(newdsm->addr,dsmnull);
  if(lstdsm==NULL)
    return true;
  // go through the disassembly items nearby and check for any overlaps.
  do
  { if(lstdsm->length)
	 { if(newdsm->addr.between(lstdsm->addr,lstdsm->addr+lstdsm->length-1))
		  return false;
      if(lstdsm->addr.between(newdsm->addr,newdsm->addr+newdsm->length-1))
		  return false;
	 }
	 if(lstdsm->addr>=(newdsm->addr+newdsm->length))
      break;
	 lstdsm=nextiterator();
  } while(lstdsm!=NULL);
  deldsm=dsmfindaddrtype(newdsm->addr,dsmnull);
  // now go through them again, and this time delete any names/xrefs which get
  // in the way.
  do
  { if((!deldsm->length)&&(deldsm->addr.segm==newdsm->addr.segm))
	 { if(dsmitem_contains_loc(newdsm,deldsm->addr))
      { switch(deldsm->type)
		  { case dsmnameloc:
#ifdef DEBUG
				DebugMessage("Deleting Name : %s",deldsm->tptr);
#endif
				name.delname(deldsm->addr);
				break;
			 case dsmxref:
#ifdef DEBUG
				DebugMessage("Deleting Xref at : %04lx:%04lx",deldsm->addr.segm,deldsm->addr.offs);
#endif
				break;
			 default:
				break;
		  }
		  delfrom(deldsm);
		  deldsm=dsmfindaddrtype(newdsm->addr,dsmnull);
		  if(deldsm==NULL)
          return true;
		}
	 }
	 if(newdsm->addr+newdsm->length<=deldsm->addr)
      break;
	 deldsm=nextiterator();
  } while(lstdsm!=NULL);
  return true;
}

/************************************************************************
* setcodeoverride                                                       *
* - sets a particular override for a given location                     *
* - this subfunction was created in v211 as it was duplicated code in   *
*   several functions                                                   *
************************************************************************/
void disasm::setcodeoverride(lptr loc,byteoverride typ)
{ dsmitem *findit;
  findit=dsmfindaddrtype(loc,dsmcode);
  if(findit!=NULL)
  { if((findit->addr==loc)&&(findit->type==dsmcode))
	 {	findit->override=typ;
		dio.updatewindowifinrange(loc);
	 }
  }
}

/************************************************************************
* disargnegate                                                          *
* - as well as an override I added some displayflags for a disassembly  *
*   item. There is a negation item which allows immediates to be        *
*   negated when displayed, and this function sets or resets the flag   *
************************************************************************/
void disasm::disargnegate(lptr loc)
{ dsmitem *findit;
  findit=dsmfindaddrtype(loc,dsmcode);
  if(findit!=NULL)
  { if((findit->addr==loc)&&(findit->type==dsmcode))
	 {	findit->displayflags=(byte)(findit->displayflags^DISPFLAG_NEGATE);
		dio.updatewindowifinrange(loc);
	 }
  }
}

/************************************************************************
* disargoverdec                                                         *
* - sets the decimal override for a disassembly item, given the         *
*   location                                                            *
************************************************************************/
void disasm::disargoverdec(lptr loc)
{ setcodeoverride(loc,over_decimal);
}

/************************************************************************
* disargoversingle                                                      *
* - sets the single (float) override for a disassembly item, given the  *
*   location                                                            *
************************************************************************/
void disasm::disargoversingle(lptr loc)
{ setcodeoverride(loc,over_single);
}

/************************************************************************
* disargoverhex                                                         *
* - sets the hex (null) override for a disassembly item, given the      *
*   location                                                            *
************************************************************************/
void disasm::disargoverhex(lptr loc)
{ setcodeoverride(loc,over_null);
}

/************************************************************************
* disargoverchar                                                        *
* - sets the character override for a disassembly item, given the       *
*   location                                                            *
************************************************************************/
void disasm::disargoverchar(lptr loc)
{ setcodeoverride(loc,over_char);
}

/************************************************************************
* disargoveroffsetdseg                                                  *
* - sets the dseg override for a disassembly item, given the location   *
* - NB at present it does not affect xrefs, to be done.......           *
************************************************************************/
void disasm::disargoveroffsetdseg(lptr loc)
{ dsmitem *findit;
  lptr j;
  findit=dsmfindaddrtype(loc,dsmcode);
  if(findit!=NULL)
  { if((findit->addr==loc)&&(findit->type==dsmcode))
	 {	findit->override=over_dsoffset;
		dio.updatewindowifinrange(loc);
		if((options.mode32)&&(findit->length>=4))
		{ j.assign(options.dseg,((dword *)(&findit->data[findit->length-4]))[0]);
		  xrefs.addxref(j,loc);
		}
	 }
  }
}

/************************************************************************
* disdatastring                                                         *
* - disassembles a string at location loc.                              *
* - also names the location using the string.                           *
* - C style                                                             *
************************************************************************/
void disasm::disdatastring(lptr loc)
{ dsegitem *dblock;
  dsmitem *newdsm;
  dword maxlen,actuallen;
  char callit[GNAME_MAXLEN+1];
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return;
  actuallen=0;
  while((dblock->data+(loc-dblock->addr)+actuallen)[0])
  { actuallen++;
	 maxlen--;
	 if(!maxlen)
      return;
  }
  actuallen++;
  if(actuallen>0xffff)
    return; // tooo big
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=&asmstr[0];
  newdsm->length=(word)actuallen;              // string length
  newdsm->data=dblock->data+(loc-dblock->addr);
  if(checkvalid(newdsm))
  { callit[0]='s';
	 callit[1]='_';
	 if(actuallen>GNAME_MAXLEN-2)
	 { callit[GNAME_MAXLEN]=0;
		lstrcpyn(callit+2,(char *)newdsm->data,GNAME_MAXLEN-3);
	 }
	 else lstrcpy(callit+2,(char *)newdsm->data);
	 cleanstring(callit);
	 addto(newdsm);
	 name.addname(newdsm->addr,callit);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdataucstring                                                       *
* - disassembles a string at location loc.                              *
* - also names the location using the string.                           *
* - unicode C style                                                     *
************************************************************************/
void disasm::disdataucstring(lptr loc)
{ // unicode c-style string.
  dsegitem *dblock;
  dsmitem *newdsm;
  dword maxlen,actuallen;
  unsigned int i;
  char callit[GNAME_MAXLEN+1];
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return;
  actuallen=0;
  while((dblock->data+(loc-dblock->addr)+actuallen)[0])
  { actuallen++;
	 maxlen--;
	 if(!maxlen)
      return;
	 actuallen++;
	 maxlen--;
	 if(!maxlen)
      return;
  }
  actuallen+=2;
  if(actuallen>0xffff)
    return; // tooo big
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=&asmstr[3];
  newdsm->length=(word)actuallen;              // string length
  newdsm->data=dblock->data+(loc-dblock->addr);
  if(checkvalid(newdsm))
  { callit[0]='s';
	 callit[1]='_';
	 i=2;
	 while(i<GNAME_MAXLEN-1)
	 { callit[i]=newdsm->data[(i-2)*2];
		i++;
		if(i*2>actuallen)
        break;
	 }
	 callit[i]=0;
	 cleanstring(callit);
	 addto(newdsm);
	 name.addname(newdsm->addr,callit);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdataupstring                                                       *
* - disassembles a string at location loc.                              *
* - also names the location using the string.                           *
* - Unicode Pascal style                                                *
************************************************************************/
void disasm::disdataupstring(lptr loc)
{ // unicode pascal style string.
  dsegitem *dblock;
  dsmitem *newdsm;
  word tlen;
  dword maxlen,actuallen;
  unsigned int i;
  char callit[GNAME_MAXLEN+1];
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return;
  maxlen-=1;
  actuallen=0;
  tlen=((word *)(dblock->data+(loc-dblock->addr)))[0];
  while(tlen)
  { actuallen++;
	 tlen--;
	 maxlen--;
	 if(!maxlen)
      return;
	 actuallen++;
	 maxlen--;
	 if(!maxlen)
      return;
  }
  actuallen+=2;
  if(actuallen>0xffff)
    return; // tooo big
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=&asmstr[4];
  newdsm->length=(word)actuallen;              // string length
  newdsm->data=dblock->data+(loc-dblock->addr);
  if(checkvalid(newdsm))
  { callit[0]='s';
	 callit[1]='_';
	 i=2;
	 while(i<GNAME_MAXLEN-1)
	 { callit[i]=newdsm->data[(i-1)*2];
		i++;
		if(i*2>actuallen)
        break;
	 }
	 callit[i]=0;
	 cleanstring(callit);
	 addto(newdsm);
	 name.addname(newdsm->addr,callit);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdatadosstring                                                      *
* - disassembles a string at location loc.                              *
* - also names the location using the string.                           *
* - DOS style                                                           *
************************************************************************/
void disasm::disdatadosstring(lptr loc)
{ dsegitem *dblock;
  dsmitem *newdsm;
  dword maxlen,actuallen;
  char callit[GNAME_MAXLEN+1];
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return;
  actuallen=0;
  while((dblock->data+(loc-dblock->addr)+actuallen)[0]!='$')
  { actuallen++;
	 maxlen--;
	 if(!maxlen)
      return;
  }
  actuallen++;
  if(actuallen>0xffff)
    return; // tooo big
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=&asmstr[2];
  newdsm->length=(word)actuallen;              // string length
  newdsm->data=dblock->data+(loc-dblock->addr);
  if(checkvalid(newdsm))
  { callit[0]='s';
	 callit[1]='_';
	 callit[GNAME_MAXLEN]=0;
	 if(actuallen>GNAME_MAXLEN-2)
      lstrcpyn(callit+2,(char *)newdsm->data,GNAME_MAXLEN-3);
	 else
      lstrcpyn(callit+2,(char *)newdsm->data,actuallen);
	 cleanstring(callit);
	 addto(newdsm);
	 name.addname(newdsm->addr,callit);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdatageneralstring                                                  *
* - disassembles a string at location loc.                              *
* - also names the location using the string.                           *
* - general string is defined as printable characters                   *
************************************************************************/
void disasm::disdatageneralstring(lptr loc)
{ dsegitem *dblock;
  dsmitem *newdsm;
  dword maxlen,actuallen;
  char callit[GNAME_MAXLEN+1];
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return;
  actuallen=0;
  while(isprint((dblock->data+(loc-dblock->addr)+actuallen)[0]))
  { actuallen++;
	 maxlen--;
	 if(!maxlen)
      return;
  }
  actuallen++;
  if(actuallen>0xffff)
    return; // tooo big
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=&asmstr[2];
  newdsm->length=(word)actuallen;              // string length
  newdsm->data=dblock->data+(loc-dblock->addr);
  if(checkvalid(newdsm))
  { callit[0]='s';
	 callit[1]='_';
	 callit[GNAME_MAXLEN]=0;
	 if(actuallen>GNAME_MAXLEN-2)
      lstrcpyn(callit+2,(char *)newdsm->data,GNAME_MAXLEN-3);
	 else
      lstrcpyn(callit+2,(char *)newdsm->data,actuallen);
	 cleanstring(callit);
	 addto(newdsm);
	 name.addname(newdsm->addr,callit);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdatapstring                                                        *
* - disassembles a string at location loc.                              *
* - also names the location using the string.                           *
* - Pascal style                                                        *
************************************************************************/
void disasm::disdatapstring(lptr loc)
{ dsegitem *dblock;
  dsmitem *newdsm;
  byte tlen;
  dword maxlen,actuallen;
  char callit[GNAME_MAXLEN+1];
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return;
  actuallen=0;
  tlen=(dblock->data+(loc-dblock->addr))[0];
  while(tlen)
  { actuallen++;
	 tlen--;
	 maxlen--;
	 if(!maxlen)
      return;
  }
  actuallen++;
  if(actuallen>0xffff)
    return; // tooo big
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=&asmstr[1];
  newdsm->length=(word)actuallen;              // string length
  newdsm->data=dblock->data+(loc-dblock->addr);
  if(checkvalid(newdsm))
  { callit[0]='s';
	 callit[1]='_';
	 if(actuallen>GNAME_MAXLEN-2)
	 { callit[GNAME_MAXLEN]=0;
		lstrcpyn(callit+2,(char *)newdsm->data+1,GNAME_MAXLEN-3);
	 }
	 else
	 { callit[actuallen+2]=0;
		lstrcpyn(callit+2,(char *)newdsm->data+1,actuallen);
	 }
	 cleanstring(callit);
	 addto(newdsm);
	 name.addname(newdsm->addr,callit);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdata                                                               *
* - disassembles a dataitem, the common parts of the routines which     *
*   disassemble particular types (words,dwords, etc)                    *
************************************************************************/
void disasm::disdata(lptr loc,asminstdata *asmwd,byte len,byteoverride overr)
{ dsegitem *dblock;
  dsmitem *newdsm;
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmcode);
  newdsm->tptr=asmwd;
  newdsm->length=len;
  newdsm->data=dblock->data+(loc-dblock->addr);
  newdsm->override=overr;
  if(checkvalid(newdsm))
  { addto(newdsm);
	 //check if need to update window.
	 dio.updatewindowifinrange(loc);
  }
  else
	 delete newdsm;
}

/************************************************************************
* disdataword                                                           *
* - disassembles a word at location loc.                                *
************************************************************************/
void disasm::disdataword(lptr loc)
{ disdata(loc,&asmword[0],2,over_null);
}

/************************************************************************
* disdatadword                                                          *
* - disassembles a dword at location loc.                                *
************************************************************************/
void disasm::disdatadword(lptr loc)
{ disdata(loc,&asmdword[0],4,over_null);
}

/************************************************************************
* disdatasingle                                                         *
* - disassembles a single (float) at location loc.                      *
************************************************************************/
void disasm::disdatasingle(lptr loc)
{ disdata(loc,&asm_fp[0],4,over_null);
}

/************************************************************************
* disdatadouble                                                         *
* - disassembles a double (float) at location loc.                      *
************************************************************************/
void disasm::disdatadouble(lptr loc)
{ disdata(loc,&asm_fp[1],8,over_null);
}

/************************************************************************
* disdatalongdouble                                                     *
* - disassembles a long double at location loc.                         *
************************************************************************/
void disasm::disdatalongdouble(lptr loc)
{ disdata(loc,&asm_fp[2],10,over_null);
}

/************************************************************************
* disdatadsoffword                                                      *
* - disassembles an offset data item stored at a location               *
* - short for disdatadword and changing it to an offset,ie dword offset *
************************************************************************/
void disasm::disdatadsoffword(lptr loc)
{ disdata(loc,&asmdword[0],4,over_dsoffset);
}

/************************************************************************
* addcomment                                                            *
* - called to add an auto comment to the disassembly, where the auto    *
*   comment will be edittable by the user. this is used in resource     *
*   analysis.                                                           *
************************************************************************/
void disasm::addcomment(lptr loc,char *comment)
{ char *nm;
  nm=new char[strlen(comment)+1];
  strcpy(nm,comment);
  disautocomment(loc,dsmcomment,(unsigned char *)nm);
}

/************************************************************************
* disname_or_ordinal                                                    *
* - part of the resource item analysis, this looks for either an        *
*   ordinal number, or a name, and disassembles the data adding auto    *
*   comments too.                                                       *
************************************************************************/
int disasm::disname_or_ordinal(lptr loc,bool comment_ctrl)
{ dsegitem *dblock;
  dword maxlen,idnum;
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return 0;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<2)
    return 0;
  idnum=((word *)(dblock->data+(loc-dblock->addr)))[0];
  if(idnum==0xffff) // ordinal follows
  { disdataword(loc);
    idnum=((word *)(dblock->data+(loc-dblock->addr)))[1];
    disdataword(loc+2);
    // ctrl class -> add description for class
    if(comment_ctrl)
    { switch(idnum)
      { case 0x0080:
          addcomment(loc+2,"[Button]");
          break;
        case 0x0081:
          addcomment(loc+2,"[Edit]");
          break;
        case 0x0082:
          addcomment(loc+2,"[Static]");
          break;
        case 0x0083:
          addcomment(loc+2,"[List Box]");
          break;
        case 0x0084:
          addcomment(loc+2,"[Scroll Bar]");
          break;
        case 0x0085:
          addcomment(loc+2,"[Combo Box]");
          break;
        default:
          break;
      }
    }
    return 4;
  }
  disdataucstring(loc);
  return getlength(loc);
}

/************************************************************************
* disdialog                                                             *
* - disassembles a dialog resource, by which I mean that data items are *
*   disassembled to dwords, strings, etc, and the whole lot is          *
*   commented with what the fields are                                  *
* - at the moment basename is unused, for future refinement ?           *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
void disasm::disdialog(lptr loc,char *basename)
{ lptr cloc;
  int ilen,numctrls;
  dsmitem *findit;
  bool exd;
  dsegitem *dblock;
  dword maxlen,idnum,hdrstyle;
  // ho hum, things are never simple
  // - after adding the dialog format i found that some dialogs were just completely
  // different - the so called dialogex dialogs, and after much hunting around i found
  // some details on microsofts site (wow). so then i hacked the code up to do it....
  // dialog header
  exd=false;
  dblock=dta.findseg(loc);
  if(dblock==NULL)
    return;
  maxlen=dblock->size-(loc-dblock->addr);
  if(maxlen<4)
    return;
  idnum=((dword *)(dblock->data+(loc-dblock->addr)))[0];
  if(idnum==0xffff0001)
    exd=true; // whoah, dialogex found
  /* basic struct is as follows:
  struct dialogboxheader
  { unsigned long style,extendedstyle;
    unsigned short numberofitems;
    unsigned short x,y;
    unsigned short cx,cy;
  };*/
  if(exd)
  { addcomment(loc,"Signature+Version");
    disdatadword(loc);
    addcomment(loc+4,"HelpID");
    disdatadword(loc+4);
    loc=loc+8;
  }
  if(exd)
    addcomment(loc,"Extended Style");
  else
    addcomment(loc,"Style");
  disdatadword(loc);
  if(exd)
    addcomment(loc+4,"Style");
  else
    addcomment(loc+4,"Extended Style");
  disdatadword(loc+4);
  addcomment(loc+8,"Number of Items");
  disdataword(loc+8);
  addcomment(loc+10,"x");
  disdataword(loc+10);
  addcomment(loc+12,"y");
  disdataword(loc+12);
  addcomment(loc+14,"cx");
  disdataword(loc+14);
  addcomment(loc+16,"cy");
  disdataword(loc+16);
  cloc=loc+18;
  addcomment(cloc,"Menu name/ordinal");
  ilen=disname_or_ordinal(cloc,false);
  cloc+=ilen;
  addcomment(cloc,"Class name/ordinal");
  ilen=disname_or_ordinal(cloc,false);
  cloc+=ilen;
  addcomment(cloc,"Caption");
  disdataucstring(cloc);
  cloc+=getlength(cloc);
  findit=dsmfindaddrtype(loc+4*(exd?1:0),dsmcode);
  hdrstyle=0;
  if(findit!=NULL)
  { if((findit->addr==loc+4*(exd?1:0))&&(findit->type==dsmcode))
      hdrstyle=((dword *)findit->data)[0];
  }
  // i noticed a reference to DS_SHELLFONT on msdn, but what is that ????????
  if (hdrstyle&DS_SETFONT)  // if ds_setfont then 2 more items...in theory
  { addcomment(cloc,"font pointsize");
    disdataword(cloc);
    cloc+=2;
    if(exd) // more options with exd
    { addcomment(cloc,"weight");
      disdataword(cloc);
      cloc+=2;
      addcomment(cloc,"italic");
      //disdatabyte(cloc);
      cloc+=1;
      addcomment(cloc,"charset");
      //disdatabyte(cloc);
      cloc+=1;
    }
    addcomment(cloc,"font");
    disdataucstring(cloc);
    cloc+=getlength(cloc);
  }
  // now do controls
  findit=dsmfindaddrtype(loc+8,dsmcode);
  numctrls=0;
  if(findit!=NULL)
  { if((findit->addr==loc+8)&&(findit->type==dsmcode))
      numctrls=((word *)findit->data)[0];
  }
  /*struct ctrlheader
  { unsigned long style,extendedstyle;
    unsigned short x,y;
    unsigned short cx,cy;
    unsigned short wid;
  };*/
  while(numctrls)
  { if(cloc.offs&0x03)
      cloc.offs=(cloc.offs|0x03)+1;
    if(exd)
    { addcomment(cloc,"HelpID");
      disdatadword(cloc);
      cloc+=4;
    }
    if(exd)
      addcomment(cloc,"Extended Style");
    else
      addcomment(cloc,"Style");
    disdatadword(cloc);
    if(exd)
      addcomment(cloc+4,"Style");
    else
      addcomment(cloc+4,"Extended Style");
    disdatadword(cloc+4);
    addcomment(cloc+8,"x");
    disdataword(cloc+8);
    addcomment(cloc+10,"y");
    disdataword(cloc+10);
    addcomment(cloc+12,"cx");
    disdataword(cloc+12);
    addcomment(cloc+14,"cy");
    disdataword(cloc+14);
    addcomment(cloc+16,"wid");
    disdataword(cloc+16);
    cloc+=18;
    if(exd)
      if(cloc.offs&0x03)
        cloc.offs=(cloc.offs|0x03)+1;
    addcomment(cloc,"Class id");
    ilen=disname_or_ordinal(cloc,true);
    cloc+=ilen;
    addcomment(cloc,"Text");
    ilen=disname_or_ordinal(cloc,false);
    cloc+=ilen;
    addcomment(cloc,"Extra Stuff");
    disdataword(cloc);
    findit=dsmfindaddrtype(cloc,dsmcode);
    if(findit!=NULL)
    { if((findit->addr==cloc)&&(findit->type==dsmcode))
        cloc+=((word *)findit->data)[0];
    }
    cloc+=2;
    numctrls--;
  }
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* disstringtable                                                        *
* - disassembles a string table, by which I mean that the strings are   *
*   disassembled into strings and the locations are named according to  *
*   the strings id numbers (I chose this in preference to naming by     *
*   string because they are a resource, and so the locations will not   *
*   be referenced, but if the program wants to use them then it will    *
*   use the id numbers, and make an API call to get hold of them).      *
************************************************************************/
void disasm::disstringtable(lptr loc,char *basename)
{ int i;
  lptr cloc;
  char callit[40];
  dword idnum;
  int ipt;
  dsmitem *lastdsm;
  ipt=0;
  idnum=0;
  if(basename!=NULL)
  { while((basename[ipt])&&(basename[ipt]!=':'))
      ipt++;
    if(basename[ipt]==':')
    { ipt++;
      while(basename[ipt])
      { if(basename[ipt]>='a')
          idnum=idnum*16+basename[ipt]-'a'+10;
        else
          idnum=idnum*16+basename[ipt]-'0';
        ipt++;
        if(basename[ipt]=='h')
          break;
      }
    }
  }
  cloc=loc;
  for(i=0;i<16;i++)  // 16 strings in a stringtable, of type unicode_pascal.
  { disdataupstring(cloc);
    if(idnum)
    { wsprintf(callit,"String_ID_%lx",(idnum-1)*16+i);
      name.addname(cloc,callit);
    }
    lastdsm=dsmfindaddrtype(cloc,dsmcode);
    if(lastdsm==NULL)
      break;
    if((lastdsm->type!=dsmcode)||(lastdsm->addr!=cloc))
      break;
    cloc+=lastdsm->length;
  }
}

/************************************************************************
* codeseek                                                              *
* - this is the aggressive code search if it is chosen as an option. It *
*   has a low priority, and so appears near the end of the queue. It    *
*   hunts for possible code to disassemble in the code segments (added  *
*   as a task during file load for each code segment). When it finds an *
*   undisassembled byte it tries to disassemble from that point, and    *
*   drops into the background again until disassembly has been done     *
* - note that if it doesnt find anything in a short time it exits and   *
*   puts a continuation request in to the scheduler, this ensures that  *
*   userrequests are answered quickly                                   *
************************************************************************/
void disasm::codeseek(lptr loc)
{ bool doneblock;
  dsegitem *dblock;
  dsmitem *tblock;
  int dcount,ilength;
  // check if already done.
  dblock=dta.findseg(loc);      // find segment item.
  dcount=0;
  if(dblock==NULL)
    return;
  do{
    doneblock=false;
    ilength=1;
    dsmfindaddrtype(loc,dsmnull);
    tblock=nextiterator();
    tblock=nextiter_code(tblock);
    if(tblock!=NULL)
	 { if((tblock->addr.segm==loc.segm)&&(dsmitem_contains_loc(tblock,loc)))
		  doneblock=true;
      while(tblock->addr==loc)
	   { if(tblock->length)
          ilength=tblock->length;
    	  if(tblock->type!=dsmcomment)
	     { doneblock=true;
	 	    break;
        }
		  tblock=nextiterator();
		  if(tblock==NULL)
          break;
      }
    }
    if(doneblock)
    { loc+=ilength;
      dcount++;
      if(loc>=(dblock->addr+dblock->size))
        return;
      if(dcount>1000)
      {	// dont forget the main thread!
        scheduler.addtask(seek_code,priority_continuation,loc,NULL);
        return;
      }
    }
  } while (doneblock);
  // decode it.
  scheduler.addtask(seek_code,priority_aggressivesearch,loc+1,NULL);
  scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
}

/************************************************************************
* disexportblock                                                        *
* - this disassembles a block of code, from an export address, but only *
*   if the export address is in a code segment                          *
************************************************************************/
void disasm::disexportblock(lptr loc)
{  dsegitem *dblock;
   dblock=dta.findseg(loc);
   if(dblock==NULL)
     return;
   if(dblock->typ==code32)
   	disblock(loc);
}

/************************************************************************
* disblock                                                              *
* - disassembles a block from the starting point loc, calling           *
*   decodeinst for each instruction.                                    *
* - we dont just keep going in here, but only disassemble a few         *
*   instructions then ask for a continuation and quit back to the       *
*   scheduler for userrequests to be processed. If Borg seems to slow   *
*   on your machine when you scroll around and it is still analysing    *
*   then try dropping the number of instructions to disassemble and     *
*   window updates should occur more often.                             *
* - If Borg cannot disassemble for whatever reason, or if a ret or jmp  *
*   type instruction is reached then we finish.                         *
************************************************************************/
void disasm::disblock(lptr loc)
{ byte ibuff[20];                     // ibuff is the disassembly buffer - m/c is moved here
												  // for disassembly - no checking of eof is then needed
												  // until after the code is identified.
  dsegitem *dblock;
  int i,disasmcount;
  bool doneblock;
  dsmitem *tblock;
  dword aaddr;
  byte *mcode;
  asminstdata *codefound;

  dblock=dta.findseg(loc);      // find segment item.
  disasmcount=0;
  if(dblock==NULL)
    return;
  if(dblock->typ==uninitdata)
    return;
  if(loc>=dblock->addr+dblock->size)
    return;
  doneblock=false;
  while(!doneblock)
  { // don't spend too long in here
	 disasmcount++;
	 if(disasmcount>50)
	 { scheduler.addtask(dis_code,priority_continuation,loc,NULL);
		return;
	 }
	 memset(ibuff,0,20);
	 // check if already done.
	 dsmfindaddrtype(loc,dsmnull);
	 tblock=nextiterator();
    tblock=nextiter_code(tblock);
	 if(tblock!=NULL)
	 {	if((tblock->addr.segm==loc.segm)&&(dsmitem_contains_loc(tblock,loc)))
		  doneblock=true;
	 	while(tblock->addr==loc)
		{ if(tblock->type!=dsmcomment)
		  { doneblock=true;
			 break;
		  }
		  tblock=nextiterator();
		  if(tblock==NULL)
          break;
		}
	 }
	 // decode it.
	 if(doneblock)
      break;
	 aaddr=loc-dblock->addr;
	 mcode=dblock->data+aaddr;
	 i=0;
	 while((i<15)&&(aaddr<dblock->size))
	 { ibuff[i]=mcode[i];
		i++;
		aaddr++;
	 }
	 tblock=decodeinst(ibuff,mcode,loc,TABLE_MAIN,options.mode32,0);
	 if(tblock!=NULL)
    { if(!checkvalid(tblock))
	   { delete tblock;
	     tblock=NULL;
	   }
    }
	 if(tblock!=NULL)
	 {	addto(tblock);
		//check if need to update window.
		dio.updatewindowifinrange(loc);
      loc.offs+=tblock->length;
		if((loc-dblock->addr)>dblock->size)
		{ doneblock=true;
		  delfrom(tblock);
		}
	 }
	 else
      doneblock=true;
	 if(doneblock)
      break;
	 // check if end (jmp,ret,etc)
	 codefound=(asminstdata *)tblock->tptr;
	 if((codefound->flags&FLAGS_JMP)||(codefound->flags&FLAGS_RET))
		doneblock=true;
  }
}

/************************************************************************
* decodeinst                                                            *
* - disassembles one instruction                                        *
* - in some ways this is the single most important function in Borg. It *
*   disassembles an instruction adding a disassembled item to the       *
*   database. It uses the options we have set, and the processor tables *
*   identified.                                                         *
* - If some kind of call or conditional jump is reached then Borg       *
*   just adds another disassembly task to the scheduler to look at      *
*   later, and then carries on.                                         *
* - Note that this function is recursive, for handling some complex     *
*   x86 overrides, note that the recursion depth is limited but Borg    *
*   should handle complex prefix byte sequences and 'double sequences'  *
* - The majority of the code here is some of the oldest code in Borg,   *
*   and probably some of the most complex.                              *
* - xreffing and windowupdates are performed from here                  *
************************************************************************/
dsmitem *disasm::decodeinst(byte *ibuff,byte *mcode,lptr loc,byte tabtype,bool omode32,int depth)
{ int tablenum=0;             // asmtable table number
  int instnum;                // instruction num in table
  asminstdata *insttab;
  dsmitem *newdsm;
  dword flgs;  					// inst flags
  argtype a1,a2,a3;           // inst args
  lptr j;                     // jump/call target
  byte *dta;
  byte length;
  gnameitem *imp;
  char impname[GNAME_MAXLEN+1];
  bool righttable;
  byte cbyte,mbyte;
  bool fupbyte;
  cbyte=ibuff[0];
  if(tabtype==TABLE_EXT)
    cbyte=ibuff[1];
  if(tabtype==TABLE_EXT2)
    cbyte=ibuff[2];
  if((tabtype==TABLE_EXT2)&&(options.processor==PROC_Z80))
    cbyte=ibuff[3];
  while(itable[tablenum].table!=NULL)  // search tables
  { righttable=true;
	 if((itable[tablenum].type!=tabtype)||(itable[tablenum].minlim>cbyte)||
		(itable[tablenum].maxlim<cbyte)) righttable=false;
	 if(((tabtype==TABLE_EXT)||(tabtype==TABLE_EXT2))&&(ibuff[0]!=itable[tablenum].extnum))
		righttable=false;
	 if((tabtype==TABLE_EXT2)&&(ibuff[1]!=itable[tablenum].extnum2))
		righttable=false;
	 if(righttable)
	 { insttab=itable[tablenum].table;      // need to search this now
		instnum=0;
		mbyte=cbyte;
		fupbyte=false;
		if(itable[tablenum].divisor)
        mbyte=(byte)(mbyte/itable[tablenum].divisor);
		if(itable[tablenum].mask)
        mbyte=mbyte&itable[tablenum].mask;
		else      // follow up byte encodings (KNI,AMD3DNOW)
		{ fupbyte=true;
		  flgs=insttab[instnum].flags;
		  a1=insttab[instnum].arg1;
		  a2=insttab[instnum].arg2;
		  a3=insttab[instnum].arg3;
#ifdef __BORLANDC__
#pragma warn -sig
#endif
		  length=1+arglength(a1,mcode[1+itable[tablenum].modrmpos],mcode[2+itable[tablenum].modrmpos],flgs,true)
					+arglength(a2,mcode[1+itable[tablenum].modrmpos],mcode[2+itable[tablenum].modrmpos],flgs,true)
					+arglength(a3,mcode[1+itable[tablenum].modrmpos],mcode[2+itable[tablenum].modrmpos],flgs,true)
					+itable[tablenum].modrmpos;
#ifdef __BORLANDC__
#pragma warn +sig
#endif
		  // addition for table extensions where inst is part of modrm byte
		  if(((tabtype==TABLE_EXT)||(tabtype==TABLE_EXT2))&&(length==1))
          length++;
		  mbyte=ibuff[length];
		}
		while((insttab[instnum].name!=NULL)||(insttab[instnum].instbyte)||(insttab[instnum].processor))
		{ if((omode32)&&(insttab[instnum].flags&FLAGS_OMODE16));
		  else if((!omode32)&&(insttab[instnum].flags&FLAGS_OMODE32));
		  else if((insttab[instnum].instbyte==mbyte)&&(insttab[instnum].processor&options.processor))
		  { // found it
			 if(insttab[instnum].name==NULL)
			 { if(tabtype==TABLE_MAIN)
              return decodeinst(ibuff,mcode,loc,TABLE_EXT,omode32,5);
				else
              return decodeinst(ibuff,mcode,loc,TABLE_EXT2,omode32,5);
			 }
			 else
			 { // interpret flags,etc
				flgs=insttab[instnum].flags;
				if((flgs&FLAGS_OPERPREFIX)&&(depth<5))
				{ newdsm=decodeinst(ibuff+1,mcode+1,loc+1,tabtype,!omode32,depth+1);
				  if(newdsm==NULL)
                return NULL;
				  newdsm->addr.offs--;
				  newdsm->length++;
				  newdsm->modrm++;
				  newdsm->data--;
				  return newdsm;
				}
				if((flgs&FLAGS_ADDRPREFIX)&&(depth<5))
				{ options.mode32=!options.mode32;
				  options.mode16=!options.mode16;
				  newdsm=decodeinst(ibuff+1,mcode+1,loc+1,tabtype,omode32,depth+1);
				  options.mode32=!options.mode32;
				  options.mode16=!options.mode16;
				  if(newdsm==NULL)
                return NULL;
				  newdsm->addr.offs--;
				  newdsm->length++;
				  newdsm->modrm++;
				  newdsm->data--;
				  newdsm->flags=newdsm->flags|FLAGS_ADDRPREFIX;
				  return newdsm;
				}
				if((flgs&FLAGS_SEGPREFIX)&&(depth<5))
				{ newdsm=decodeinst(ibuff+1,mcode+1,loc+1,tabtype,omode32,depth+1);
				  if(newdsm==NULL)
                return NULL;
				  newdsm->addr.offs--;
				  newdsm->length++;
				  newdsm->modrm++;
				  newdsm->data--;
				  newdsm->flags=newdsm->flags|FLAGS_SEGPREFIX;
				  return newdsm;
				}
				if((flgs&FLAGS_PREFIX)&&(depth<5))
				{ newdsm=decodeinst(ibuff+1,mcode+1,loc+1,tabtype,omode32,depth+1);
				  if(newdsm==NULL)
                return NULL;
				  newdsm->addr.offs--;
				  newdsm->length++;
				  newdsm->modrm++;
				  newdsm->data--;
				  newdsm->flags=newdsm->flags|FLAGS_PREFIX;
				  return newdsm;
				}
				newdsm=new dsmitem;
            initnewdsm(newdsm,loc,dsmcode);
				newdsm->tptr=(void *)&insttab[instnum];
				newdsm->modrm=(byte)(1+itable[tablenum].modrmpos);
				newdsm->data=mcode;
				newdsm->mode32=omode32;
				if(flgs&FLAGS_16BIT)
              newdsm->mode32=false;
				if(flgs&FLAGS_32BIT)
              newdsm->mode32=true;
				newdsm->flags=flgs;
				a1=insttab[instnum].arg1;
				a2=insttab[instnum].arg2;
				a3=insttab[instnum].arg3;
#ifdef __BORLANDC__
#pragma warn -sig
#endif
				length=1+arglength(a1,mcode[1+itable[tablenum].modrmpos],mcode[2+itable[tablenum].modrmpos],flgs,newdsm->mode32)
					+arglength(a2,mcode[1+itable[tablenum].modrmpos],mcode[2+itable[tablenum].modrmpos],flgs,newdsm->mode32)
					+arglength(a3,mcode[1+itable[tablenum].modrmpos],mcode[2+itable[tablenum].modrmpos],flgs,newdsm->mode32)
					+itable[tablenum].modrmpos;
#ifdef __BORLANDC__
#pragma warn +sig
#endif
				// addition for table extensions where inst is part of modrm byte
				if(((tabtype==TABLE_EXT)||(tabtype==TABLE_EXT2))&&((length==1)||(options.processor==PROC_Z80)))
				  length++;
				if(options.processor==PROC_Z80)
				{ if(tabtype==TABLE_EXT2)
                length++;
				  if(flgs&FLAGS_INDEXREG)
                length++;
				}
				if(fupbyte)
              length++;
				newdsm->length=length;
				if(!checkvalid(newdsm))
				{ delete newdsm;
				  return NULL;
				}
				if(flgs&(FLAGS_JMP|FLAGS_CALL|FLAGS_CJMP))
				{ switch(a1)
				  { case ARG_RELIMM:
						j=loc;
						dta=mcode+length;
						if(options.mode32)
						{ dta-=4;
						  j+=((dword *)dta)[0]+length;
						}
						else
						{ dta-=2;
						  j+=(word)(((word *)dta)[0]+length);
						}
						scheduler.addtask(dis_code,priority_definitecode,j,NULL);
						xrefs.addxref(j,newdsm->addr);
						break;
					 case ARG_RELIMM8:
						j=loc;
						dta=mcode+length-1;
						if(options.mode32)
						{ if(dta[0]&0x80)
							 j+=(dword)(dta[0]+0xffffff00+length);
						  else
							 j+=(dword)(dta[0]+length);
						}
						else
						{ if(dta[0]&0x80)
							 j+=(word)(dta[0]+0xff00+length);
						  else
							 j+=(word)(dta[0]+length);
						}
						scheduler.addtask(dis_code,priority_definitecode,j,NULL);
						xrefs.addxref(j,newdsm->addr);
						break;
					 case ARG_FADDR:
						dta=mcode+length;
						if(options.mode32)
						{ dta-=6;
						  j.assign(((word *)(&dta[4]))[0],((dword *)(&dta[0]))[0]);
						}
						else
						{ dta-=4;
						  j.assign(((word *)(&dta[2]))[0],((word *)(&dta[0]))[0]);
						}
						scheduler.addtask(dis_code,priority_definitecode,j,NULL);
						xrefs.addxref(j,newdsm->addr);
						break;
					 case ARG_MODRM:
					 case ARG_MODRM_FPTR:
						if(options.mode32)
						{ if((newdsm->data[0]==0xff)&&(newdsm->data[1]==0x25)&&(tabtype==TABLE_EXT))
						  { j.assign(loc.segm,((dword *)(&newdsm->data[2]))[0]);
							 if(import.isname(j))
							 { imp=import.nextiterator();
								impname[0]='_';
								impname[GNAME_MAXLEN]=0;
								lstrcpyn(&impname[1],imp->name,GNAME_MAXLEN-2);
								//if(!name.isname(segm,offs))
								scheduler.addtask(namecurloc,priority_nameloc,loc,impname);
							 }
						  }
						}
						scheduler.addtask(dis_jumptable,priority_definitecode,loc,NULL);
						break;
					 default:
						break;
				  }
				}
				switch(a1)
				{ case ARG_IMM32:
					 if(reloc.isreloc(loc+length-4))
					 {	newdsm->override=over_dsoffset;
						dta=mcode+length-4;
						j.assign(options.dseg,((dword *)(&dta[0]))[0]);
						xrefs.addxref(j,newdsm->addr);
					 }
					 break;
				  case ARG_IMM:
					 if(options.mode32)
					 { if(reloc.isreloc(loc+length-4))
						{ newdsm->override=over_dsoffset;
						  dta=mcode+length-4;
						  j.assign(options.dseg,((dword *)(&dta[0]))[0]);
						  xrefs.addxref(j,newdsm->addr);
						}
					 }
					 break;
              case ARG_MEMLOC:
                if(options.mode32)
                { newdsm->override=over_dsoffset;
						dta=mcode+length-4;
						j.assign(options.dseg,((dword *)(&dta[0]))[0]);
                  xrefs.addxref(j,newdsm->addr);
                }
                break;
	 			  case ARG_MMXMODRM:
	 			  case ARG_XMMMODRM:
	 			  case ARG_MODRM_S:
	 			  case ARG_MODRMM512:
	 			  case ARG_MODRMQ:
	 			  case ARG_MODRM_SREAL:
	 			  case ARG_MODRM_PTR:
	 			  case ARG_MODRM_WORD:
	 			  case ARG_MODRM_SINT:
	 			  case ARG_MODRM_EREAL:
		 		  case ARG_MODRM_DREAL:
	 			  case ARG_MODRM_WINT:
	 			  case ARG_MODRM_LINT:
	 			  case ARG_MODRM_BCD:
	 			  case ARG_MODRM_FPTR:
	 			  case ARG_MODRM:
                if(options.mode32)
                	if((newdsm->data[newdsm->modrm]&0xc7)==5) // straight disp32
                	{ dta=mcode+newdsm->modrm+1;
						  j.assign(options.dseg,((dword *)(&dta[0]))[0]);
                    xrefs.addxref(j,newdsm->addr);
                	}
                break;
				  default:
					 break;
				}
				switch(a2)
				{ case ARG_IMM32:
					 if(reloc.isreloc(loc+length-4))
					 {	newdsm->override=over_dsoffset;
						dta=mcode+length-4;
						j.assign(options.dseg,((dword *)(&dta[0]))[0]);
						xrefs.addxref(j,newdsm->addr);
					 }
					 break;
				  case ARG_IMM:
					 if(options.mode32)
					 { if(reloc.isreloc(loc+length-4))
						{ newdsm->override=over_dsoffset;
						  dta=mcode+length-4;
						  j.assign(options.dseg,((dword *)(&dta[0]))[0]);
						  xrefs.addxref(j,newdsm->addr);
						}
					 }
					 break;
              case ARG_MEMLOC:
                if(options.mode32)
                { newdsm->override=over_dsoffset;
						dta=mcode+length-4;
						j.assign(options.dseg,((dword *)(&dta[0]))[0]);
                  xrefs.addxref(j,newdsm->addr);
                }
                break;
	 			  case ARG_MMXMODRM:
	 			  case ARG_XMMMODRM:
	 			  case ARG_MODRM_S:
	 			  case ARG_MODRMM512:
	 			  case ARG_MODRMQ:
	 			  case ARG_MODRM_SREAL:
	 			  case ARG_MODRM_PTR:
	 			  case ARG_MODRM_WORD:
	 			  case ARG_MODRM_SINT:
	 			  case ARG_MODRM_EREAL:
		 		  case ARG_MODRM_DREAL:
	 			  case ARG_MODRM_WINT:
	 			  case ARG_MODRM_LINT:
	 			  case ARG_MODRM_BCD:
	 			  case ARG_MODRM_FPTR:
	 			  case ARG_MODRM:
                if(options.mode32)
                	if((newdsm->data[newdsm->modrm]&0xc7)==5) // straight disp32
                	{ dta=mcode+newdsm->modrm+1;
						  j.assign(options.dseg,((dword *)(&dta[0]))[0]);
                    xrefs.addxref(j,newdsm->addr);
                	}
                break;
				  default:
					 break;
				}
				return newdsm;
			 }
		  }
		  instnum++;
		}
	 }
	 tablenum++;
  }
  return NULL;
}

/************************************************************************
* arglength                                                             *
* - a function which returns the increase in length of an instruction   *
*   due to its argtype, used by the decodeinst engine in calculating    *
*   the instruction length                                              *
************************************************************************/
byte disasm::arglength(argtype a,byte modrmbyte,byte sibbyte,dword flgs,bool omode32)
{ byte rm;
  switch(a)
  { case ARG_IMM:
		if(flgs&FLAGS_8BIT)
        return 1;
		if(!omode32)
        return 2;
		else
        return 4;
	 case ARG_NONEBYTE:
		return 1;
	 case ARG_RELIMM:
	 case ARG_MEMLOC:
		if(options.mode16)
        return 2;
		else
        return 4;
	 case ARG_RELIMM8:
	 case ARG_SIMM8:
	 case ARG_IMM8:
	 case ARG_IMM8_IND:
		return 1;
	 case ARG_IMM32:
		return 4;
	 case ARG_IMM16_A:
	 case ARG_IMM16:
	 case ARG_MEMLOC16:
		return 2;
	 case ARG_FADDR:
		if(options.mode16)
        return 4;
		else
        return 6;
	 case ARG_MODREG:
	 case ARG_MMXMODRM:
	 case ARG_XMMMODRM:
	 case ARG_MODRM8:
	 case ARG_MODRM16:
	 case ARG_MODRM_S:
	 case ARG_MODRMM512:
	 case ARG_MODRMQ:
	 case ARG_MODRM_SREAL:
	 case ARG_MODRM_PTR:
	 case ARG_MODRM_WORD:
	 case ARG_MODRM_SINT:
	 case ARG_MODRM_EREAL:
	 case ARG_MODRM_DREAL:
	 case ARG_MODRM_WINT:
	 case ARG_MODRM_LINT:
	 case ARG_MODRM_BCD:
	 case ARG_MODRM_FPTR:
	 case ARG_MODRM:
		rm=(byte)((modrmbyte&0xc0)>>6);
		switch(rm)
		{ case 0:
			 if(options.mode32)
			 { if((modrmbyte&0x07)==5)
				  return 5; // disp32
				if((modrmbyte&0x07)==4)
				{ if((sibbyte&0x07)==5)
					 return 6;
				  return 2; //sib byte - need to check if r=5 also.
				}
			 }
			 else
            if((modrmbyte&0x07)==6)
              return 3;
			 break;
		  case 1:
			 if(options.mode32)
			 { if((modrmbyte&0x07)==4)
				{ return 3; //sib byte
				}
			 }
			 return 2; // disp8
		  case 2:
			 if(options.mode32)
			 { if((modrmbyte&0x07)==4)
				  return 6; //sib byte
				return 5; // disp32
			 }
			 else
            return 3; // disp16
		  case 3:
			 return 1;
		}
		return 1;
	 default:
		break;
  }
  return 0;
}

/************************************************************************
* compare function                                                      *
* - the compare function for the list of disassembled instructions.     *
* - the disassembled instructions are kept in order using location, and *
*   type where type indicates instruction, comment, segheader line, etc *
*   as these are kept in the database of disassembled instructions. The *
*   window that the user sees into the disassembly is simply a view of  *
*   this database with one line per record.                             *
************************************************************************/
int disasm::compare(dsmitem *i,dsmitem *j)
{ if(i->addr==j->addr)
  { if(i->type==j->type)
		return 0;
	 else if(i->type>j->type)
		return 1;
	 return -1;
  }
  if(i->addr>j->addr)
	 return 1;
  return -1;
}

/************************************************************************
* deletion function                                                     *
* - in deleting the database we delete any comments that may be         *
*   attached, overrides the standard list item delete function          *
************************************************************************/
//deletion function for list
void disasm::delfunc(dsmitem *i)
{ // bugfix by Mark Ogden - added dsmnameloc
  if((i->type!=dsmcode)&&(i->type!=dsmnameloc))
	 if(i->tptr!=NULL)
      delete i->tptr;
  delete i;
}

/************************************************************************
* undefineline                                                          *
* - this simply deletes any code item in the disassembly database, for  *
*   the users current line in the database/window                       *
************************************************************************/
void disasm::undefineline(void)
{ dsmitem *tblock;
  lptr outhere;
  tblock=dio.findcurrentline();
  if(tblock==NULL)
    return;
  outhere=tblock->addr;
  tblock=nextiter_code(tblock);
  if(tblock!=NULL)
  { if(outhere==tblock->addr)
	 {	delfrom(tblock);
      dio.updatewindow();
    }
  }
}

/************************************************************************
* undefinelines                                                         *
* - undefines the next 10 lines of code, or until a non-disassembled    *
*   item is found                                                       *
************************************************************************/
void disasm::undefinelines(void)
{ dsmitem *tblock;
  lptr outhere;
  int i;
  tblock=dio.findcurrentline();
  if(tblock==NULL)
    return;
  outhere=tblock->addr;
  for(i=0;i<10;i++)
  { tblock=nextiter_code(tblock);
	 if(tblock!=NULL)
	 { if(outhere==tblock->addr)
		{ outhere+=tblock->length;
		  delfrom(tblock);
		  tblock=nextiterator();
		}
		else
        break;
	 }
  }
  dio.updatewindow();
}

/************************************************************************
* undefinelines_long                                                    *
* - here we continue undefining any code items in the database from the *
*   users line until we come to a location which is not code, or some   *
*   other kind of item in the database, like a comment, name, or xref   *
************************************************************************/
void disasm::undefinelines_long(void)
{ dsmitem *tblock;
  lptr outhere;
  tblock=dio.findcurrentline();
  if(tblock==NULL)
    return;
  outhere=tblock->addr;
  tblock=nextiter_code(tblock);
  while(tblock!=NULL)
  { if(!tblock->length)
      break;
    if(outhere==tblock->addr)
	 { outhere+=tblock->length;
	   delfrom(tblock);
	   tblock=nextiterator();
	 }
	 else
      break;
  }
  dio.updatewindow();
}

/************************************************************************
* undefineblock                                                         *
* - a block undefine using a selected block of code, we simply undefine *
*   any code items found between the start and end points of the block  *
************************************************************************/
void disasm::undefineblock(lptr ufrom,lptr uto)
{ dsmitem *tblock;
  lptr outhere;
  tblock=dsmfindaddrtype(ufrom,dsmcode);
  if(tblock==NULL)
    return;
  outhere=tblock->addr;
  while(tblock!=NULL)
  { if(tblock->addr>uto)
      break;
    if((tblock->type==dsmcode)&&(tblock->addr>=ufrom))
	   delfrom(tblock);
	 tblock=nextiterator();
  }
  dio.updatewindow();
}

/************************************************************************
* discomment                                                            *
* - this adds a comment to the disassembly database. It is used to add  *
*   different types of comments (like segheaders and user entered       *
*   comments)                                                           *
************************************************************************/
void disasm::discomment(lptr loc,dsmitemtype typ,byte *comment)
{ dsmitem *newdsm;
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,typ);
  newdsm->tptr=(void *)comment;
  newdsm->length=0;
  newdsm->data=comment;
  addto(newdsm);
  dio.updatewindowifinrange(loc);
}

/************************************************************************
* disautocomment                                                        *
* - this is a second function to add a comment to the disassembly       *
*   database, but only if there is no comment already there. This is    *
*   used to add disassembler autocomments. This is currently only used  *
*   in resource disassembly, but could easily be used to add standard   *
*   comments for particular instructions or for API calls, or for DOS   *
*   INTs.                                                               *
************************************************************************/
void disasm::disautocomment(lptr loc,dsmitemtype typ,byte *comment)
{ dsmitem *newdsm,*fdsm;
  fdsm=dsmfindaddrtype(loc,typ);
  if(fdsm!=NULL)
    if((fdsm->addr==loc)&&(fdsm->type==typ))
    { delete comment;
      return;
    }
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,typ);
  newdsm->tptr=(void *)comment;
  newdsm->length=0;
  newdsm->data=comment;
  addto(newdsm);
  dio.updatewindowifinrange(loc);
}

/************************************************************************
* delcomment                                                            *
* - this is used to delete comments from the database. Typically it is  *
*   called when the user enters a comment for a location. We delete the *
*   old one and then add the new one later.                             *
************************************************************************/
void disasm::delcomment(lptr loc,dsmitemtype typ)
{ dsmitem *fdsm;
  fdsm=dsmfindaddrtype(loc,typ);
  if(fdsm!=NULL)
	 if((fdsm->addr==loc)&&(fdsm->type==typ))
	 { delfrom(fdsm);
      dio.updatewindowifinrange(loc);
	 }
}

/************************************************************************
* interpretmod                                                          *
* - this is used by the jumptable detection routines in order to        *
*   examine a modrm/sib encoding for information                        *
* - it returns information about offsets and indexes and the registers  *
*   in use, and any multiplier                                          *
************************************************************************/
bool disasm::interpretmod(byte *data,dword *toffs,byte *indexreg,byte *indexreg2,byte *indexamount,int *numjumps)
{ byte rm,modrm,sib;
  rm=(byte)((data[0]&0xc0)>>6);
  modrm=(byte)(data[0]&0x07);
  switch(rm)
  { case 0:
		if(options.mode32)
		{ if(modrm==5)   // disp32 only.
		  { *toffs=((dword *)(&data[1]))[0];
			 *numjumps=1;
		  }
		  else if(modrm==4)        // case 4=sib
		  { sib=data[1];
			 if((sib&0x07)==5)
            *toffs=((dword *)(&data[2]))[0];  // disp32
			 else
            return false; // no disp
			 if(((sib>>3)&0x07)==4)
            *numjumps=1;  // no scaled index reg
			 else
			 { *indexreg=(byte)((sib>>3)&0x07);
				switch(sib>>6)
				{ case 0:
					 *indexamount=1;
					 break;
				  case 1:
					 *indexamount=2;
					 break;
				  case 2:
					 *indexamount=4;
					 break;
				  case 3:
					 *indexamount=8;
					 break;
				}
			 }
		  }
		  else
          return false; // no disp
		}
		else // 16-bit mode
		{ if(modrm==6) // disp16 only
		  { *toffs=((word *)(&data[1]))[0];
			 *numjumps=1;
		  }
		  else
          return false; // no disp
		}
		break;
	 case 1:
		return false; // all disp8 offsets - don't follow
	 case 2:
		if(options.mode32)
		{ if(modrm==4)        // case 4=sib
		  { sib=data[1];
			 *toffs=((dword *)(&data[2]))[0];
			 *indexreg2=(byte)(sib&0x07);
			 if(((sib>>3)&0x07)==4); // no scaled index reg
			 else
			 { *indexreg=(byte)((sib>>3)&0x07);
				switch(sib>>6)
				{ case 0:
					 *indexamount=1;
					 break;
				  case 1:
					 *indexamount=2;
					 break;
				  case 2:
					 *indexamount=4;
					 break;
				  case 3:
					 *indexamount=8;
					 break;
				}
			 }
		  }
		  else
		  { *toffs=((dword *)(&data[1]))[0];
			 *indexreg2=(byte)(data[0]&0x07);
		  }
		}
		else // 16bit mode
		{ *toffs=((word *)(&data[1]))[0];
		  *indexreg=(byte)(data[0]&0x07); // NB double index reg eg bx+si
		}
		break;
	 case 3:
		// case 3 - no jump table offset present. indirect jump.
		return false;
  }
  return true;
}

/************************************************************************
* disjumptable                                                          *
* - this was written some ago as a quick hack for decoding jump tables  *
* - it tries to obtain information on the table itself, and looks for   *
*   an indication of the number of items in the table by examining      *
*   prior instructions, although it is quite unintelligent in some      *
*   ways.                                                               *
* - it also looks for indextables which are used in complex jumptables  *
*   to decode an initial case number for the jumptable.                 *
* - although good for some compilers the output from some modern        *
*   compilers does not fare well in the analysis.                       *
************************************************************************/
void disasm::disjumptable(lptr loc)
{ dsmitem *investigate;
  dsegitem *dblock,*idblock;
  byte *data;
  byte pbyte;       // prefix byte
  lptr t,it,index,xr;
  int numjumps,inumjumps;
  int i;
  byte indexreg,indexamount,indexreg2;
  byte iindexreg,iindexamount,iindexreg2;
  char tablename[20],tablenum[10];
  bool itable;
  pbyte=0;
  numjumps=0;
  indexreg=0;
  indexreg2=0;
  indexamount=0;
  inumjumps=0;
  iindexreg=0;
  iindexreg2=0;
  iindexamount=0;
  investigate=dsmfindaddrtype(loc,dsmcode);
  if(investigate==NULL)
    return;
  // check that inst is still there/ correct type of jump
  // adjust for any segment overrides added since
  if((loc-investigate->addr)<4)
    loc.offs=investigate->addr.offs;
  if((investigate->addr!=loc)||(investigate->type!=dsmcode))
	 return;
  if(DSMITEM_ARG1(investigate)!=ARG_MODRM)
	 return;
  if(!(investigate->flags&(FLAGS_JMP|FLAGS_CALL|FLAGS_CJMP)))
    return;
  data=investigate->data+investigate->modrm;
  if(!interpretmod(data,&t.offs,&indexreg,&indexreg2,&indexamount,&numjumps))
    return;
  // find target - jump table, need to use default ds:/ check for cs: override
  if(investigate->flags&FLAGS_SEGPREFIX)
  { pbyte=investigate->data[0];
	 if((pbyte==0x66)||(pbyte==0x67))
      pbyte=investigate->data[1];
	 if((pbyte==0x66)||(pbyte==0x67))
      pbyte=investigate->data[2];
  }
  t.segm=options.dseg;
  if(pbyte==0x2e)t.segm=loc.segm;
  dblock=dta.findseg(t);      // find segment item.
  if(dblock==NULL)
    return;
  // look at previous instructions for number of entries
  itable=false;
  if(!numjumps)
  { for(i=0;i<10;i++)
	 { investigate=(dsmitem *)lastiterator();
		if(investigate==NULL)
        break;                                      // no previous insts
		if((investigate->addr.segm!=loc.segm)||(investigate->addr.offs+50<loc.offs))
        break; // too far back
		if(investigate->type!=dsmcode)
        i--;                               // skip non-code
		else
		if(((!strcmp(DSMITEM_NAME(investigate),"mov"))
		  ||(!strcmp(DSMITEM_NAME(investigate),"movzx")))
		  &&((DSMITEM_ARG1(investigate)==ARG_MODRM)
		  ||(DSMITEM_ARG1(investigate)==ARG_MODRM8)))
		{ if(!itable)
			 if(interpretmod(investigate->data+investigate->modrm,&it.offs,&iindexreg,&iindexreg2,&iindexamount,&inumjumps))
			 { itable=true;
				index.offs=investigate->addr.offs;
			 }
		}
		else
		if(((!strcmp(DSMITEM_NAME(investigate),"mov"))
		  ||(!strcmp(DSMITEM_NAME(investigate),"movzx")))
		  &&((DSMITEM_ARG2(investigate)==ARG_MODRM)
		  ||(DSMITEM_ARG2(investigate)==ARG_MODRM8)))
		{ if(!itable)
			 if(interpretmod(investigate->data+investigate->modrm,&it.offs,&iindexreg,&iindexreg2,&iindexamount,&inumjumps))
			 { itable=true;
				index.offs=investigate->addr.offs;
			 }
		}
		else
		if(investigate->data[0]==0x3b)    // cmp inst
		{ if(options.mode32)
          numjumps=((dword *)(&investigate->data[1]))[0]+1;
		  else
          numjumps=((word *)(&investigate->data[1]))[0]+1;
		  break;
		}
		else
		if(investigate->data[0]==0x3d)    // cmp inst
		{ if(options.mode32)
          numjumps=((dword *)(&investigate->data[1]))[0]+1;
		  else
          numjumps=((word *)(&investigate->data[1]))[0]+1;
		  break;
		}
		else
		if((investigate->data[0]==0x83)&&(investigate->data[1]>=0xc0)) // cmp reg,imm8
		{ numjumps=investigate->data[2]+1;
		  break;
		}
	 }
  }
  if(itable)
  { it.segm=t.segm;
	 idblock=dta.findseg(it);
	 if(idblock==NULL)
      return;
	 inumjumps=numjumps;
	 if((inumjumps<2)||(inumjumps>0x100))
      return;
	 numjumps=0;
	 for(i=0;i<inumjumps;i++)
	 { if(it+i>dblock->addr+dblock->size)
        return;
		if((dblock->data+(it-dblock->addr)+i)[0]>numjumps)
		  numjumps=(dblock->data+(it-dblock->addr)+i)[0];
	 }
	 numjumps++;
  }
  // add code disassemblies to scheduler
  // name it
  if((!numjumps)||(numjumps>0x100))
    return;
  if(numjumps>1)
  { jtables++;
	 wsprintf(tablenum,"%d",jtables);
	 strcpy(tablename,"jumptable_");
  }
  else
  { irefs++;
	 wsprintf(tablenum,"%d",irefs);
	 strcpy(tablename,"indirectref_");
  }
  strcat(tablename,tablenum);
  // imports and exports added to this list - build 17
  if((!name.isname(t))&&(!expt.isname(t))&&(!import.isname(t)))
    scheduler.addtask(namecurloc,priority_nameloc,t,tablename);
  xrefs.addxref(t,loc);
  if(itable)
  {  itables++;
	 wsprintf(tablenum,"%d",itables);
	 strcpy(tablename,"indextable_");
	 strcat(tablename,tablenum);
	 if(!name.isname(it))
      scheduler.addtask(namecurloc,priority_nameloc,it,tablename);
	 index.segm=loc.segm;
	 xrefs.addxref(it,index);
  }
  // disassemble data
  // disassemble code
  if(!indexamount)
	 if(options.mode32)
      indexamount=4;
	 else
      indexamount=2;
  for(i=0;i<numjumps;i++)
  { if(t+i*indexamount>dblock->addr+dblock->size)
      return;
	 if(options.mode32)
	 { xr.assign(loc.segm,((dword *)(dblock->data+(t-dblock->addr)+i*indexamount))[0]);
		scheduler.addtask(dis_datadsoffword,priority_data,t+i*indexamount,NULL);
		scheduler.addtask(dis_code,priority_definitecode,xr,NULL);
		xrefs.addxref(xr,t+i*indexamount);
	 }
	 else
	 { xr.assign(loc.segm,((word *)(dblock->data+(t-dblock->addr)+i*indexamount))[0]);
		scheduler.addtask(dis_dataword,priority_data,t+i*indexamount,NULL);
		scheduler.addtask(dis_code,priority_definitecode,xr,NULL);
		xrefs.addxref(xr,t+i*indexamount);
	 }
  }
}

/************************************************************************
* disxref                                                               *
* - this puts an xref line into the disassembly database for a given    *
*   loc, but only if one is not already present.                        *
* - rewritten Borg 2.22 so that new is only called when necessary       *
************************************************************************/
void disasm::disxref(lptr loc)
{ dsmitem *newdsm,*chk;
  char locname[20];
  chk=dsmfindaddrtype(loc,dsmxref);
  if(chk!=NULL)
  { if((chk->type==dsmxref)&&(chk->addr==loc))
	   return;
	 if((chk->length)&&(chk->addr.segm==loc.segm)&&(dsmitem_contains_loc(chk,loc)))
	   return;
  }
  newdsm=new dsmitem;
  initnewdsm(newdsm,loc,dsmxref);
  newdsm->tptr=NULL;
  newdsm->length=0;
  newdsm->data=NULL;
  addto(newdsm);
  if(!((expt.isname(loc))||(import.isname(loc))||(name.isname(loc))))
  { sprintf(locname,"loc_%08x",loc.offs);
    scheduler.addtask(namecurloc,priority_nameloc,loc,locname);
  }
  dio.updatewindowifinrange(loc);
}

/************************************************************************
* getlength                                                             *
* - an external interface routine which just returns the given          *
*   locations disassembled code length. It is used by the search engine *
* - default return value of 1 means 'db'                                *
************************************************************************/
int disasm::getlength(lptr loc)
{ dsmitem *fnd;
  fnd=dsmfindaddrtype(loc,dsmcode);
  if(fnd==NULL)
    return 1;
  if((fnd->addr!=loc)||(fnd->type!=dsmcode))
    return 1;
  return fnd->length;
}


