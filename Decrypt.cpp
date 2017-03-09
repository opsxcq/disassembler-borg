/************************************************************************
*              decrypt.cpp                                              *
* This class adds some simple block decryption/encryption with file     *
* patching to Borg. By storing decryptors in blocks it is possible to   *
* reconstruct the file when it is saved to database and reloaded, even  *
* if some patches were applied to the file and some were not.           *
* Added in Borg 2.19                                                    *
* NB Any future general file patching will need to be included in this  *
* class and saved in a similar way. This opens up the way to decrypting *
* patching and reencrypting within Borg :)                              *
* Current decryptors are fairly simple, but when used in combination    *
* they are very powerful. The Xadd is a bit obscure, but could be       *
* simply modified as needed, and recompiled for some powerful routines. *
************************************************************************/
#include <windows.h>
#include <stdio.h>

#include "decrypt.h"
#include "data.h"
#include "disio.h"
#include "disasm.h"
#include "exeload.h"
#include "debug.h"

/************************************************************************
* constructor function                                                  *
* - resets a few global variables                                       *
************************************************************************/
decrypt::decrypt()
{ nextitemnum=1;
  loading_db=false;
}

/************************************************************************
* destructor function                                                   *
* - currently null                                                      *
************************************************************************/
decrypt::~decrypt()
{
}

/************************************************************************
* compare function for decryptor                                        *
* - these are simply stored in uid order, the uid being increased each  *
*   time a new one is applied                                           *
************************************************************************/
int decrypt::compare(declist *i,declist *j)
{ if(i->uid == j->uid)
	 return 0;
  if(i->uid > j->uid)
	 return 1;
  return -1;
}

/************************************************************************
* add_decrypted                                                         *
* - just adds another item to the decrypt list                          *
* - the decrypt list is simply a list of blocks and how they were       *
*   changed and whether the exe was patched                             *
* - the list is just to enable reconstruction of the state of the file  *
*   on saving and loading databases with decryptors which may or may    *
*   not have been saved to the exe file                                 *
************************************************************************/
dword decrypt::add_decrypted(lptr dstart,lptr dend,dectype t,ditemtype ditem,dword val,lptr adr,bool patchedexe)
{ declist *ndec;
  ndec=new struct declist;
  ndec->dec_start=dstart;
  ndec->dec_end=dend;
  ndec->typ=t;
  ndec->dlength=ditem;
  ndec->value=val;
  ndec->arrayaddr=adr;
  ndec->patch=patchedexe;
  ndec->uid=nextitemnum;
  nextitemnum++;
  addto(ndec);
  return ndec->uid;
}

