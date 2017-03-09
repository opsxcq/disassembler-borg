/************************************************************************
*                disio.cpp                                              *
* This is a fairly big class, and yet it was originally part of the     *
* larger disasm class. I finally got around to splitting off some of    *
* 3500 lines of code from that class into here. This represents the     *
* disasm I/O functions for window I/O and file I/O. Version 2.11 was    *
* when this was split. There are still a huge number of confusing       *
* functions and code in here which probably needs more work to clean it *
* up, and better split/define the classes. Some of the functions are    *
* friends of disasm because it has been difficult to split the classes  *
* effectively and both access the complex main disassembly structures   *
************************************************************************/

#include <windows.h>
#include <stdio.h>            // for sprintf, for float printing

#include "disio.h"
#include "disasm.h"
#include "data.h"
#include "dasm.h"
#include "schedule.h"
#include "mainwind.h"
#include "gname.h"
#include "xref.h"
#include "range.h"
#include "debug.h"

/************************************************************************
* globals                                                               *
* - actually some constants used in file i/o as a header                *
************************************************************************/
char hdr[] =";             Created by Borg Disassembler\r\n";
char hdr2[]=";                   written by Cronos\r\n";

/************************************************************************
* constructor function                                                  *
* - this just enables window updates and sets the subline to null       *
* - the subline (subitem) is basically the line in the disassembly that *
*   you see for any given loc. As a loc may refer to many lines (like   *
*   segheader, comments, xref, actual instruction are all separate      *
*   line, and subline says which of these it is)                        *
************************************************************************/
disio::disio()
{  subitem=dsmnull;
}

/************************************************************************
* destructor function                                                   *
* - currently null                                                      *
* - note that disio has no particular structs the same way most of the  *
*   other classes in Borg do                                            *
************************************************************************/
disio::~disio()
{
}

/************************************************************************
* argoverdec                                                            *
* - disio also acts as a go between for the user and the disasm engine  *
* - here we translate the users current line of the disassembly and     *
*   their request for a decimal override into a call to the disasm      *
*   engine to add the override to the instruction that is there         *
* - note that these kind of calls come from the scheduler and are part  *
*   of the secondary thread                                             *
************************************************************************/
void disio::argoverdec(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  dsm.disargoverdec(outhere);
}

/************************************************************************
* argoversingle                                                         *
* - very similar to argoverdec, for a single (float) override           *
************************************************************************/
void disio::argoversingle(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  dsm.disargoversingle(outhere);
}

/************************************************************************
* argovernegate                                                         *
* - very similar to argoverdec, for an argument negation override       *
************************************************************************/
void disio::arg_negate(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  dsm.disargnegate(outhere);
}

/************************************************************************
* argoverhex                                                            *
* - very similar to argoverdec, for a hex override                      *
************************************************************************/
void disio::argoverhex(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  dsm.disargoverhex(outhere);
}

/************************************************************************
* argoveroffsetdseg                                                     *
* - very similar to argoverdec, for a dseg override                     *
************************************************************************/
void disio::argoveroffsetdseg(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  dsm.disargoveroffsetdseg(outhere);
}

/************************************************************************
* argoverchar                                                           *
* - very similar to argoverdec, for a char override                     *
************************************************************************/
void disio::argoverchar(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  dsm.disargoverchar(outhere);
}

/************************************************************************
* makedword                                                             *
* - disio acts as a go between for the user and the disasm engine       *
* - here we translate the users current line of the disassembly and     *
*   their request for a dword into a call to the disasm engine to       *
*   disassemble a dword at the current point                            *
* - note that these kind of calls come from the scheduler and are part  *
*   of the secondary thread                                             *
************************************************************************/
void disio::makedword(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatadword(outhere);
}

/************************************************************************
* makesingle                                                            *
* - very similar to makedword, this time to disassemble a single        *
*   (float)                                                             *
************************************************************************/
void disio::makesingle(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatasingle(outhere);
}

/************************************************************************
* makedouble                                                            *
* - very similar to makedword, this time to disassemble a double        *
*   (double float)                                                      *
************************************************************************/
void disio::makedouble(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatadouble(outhere);
}

/************************************************************************
* makelongdouble                                                        *
* - very similar to makedword, this time to disassemble a long double   *
*   (long double)                                                       *
************************************************************************/
void disio::makelongdouble(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatalongdouble(outhere);
}

/************************************************************************
* makeword                                                              *
* - very similar to makedword, this time to disassemble a word          *
************************************************************************/
void disio::makeword(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdataword(outhere);
}

/************************************************************************
* makestring                                                            *
* - very similar to makedword, this time to disassemble a string        *
*   (standard C string)                                                 *
************************************************************************/
void disio::makestring(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatastring(outhere);
}

/************************************************************************
* pascalstring                                                          *
* - very similar to makedword, this time to disassemble a string        *
*   (standard pascal string)                                            *
************************************************************************/
void disio::pascalstring(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatapstring(outhere);
}

/************************************************************************
* ucstring                                                              *
* - very similar to makedword, this time to disassemble a string        *
*   (unicode C string)                                                  *
************************************************************************/
void disio::ucstring(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdataucstring(outhere);
}

/************************************************************************
* upstring                                                              *
* - very similar to makedword, this time to disassemble a string        *
*   (unicode pascal string)                                             *
************************************************************************/
void disio::upstring(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdataupstring(outhere);
}

/************************************************************************
* dosstring                                                             *
* - very similar to makedword, this time to disassemble a string        *
*   (dos style string)                                                  *
************************************************************************/
void disio::dosstring(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatadosstring(outhere);
}

/************************************************************************
* generalstring                                                         *
* - very similar to makedword, this time to disassemble a string        *
*   (general string)                                                    *
************************************************************************/
void disio::generalstring(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disdatageneralstring(outhere);
}

/************************************************************************
* makecode                                                              *
* - very similar to makedword, but this time to disassemble as code     *
*   from the current location, and continue disassembling               *
************************************************************************/
void disio::makecode(void)
{ lptr outhere;
  findcurrentaddr(&outhere);
  if(outhere==curraddr)
    subitem=dsmnull;
  dsm.disblock(outhere);
}

/************************************************************************
* vertsetpos                                                            *
* - this function takes a position of the vertical scroll bar, from 0   *
*   to the VERTSCROLLRANGE and it sets the current address to that      *
*   point                                                               *
************************************************************************/
void disio::vertsetpos(int pos)
{ float sbarpos;
  sbarpos=((float)pos)/((float)VERTSCROLLRANGE)*(float)total_data_size;
  setcuraddr(dta.getlocpos((unsigned long)sbarpos));
}

/************************************************************************
* setcuraddr                                                            *
* - sets the address as a location for the screen output to start at    *
* - also sets the scroll bar position for the vertical scroll bar       *
* - also ensures that its not set to a mid-instruction address and      *
*   sets the subline                                                    *
* - called from many places in the source code                          *
************************************************************************/
void disio::setcuraddr(lptr loc)
{ dsmitem titem,*tblock;
  float sbarpos;
  dsegitem *l_ds;
  // check for non-existant address added v2.20
  l_ds=dta.findseg(loc);
  if(l_ds==NULL)
    return;
  curraddr=loc;
  titem.addr=curraddr;
  titem.type=dsmnull;
  dsm.findnext(&titem);
  tblock=dsm.nextiterator();
  if(tblock!=NULL)
  	 subitem=tblock->type;
  else
    subitem=dsmnull;
  tblock=dsm.nextiter_code(tblock);
  if(tblock!=NULL)
  { if(loc.between(tblock->addr,tblock->addr+tblock->length-1))
		curraddr=tblock->addr;
  }
  outend=loc+50;
  usersel=0;
  userselonscreen=true;
  updatewindow();
  current_data_pos=dta.datagetpos(loc);
  sbarpos=((float)current_data_pos)/((float)total_data_size+(float)1.0)*(float)VERTSCROLLRANGE;
  SetScrollPos(mainwindow,SB_VERT,(int)sbarpos,true);
}

/************************************************************************
* setpos                                                                *
* - when the left mouse button is pressed the line is changed to the    *
*   line the mouse points at. This routine sets the line, and asks for  *
*   a screen update if needed (the screen update method here is a       *
*   primary thread request that simply invalidates the window rather    *
*   than requesting through the scheduler...... but this is very very   *
*   old code....                                                        *
************************************************************************/
void disio::setpos(int ypos)
{ if(usersel!=ypos/cyc)
  { usersel=ypos/cyc;
	 InvalidateRect(mainwindow,NULL,true);
  }
}

/************************************************************************
* printlineheader                                                       *
* - prints an appropriate line header with the address if needed and    *
*   sets the cursor position for the next LastPrintBuff call            *
************************************************************************/
void disio::printlineheader(lptr loc,bool printaddrs)
{ if(options.mode32)
  { if(printaddrs)
      PrintBuff("%04lx:%08lx",loc.segm,loc.offs);
	 else
      PrintBuff("");
	 LastPrintBuffEpos(BYTEPOS);
  }
  else
  { if(printaddrs)
      PrintBuff("%04lx:%04lx",loc.segm,loc.offs);
	 else
      PrintBuff("");
	 LastPrintBuffEpos(BYTEPOS-4);
  }
}

