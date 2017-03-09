/************************************************************************
*				      data.cpp                                              *
* - this is the set of functions which looks after segments/sections    *
* blocks of code or data can be added to the database, where they are   *
* kept and ordered and can be interrogated. functions should be segment *
* level functions in here. the possibleentrycode function was put here  *
* on this basis rather than being a disasm function as it simply        *
* looks for likely entry code and adds scheduler items                  *
*                                                                       *
* NB segment address with segment=0 is treated as a special return      *
* value, and segments should not be created with this value             *
************************************************************************/

#include <windows.h>

#include "data.h"
#include "proctab.h"
#include "schedule.h"
#include "dasm.h"
#include "disasm.h"
#include "debug.h"

// save structure used in databases
struct dsegitemsave
{ lptr addr;
  dword size;
  dword fileoffset;
  segtype typ;
  bool name;
};

/************************************************************************
* global variables                                                      *
* - total_data_size and current_data_pos are basically used to control  *
*   the vertical scroll bar. They are set up in here where the values   *
*   are first initialised as segments are added                         *
************************************************************************/
unsigned long total_data_size;
unsigned long current_data_pos;

/************************************************************************
* constructor function                                                  *
* - we reset the global variables used to track data size and position  *
************************************************************************/
dataseg::dataseg()
{ total_data_size=0;
  current_data_pos=0;
}

/************************************************************************
* destructor                                                            *
* - null at present. the list destructor should handle all segment      *
*   destruction                                                         *
************************************************************************/
dataseg::~dataseg()
{
}

/************************************************************************
* compare function                                                      *
* this is the compare function for the inherited list class             *
* it determines how segments are ordered - by start address             *
************************************************************************/
int dataseg::compare(dsegitem *i,dsegitem *j)
{ if(i->addr==j->addr)
	 return 0;
  if(i->addr>j->addr)
	 return 1;
  return -1;
}

/************************************************************************
* deletion function                                                     *
* this function is called for each list item (segment) and is used to   *
* delete the list items (dsegitem)                                      *
* - we delete any name which was created for the segment, and also any  *
*   data which was created for uninitdata types (we generate a memory   *
*   space for uninitdata for simplicity of analysis later)              *
* - overrides the standard listitem deletion function                   *
************************************************************************/
void dataseg::delfunc(dsegitem *i)
{ if(i->name!=NULL)
    delete i->name;
  if(i->typ==uninitdata)
    delete i->data;
  delete i;
}

/************************************************************************
* insegloc                                                              *
* returns true if loc is in the segment/section ds, otherwise false     *
************************************************************************/
bool dataseg::insegloc(dsegitem *ds,lptr loc)
{ if(ds==NULL)
    return false;
  if(loc.between(ds->addr,ds->addr+ds->size-1))
    return true;
  return false;
}