/************************************************************************
* process_dec                                                           *
* - this processes a decryptor given the uid, actually applying it to   *
*   the file in memory. If a block contains any disassembly then this   *
*   is also deleted.                                                    *
************************************************************************/
void decrypt::process_dec(dword dec_id)
{ declist fnd,*patch;
  dsegitem *pseg,*aseg;
  lptr cpos;
  unsigned int plen,ctr;
  dword doitval,lval,tval;
  fnd.uid=dec_id;
  patch=find(&fnd);
  if(patch==NULL)
    return;
  if(patch->uid!=dec_id)
    return;
  pseg=dta.findseg(patch->dec_start);
  if(pseg==NULL)
    return;
  ctr=0;
  switch(patch->dlength)
  { case decbyte:
      plen=1;
      break;
    case decword:
      plen=2;
      break;
    case decdword:
      plen=4;
      break;
    case decarray:
      plen=1;
      aseg=dta.findseg(patch->arrayaddr);
      if(aseg==NULL)
        return;
      ctr=patch->arrayaddr-aseg->addr;
      break;
    default:
      plen=1;
      break;
  }
  cpos=patch->dec_start;
  lval=patch->value;
  while(cpos<=patch->dec_end)
  { // check within seg, and move to the next seg if we arent
    while(cpos>pseg->addr+(pseg->size-plen))
    { cpos=pseg->addr+(pseg->size-1);
      dta.nextseg(&cpos);
      if(!cpos.segm)
        break;
      if(cpos>patch->dec_end)
        break;
      pseg=dta.findseg(cpos);
    }
    if(!cpos.segm)
      break;
    if(cpos>patch->dec_end)
      break;
    switch(plen)
    { case 1:
        doitval=((byte *)(pseg->data+(cpos-pseg->addr)))[0];
        break;
      case 2:
        doitval=((word *)(pseg->data+(cpos-pseg->addr)))[0];
        break;
      case 4:
        doitval=((dword *)(pseg->data+(cpos-pseg->addr)))[0];
        break;
    }
    if(patch->dlength==decarray)
    { if(ctr+plen>aseg->size)
        break;
      switch(plen)
      { case 1:
          patch->value=((byte *)(aseg->data+ctr))[0];
          break;
        case 2:
          patch->value=((word *)(aseg->data+ctr))[0];
          break;
        case 4:
          patch->value=((dword *)(aseg->data+ctr))[0];
          break;
      }
    }
    switch(patch->typ)
    { case decxor:
        doitval=doitval^patch->value;
        break;
      case decmul:
        doitval=doitval*patch->value;
        break;
      case decadd:
        doitval=doitval+patch->value;
        break;
      case decsub:
        doitval=doitval-patch->value;
        break;
      case decxadd:
        tval=doitval;
        doitval=lval;
        lval=tval;
        doitval=doitval+lval;
        break;
      case decrot:
        switch(plen)
        { case 1:
            doitval=(doitval<<(patch->value&0x07))+(doitval>>(8-(patch->value&0x07)));
            break;
          case 2:
            doitval=(doitval<<(patch->value&0x0f))+(doitval>>(16-(patch->value&0x0f)));
            break;
          case 4:
            doitval=(doitval<<(patch->value&0x1f))+(doitval>>(32-(patch->value&0x1f)));
            break;
        }
        break;
      default:
        break;
    }
    switch(plen)
    { case 1:
        ((byte *)(pseg->data+(cpos-pseg->addr)))[0]=(byte)doitval;
        break;
      case 2:
        ((word *)(pseg->data+(cpos-pseg->addr)))[0]=(word)doitval;
        break;
      case 4:
        ((dword *)(pseg->data+(cpos-pseg->addr)))[0]=doitval;
        break;
    }
    cpos+=plen;
    ctr+=plen;
  }
  if(!loading_db)
    dsm.undefineblock(patch->dec_start,patch->dec_end);
  dio.updatewindowifwithinrange(patch->dec_start,patch->dec_end);
}

/************************************************************************
* exepatch                                                              *
* - given a uid this steps through a decryptor and writes the patch to  *
*   the exe file.                                                       *
************************************************************************/
void decrypt::exepatch(dword dec_id)
{ declist fnd,*patch;
  dsegitem *pseg;
  lptr cpos;
  int plen;
  dword doitval;
  fnd.uid=dec_id;
  patch=find(&fnd);
  if(patch==NULL)
    return;
  if(patch->uid!=dec_id)
    return;
  pseg=dta.findseg(patch->dec_start);
  switch(patch->dlength)
  { case decbyte:
      plen=1;
      break;
    case decword:
      plen=2;
      break;
    case decdword:
      plen=4;
      break;
    case decarray:
      plen=1;
      break;
    default:
      plen=1;
      break;
  }
  cpos=patch->dec_start;
  while(cpos<=patch->dec_end)
  { // check within seg, and move to the next seg if we arent
    while(cpos>pseg->addr+(pseg->size-plen))
    { cpos=pseg->addr+(pseg->size-1);
      dta.nextseg(&cpos);
      if(!cpos.segm)
        break;
      if(cpos>patch->dec_end)
        break;
      pseg=dta.findseg(cpos);
    }
    if(!cpos.segm)
      break;
    if(cpos>patch->dec_end)
      break;
    doitval=floader.fileoffset(cpos);
    // write patch
    switch(plen)
    { case 1:
        floader.patchfile(doitval,1,pseg->data+(cpos-pseg->addr));
        break;
      case 2:
        floader.patchfile(doitval,2,pseg->data+(cpos-pseg->addr));
        break;
      case 4:
        floader.patchfile(doitval,4,pseg->data+(cpos-pseg->addr));
        break;
      default:
        floader.patchfile(doitval,1,pseg->data+(cpos-pseg->addr));
        break;
    }
    cpos+=plen;
  }
}