/************************************************************************
* outinst                                                               *
* - this is the routine which results in the output of an address, hex  *
*   bytes, instruction and arguments                                    *
************************************************************************/
void disio::outinst(dsmitem *inst,bool printaddrs)
{ dsegitem *dblock;
  int i;
  int prefixptr;
  byte pbyte;
  printlineheader(inst->addr,printaddrs);
  dblock=dta.findseg(inst->addr);      // find segment item.
  switch(inst->type)
  { case dsmcode:
      i=inst->length;
		if(printaddrs)
		{ while(i)
		  { if(dblock!=NULL)
			 {	if(dblock->typ==uninitdata)
              LastPrintBuff("??");
				else
              LastPrintBuff("%02x",inst->data[inst->length-i]);
			 }
			 else
            LastPrintBuff("%02x",inst->data[inst->length-i]);
			 i--;
			 if(((i+8)<inst->length)&&(inst->length>10))
			 { LastPrintBuff("..");
				break;
			 }
		  }
		}
		if(options.mode32)
		  LastPrintBuffEpos(ASMPOS+4);
		else
		  LastPrintBuffEpos(ASMPOS);
		if(inst->flags&FLAGS_PREFIX)
      { prefixptr=0;
        while(!isprefix(inst->data[prefixptr])&&(prefixptr<15))
          prefixptr++;
        pbyte=inst->data[prefixptr];
        outprefix(pbyte);
      }
		LastPrintBuff(DSMITEM_NAME(inst));
		LastPrintBuff(" ");
		if(options.mode32)
		  LastPrintBuffEpos(ARGPOS+4);
		else
		  LastPrintBuffEpos(ARGPOS);
		if(dblock!=NULL)
		{ if(dblock->typ==uninitdata)
          LastPrintBuff("?");
		  else
		  { outargs(inst,DSMITEM_ARG1(inst));
			 if(DSMITEM_ARG2(inst)!=ARG_NONE)
			 { LastPrintBuff(", ");
				outargs(inst,DSMITEM_ARG2(inst));
			 }
			 if(DSMITEM_ARG3(inst)!=ARG_NONE)
			 { LastPrintBuff(", ");
				outargs(inst,DSMITEM_ARG3(inst));
			 }
		  }
		}
		else
		{ outargs(inst,DSMITEM_ARG1(inst));
		  if(DSMITEM_ARG2(inst)!=ARG_NONE)
		  { LastPrintBuff(", ");
			 outargs(inst,DSMITEM_ARG2(inst));
		  }
		  if(DSMITEM_ARG3(inst)!=ARG_NONE)
		  { LastPrintBuff(", ");
			 outargs(inst,DSMITEM_ARG3(inst));
		  }
		}
		break;
	 default:
		break;
  }
}

/************************************************************************
* outdb                                                                 *
* - this is similar to outinst, but when there is no disassembly for a  *
*   loc we call this to output a db xxh or a db ? if its uninitdata    *
************************************************************************/
void disio::outdb(lptr *lp,bool printaddrs)
{ dsegitem *dblock;
  dword aaddr;
  byte *mcode;
  printlineheader(*lp,printaddrs);
  dblock=dta.findseg(*lp);      // find segment item.
  if(dblock==NULL)
  { if(printaddrs)
      LastPrintBuff("??");
  }
  else if(dblock->typ==uninitdata)
  { if(printaddrs)
      LastPrintBuff("??");
  }
  else
  { aaddr=(*lp-dblock->addr);
	 mcode=dblock->data+aaddr;
	 if(printaddrs)
      LastPrintBuff("%02x",mcode[0]);
  }
  if(options.mode32)
	 LastPrintBuffEpos(ASMPOS+4);
  else
	 LastPrintBuffEpos(ASMPOS);

  LastPrintBuff("db");

  if(options.mode32)
	 LastPrintBuffEpos(ARGPOS+4);
  else
	 LastPrintBuffEpos(ARGPOS);
  // changed to single ? - 2.25
  if(dblock==NULL)
	 LastPrintBuff("?");
  else if(dblock->typ==uninitdata)
  { LastPrintBuff("?");
  }
  else
  { LastPrintBuffHexValue(mcode[0]);
	 if(isprint(mcode[0]))
	 { LastPrintBuffEpos(COMMENTPOS);
		LastPrintBuff(";\'%c\'",mcode[0]);
	 }
  }
}

/************************************************************************
* issegprefix                                                           *
* - returns true if byte is a segment prefix valid value                *
************************************************************************/
bool disio::issegprefix(byte byt)
{ if((byt==0x2e)||(byt==0x36)||(byt==0x3e)||(byt==0x26)||(byt==0x64)||(byt==0x65))
    return true;
  return false;
}

/************************************************************************
* isprefix                                                              *
* - returns true if byte is a prefix valid value (rep/repne/lock)       *
************************************************************************/
bool disio::isprefix(byte byt)
{ if((byt==0xf0)||(byt==0xf2)||(byt==0xf3))
    return true;
  return false;
}

/************************************************************************
* outprefix                                                             *
* - here we output a prefix segment override                            *
************************************************************************/
void disio::outprefix(byte prefixbyte)
{ switch(prefixbyte)
  { case 0x2e:
		LastPrintBuff("cs:");
		break;
	 case 0x36:
		LastPrintBuff("ss:");
		break;
	 case 0x3e:
		LastPrintBuff("ds:");
		break;
	 case 0x26:
		LastPrintBuff("es:");
		break;
	 case 0x64:
		LastPrintBuff("fs:");
		break;
	 case 0x65:
		LastPrintBuff("gs:");
		break;
    case 0xf0:
      LastPrintBuff("lock ");
      break;
    case 0xf2:
      LastPrintBuff("repne ");
      break;
    case 0xf3:
      LastPrintBuff("rep ");
      break;
	 default:
		LastPrintBuff("err:");
		break;
  }
}

/************************************************************************
* jumpback                                                              *
* - when the user presses ESC, or selects jump back we get the last     *
*   address from the top of the address stack and call setcuraddr and   *
*   update the window and so the disassembly listing flicks back        *
************************************************************************/
void disio::jumpback(void)
{ lptr outhere;
  outhere=retstack.pop();
  if(outhere.segm)
    setcuraddr(outhere);
  updatewindow();
}