/************************************************************************
* addseg                                                                *
* this adds a segment to the list of segments. It is called on loading  *
* the file, and segments should be setup before any analysis - here a   *
* segment is just a block of data of a single attribute, which will be  *
* referenced later. It could be a true segment, or just a PE section,   *
* or even just a single resource. Segments should not overlap           *
* the function checks for any overlaps, adds a task to the scheduler    *
* for a segment header comment block, keeps track of total data size    *
************************************************************************/
void dataseg::addseg(lptr loc,dword size,byte *dataptr,segtype t,char *name)
{ dsegitem *addit,*chker;
  char warning[100];
  dword tmpnum,tsize;
#ifdef DEBUG
  DebugMessage("Addseg %04lx:%04lx Size %04lx",loc.segm,loc.offs,size);
#endif
  addit=new dsegitem;
  addit->addr=loc;
  addit->size=size;
  addit->data=dataptr;
  addit->typ=t;
  if(name!=NULL)
  { addit->name=new char [strlen(name)+1];
	 strcpy(addit->name,name);
  }
  else
    addit->name=NULL;
  resetiterator();
  chker=nextiterator();
  while(chker!=NULL)
  { if(insegloc(addit,chker->addr))
	 { //need to cut addit short.
		addit->size=chker->addr-addit->addr;
		if(!addit->size)
		{ tmpnum=loc.segm;
		  wsprintf(warning,"Warning : Unable to create segment %04lx",tmpnum);
		  wsprintf(warning+strlen(warning),":%04lx",loc.offs);
		  wsprintf(warning+strlen(warning)," size :%04lx",size);
		  MessageBox(mainwindow,warning,"Borg Warning",MB_ICONEXCLAMATION|MB_OK);
		  return;
		}
		else
		{ tmpnum=loc.segm;
		  wsprintf(warning,"Warning : Segment overlap %04lx",tmpnum);
		  wsprintf(warning+strlen(warning),":%04lx",loc.offs);
		  wsprintf(warning+strlen(warning)," size :%04lx",size);
		  wsprintf(warning+strlen(warning)," reduced to size :%04lx",addit->size);
		  MessageBox(mainwindow,warning,"Borg Warning",MB_ICONEXCLAMATION|MB_OK);
		}
	 }
	 if(insegloc(chker,addit->addr))
	 { //need to cut chkit short.
		tsize=chker->size;
		chker->size=addit->addr-chker->addr;
		if(!chker->size)
		{ tmpnum=chker->addr.segm;
		  wsprintf(warning,"Warning : Unable to create segment %04lx",tmpnum);
		  wsprintf(warning+strlen(warning),":%04lx",chker->addr.offs);
		  wsprintf(warning+strlen(warning)," size :%04lx",tsize);
		  MessageBox(mainwindow,warning,"Borg Warning",MB_ICONEXCLAMATION|MB_OK);
		  delfrom(chker);
		  resetiterator();
		}
		else
		{ tmpnum=chker->addr.segm;
		  wsprintf(warning,"Warning : Segment overlap %04lx",tmpnum);
		  wsprintf(warning+strlen(warning),":%04lx",chker->addr.offs);
		  wsprintf(warning+strlen(warning)," size :%04lx",tsize);
		  wsprintf(warning+strlen(warning)," reduced to size :%04lx",chker->size);
		  MessageBox(mainwindow,warning,"Borg Warning",MB_ICONEXCLAMATION|MB_OK);
		}
	 }
	 chker=nextiterator();
  }
  addto(addit);
#ifdef DEBUG
  DebugMessage("Adding Seg %04lx:%04lx Size %04lx",loc.segm,loc.offs,size);
  if(name!=NULL)
    DebugMessage("Name:%s",name);
#endif
  scheduler.addtask(dis_segheader,priority_segheader,loc,NULL);
  total_data_size+=size;
}

/************************************************************************
* findseg                                                               *
* - the segment locator takes a loc and searches for the segment        *
*   containing the loc. If its found then it returns a pointer to the   *
*   dsegitem. Note the iterator is changed, and will be left pointing   *
*   to the next segment.                                                *
************************************************************************/
dsegitem *dataseg::findseg(lptr loc)
{ dsegitem t1,*findd;
  t1.addr.assign(loc.segm,0);
  findnext(&t1);
  findd=nextiterator();
  while(findd!=NULL)
  { if(insegloc(findd,loc))
		return findd;
	 findd=nextiterator();
  }
  return NULL;
}

/************************************************************************
* datagetpos                                                            *
* - this simply calculates how far along the total data a loc is, which *
*   is used for the vertical scroll bar to determine how far down it    *
*   should be.                                                          *
************************************************************************/
unsigned long dataseg::datagetpos(lptr loc)
{  dsegitem *findd;
	unsigned long ctr;
	resetiterator();
   findd=nextiterator();
   ctr=0;
   while(findd!=NULL)
   {	if(insegloc(findd,loc))
      {	ctr+=(loc-findd->addr);
      	break;
      }
      ctr+=findd->size;
      findd=nextiterator();
   }
   return ctr;
}

/************************************************************************
* getlocpos                                                             *
* - for a data position this returns the loc, treating all segments as  *
*   continous. It is the opposite function to datagetpos. It is used    *
*   when the vertical scroll bar is dragged to a position and stopped,  *
*   in order to calculate the new loc.                                  *
************************************************************************/
lptr dataseg::getlocpos(unsigned long pos)
{  dsegitem *findd;
	lptr ctr;
	resetiterator();
   findd=nextiterator();
   ctr=findd->addr;
   while(findd!=NULL)
   {  if(pos<findd->size)
   	{	ctr=findd->addr+pos;
      	break;
      }
      pos-=findd->size;
      ctr=findd->addr;
      if(findd->size)
        ctr+=(findd->size-1); // last byte in seg in case we dont find the addr
      findd=nextiterator();
   }
   return ctr;
}

/************************************************************************
* beyondseg                                                             *
* - returns a bool value indicating whether the loc is outside a        *
*   segment. Used a lot to determine if we have moved outside a data    *
*   area. If two segments have the same segm value and are contiguous   *
*   then it will return false, not true                                 *
************************************************************************/
bool dataseg::beyondseg(lptr loc)
{ dsegitem *findd;
  resetiterator();
  findd=nextiterator();
  while(findd!=NULL)
  { if(insegloc(findd,loc))
		return false;
	 findd=nextiterator();
  }
  return true;
}