/************************************************************************
* process_reload                                                        *
* - given a uid this steps through a patch and re-reads the bytes in    *
*   that would have been changed. This is used in file reconstruction   *
************************************************************************/
void decrypt::process_reload(dword dec_id)
{ declist fnd,*patch;
  dsegitem *pseg;
  lptr cpos;
  int plen;
  dword doitval;
  fnd.uid=dec_id;
  patch=find(&fnd);
  if(patch==NULL)
    return;
  if(patch->uid!=dec_id)
    return;
  pseg=dta.findseg(patch->dec_start);
  switch(patch->dlength)
  { case decbyte:
      plen=1;
      break;
    case decword:
      plen=2;
      break;
    case decdword:
      plen=4;
      break;
    case decarray:
      plen=1;
      break;
    default:
      plen=1;
      break;
  }
  cpos=patch->dec_start;
  while(cpos<=patch->dec_end)
  { // check within seg, and move to the next seg if we arent
    while(cpos>pseg->addr+(pseg->size-plen))
    { cpos=pseg->addr+(pseg->size-1);
      dta.nextseg(&cpos);
      if(!cpos.segm)
        break;
      if(cpos>patch->dec_end)
        break;
      pseg=dta.findseg(cpos);
    }
    if(!cpos.segm)
      break;
    if(cpos>patch->dec_end)
      break;
    doitval=floader.fileoffset(cpos);
    // write patch
    switch(plen)
    { case 1:
        floader.reloadfile(doitval,1,pseg->data+(cpos-pseg->addr));
        break;
      case 2:
        floader.reloadfile(doitval,2,pseg->data+(cpos-pseg->addr));
        break;
      case 4:
        floader.reloadfile(doitval,4,pseg->data+(cpos-pseg->addr));
        break;
      default:
        floader.reloadfile(doitval,1,pseg->data+(cpos-pseg->addr));
        break;
    }
    cpos+=plen;
  }
}

/************************************************************************
* write_item                                                            *
* - writes a decrypt item to the savefile specified                     *
*   uses the current item, and moves the iterator on                    *
************************************************************************/
bool decrypt::write_item(savefile *sf)
{ declist *currdec;
  currdec=nextiterator();
  if(!sf->swrite(currdec,sizeof(declist)))
    return false;
  return true;
}

/************************************************************************
* read_item                                                             *
* - read a decrypt item from the savefile specified                     *
*   adds it to the list and restores any patch                          *
*   If we find a decryptor which was saved to disk then we              *
*   reload that block from the exe file. In this way after all of the   *
*   decryptors have been loaded we have synchronised the file in memory *
*   to the file on disk, plus any further patches made but not written  *
*   to disk. [Any byte in the file was synchronised to the file at the  *
*   time of the last patch which was written to file. Subsequent        *
*   patches have been made to memory only, and are just redone. So the  *
*   loaded file is in the same state as when the database was saved]    *
************************************************************************/
bool decrypt::read_item(savefile *sf)
{ dword num;
  declist *currdec;
  currdec=new declist;
  if(!sf->sread(currdec,sizeof(declist),&num))
    return false;
  addto(currdec);
  nextitemnum=currdec->uid+1;
  loading_db=true;
  if(!currdec->patch)
    process_dec(currdec->uid);
  else
    process_reload(currdec->uid);
  loading_db=false;
  return true;
}