/************************************************************************
* jumpto                                                                *
* - more complex than the jumpback is the jumpto. The complexity lies   *
*   in deciding where we are jumping to and what the arguments value is *
*   and if the location exists. Actually making the jump consists of    *
*   saving the current location to the address stack and then changing  *
*   the curraddr for output, and updating the window                    *
* - most of this routine is a complex decipherment of a modrm address   *
*   to jump to                                                          *
* NB I need to stick this in a function of its own at some point, as it *
* would be quite useful to just get an argument address in several      *
* places in the code                                                    *
************************************************************************/
void disio::jumpto(bool arg1)
{ dsmitem *tblock;
  lptr outhere;
  byte *data;
  bool madejump;
  byte modrm,sib;
  word rm;
  tblock=findcurrentline();
  outhere.assign(0,0);
  if(tblock!=NULL)
  		if(tblock->type!=dsmcode)
        return;
  madejump=false;
  if(tblock!=NULL)
  { if(arg1)
    { switch(DSMITEM_ARG1(tblock))
	   { case ARG_FADDR:
		    data=tblock->data+tblock->length;
		    if(tblock->mode32)
		    { data-=6;
			   outhere.assign(((word *)(&data[4]))[0],((dword *)(&data[0]))[0]);
		    }
		    else
		    { data-=4;
			   outhere.assign(((word *)(&data[2]))[0],((word *)(&data[0]))[0]);
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
			   madejump=true;
		    }
		    break;
		  case ARG_IMM32:
		    if(tblock->override!=over_dsoffset)
            break;
		    data=tblock->data+tblock->length;
		    data-=4;
		    outhere.assign(options.dseg,((dword *)(&data[0]))[0]);
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
			   madejump=true;
  		    }
		    break;
	     case ARG_MEMLOC:
          data=tblock->data+tblock->length;
		    if(options.mode32)
		    { data-=4;
		      outhere.assign(tblock->addr.segm,((dword *)data)[0]);
		    }
		    else
		    { data-=2;
		      outhere.assign(tblock->addr.segm,((word *)data)[0]);
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
			   madejump=true;
		    }
		    break;
		  case ARG_IMM:
		    if(tblock->override!=over_dsoffset)
            break;
		    if(options.mode32)
		    { data=tblock->data+tblock->length;
			   data-=4;
			   outhere.assign(options.dseg,((dword *)(&data[0]))[0]);
			   if(dta.findseg(outhere)!=NULL)
			   { retstack.push(curraddr);
				  setcuraddr(outhere);
				  updatewindow();
				  madejump=true;
			   }
		    }
		    break;
		  case ARG_RELIMM:
		    data=tblock->data+tblock->length;
		    outhere=tblock->addr;
		    if(tblock->mode32)
		    { data-=4;
			   outhere+=((dword *)data)[0]+tblock->length;
		    }
		    else
		    { data-=2;
			   outhere+=(word)(((word *)data)[0]+tblock->length);
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
			   madejump=true;
		    }
		    break;
		  case ARG_RELIMM8:
		    data=tblock->data+tblock->length-1;
		    outhere=tblock->addr;
		    if(tblock->mode32)
		    { if(data[0]&0x80)
				  outhere+=(dword)(data[0]+0xffffff00+tblock->length);
			   else
				  outhere+=(dword)(data[0]+tblock->length);
		    }
		    else
		    { if(data[0]&0x80)
				  outhere+=(word)(data[0]+0xff00+tblock->length);
			   else
				  outhere+=(word)(data[0]+tblock->length);
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
			   madejump=true;
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
		    data=tblock->data+tblock->modrm;
		    rm=(byte)((data[0]&0xc0)>>6);
		    modrm=(byte)(data[0]&0x07);
		    switch(rm)
		    { case 0:
			     if(options.mode32)
			     { if(modrm==5)
				    { outhere.assign(tblock->addr.segm,((dword *)(&data[1]))[0]);
 				    }
				    else if(modrm==4)        // case 4=sib
				    { sib=data[1];
				      if((sib&0x07)==5) // disp32
				      { outhere.assign(tblock->addr.segm,((dword *)(&data[2]))[0]);
				      }
				    }
			     }
			     else
			     { if(modrm==6)
				    { outhere.assign(tblock->addr.segm,((word *)(&data[1]))[0]);
				    }
			     }
			     break;
		      case 1:
			     break;
		      case 2:
			     if(options.mode32)
			     { outhere.assign(tblock->addr.segm,((dword *)(&data[1]))[0]);
				    if(modrm==4)        // case 4=sib
				    { outhere.assign(tblock->addr.segm,((dword *)(&data[2]))[0]);
				    }
				  }
			     else
			     { outhere.assign(tblock->addr.segm,((word *)(&data[1]))[0]);
			     }
			     break;
		      case 3:
			     break;
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
			   madejump=true;
		    }
          break;
        default:
		    break;
	   }
    }
	 if(!madejump)
	 { switch(DSMITEM_ARG2(tblock))
		{ case ARG_IMM32:
			 if(tblock->override!=over_dsoffset)
            break;
			 data=tblock->data+tblock->length;
			 data-=4;
			 outhere.assign(options.dseg,((dword *)(&data[0]))[0]);
			 if(dta.findseg(outhere)!=NULL)
			 { retstack.push(curraddr);
				setcuraddr(outhere);
				updatewindow();
			 }
			 break;
		  case ARG_IMM:
			 if(tblock->override!=over_dsoffset)
            break;
			 if(options.mode32)
			 { data=tblock->data+tblock->length;
				data-=4;
				outhere.assign(options.dseg,((dword *)(&data[0]))[0]);
				if(dta.findseg(outhere)!=NULL)
				{ retstack.push(curraddr);
				  setcuraddr(outhere);
				  updatewindow();
				}
			 }
			 break;
	     case ARG_MEMLOC:
          data=tblock->data+tblock->length;
		    if(options.mode32)
		    { data-=4;
		      outhere.assign(tblock->addr.segm,((dword *)data)[0]);
		    }
		    else
		    { data-=2;
		      outhere.assign(tblock->addr.segm,((word *)data)[0]);
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
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
		    data=tblock->data+tblock->modrm;
		    rm=(byte)((data[0]&0xc0)>>6);
		    modrm=(byte)(data[0]&0x07);
		    switch(rm)
		    { case 0:
			     if(options.mode32)
			     { if(modrm==5)
				    { outhere.assign(tblock->addr.segm,((dword *)(&data[1]))[0]);
				    }
				    else if(modrm==4)        // case 4=sib
				    { sib=data[1];
				      if((sib&0x07)==5) // disp32
				      { outhere.assign(tblock->addr.segm,((dword *)(&data[2]))[0]);
				      }
				    }
			     }
			     else
			     { if(modrm==6)
				    { outhere.assign(tblock->addr.segm,((word *)(&data[1]))[0]);
				    }
			     }
			     break;
		      case 1:
			     break;
		      case 2:
			     if(options.mode32)
			     { outhere.assign(tblock->addr.segm,((dword *)(&data[1]))[0]);
				    if(modrm==4)        // case 4=sib
				    { outhere.assign(tblock->addr.segm,((dword *)(&data[2]))[0]);
				    }
				  }
			     else
			     { outhere.assign(tblock->addr.segm,((word *)(&data[1]))[0]);
			     }
			     break;
		      case 3:
			     break;
		    }
		    if(dta.findseg(outhere)!=NULL)
		    { retstack.push(curraddr);
			   setcuraddr(outhere);
			   updatewindow();
		    }
          break;
		  default:
			 break;
		}
	 }
  }
}

/************************************************************************
* findcurrentline                                                       *
* - this routine finds the current screen address and output line in    *
*   the disassembly database and from there works out the disassembly   *
*   item on the currently selected line.                                *
* - it is used by jumpto, when the user presses return to follow a jump *
* - comments added to the procedure, it is a useful one to follow and   *
*   see the strategy employed, which is a fairly common Borg strategy   *
************************************************************************/
dsmitem *disio::findcurrentline(void)
{ dsmitem titem,*tblock;
  lptr outhere;
  unsigned int i;
  // strategy
  // - use pointer to first item if available (so comments,etc included in list
  // - otherwise use address.
  titem.addr=curraddr;
  titem.type=subitem;
  // hunt for current addr and subitem
  tblock=dsm.find(&titem);
  if(tblock!=NULL)
    tblock=dsm.nextiterator();
  // on overlap - reset the curraddr.
  // [on the spot error correction]
  if(tblock!=NULL)
  { if(curraddr.between(tblock->addr,tblock->addr+tblock->length-1))
	  curraddr.offs=tblock->addr.offs;
  }
  // ensure we point to the right item, or the next one
  if(tblock!=NULL)
  { if((tblock->addr<curraddr)||((tblock->addr==curraddr)&&(tblock->type<subitem)))
		tblock=dsm.nextiterator();
  }
  // now at the top of the screen, the next loop moves down to the user selection line
  outhere=curraddr;
  for(i=0;i<usersel;i++)
  { if(tblock!=NULL)
	 { if(outhere==tblock->addr)
		{ outhere+=tblock->length;
		  tblock=dsm.nextiterator();
		}
		else
        outhere++;
	 }
	 else
      outhere++;
	 // check if gone beyond seg, get next seg.
	 if(dta.beyondseg(outhere))
	 { outhere.offs--;
      dta.nextseg(&outhere);
    }
	 if(!outhere.segm)
      break;
  }
  // now we either have the line we are pointing to, in the database
  // or we have moved beyond the database and have a null
  // or we have an address which would be a db
  if(tblock!=NULL)
	 if(outhere!=tblock->addr)
		return NULL;
  return tblock;
}

/************************************************************************
* findcurrentaddr                                                       *
* - this is very similar to findcurrentline, but it instead finds just  *
*   the location. the search strategy is the same                       *
************************************************************************/
void disio::findcurrentaddr(lptr *loc)
{ dsmitem titem,*tblock;
  lptr outhere;
  unsigned int i;
  // strategy
  // - use pointer to first item if available (so comments,etc included in list
  // - otherwise use address.
  titem.addr=curraddr;
  titem.type=subitem;
  tblock=dsm.find(&titem);
  if(tblock!=NULL)
    tblock=dsm.nextiterator();
  if(tblock!=NULL)
  { if(curraddr.between(tblock->addr,tblock->addr+tblock->length-1))
	  curraddr.offs=tblock->addr.offs;
  }
  if(tblock!=NULL)
  { if(tblock->addr<curraddr)
		tblock=dsm.nextiterator();
  }
  // added 2.25 - bugfix
  // - wasnt finding correct lines when in mid-line......
  if(tblock!=NULL)
  { while((tblock->addr==curraddr)&&(tblock->type<subitem))
  	 { tblock=dsm.nextiterator();
      if(tblock==NULL)
        break;
    }
  }
  outhere=curraddr;
  for(i=0;i<usersel;i++)
  { if(tblock!=NULL)
	 { if(outhere==tblock->addr)
		{ outhere+=tblock->length;
		  tblock=dsm.nextiterator();
		}
		else
        outhere++;
	 }
	 else
      outhere++;
	 // check if gone beyond seg, get next seg.
	 if(dta.beyondseg(outhere))
	 { outhere.offs--;
      dta.nextseg(&outhere);
    }
	 if(!outhere.segm)
      break;
  }
  if(outhere.segm)
    (*loc)=outhere;
  else
    (*loc)=curraddr;
}

/************************************************************************
* savecuraddr                                                           *
* - this simply saves the window top line location to the return stack  *
* - this is called when the user selects to jump somewhere (like to a   *
*   named location) rather than the 'jumpto' routine used to jump to a  *
*   location specified by a disassembly argument                        *
************************************************************************/
void disio::savecuraddr(void)
{ retstack.push(curraddr);
}

/************************************************************************
* updatewindowifwithinrange                                             *
* - adds a scheduler task for a window update if the current window     *
*   overlaps with the specified range                                   *
************************************************************************/
void disio::updatewindowifwithinrange(lptr loc_start,lptr loc_end)
{ if((loc_end>=curraddr)&&(loc_start<=outend))
    scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* updatewindowifinrange                                                 *
* - adds a scheduler task for a window update if the current window     *
*   contains the loc specified                                          *
************************************************************************/
void disio::updatewindowifinrange(lptr loc)
{ if(loc.between(curraddr,outend))
	 scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* updatewindow                                                          *
* - this function rewrites the buffers for the disassembly output to    *
*   the window, and then requests a screen refresh which will paint the *
*   buffers into the window. The ClearBuff and DoneBuff functions       *
*   when the buffers are full and it is safe to refresh the screen with *
*   a repaint                                                           *
************************************************************************/
// this functions job to output a window
// of disassembly to the buffer.
void disio::updatewindow(void)
{ dsmitem titem,*tblock;
  lptr outhere;
  int i;
  // find current position.
  titem.addr=curraddr;
  titem.type=subitem;
  tblock=dsm.find(&titem);
  if(tblock!=NULL)
    tblock=dsm.nextiterator();
  // now tblock= current position, or previous one.
  // check if in middle of instruction
  if(tblock!=NULL)
  { // bugfix 2.27
    if(tblock->length>1)
      if(curraddr.between(tblock->addr,tblock->addr+tblock->length-1))
	   { curraddr.offs=tblock->addr.offs;
        subitem=tblock->type;
      }
  }
  // check if previous one. get next
  if(tblock!=NULL)
  { if((tblock->addr<curraddr)||
		((tblock->addr==curraddr)&&(tblock->type<subitem)))
		tblock=dsm.nextiterator();
  }
  // tblock is now top of the page.
  outhere=curraddr;
  ClearBuff();                        // start again
  for(i=0;i<buffer_lines;i++)
  { if(tblock!=NULL)
	 { if(outhere==tblock->addr)
		{ switch(tblock->type)
		  { case dsmcode:
				outinst(tblock,true);
				break;
			 case dsmnameloc:
            printlineheader(tblock->addr,true);
          	LastPrintBuff("%s:",tblock->data);
				break;
			 case dsmxref:
            printlineheader(tblock->addr,true);
				LastPrintBuff(";");
				LastPrintBuffEpos(COMMENTPOS);
				LastPrintBuff("XREFS First: ");
				xrefs.printfirst(tblock->addr);
				break;
			 default:
            printlineheader(tblock->addr,true);
				outcomment(tblock);
				break;
		  }
		  outhere+=tblock->length;
		  tblock=dsm.nextiterator();
		}
		else
		{ outdb(&outhere,true);
		  outhere++;
		}
	 }
	 else
	 { outdb(&outhere,true);
		outhere++;
	 }
	 // check if gone beyond seg, get next seg.
	 if(dta.beyondseg(outhere))
	 { outhere.offs--;
      dta.nextseg(&outhere);
    }
	 if(!outhere.segm)
      break;
	 else
      outend=outhere;
  }
  DoneBuff();                         // mark as done
  InvalidateRect(mainwindow,NULL,true);
}

/************************************************************************
* scroller                                                              *
* - this routine controls simple vertical scrolls. A vertical scroll is *
*   a movement of the currently selected line. Only if this line moves  *
*   beyond the boundaries of the window do we need to move the windowed *
*   disassembly output itself. Simple moves are handled quickly, and if *
*   we need to regenerate the buffers then we first recalculate the top *
*   position and then we call windowupdate to handle the rest.          *
************************************************************************/
void disio::scroller(dword amount)
{ dsmitem titem,*tblock;
  RECT r;
  float sbarpos;
  // simple move on screen
  if((amount==1)&&(usersel<=nScreenRows-3))
  { usersel++;
	 r.left=0;
	 r.right=rrr;
	 r.top=(usersel-1)*cyc;
	 r.bottom=(usersel+1)*cyc;
	 InvalidateRect(mainwindow,&r,true);
	 return;
  }
  // simple move on screen
  if((amount==(dword)(-1))&&(usersel))
  { usersel--;
	 r.left=0;
	 r.right=rrr;
	 r.top=usersel*cyc;
	 r.bottom=(usersel+2)*cyc;
	 InvalidateRect(mainwindow,&r,true);
	 return;
  }
  // find top of page
  titem.addr=curraddr;
  titem.type=subitem;
  tblock=dsm.find(&titem);
  // tblock is now top of page or previous item.
  // if moving up find previous inst
  if(amount<100)
    if(tblock!=NULL)
      tblock=dsm.nextiterator();
  if(amount>100)
	 if(tblock!=NULL)
		if((tblock->addr==titem.addr)&&(tblock->type==subitem))
			 tblock=dsm.lastiterator();
  // move up - tblock=previous inst.
  // if moving down, check if tblock=previous item.
  if((tblock!=NULL)&&(amount<100))
  { if((tblock->addr<curraddr)||
		((tblock->addr==curraddr)&&(tblock->type<subitem)))
		tblock=dsm.nextiterator();
  }
  // moving down - tblock=current top.
  if(amount<100)
	 while(amount)
	 { if(usersel<=nScreenRows-3)
        usersel++;
		else
		{ if(tblock!=NULL)
		  { if(curraddr==tblock->addr)
			 { curraddr+=tblock->length;
				tblock=dsm.nextiterator();
				if(tblock!=NULL)
				{ if(curraddr==tblock->addr)
                subitem=tblock->type;
				  else
                subitem=dsmcode;
				}
				else
              subitem=dsmcode;
			 }
			 else
			 { subitem=dsmnull;
				curraddr++;
			 }
		  }
		  else
		  { subitem=dsmnull;
			 curraddr++;
		  }
		  // check if gone beyond seg, get next seg.
		  if(dta.beyondseg(curraddr))
		  { curraddr.offs--;
          dta.nextseg(&curraddr);
			 subitem=dsmnull;
		  }
		  if(!curraddr.segm)
          break;
		  titem.addr=curraddr;
		}
		amount--;
	 }
  else
  { while(amount)
	 { if(usersel)
        usersel--;
		else
		{ if(tblock!=NULL)
		  { if(curraddr==tblock->addr+tblock->length)
			 { curraddr.offs-=tblock->length;
				subitem=tblock->type;
				tblock=dsm.lastiterator();
			 }
			 else
			 { subitem=dsmcode;
				curraddr.offs--;
			 }
		  }
		  else
		  { subitem=dsmnull;
			 curraddr.offs--;
		  }
		  // check if gone beyond seg, get previous seg.
		  if(dta.beyondseg(curraddr))
		  { curraddr++;
          dta.lastseg(&curraddr);
        }
		  if(!curraddr.segm)
          break;
		  titem.addr=curraddr;
		}
		amount++;
	 }
  }
  if(!curraddr.segm)
    curraddr=titem.addr;
  updatewindow();
  current_data_pos=dta.datagetpos(curraddr);
  sbarpos=((float)current_data_pos)/((float)total_data_size+(float)1.0)*(float)VERTSCROLLRANGE;
  SetScrollPos(mainwindow,SB_VERT,(int)sbarpos,true);
}

/************************************************************************
* outargs                                                               *
* - this is a very long routine which handles every kind of argument    *
*   that we have set up in the processor tables. It outputs the ascii   *
*   form of the instructions arguments to the buffer. It handles        *
*   complex modrm and sib encodings, the display of names and locations *
*   which must be decoded, etc.                                         *
************************************************************************/
void disio::outargs(dsmitem *inst,argtype a)
{ byte *dta,modrm,sib;
  // rm extended to word. build 15. re M Ogden and VC++ warnings.
  word rm;
  argtype a1,a2;
  dword targetd;
  dword targetw;
  lptr loc;
  char fbuff[40];
  int i,sp;           // sp= string printed
  byte pbyte; // prefix byte.
  int prefixptr;
  if(inst->flags&FLAGS_SEGPREFIX)
  { prefixptr=0;
    while(!issegprefix(inst->data[prefixptr])&&(prefixptr<15))
      prefixptr++;
    pbyte=inst->data[prefixptr];
  }
  if(inst->flags&FLAGS_ADDRPREFIX)
    options.mode32=!options.mode32;
  switch(a)
  { case ARG_REG_AX:
		if(inst->mode32)
        LastPrintBuff("eax");
		else
        LastPrintBuff("ax");
		break;
	 case ARG_REG_BX:
		if(inst->mode32)
        LastPrintBuff("ebx");
		else
        LastPrintBuff("bx");
		break;
	 case ARG_REG_CX:
		if(inst->mode32)
        LastPrintBuff("ecx");
		else
        LastPrintBuff("cx");
		break;
	 case ARG_REG_DX:
		if(inst->mode32)
        LastPrintBuff("edx");
		else
        LastPrintBuff("dx");
		break;
	 case ARG_16REG_DX:
		LastPrintBuff("dx");
		break;
	 case ARG_REG_SP:
		if(inst->mode32)
        LastPrintBuff("esp");
		else
        LastPrintBuff("sp");
		break;
	 case ARG_REG_BP:
		if(inst->mode32)
        LastPrintBuff("ebp");
		else
        LastPrintBuff("bp");
		break;
	 case ARG_REG_SI:
		if(inst->mode32)
        LastPrintBuff("esi");
		else
        LastPrintBuff("si");
		break;
	 case ARG_REG_DI:
		if(inst->mode32)
        LastPrintBuff("edi");
		else
        LastPrintBuff("di");
		break;
	 case ARG_REG_AL:
		LastPrintBuff("al");
		break;
	 case ARG_REG_AH:
		LastPrintBuff("ah");
		break;
	 case ARG_REG_BL:
		LastPrintBuff("bl");
		break;
	 case ARG_REG_BH:
		LastPrintBuff("bh");
		break;
	 case ARG_REG_CL:
		LastPrintBuff("cl");
		break;
	 case ARG_REG_CH:
		LastPrintBuff("ch");
		break;
	 case ARG_REG_DL:
		LastPrintBuff("dl");
		break;
	 case ARG_REG_DH:
		LastPrintBuff("dh");
		break;
	 case ARG_REG_ST0:
		LastPrintBuff("st(0)");
		break;
	 case ARG_REG_ES:
		LastPrintBuff("es");
		break;
	 case ARG_REG_CS:
		LastPrintBuff("cs");
		break;
	 case ARG_REG_DS:
		LastPrintBuff("ds");
		break;
	 case ARG_REG_SS:
		LastPrintBuff("ss");
		break;
	 case ARG_REG_FS:
		LastPrintBuff("fs");
		break;
	 case ARG_REG_GS:
		LastPrintBuff("gs");
		break;
	 case ARG_REG_A:
		LastPrintBuff("a");
		break;
	 case ARG_REG_B:
		LastPrintBuff("b");
		break;
	 case ARG_REG_C:
		LastPrintBuff("c");
		break;
	 case ARG_REG_D:
		LastPrintBuff("d");
		break;
	 case ARG_REG_E:
		LastPrintBuff("e");
		break;
	 case ARG_REG_H:
		LastPrintBuff("h");
		break;
	 case ARG_REG_L:
		LastPrintBuff("l");
		break;
	 case ARG_REG_I:
		LastPrintBuff("i");
		break;
	 case ARG_REG_R:
		LastPrintBuff("r");
		break;
	 case ARG_REG_HL_IND:
		LastPrintBuff("(hl)");
		break;
	 case ARG_REG_BC:
		LastPrintBuff("bc");
		break;
	 case ARG_REG_DE:
		LastPrintBuff("de");
		break;
	 case ARG_REG_HL:
		LastPrintBuff("hl");
		break;
	 case ARG_REG_BC_IND:
		LastPrintBuff("(bc)");
		break;
	 case ARG_REG_DE_IND:
		LastPrintBuff("(de)");
		break;
	 case ARG_REG_SP_IND:
		LastPrintBuff("(sp)");
		break;
	 case ARG_REG_IX:
		LastPrintBuff("ix");
		break;
	 case ARG_REG_IX_IND:
		LastPrintBuff("(ix");
		if(inst->flags&FLAGS_INDEXREG)
		{ dta=inst->data+2;
		  LastPrintBuff("+");
        LastPrintBuffLongHexValue((word)(dta[0]));
		}
		LastPrintBuff(")");
		break;
	 case ARG_REG_IY:
		LastPrintBuff("iy");
		break;
	 case ARG_REG_IY_IND:
		LastPrintBuff("(iy");
		if(inst->flags&FLAGS_INDEXREG)
		{ dta=inst->data+2;
		  LastPrintBuff("+");
        LastPrintBuffLongHexValue((word)(dta[0]));
		}
		LastPrintBuff(")");
		break;
	 case ARG_REG_C_IND:
		LastPrintBuff("(c)");
		break;
	 case ARG_REG_AF:
		LastPrintBuff("af");
		break;
	 case ARG_REG_AF2:
		LastPrintBuff("af\'");
		break;
	 case ARG_IMM:
		dta=inst->data+inst->length;
		if(inst->mode32)
		{ dta-=4;
		  switch(inst->override)
		  { case over_decimal:
            if(inst->displayflags&DISPFLAG_NEGATE)
				  LastPrintBuff("-%02lu",0-((dword *)dta)[0]);
            else
				  LastPrintBuff("%02lu",((dword *)dta)[0]);
				break;
			 case over_char:
				{ LastPrintBuff("\"");
				  for(i=3;i>=0;i--)
					 if(dta[i])
                  LastPrintBuff("%c",dta[i]);
				  LastPrintBuff("\"");
				}
				break;
			 case over_dsoffset:
				loc.assign(inst->addr.segm,((dword *)dta)[0]);
				LastPrintBuff("offset ");
				if(name.isname(loc))
              name.printname(loc);
			   else if(import.isname(loc))
              import.printname(loc);
			   else if(expt.isname(loc))
              expt.printname(loc);
				else
              LastPrintBuffLongHexValue(((dword *)dta)[0]);

				break;
          case over_single:
      		sprintf(fbuff,"(float)%g",((float *)dta)[0]);
      		LastPrintBuff(fbuff);
            break;
			 default:
            if(inst->displayflags&DISPFLAG_NEGATE)
     			{ LastPrintBuff("-");
              LastPrintBuffLongHexValue(0-((dword *)dta)[0]);
            }
            else
     			  LastPrintBuffLongHexValue(((dword *)dta)[0]);
				break;
		  }
		}
		else
		{ dta-=2;
		  if(inst->override==over_decimal)
        { if(inst->displayflags&DISPFLAG_NEGATE)
			   LastPrintBuff("-%02lu",0x10000-((word *)dta)[0]);
          else
			   LastPrintBuff("%02lu",((word *)dta)[0]);
        }
		  else if(inst->override==over_char)
		  { LastPrintBuff("\"");
			 for(i=1;i>=0;i--)
				if(dta[i])
              LastPrintBuff("%c",dta[i]);
			 LastPrintBuff("\"");
		  }
		  else
        { if(inst->displayflags&DISPFLAG_NEGATE)
			 { LastPrintBuff("-");
            LastPrintBuffLongHexValue(0x10000-((word *)dta)[0]);
          }
          else
			   LastPrintBuffLongHexValue(((word *)dta)[0]);
        }
		}
		break;
    case ARG_IMM_SINGLE:
      dta=inst->data+inst->length-4;
      sprintf(fbuff,"%g",((float *)dta)[0]);
      LastPrintBuff(fbuff);
      break;
    case ARG_IMM_DOUBLE:
      dta=inst->data+inst->length-8;
      sprintf(fbuff,"%g",((double *)dta)[0]);
      LastPrintBuff(fbuff);
      break;
    case ARG_IMM_LONGDOUBLE:
      dta=inst->data+inst->length-10;
      sprintf(fbuff,"%Lg",((long double *)dta)[0]);
      LastPrintBuff(fbuff);
      break;
	 case ARG_IMM32:
		dta=inst->data+inst->length-4;
		switch(inst->override)
		{ case over_decimal:
          if(inst->displayflags&DISPFLAG_NEGATE)
			   LastPrintBuff("-%02lu",0-((dword *)dta)[0]);
          else
			   LastPrintBuff("%02lu",((dword *)dta)[0]);
			 break;
		  case over_char:
			 LastPrintBuff("\"");
			 for(i=3;i>=0;i--)
				if(dta[i])
              LastPrintBuff("%c",dta[i]);
			 LastPrintBuff("\"");
			 break;
		  case over_dsoffset:
			 loc.assign(inst->addr.segm,((dword *)dta)[0]);
			 LastPrintBuff("offset ");
			 if(name.isname(loc))
            name.printname(loc);
	       else if(import.isname(loc))
            import.printname(loc);
		    else if(expt.isname(loc))
            expt.printname(loc);
			 else
            LastPrintBuffLongHexValue(loc.offs);
			 break;
        case over_single:
      	 sprintf(fbuff,"(float)%g",((float *)dta)[0]);
      	 LastPrintBuff(fbuff);
          break;
		  default:
          if(inst->displayflags&DISPFLAG_NEGATE)
			 { LastPrintBuff("-");
            LastPrintBuffLongHexValue(0-((dword *)dta)[0]);
          }
          else
			   LastPrintBuffLongHexValue(((dword *)dta)[0]);
			 break;
		}
		break;
	 case ARG_STRING:
      rm=0;
		LastPrintBuff("\"");
      sp=0;
      while(inst->data[rm])
      { if(inst->data[rm]<32)
        { LastPrintBuff("\",");
          LastPrintBuffHexValue(inst->data[rm]);
          LastPrintBuff(",\"");
        }
        else
      	 LastPrintBuff("%c",inst->data[rm]);
        rm++;
        sp++;
        if(sp>max_stringprint)
          break;
      }
		LastPrintBuff("\",00h");
		break;
	 case ARG_PSTRING:
		dta=inst->data;
		rm=dta[0];
		dta++;
      LastPrintBuffHexValue((unsigned char)rm);
		LastPrintBuff(",\"");
      sp=0;
		while(rm)
		{ if(dta[0]<32)
        { LastPrintBuff("\",");
          LastPrintBuffHexValue(dta[0]);
          LastPrintBuff(",\"");
        }
        else
      	 LastPrintBuff("%c",dta[0]);
		  dta++;
		  rm--;
        sp++;
        if(sp>max_stringprint)
          break;
		}
		LastPrintBuff("\"");
		break;
	 case ARG_DOSSTRING:
		dta=inst->data;
		rm=inst->length;
		rm--;
		LastPrintBuff("\"");
      sp=0;
		while(rm)
		{ if(dta[0]<32)
        { LastPrintBuff("\",");
          LastPrintBuffHexValue(dta[0]);
          LastPrintBuff(",\"");
        }
        else
          LastPrintBuff("%c",dta[0]);
		  dta++;
		  rm--;
        sp++;
        if(sp>max_stringprint)
          break;
		}
		LastPrintBuff("\"");
		break;
	 case ARG_CUNICODESTRING:
		dta=inst->data;
		rm=inst->length;
		rm-=(byte)2;
		LastPrintBuff("\"");
      sp=0;
		while(rm)
		{ if(dta[0]<32)
        { LastPrintBuff("\",");
          LastPrintBuffHexValue(dta[0]);
          LastPrintBuff(",\"");
        }
        else
          LastPrintBuff("%c",dta[0]);
		  dta+=2;
		  rm-=(byte)2;
        sp++;
        if(sp>max_stringprint)
          break;
		}
		LastPrintBuff("\"");
		break;
	 case ARG_PUNICODESTRING:
		dta=inst->data+2;
		rm=inst->length;
		rm-=(byte)2;
      sp=0;
		LastPrintBuffHexValue(rm/2);
		LastPrintBuff(",\"");
      while(rm)
		{ if(dta[0]<32)
        { LastPrintBuff("\",");
          LastPrintBuffHexValue(dta[0]);
          LastPrintBuff(",\"");
        }
        else
          LastPrintBuff("%c",dta[0]);
		  dta+=2;
		  rm-=(byte)2;
        sp++;
        if(sp>max_stringprint)
          break;
		}
		LastPrintBuff("\"");
		break;
	 case ARG_MEMLOC:
		if(inst->flags&FLAGS_SEGPREFIX)
        outprefix(pbyte);
		dta=inst->data+inst->length;
		if(options.mode32)
		{ dta-=4;
		  loc.assign(inst->addr.segm,((dword *)dta)[0]);
		  if(inst->flags&FLAGS_8BIT)
          LastPrintBuff("byte ptr ");
        else if(inst->flags&FLAGS_ADDRPREFIX)
          LastPrintBuff("word ptr");
        else
          LastPrintBuff("dword ptr");
        LastPrintBuff(" [");
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
        LastPrintBuff("]");
		}
		else
		{ dta-=2;
		  loc.assign(inst->addr.segm,((word *)dta)[0]);
		  if(inst->flags&FLAGS_8BIT)
          LastPrintBuff("byte ptr ");
        if(inst->flags&FLAGS_ADDRPREFIX)
          LastPrintBuff("dword ptr");
        else
          LastPrintBuff("word ptr");
        LastPrintBuff("[");
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
        LastPrintBuff("]");
		}
		break;
	 case ARG_MEMLOC16:
		if(inst->flags&FLAGS_SEGPREFIX)
        outprefix(pbyte);
		dta=inst->data+inst->length-2;
		if(options.processor==PROC_Z80)
		{ loc.assign(inst->addr.segm,((dword *)dta)[0]);
        LastPrintBuff("[");
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
        LastPrintBuff("]");
		}
		else
		{ loc.assign(inst->addr.segm,((word *)dta)[0]);
        LastPrintBuff("[");
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
        LastPrintBuff("]");
		}
		break;
	 case ARG_SIMM8:
		dta=inst->data+inst->length-1;
		if(inst->override==over_char)
		{ LastPrintBuff("\"");
		  for(i=0;i>=0;i--)
			 if(dta[i])
            LastPrintBuff("%c",dta[i]);
		  LastPrintBuff("\"");
		}
		else if(dta[0]&0x80)
		{ if(inst->override==over_decimal)
			 LastPrintBuff("%02lu",(word)(0x100-dta[0]));
		  else
		  { LastPrintBuff("-");
          LastPrintBuffLongHexValue((word)(0x100-dta[0]));
        }
		}
		else
		{ if(inst->override==over_decimal)
			 LastPrintBuff("%02lu",(word)(dta[0]));
		  else
			 LastPrintBuffLongHexValue((word)(dta[0]));
		}
		break;
	 case ARG_IMM8:
		dta=inst->data+inst->length-1;
		if(inst->override==over_decimal)
      { if(inst->displayflags&DISPFLAG_NEGATE)
		    LastPrintBuff("-%02lu",0x100-(word)(dta[0]));
        else
		    LastPrintBuff("%02lu",(word)(dta[0]));
      }
		else if(inst->override==over_char)
		{ LastPrintBuff("\"");
		  for(i=0;i>=0;i--)
			 if(dta[i])
            LastPrintBuff("%c",dta[i]);
		  LastPrintBuff("\"");
		}
		else
      { if(inst->displayflags&DISPFLAG_NEGATE)
		  {  LastPrintBuff("-");
           LastPrintBuffLongHexValue(0x100-(word)(dta[0]));
        }
        else
		    LastPrintBuffLongHexValue((word)(dta[0]));
      }
		break;
	 case ARG_IMM8_IND:
		dta=inst->data+inst->length-1;
		LastPrintBuff("(");
      LastPrintBuffLongHexValue((word)(dta[0]));
      LastPrintBuff(")");
		break;
	 case ARG_IMM16:
		dta=inst->data+inst->length-2;
		if(inst->override==over_decimal)
      { if(inst->displayflags&DISPFLAG_NEGATE)
		    LastPrintBuff("-%02lu",0x10000-((word *)dta)[0]);
        else
		    LastPrintBuff("%02lu",((word *)dta)[0]);
      }
		else if(inst->override==over_char)
		{ LastPrintBuff("\"");
		  for(i=1;i>=0;i--)
			 if(dta[i])
            LastPrintBuff("%c",dta[i]);
		  LastPrintBuff("\"");
		}
		else
      { if(inst->displayflags&DISPFLAG_NEGATE)
		  {  LastPrintBuff("-");
           LastPrintBuffLongHexValue(0x10000-((word *)dta)[0]);
        }
        else
		    LastPrintBuffLongHexValue(((word *)dta)[0]);
      }
		break;
	 case ARG_IMM16_A:
		dta=inst->data+inst->length-3;
		LastPrintBuffLongHexValue(((word *)dta)[0]);
		break;
	 case ARG_RELIMM8:
		dta=inst->data+inst->length-1;
		if(inst->mode32)
		{ if(dta[0]&0x80)
			 targetd=(dword)(dta[0]+0xffffff00+inst->addr.offs+inst->length);
		  else
			 targetd=(dword)(dta[0]+inst->addr.offs+inst->length);
		  loc.assign(inst->addr.segm,targetd);
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
		}
		else
		{ if(dta[0]&0x80)
			 targetw=(word)(dta[0]+0xff00+inst->addr.offs+inst->length);
		  else
			 targetw=(word)(dta[0]+inst->addr.offs+inst->length);
		  loc.assign(inst->addr.segm,targetw);
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
		}
		break;
	 case ARG_RELIMM:
		dta=inst->data+inst->length;
		if(inst->mode32)
		{ dta-=4;
		  targetd=((dword *)dta)[0]+inst->addr.offs+inst->length;
		  loc.assign(inst->addr.segm,targetd);
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
		}
		else
		{ dta-=2;
		  targetw=(word)(((word *)dta)[0]+inst->addr.offs+inst->length);
		  loc.assign(inst->addr.segm,targetw);
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuffLongHexValue(loc.offs);
		}
		break;
	 case ARG_REG:
		dta=inst->data+inst->modrm;
		if(options.processor==PROC_Z80)
        LastPrintBuff(regzascii[dta[0]&0x07]);
		else if(((asminstdata *)(inst->tptr))->flags&FLAGS_8BIT)
        LastPrintBuff(reg8ascii[(dta[0]>>3)&0x07]);
		else if(inst->mode32)
        LastPrintBuff(reg32ascii[(dta[0]>>3)&0x07]);
		else
        LastPrintBuff(reg16ascii[(dta[0]>>3)&0x07]);
		break;
	 case ARG_MREG:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regmascii[(dta[0]>>3)&0x07]);
		break;
	 case ARG_XREG:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regxascii[(dta[0]>>3)&0x07]);
		break;
	 case ARG_FREG:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regfascii[dta[0]&0x07]);
		break;
	 case ARG_SREG:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regsascii[(dta[0]>>3)&0x07]);
		break;
	 case ARG_CREG:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regcascii[(dta[0]>>3)&0x07]);
		break;
	 case ARG_DREG:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regdascii[(dta[0]>>3)&0x07]);
		break;
	 case ARG_TREG:
	 case ARG_TREG_67:
		dta=inst->data+inst->modrm;
		LastPrintBuff(regtascii[(dta[0]>>3)&0x07]);
		break;
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
	 case ARG_MODRM_BCD:
	 case ARG_MODRM_SINT:
	 case ARG_MODRM_EREAL:
	 case ARG_MODRM_DREAL:
	 case ARG_MODRM_WINT:
	 case ARG_MODRM_LINT:
	 case ARG_MODRM_FPTR:
	 case ARG_MODRM:
		dta=inst->data+inst->modrm;
		rm=(byte)((dta[0]&0xc0)>>6);
		modrm=(byte)(dta[0]&0x07);
		a1=DSMITEM_ARG1(inst);
		a2=DSMITEM_ARG2(inst);
      sib=dta[1];
		if((a1==ARG_IMM)||(a2==ARG_IMM)||(a1==ARG_IMM8)||(a2==ARG_IMM8)||(a2==ARG_NONE)
		  ||(a1==ARG_SIMM8)||(a2==ARG_SIMM8)||((modrm==5)&&(rm==0))||((modrm==4)&&(rm==2)&&(((sib)&0x07)==5))
        ||((modrm==4)&&(rm==0)&&(((sib)&0x07)==5)))
		{ if(rm<3)
		  { switch(a)
			 { case ARG_MODRM8:
				  LastPrintBuff("byte ptr ");
				  break;
				case ARG_MODRM16:
				case ARG_MODRM_WORD:
				  LastPrintBuff("word ptr ");
				  break;
				case ARG_MMXMODRM:
				case ARG_XMMMODRM:
				  LastPrintBuff("dword ptr ");
				  break;
				case ARG_MODRMQ:
				  LastPrintBuff("qword ptr ");
				  break;
				case ARG_MODRM_S:
              // 6 bytes=fword
				  LastPrintBuff("fword ptr ");
				  break;
				case ARG_MODRM_SREAL:
              // single real=4 bytes=dword
				  LastPrintBuff("dword ptr ");
				  break;
				case ARG_MODRM_BCD:
              // packed bcd=10 bytes=tbyte
				  LastPrintBuff("tbyte ptr ");
				  break;
				case ARG_MODRM_SINT:
              // short int=4 bytes
				  LastPrintBuff("dword ptr ");
				  break;
				case ARG_MODRM_WINT:
              // word int =2 bytes
				  LastPrintBuff("word ptr ");
				  break;
				case ARG_MODRM_LINT:
              // long int = 8 bytes
				  LastPrintBuff("qword ptr ");
				  break;
				case ARG_MODRMM512:
              // points to 512 bits=64 bytes of memory......
				  LastPrintBuff("byte ptr ");
				  break;
				case ARG_MODRM_EREAL:
              // extended real=10 bytes
				  LastPrintBuff("tbyte ptr ");
				  break;
				case ARG_MODRM_DREAL:
              // double real=8 bytes
				  LastPrintBuff("qword ptr ");
				  break;
				case ARG_MODRM:
				  if(inst->flags&FLAGS_8BIT)
                LastPrintBuff("byte ptr ");
				  else if(inst->mode32)
                LastPrintBuff("dword ptr ");
				  else
                LastPrintBuff("word ptr ");
				  break;
				default:
				  break;
			 }
		  }
		}
      else if ((a1==ARG_REG)||(a2==ARG_REG))
      {	if(rm<3)
      	{	switch(a)               // re movzx, movsx type instructions
         	{  case ARG_MODRM8:
					  LastPrintBuff("byte ptr ");
					  break;
            	case ARG_MODRM16:
					  LastPrintBuff("word ptr ");
					  break;
            	default:
            	  break;
            }
         }
      }
		switch(rm)
		{ case 0:
			 if(inst->flags&FLAGS_SEGPREFIX)
            outprefix(pbyte);
			 if(options.mode32)
			 { if(modrm==5)
				{ loc.assign(inst->addr.segm,((dword *)(&dta[1]))[0]);
              LastPrintBuff("[");
				  if(name.isname(loc))
                name.printname(loc);
				  else if(import.isname(loc))
                import.printname(loc);
              else if(expt.isname(loc))
                expt.printname(loc);
				  else
                LastPrintBuffLongHexValue(loc.offs);
        		  LastPrintBuff("]");
				}
				else if(modrm==4)        // case 4=sib
				{ sib=dta[1];
				  if((sib&0x07)==5) // disp32
				  { loc.assign(inst->addr.segm,((dword *)(&dta[2]))[0]);
                LastPrintBuff("[");
					 if(name.isname(loc))
					 {	name.printname(loc);
					 }
					 else if(import.isname(loc))
					 {	import.printname(loc);
					 }
					 else if(expt.isname(loc))
					 {	expt.printname(loc);
					 }
					 else
					 {	LastPrintBuffLongHexValue(loc.offs);
					 }
                LastPrintBuff("]");
				  }
				  else
				  { LastPrintBuff("[%s]",reg32ascii[sib&0x07]);
				  }
				  if(((sib>>3)&0x07)==4) // no scaled index reg
				  {
				  }
				  else
				  { LastPrintBuff("[%s",reg32ascii[(sib>>3)&0x07]);
					 switch(sib>>6)
					 { case 0:
						  LastPrintBuff("]");
						  break;
						case 1:
						  LastPrintBuff("*2]");
						  break;
						case 2:
						  LastPrintBuff("*4]");
						  break;
						case 3:
						  LastPrintBuff("*8]");
						  break;
					 }
				  }
				}
				else
				  LastPrintBuff("[%s]",reg32ascii[dta[0]&0x07]);
			 }
			 else
			 { if(modrm==6)
				{ loc.assign(inst->addr.segm,((word *)(&dta[1]))[0]);
				  LastPrintBuff("[");
              if(name.isname(loc))
                name.printname(loc);
				  else if(import.isname(loc))
                import.printname(loc);
				  else if(expt.isname(loc))
                expt.printname(loc);
				  else
                LastPrintBuffLongHexValue(loc.offs);
              LastPrintBuff("]");
				}
				else
				  LastPrintBuff("[%s]",regind16ascii[dta[0]&0x07]);
			 }
			 break;
		  case 1:
			 if(inst->flags&FLAGS_SEGPREFIX)
            outprefix(pbyte);
			 if(options.mode32)
			 {	if(modrm==4)        // case 4=sib
				{ sib=dta[1];
				  if(dta[2]&0x80)
				  { LastPrintBuff("[%s-",reg32ascii[dta[1]&0x07]);
				    LastPrintBuffHexValue((byte)(-dta[2]));
              }
				  else
				  { LastPrintBuff("[%s+",reg32ascii[dta[1]&0x07]);
				    LastPrintBuffHexValue(dta[2]);
              }
				  if(((sib>>3)&0x07)==4) // no scaled index reg
					 LastPrintBuff("]");
				  else
				  { LastPrintBuff("][%s",reg32ascii[(sib>>3)&0x07]);
					 switch(sib>>6)
					 { case 0:
						  LastPrintBuff("]");
						  break;
						case 1:
						  LastPrintBuff("*2]");
						  break;
						case 2:
						  LastPrintBuff("*4]");
						  break;
						case 3:
						  LastPrintBuff("*8]");
						  break;
					 }
				  }
				}
				else if(dta[1]&0x80)
				{ LastPrintBuff("[%s-",reg32ascii[dta[0]&0x07]);
				  LastPrintBuffHexValue((byte)(-dta[1]));
				  LastPrintBuff("]");
            }
				else
				{ LastPrintBuff("[%s+",reg32ascii[dta[0]&0x07]);
				  LastPrintBuffHexValue(dta[1]);
				  LastPrintBuff("]");
            }
			 }
			 else
			 { if(dta[1]&0x80)
				{ LastPrintBuff("[%s-",regind16ascii[dta[0]&0x07]);
				  LastPrintBuffHexValue((byte)(-dta[1]));
				  LastPrintBuff("]");
            }
				else
				{ LastPrintBuff("[%s+",regind16ascii[dta[0]&0x07]);
				  LastPrintBuffHexValue(dta[1]);
				  LastPrintBuff("]");
            }
			 }
			 break;
		  case 2:
			 if(inst->flags&FLAGS_SEGPREFIX)
            outprefix(pbyte);
			 if(options.mode32)
			 { loc.assign(inst->addr.segm,((dword *)(&dta[1]))[0]);
				if(modrm==4)        // case 4=sib
				{ sib=dta[1];
				  loc.assign(inst->addr.segm,((dword *)(&dta[2]))[0]);
              LastPrintBuff("[");
              if(name.isname(loc))
				  { LastPrintBuff("%s+",reg32ascii[sib&0x07]);
				    name.printname(loc);
				  }
				  else if(import.isname(loc))
				  { LastPrintBuff("%s+",reg32ascii[sib&0x07]);
				    import.printname(loc);
				  }
              else if(expt.isname(loc))
				  { LastPrintBuff("%s+",reg32ascii[sib&0x07]);
				    expt.printname(loc);
				  }
				  else if(dta[5]&0x80)
				  { LastPrintBuff("%s-",reg32ascii[sib&0x07]);
                LastPrintBuffLongHexValue(0-((dword *)(&dta[2]))[0]);
              }
				  else
				  { LastPrintBuff("%s+",reg32ascii[sib&0x07]);
                LastPrintBuffLongHexValue(((dword *)(&dta[2]))[0]);
              }
				  if(((sib>>3)&0x07)==4) // no scaled index reg
					 LastPrintBuff("]");
				  else
				  { LastPrintBuff("][%s",reg32ascii[(sib>>3)&0x07]);
					 switch(sib>>6)
					 { case 0:
						  LastPrintBuff("]");
						  break;
						case 1:
						  LastPrintBuff("*2]");
						  break;
						case 2:
						  LastPrintBuff("*4]");
						  break;
						case 3:
						  LastPrintBuff("*8]");
						  break;
					 }
				  }
				}
				else if(name.isname(loc))
				{ name.printname(loc);
				  LastPrintBuff("[%s]",reg32ascii[dta[0]&0x07]);
				}
				else if(import.isname(loc))
				{ import.printname(loc);
				  LastPrintBuff("[%s]",reg32ascii[dta[0]&0x07]);
				}
				else if(expt.isname(loc))
				{ expt.printname(loc);
				  LastPrintBuff("[%s]",reg32ascii[dta[0]&0x07]);
				}
				else if(dta[4]&0x80)
				{ LastPrintBuff("[%s-",reg32ascii[dta[0]&0x07]);
              LastPrintBuffLongHexValue(0-((dword *)(&dta[1]))[0]);
              LastPrintBuff("]");
            }
				else
				{ LastPrintBuff("[%s+",reg32ascii[dta[0]&0x07]);
              LastPrintBuffLongHexValue(((dword *)(&dta[1]))[0]);
              LastPrintBuff("]");
            }
			 }
			 else
			 { loc.assign(inst->addr.segm,((word *)(&dta[1]))[0]);
				if(name.isname(loc))
				{ name.printname(loc);
				  LastPrintBuff("[%s]",regind16ascii[dta[0]&0x07]);
				}
				else if(import.isname(loc))
				{ import.printname(loc);
				  LastPrintBuff("[%s]",regind16ascii[dta[0]&0x07]);
				}
				else if(expt.isname(loc))
				{ expt.printname(loc);
				  LastPrintBuff("[%s]",regind16ascii[dta[0]&0x07]);
				}
				else if(dta[2]&0x80)
				{ LastPrintBuff("[%s-",regind16ascii[dta[0]&0x07]);
              LastPrintBuffLongHexValue(0x10000-((word *)(&dta[1]))[0]);
              LastPrintBuff("]");
            }
				else
				{ LastPrintBuff("[%s+",regind16ascii[dta[0]&0x07]);
              LastPrintBuffLongHexValue(((word *)(&dta[1]))[0]);
              LastPrintBuff("]");
            }
			 }
			 break;
		  case 3:
			 if(a==ARG_MMXMODRM)
            LastPrintBuff(regmascii[dta[0]&0x07]);
			 else if(a==ARG_XMMMODRM)
            LastPrintBuff(regxascii[dta[0]&0x07]);
			 else if((((asminstdata *)(inst->tptr))->flags&FLAGS_8BIT)||(a==ARG_MODRM8))
            LastPrintBuff(reg8ascii[dta[0]&0x07]);
			 else if((inst->mode32)&&(a!=ARG_MODRM16))
            LastPrintBuff(reg32ascii[dta[0]&0x07]);
			 else
            LastPrintBuff(reg16ascii[dta[0]&0x07]);
			 break;
		}
		break;
	 case ARG_IMM_1:
		if(inst->override==over_decimal)
		  LastPrintBuff("1");
		else
		  LastPrintBuff("1h");
		break;
	 case ARG_FADDR:
		dta=inst->data+inst->length;
		if(options.mode32)
		{ dta-=6;
		  loc.assign(((word *)(&dta[4]))[0],((dword *)(&dta[0]))[0]);
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuff("%04x:%08lxh",loc.segm,loc.offs);
		}
		else
		{ dta-=4;
		  loc.assign(((word *)(&dta[2]))[0],((word *)(&dta[0]))[0]);
		  if(name.isname(loc))
          name.printname(loc);
		  else if(import.isname(loc))
          import.printname(loc);
		  else if(expt.isname(loc))
          expt.printname(loc);
		  else
          LastPrintBuff("%04x:%04xh",loc.segm,loc.offs);
		}
		break;
	 case ARG_BIT:
		dta=inst->data+inst->length-1;
		LastPrintBuff("%x",(dta[0]>>3)&7);
		break;
	 case ARG_NONE:
		if(inst->flags&FLAGS_SEGPREFIX)
        outprefix(pbyte);
		break;
	 case ARG_NONEBYTE:
	 default:
		break;
  }
  if(inst->flags&FLAGS_ADDRPREFIX)
    options.mode32=!options.mode32;
}