/************************************************************************
* nextseg                                                               *
* - this function takes a loc, and returns the next segment first loc   *
*   or a segm=0 if its not found                                        *
* mainly used in output and display routines                            *
************************************************************************/
void dataseg::nextseg(lptr *loc)
{ dsegitem *findd;
  resetiterator();
  findd=nextiterator();
  while(findd!=NULL)
  { if(insegloc(findd,*loc))
	 { findd=nextiterator();
		if(findd==NULL)
        loc->segm=0;
		else
        (*loc)=findd->addr;
		return;
	 }
	 findd=nextiterator();
  }
  loc->segm=0;
}

/************************************************************************
* lastseg                                                               *
* - this function takes a loc, and returns the last segment last loc    *
*   or a segm=0 if its not found                                        *
* mainly used in output and display routines                            *
************************************************************************/
void dataseg::lastseg(lptr *loc)
{ dsegitem *findd,*prevd;
  resetiterator();
  findd=nextiterator();
  prevd=NULL;
  while(findd!=NULL)
  { if(insegloc(findd,*loc))
	 { if(prevd==NULL)
        loc->segm=0;
		else
		  (*loc)=prevd->addr+prevd->size-1;
		return;
	 }
	 prevd=findd;
	 findd=nextiterator();
  }
  loc->segm=0;
}

/************************************************************************
* possibleentrycode                                                     *
* - this just scans a whole segment for possible routine entrycode, as  *
* defined by options, and adds scheduler items for proper analysis by   *
* disasm. currently all specific to the 80x86 processor                 *
************************************************************************/
void dataseg::possibleentrycode(lptr loc)
{ dsegitem t1,*findd;
  dword length;
  if(options.processor==PROC_Z80)
    return;
  t1.addr=loc;
  findd=find(&t1);
  if(findd==NULL)
    return;
  length=findd->size;
  loc.offs=findd->addr.offs;
  if(options.codedetect&CD_AGGRESSIVE)
    scheduler.addtask(seek_code,priority_aggressivesearch,loc,NULL);
  while(length)
  { if((options.codedetect&CD_PUSHBP)&&(length>3))
	 { if(findd->data[loc-findd->addr]==0x55)  // push bp
		{ // two encodings of mov bp,sp
		  if((findd->data[(loc-findd->addr)+1]==0x8b)&&(findd->data[(loc-findd->addr)+2]==0xec))
			 scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
		  if((findd->data[(loc-findd->addr)+1]==0x89)&&(findd->data[(loc-findd->addr)+2]==0xe5))
			 scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
		}
	 }
	 if((options.codedetect&CD_EAXFROMESP)&&(length>4))
	 { if(findd->data[loc-findd->addr]==0x55)  // push bp
		{ if((findd->data[(loc-findd->addr)+1]==0x8b)&&(findd->data[(loc-findd->addr)+2]==0x44)&&(findd->data[(loc-findd->addr)+3]==0x24)) // mov ax,[sp+xx]
			 scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
		}
	 }
	 if((options.codedetect&CD_MOVEAX)&&(length>3))
	 { if((findd->data[(loc-findd->addr)]==0x8b)&&(findd->data[(loc-findd->addr)+1]==0x44)&&(findd->data[(loc-findd->addr)+2]==0x24)) // mov ax,[sp+xx]
		  scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
	 }
	 if((options.codedetect&CD_ENTER)&&(length>4))
	 { if(findd->data[loc-findd->addr]==0xc8)  // enter
		{ if(findd->data[(loc-findd->addr)+3]==0x00) // enter xx,00
			 scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
		}
	 }
	 if((options.codedetect&CD_MOVBX)&&(length>2)) // mov bx,sp
	 { if((findd->data[loc-findd->addr]==0x8b)&&(findd->data[(loc-findd->addr)+1]==0xdc))
		  scheduler.addtask(dis_code,priority_possiblecode,loc,NULL);
	 }
	 loc.offs++;
	 length--;
  }
}

/************************************************************************
* segheader                                                             *
* - here we add the segment header as a comment, to the disassembly     *
*  disasm adds the comment, we assemble the information here.           *
************************************************************************/
void dataseg::segheader(lptr loc)
{ byte *tmpc,tmp2[30];
  dsegitem t1,*findd;
  dword t;
  t1.addr=loc;
  findd=find(&t1);
  if(findd==NULL)
    return;
  tmpc=new byte[80];
  strcpy((char *)tmpc,"-----------------------------------------------------------------------");
  dsm.discomment(loc,dsmsegheader,tmpc);
  tmpc=new byte[80];
  t=loc.segm;
  wsprintf((char *)tmpc,"Segment : %02lxh",t);
  wsprintf((char *)tmp2,"     Offset : %02lxh",loc.offs);
  strcat((char *)tmpc,(char *)tmp2);
  wsprintf((char *)tmp2,"     Size : %02lxh",findd->size);
  strcat((char *)tmpc,(char *)tmp2);
  dsm.discomment(loc,(dsmitemtype)(dsmsegheader+1),tmpc);
  tmpc=new byte[80];
  switch(findd->typ)
  { case code16:
		strcpy((char *)tmpc,"16-bit Code");
		break;
	 case code32:
		strcpy((char *)tmpc,"32-bit Code");
		break;
	 case data16:
		strcpy((char *)tmpc,"16-bit Data");
		break;
	 case data32:
		strcpy((char *)tmpc,"32-bit Data");
		break;
	 case uninitdata:
		strcpy((char *)tmpc,"Uninit Data");
		break;
	 case debugdata:
		strcpy((char *)tmpc,"Debug Data");
		break;
	 case resourcedata:
		strcpy((char *)tmpc,"Resource Data ");
		break;
	 default:
		strcpy((char *)tmpc,"Unknown");
		break;
  }
  if(findd->name!=NULL)
  { strcat((char *)tmpc," : ");
    strncat((char *)tmpc,findd->name,60);
  }
  dsm.discomment(loc,(dsmitemtype)(dsmsegheader+2),tmpc);
  tmpc=new byte[80];
  strcpy((char *)tmpc,"-----------------------------------------------------------------------");
  dsm.discomment(loc,(dsmitemtype)(dsmsegheader+3),tmpc);
}

/************************************************************************
* write_item                                                            *
* - writes a dataseg item to the savefile specified                     *
*   uses the current item, and moves the iterator on                    *
*   saving in borg is single pass and so stringlengths are saved before *
*   strings, etc. we use a similar structure item for the save          *
*   (dsegitem - dsegitemsave). As the data item is a pointer it is      *
*   converted into an offset for saving.                                *
************************************************************************/
bool dataseg::write_item(savefile *sf,byte *filebuff)
{ dword nlen;
  dsegitemsave structsave;
  dsegitem *currseg;

  currseg=nextiterator();
  structsave.addr=currseg->addr;
  structsave.size=currseg->size;
  structsave.fileoffset=currseg->data-filebuff;
  structsave.typ=currseg->typ;
  if(currseg->name!=NULL)
    structsave.name=true;
  else
    structsave.name=false;
  if(!sf->swrite(&structsave,sizeof(dsegitemsave)))
    return false;
  if(structsave.name)
  { nlen=strlen(currseg->name)+1;
  	 if(!sf->swrite(&nlen,sizeof(dword)))
      return false;
	 if(!sf->swrite(currseg->name,nlen))
      return false;
  }
  return true;
}

/************************************************************************
* read_item                                                             *
* - reads a dataseg item from the savefile specified                    *
*   Note that addseg is not used here, since we have all the            *
*   information and just need to reconstruct the dsegitem. Hence we     *
*   also need to recalculate total_data_size in here. Also, for uninit  *
*   data segments we add a false data area.                             *
************************************************************************/
bool dataseg::read_item(savefile *sf,byte *filebuff)
{ dword num,nlen;
  dsegitemsave structsave;
  dsegitem *currseg;

  currseg=new dsegitem;
  if(!sf->sread(&structsave,sizeof(dsegitemsave),&num))
    return false;
  currseg->addr=structsave.addr;
  currseg->size=structsave.size;
  currseg->data=structsave.fileoffset+filebuff;
  currseg->typ=structsave.typ;
  // total_data_size increased
  total_data_size+=currseg->size;
  if(structsave.name)
  { if(!sf->sread(&nlen,sizeof(dword),&num))
      return false;
    currseg->name=new char[nlen];
	 if(!sf->sread(currseg->name,nlen,&num))
      return false;
  }
  else currseg->name=NULL;
  // uninitdata - add false data area
  if(currseg->typ==uninitdata)
  { currseg->data=new byte[currseg->size];
    for(nlen=0;nlen<currseg->size;nlen++)
      (currseg->data)[nlen]=0;
  }
  addto(currseg);
  return true;
}