/************************************************************************
* outcomment                                                            *
* - prints a disassembly comment to the buffer                          *
************************************************************************/
void disio::outcomment(dsmitem *inst)
{ LastPrintBuff(";%s",inst->tptr);
}

/************************************************************************
* dumpblocktofile                                                       *
* - this is the text and asm output routine. It needs a lot more work   *
*   doing to it and is presently a fairly simple dump of a block of     *
*   code.                                                               *
* - this routine was a quick hack and complete rip of the dumptofile    *
*   routine which follows. the two routines need the common workings    *
*   put together and both need rewriting                                *
************************************************************************/
void disio::dumpblocktofile(char *fname,bool printaddrs)
{ SECURITY_ATTRIBUTES securityatt;
  HANDLE efile;
  dword num;
  dsmitem *tblock,fdsm;
  dsegitem *dblock,*nxtblock;
  lptr outhere,nptr;
  int i;
  char ehdr[300];
  if(!blk.checkblock())
    return;
  securityatt.nLength=sizeof(SECURITY_ATTRIBUTES);
  securityatt.lpSecurityDescriptor=NULL;
  securityatt.bInheritHandle=false;
  efile=CreateFile(fname,GENERIC_WRITE,0,&securityatt,CREATE_NEW,0,NULL);
  if(efile==INVALID_HANDLE_VALUE)
  { MessageBox(mainwindow,"File Creation Failed","Borg Disassembler Alert",MB_OK);
	 return;
  }
  scheduler.stopthread();
  WriteFile(efile,"; ",2,&num,NULL);
  WriteFile(efile,winname,strlen(winname),&num,NULL);
  WriteFile(efile,"\r\n;\r\n",5,&num,NULL);
  WriteFile(efile,hdr,strlen(hdr),&num,NULL);
  WriteFile(efile,hdr2,strlen(hdr2),&num,NULL);
  WriteFile(efile,";\r\n",3,&num,NULL);
  wsprintf(ehdr,"; block dump from: %04x:%08lxh to %04x:%08lxh",blk.top.segm,blk.top.offs,blk.bottom.segm,blk.bottom.offs);
  WriteFile(efile,ehdr,strlen(ehdr),&num,NULL);
  WriteFile(efile,"\r\n",2,&num,NULL);
  WriteFile(efile,"\r\n",2,&num,NULL);
  // find current position.
  fdsm.addr=blk.top;
  fdsm.type=dsmnull;
  tblock=dsm.find(&fdsm);
  while(tblock!=NULL)
  { if(tblock->addr<blk.top)
      tblock=dsm.nextiterator();
    else
      break;
  }
  // now tblock= first position
  dblock=dta.findseg(blk.top);
  outhere=blk.top;
  while((dblock!=NULL)&&(outhere<=blk.bottom))
  { ClearBuff(); // clear buffer - ready to start
	 for(i=0;i<buffer_lines-1;i++)
	 { if(tblock!=NULL)
		{ if(outhere==tblock->addr)
		  { switch(tblock->type)
			 { case dsmcode:
				  outinst(tblock,printaddrs);
				  break;
				case dsmnameloc:
              printlineheader(tblock->addr,printaddrs);
              LastPrintBuff("%s:",tblock->data);
				  break;
				case dsmxref:
				  // temp measure - print first (build 17)
              printlineheader(tblock->addr,printaddrs);
				  LastPrintBuff(";");
				  LastPrintBuffEpos(COMMENTPOS);
				  LastPrintBuff("XREFS First: ");
				  xrefs.printfirst(tblock->addr);
				  break;
				default:
              printlineheader(tblock->addr,printaddrs);
				  outcomment(tblock);
				  break;
			 }
			 outhere+=tblock->length;
			 tblock=dsm.nextiterator();
		  }
		  else
		  { outdb(&outhere,printaddrs);
			 outhere++;
		  }
		}
		else
		{ outdb(&outhere,printaddrs);
		  outhere++;
		}
		// check if gone beyond seg, get next seg.
		/*if(dta.beyondseg(outhere))*/     // changed build 14
      // rewritten build 17. seeks dseg from start, and finds next now.
		if(outhere>=(dblock->addr+dblock->size))
		{ dta.resetiterator();
		  nxtblock=dta.nextiterator();
		  while(nxtblock!=NULL)
		  { if(nxtblock->addr==dblock->addr)
			 { dblock=dta.nextiterator();
				break;
			 }
			 nxtblock=dta.nextiterator();
			 if(nxtblock==NULL)
            dblock=NULL;
		  }
		  if(dblock==NULL)
          break;
		  outhere=dblock->addr;
		}
      if(outhere>blk.bottom)
        break;
		if(!outhere.segm)
        break;
	 }
	 DumpBuff(efile);
  }
  DoneBuff();
  CloseHandle(efile);
  scheduler.continuethread();
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* dumptofile                                                            *
* - this is the text and asm output routine. It needs a lot more work   *
*   doing to it and is presently a fairly simple dump of the code.      *
************************************************************************/
void disio::dumptofile(char *fname,bool printaddrs)
{ SECURITY_ATTRIBUTES securityatt;
  HANDLE efile;
  dword num;
  dsmitem *tblock;
  dsegitem *dblock,*nxtblock;
  lptr outhere,nptr;
  int i;
  securityatt.nLength=sizeof(SECURITY_ATTRIBUTES);
  securityatt.lpSecurityDescriptor=NULL;
  securityatt.bInheritHandle=false;
  efile=CreateFile(fname,GENERIC_WRITE,0,&securityatt,CREATE_NEW,0,NULL);
  if(efile==INVALID_HANDLE_VALUE)
  { MessageBox(mainwindow,"File Creation Failed","Borg Disassembler Alert",MB_OK);
	 return;
  }
  scheduler.stopthread();
  WriteFile(efile,"; ",2,&num,NULL);
  WriteFile(efile,winname,strlen(winname),&num,NULL);
  WriteFile(efile,"\r\n;\r\n",5,&num,NULL);
  WriteFile(efile,hdr,strlen(hdr),&num,NULL);
  WriteFile(efile,hdr2,strlen(hdr2),&num,NULL);
  WriteFile(efile,"\r\n",2,&num,NULL);
  // find current position.
  dsm.resetiterator();
  tblock=dsm.nextiterator();
  // now tblock= first position
  dta.resetiterator();
  dblock=dta.nextiterator();
  outhere=dblock->addr;
  while(dblock!=NULL)
  { ClearBuff(); // clear buffer - ready to start
	 for(i=0;i<buffer_lines-1;i++)
	 { if(tblock!=NULL)
		{ if(outhere==tblock->addr)
		  { switch(tblock->type)
			 { case dsmcode:
				  outinst(tblock,printaddrs);
				  break;
				case dsmnameloc:
              printlineheader(tblock->addr,printaddrs);
              LastPrintBuff("%s:",tblock->data);
				  break;
				case dsmxref:
				  // temp measure - print first (build 17)
              printlineheader(tblock->addr,printaddrs);
				  LastPrintBuff(";");
				  LastPrintBuffEpos(COMMENTPOS);
				  LastPrintBuff("XREFS First: ");
				  xrefs.printfirst(tblock->addr);
				  break;
				default:
              printlineheader(tblock->addr,printaddrs);
				  outcomment(tblock);
				  break;
			 }
			 outhere+=tblock->length;
			 tblock=dsm.nextiterator();
		  }
		  else
		  { outdb(&outhere,printaddrs);
			 outhere++;
		  }
		}
		else
		{ outdb(&outhere,printaddrs);
		  outhere++;
		}
		// check if gone beyond seg, get next seg.
		/*if(dta.beyondseg(outhere))*/     // changed build 14
      // rewritten build 17. seeks dseg from start, and finds next now.
		if(outhere>=(dblock->addr+dblock->size))
		{ dta.resetiterator();
		  nxtblock=dta.nextiterator();
		  while(nxtblock!=NULL)
		  { if(nxtblock->addr==dblock->addr)
			 { dblock=dta.nextiterator();
				break;
			 }
			 nxtblock=dta.nextiterator();
			 if(nxtblock==NULL)
            dblock=NULL;
		  }
		  if(dblock==NULL)
          break;
		  outhere=dblock->addr;
		}
		if(!outhere.segm)
        break;
	 }
	 DumpBuff(efile);
  }
  DoneBuff();
  CloseHandle(efile);
  scheduler.continuethread();
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}


