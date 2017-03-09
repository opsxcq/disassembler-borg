/************************************************************************
*                  exeload.cpp                                          *
* Contains the executable file load routines and setting up of the      *
* disassembly for the files..                                           *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "dasm.h"
#include "exeload.h"
#include "data.h"
#include "disio.h"
#include "disasm.h"
#include "schedule.h"
#include "relocs.h"
#include "gname.h"
#include "debug.h"

/************************************************************************
* constructor function                                                  *
* - resets file details.                                                *
************************************************************************/
fileloader::fileloader(void)
{ efile=INVALID_HANDLE_VALUE;
  exetype=0;
  fbuff=NULL;
}

/************************************************************************
* destructor function                                                   *
* - closes and file that is open.                                       *
************************************************************************/
fileloader::~fileloader(void)
{ if(fbuff!=NULL)
    delete fbuff;
  CloseHandle(efile);
}

/************************************************************************
* fileoffset                                                            *
* - function which returns the offset in a file of a given location,    *
*   this is to enable file patching given a location to patch           *
* - added in Borg v2.19                                                 *
************************************************************************/
dword fileloader::fileoffset(lptr loc)
{ dsegitem *ds;
  ds=dta.findseg(loc);
  if(ds==NULL)
    return 0;
  return (loc-ds->addr)+(ds->data-fbuff);
}

/************************************************************************
* patchfile                                                             *
* - writes to the currently open file (does not check it is opened with *
*   write access), given the number of bytes, data, and file offset to  *
*   write to.                                                           *
* - added in Borg v2.19                                                 *
************************************************************************/
void fileloader::patchfile(dword file_offs,dword num,byte *dat)
{ dword written;
  if(efile==INVALID_HANDLE_VALUE)
  { MessageBox(mainwindow,"File I/O Error - Invalid Handle for writing","Borg Message",MB_OK);
    return;
  }
  SetFilePointer(efile,file_offs,NULL,FILE_BEGIN);
  WriteFile(efile,dat,num,&written,NULL);
}

/************************************************************************
* patchoep                                                              *
* - writes to the currently open file (does not check it is opened with *
*   write access), given the new oep for the file is in options.oep     *
* - added in Borg v2.28                                                 *
************************************************************************/
void fileloader::patchoep(void)
{ if(exetype!=PE_EXE)
    return;
  peh->entrypoint_rva=options.oep.offs-peh->image_base;
  patchfile((byte *)&peh->entrypoint_rva-fbuff,4,(byte *)&peh->entrypoint_rva);
}

/************************************************************************
* reloadfile                                                            *
* - reads part of a file back in, given the file offset, number of      *
*   bytes and data buffer to read it in to. Used in the decryptor when  *
*   reloading a database file.                                          *
************************************************************************/
void fileloader::reloadfile(dword file_offs,dword num,byte *dat)
{ dword rd;
  if(efile==INVALID_HANDLE_VALUE)
  { MessageBox(mainwindow,"File I/O Error - Invalid Handle for reading","Borg Message",MB_OK);
    return;
  }
  SetFilePointer(efile,file_offs,NULL,FILE_BEGIN);
  ReadFile(efile,dat,num,&rd,NULL);
}

/************************************************************************
* readcomfile                                                           *
* - one of the simpler exe format loading routines, we just need to     *
*   load the file and disassemble from the start with an offset of      *
*   0x100                                                               *
************************************************************************/
void fileloader::readcomfile(dword fsize)
{ options.loadaddr.offs=0x100;
  options.dseg=options.loadaddr.segm;
  dta.addseg(options.loadaddr,fsize,fbuff,code16,NULL);
  dta.possibleentrycode(options.loadaddr);
  options.mode16=true;
  options.mode32=false;
  dio.setcuraddr(options.loadaddr);
  scheduler.addtask(dis_code,priority_definitecode,options.loadaddr,NULL);
  scheduler.addtask(nameloc,priority_nameloc,options.loadaddr,"start");
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* readsysfile                                                           *
* - very similar to the com file format, but the start offset is 0x00   *
* - detailed sys format below: (since so few documents seem to give the *
*   information correctly)                                              *
*                                                                       *
* Device Header                                                         *
*                                                                       *
* The device header is an extension to what is described in the MS-PRM. *
*                                                                       *
* DevHdr DD -1 ; Ptr to next driver in file or -1 if last driver        *
* DW ? ; Device attributes                                              *
* DW ? ; Device strategy entry point                                    *
* DW ? ; Device interrupt entry point                                   *
* DB 8 dup (?) ; Character device name field                            *
* DW 0 ; Reserved                                                       *
* DB 0 ; Drive letter                                                   *
* DB ? ; Number of units                                                *
*                                                                       *
* A device driver requires a device header at the beginning of the      *
* file.                                                                 *
*                                                                       *
* POINTER TO NEXT DEVICE HEADER FIELD                                   *
*                                                                       *
* The device header field is a pointer to the device header of the next *
* device driver. It is a doubleword field that is set by DOS at the     *
* time the device driver is loaded. The first word is an offset and the *
* second word is the segment. If you are loading only one device        *
* driver, set the device header field to -1 before loading the device.  *
* If you are loading more than one device driver, set the first word of *
* the device driver header to the offset of the next device driver's    *
* header. Set the device driver header field of the last device driver  *
* to -1.                                                                *
*                                                                       *
* ATTRIBUTE FIELD                                                       *
*                                                                       *
* The attribute field is a word field that describes the attributes of  *
* the device driver to the system. The attributes are:                  *
*                                                                       *
* word bits (decimal)                                                   *
*  15   1   character device                                            *
*       0   block device                                                *
*  14   1   supports IOCTL                                              *
*       0   doesn't support IOCTL                                       *
*  13   1   non-IBM format (block only)                                 *
*       0   IBM format                                                  *
*  12       not documented - unknown                                    *
*  11   1   supports removeable media                                   *
*       0   doesn't support removeable media                            *
*  10       reserved for DOS                                            *
*    through                                                            *
*   4       reserved for DOS                                            *
*   3   1   current block device                                        *
*       0   not current block device                                    *
*   2   1   current NUL device                                          *
*       0   not current NUL device                                      *
*   1   1   current standard output device                              *
*       0   not current standard output device                          *
*                                                                       *
* BIT 15 is the device type bit. Use it to tell the system the that     *
* driver is a block or character device.                                *
*                                                                       *
* BIT 14 is the IOCTL bit. It is used for both character and block      *
* devices. Use it to tell DOS whether the device driver can handle      *
* control strings through the IOCTL function call 44h.                  *
* If a device driver cannot process control strings, it should set bit  *
* 14 to 0. This way DOS can return an error is an attempt is made       *
* through the IOCTL function call to send or receive control strings to *
* the device. If a device can process control strings, it should set    *
* bit 14 to 1. This way, DOS makes calls to the IOCTL input and output  *
* device function to send and receive IOCTL strings.                    *
* The IOCTL functions allow data to be sent to and from the device      *
* without actually doing a normal read or write. In this way, the       *
* device driver can use the data for its own use, (for example, setting *
* a baud rate or stop bits, changing form lengths, etc.) It is up to    *
* the device to interpret the information that is passed to it, but the *
* information must not be treated as a normal I/O request.              *
*                                                                       *
* BIT 13 is the non-IBM format bit. It is used for block devices only.  *
* It affects the operation of the Get BPB (BIOS parameter block) device *
* call.                                                                 *
*                                                                       *
* BIT 11 is the open/close removeable media bit. Use it to tell DOS if  *
* the device driver can handle removeable media. (DOS 3.x only)         *
*                                                                       *
* BIT 3 is the clock device bit. It is used for character devices only. *
* Use it to tell DOS if your character device driver is the new CLOCK$  *
* device.                                                               *
*                                                                       *
* BIT 2 is the NUL attribute bit. It is used for character devices      *
* only. Use it to tell DOS if your character device driver is a NUL     *
* device. Although there is a NUL device attribute bit, you cannot      *
* reassign the NUL device. This is an attribute that exists for DOS so  *
* that DOS can tell if the NUL device is being used.                    *
*                                                                       *
* BIT 0 are the standard input and output bits. They are used for       *
* character & devices only. Use these bits to tell DOS if your          *
* character device                                                      *
*                                                                       *
* BIT 1 driver is the new standard input device or standard output      *
* device.                                                               *
*                                                                       *
* POINTER TO STRATEGY AND INTERRUPT ROUTINES                            *
*                                                                       *
* These two fields are pointers to the entry points of the strategy and *
* input routines. They are word values, so they must be in the same     *
* segment as the device header.                                         *
*                                                                       *
* NAME/UNIT FIELD                                                       *
*                                                                       *
* This is an 8-byte field that contains the name of a character device  *
* or the unit of a block device. For the character names, the name is   *
* left-justified and the space is filled to 8 bytes. For block devices, *
* the number of units can be placed in the first byte. This is optional *
* because DOS fills in this location with the value returned by the     *
* driver's INIT code.                                                   *
************************************************************************/
void fileloader::readsysfile(dword fsize)
{ lptr t;
  bool done;
  word devhdr,devlength;
  options.loadaddr.offs=0x00;
  options.dseg=options.loadaddr.segm;
  dta.addseg(options.loadaddr,fsize,fbuff,code16,NULL);
  dta.possibleentrycode(options.loadaddr);
  options.mode16=true;
  options.mode32=false;
  dio.setcuraddr(options.loadaddr);
  done=false;
  devhdr=0;
  while(!done)
  { t.assign(options.loadaddr.segm,devhdr);
    scheduler.addtask(dis_dataword,priority_data,t,NULL);
    scheduler.addtask(dis_dataword,priority_data,t+2,NULL);
    scheduler.addtask(dis_dataword,priority_data,t+4,NULL);
    scheduler.addtask(dis_dataword,priority_data,t+6,NULL);
    scheduler.addtask(dis_dataword,priority_data,t+8,NULL);
    scheduler.addtask(dis_dataword,priority_data,t+18,NULL);
    if((((word *)(fbuff+devhdr))[3])&&((((word *)(fbuff+devhdr))[3])<fsize))
    { t.assign(options.loadaddr.segm,((word *)(fbuff+devhdr))[3]+devhdr);
	   if(t.offs)
      { scheduler.addtask(dis_code,priority_definitecode,t,NULL);
	     scheduler.addtask(nameloc,priority_nameloc,t,"strategy");
      }
    }
    if((((word *)(fbuff+devhdr))[4])&&((((word *)(fbuff+devhdr))[4])<fsize))
    { t.assign(options.loadaddr.segm,((word *)(fbuff+devhdr))[4]);
	   if(t.offs)
      { scheduler.addtask(dis_code,priority_definitecode,t,NULL);
	     scheduler.addtask(nameloc,priority_nameloc,t,"interrupt");
      }
    }
    scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
    devlength=((word *)(fbuff+devhdr))[0];
    if((devlength==0xffff)||(!devlength)||((unsigned)(devhdr+devlength)>(fsize-10)))
      done=true;
    devhdr+=devlength;
  }
}

/************************************************************************
* readpefile                                                            *
* - the main file loader in Borg is for PE files. The routines here are *
*   fairly complete, and lack only proper analysis of debug sections.   *
* - needs rewriting for clarity at some point in the future, as it has  *
*   sprawled out to 360 lines now.                                      *
* - some further resource tree analysis is done in other routines which *
*   follow                                                              *
************************************************************************/
void fileloader::readpefile(dword peoffs)
{ peobjdata *pdata;
  byte *pestart,*impname,*expname;
  unsigned char *chktable;
  lptr lef,leo,len,start_addr;
  dword *fnaddr,*nnaddr;
  word *onaddr,*rdata;
  dword numsymbols,numitems,numrelocs;
  char impbuff[100],inum[10],newimpname[GNAME_MAXLEN+1];
  lptr sseg,t;
  dword j;
  int i,k,k1,clen;
  peimportdirentry *impdir;
  byte *uinit;
  peexportdirentry *expdir;
  perelocheader *per;
  dword thunkrva,*imphint,impaddr,impaddr2,numtmp;
  bool peobjdone;
  perestable *resdir;
  perestableentry *rentry;
  start_addr=nlptr;
  options.dseg=options.loadaddr.segm;
  sseg.segm=options.loadaddr.segm;
  sseg.offs=0;
  pestart=&fbuff[peoffs];
  peh=(peheader *)pestart;
  options.loadaddr.offs=peh->image_base; // bugfix build 14
  // this is not right - ver 2.19 bugfix below
  //  pdata=(peobjdata *)(pestart+sizeof(peheader)+(peh->numintitems-0x0a)*8);
  // 24=standard header size and add nt_hdr_size to it.
  pdata=(peobjdata *)(pestart+24+peh->nt_hdr_size);
  for(i=0;i<peh->objects;i++)
  { peobjdone=false;
	 if((pdata[i].rva==peh->exporttable_rva)  // export info
		||((peh->exporttable_rva>pdata[i].rva)&&(peh->exporttable_rva<pdata[i].rva+pdata[i].phys_size)))
	 { expdir=(peexportdirentry *)&fbuff[pdata[i].phys_offset+peh->exporttable_rva-pdata[i].rva];
		t.assign(options.loadaddr.segm,peh->image_base+peh->exporttable_rva);
		scheduler.addtask(dis_datadword,priority_data,t,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+4,NULL);
		scheduler.addtask(dis_dataword,priority_data,t+8,NULL);
		scheduler.addtask(dis_dataword,priority_data,t+10,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+12,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+16,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+20,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+24,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+28,NULL);
		for(k1=0;k1<peh->objects;k1++)
		{ if((expdir->namerva>=pdata[k1].rva)&&(expdir->namerva<pdata[k1].rva+pdata[k1].phys_size))
		  { expname=&fbuff[expdir->namerva-pdata[k1].rva+pdata[k1].phys_offset];
			 break;
		  }
		}
		t.offs=expdir->namerva+peh->image_base;
		scheduler.addtask(dis_datastring,priority_data,t,NULL);
		numsymbols=expdir->numfunctions;
		chktable=new unsigned char [numsymbols];
		for(j=0;j<numsymbols;j++)
        chktable[j]=0;
		if(expdir->numnames<numsymbols)
        numsymbols=expdir->numnames;
		for(k=0;k<peh->objects;k++)
		{ if((expdir->nameaddrrva>=pdata[k].rva)&&(expdir->nameaddrrva<pdata[k].rva+pdata[k].phys_size))
		  { nnaddr=(dword *)&fbuff[expdir->nameaddrrva-pdata[k].rva+pdata[k].phys_offset];
			 break;
		  }
		}
		for(k=0;k<peh->objects;k++)
		{ if((expdir->funcaddrrva>=pdata[k].rva)&&(expdir->funcaddrrva<pdata[k].rva+pdata[k].phys_size))
		  { fnaddr=(dword *)&fbuff[expdir->funcaddrrva-pdata[k].rva+pdata[k].phys_offset];
			 break;
		  }
		}
		for(k=0;k<peh->objects;k++)
		{ if((expdir->ordsaddrrva>=pdata[k].rva)&&(expdir->ordsaddrrva<pdata[k].rva+pdata[k].phys_size))
		  { onaddr=(word *)&fbuff[expdir->ordsaddrrva-pdata[k].rva+pdata[k].phys_offset];
			 break;
		  }
		}
		lef.assign(options.loadaddr.segm,expdir->funcaddrrva+peh->image_base);
		leo.assign(options.loadaddr.segm,expdir->ordsaddrrva+peh->image_base);
		len.assign(options.loadaddr.segm,expdir->nameaddrrva+peh->image_base);
		while(numsymbols)
		{ scheduler.addtask(dis_datadword,priority_data,lef,NULL);
		  scheduler.addtask(dis_dataword,priority_data,leo,NULL);
		  scheduler.addtask(dis_datadword,priority_data,len,NULL);
		  chktable[onaddr[0]]=1;
		  t.assign(options.loadaddr.segm,peh->image_base+fnaddr[onaddr[0]]);
		  scheduler.addtask(dis_export,priority_export,t,(char *)&fbuff[(*nnaddr)+pdata[k].phys_offset-pdata[k].rva]);
		  t.assign(options.loadaddr.segm,(*nnaddr)+peh->image_base);
		  scheduler.addtask(dis_datastring,priority_data,t,NULL);
        // actually not definite code since data can be exported too, eg a debug hook address
        // so uses dis_exportcode which will disassemble if its in a code segment only.
		  t.assign(options.loadaddr.segm,peh->image_base+fnaddr[onaddr[0]]);
		  scheduler.addtask(dis_exportcode,priority_definitecode,t,NULL);
		  numsymbols--;
		  onaddr++;
		  nnaddr++;
		  lef+=4;
		  leo+=2;
		  len+=4;
		}
		if(expdir->numfunctions>expdir->numnames)
		{ for(j=0;j<expdir->numfunctions;j++)
		  { if(!chktable[j])
			 { numtmp=j+expdir->base;
				wsprintf(inum,"%02d",numtmp);
				lstrcpyn(impbuff,(char *)expname,GNAME_MAXLEN-8);
				k=0;
				while((impbuff[k])&&(k<GNAME_MAXLEN-8))
				{ if(impbuff[k]=='.')
                break;
				  k++;
				}
				strcpy(&impbuff[k],"::ord_");
				strcat(impbuff,inum);
				strcpy(newimpname,impbuff);
				if(fnaddr[j])
				{ t.assign(options.loadaddr.segm,fnaddr[j]+peh->image_base);
				  scheduler.addtask(dis_ordexport,priority_export,t,newimpname);
				  scheduler.addtask(dis_code,priority_definitecode,t,NULL);
				}
			 }
		  }
		}
		delete chktable;
	 }
	 if((pdata[i].rva==peh->importtable_rva) // import info
		||((peh->importtable_rva>pdata[i].rva)&&(peh->importtable_rva<pdata[i].rva+pdata[i].phys_size)))
	 { impdir=(peimportdirentry *)&fbuff[pdata[i].phys_offset+peh->importtable_rva-pdata[i].rva];
		j=0;
		while(impdir[j].firstthunkrva)
		{ t.assign(options.loadaddr.segm,
			 peh->image_base+peh->importtable_rva+j*sizeof(struct peimportdirentry));
		  scheduler.addtask(dis_datadword,priority_data,t,NULL);
		  scheduler.addtask(dis_datadword,priority_data,t+4,NULL);
		  scheduler.addtask(dis_datadword,priority_data,t+8,NULL);
		  scheduler.addtask(dis_datadword,priority_data,t+12,NULL);
		  scheduler.addtask(dis_datadword,priority_data,t+16,NULL);
		  for(k1=0;k1<peh->objects;k1++)
		  { if((impdir[j].namerva>=pdata[k1].rva)&&(impdir[j].namerva<pdata[k1].rva+pdata[k1].phys_size))
			 { impname=&fbuff[impdir[j].namerva-pdata[k1].rva+pdata[k1].phys_offset];
				break;
			 }
		  }
		  t.assign(options.loadaddr.segm,impdir[j].namerva+peh->image_base);
		  scheduler.addtask(dis_datastring,priority_data,t,NULL);
		  if(!impdir[j].originalthunkrva)
          thunkrva=impdir[j].firstthunkrva;
		  else
          thunkrva=impdir[j].originalthunkrva;
		  for(k=0;k<peh->objects;k++)
		  { if((thunkrva>=pdata[k].rva)&&(thunkrva<pdata[k].rva+pdata[k].phys_size))
			 { imphint=(dword *)&fbuff[thunkrva-pdata[k].rva+pdata[k].phys_offset];
				break;
			 }
		  }
		  impaddr=impdir[j].firstthunkrva+peh->image_base;
		  impaddr2=impdir[j].originalthunkrva+peh->image_base;
		  while(*imphint)
		  { if((*imphint)&0x80000000)
			 { numtmp=(*imphint)&0x7fffffff;
				wsprintf(inum,"%02d",numtmp);
				strcpy(impbuff,(char *)impname);
				k=0;
				while(impbuff[k])
				{ if(impbuff[k]=='.')
                break;
				  k++;
				}
				strcpy(&impbuff[k],"::ord_");
				strcat(impbuff,inum);
				strcpy(newimpname,impbuff);
				t.assign(options.loadaddr.segm,impaddr);
				scheduler.addtask(dis_ordimport,priority_import,t,newimpname);
			 }
			 else
			 { t.assign(options.loadaddr.segm,impaddr);
				scheduler.addtask(dis_import,priority_import,t,(char *)&fbuff[(*imphint)+2+pdata[k1].phys_offset-pdata[k1].rva]);
			 }
			 t.assign(options.loadaddr.segm,peh->image_base+(*imphint));
			 scheduler.addtask(dis_dataword,priority_data,t,NULL);
			 scheduler.addtask(dis_datastring,priority_data,t+2,NULL);
			 t.assign(options.loadaddr.segm,impaddr);
			 scheduler.addtask(dis_datadword,priority_data,t,NULL);
			 t.assign(options.loadaddr.segm,impaddr2);
			 scheduler.addtask(dis_datadword,priority_data,t,NULL);
			 imphint++;
			 impaddr+=4;
			 impaddr2+=4;
		  }
		  t.assign(options.loadaddr.segm,impaddr);
		  scheduler.addtask(dis_datadword,priority_data,t,NULL);
		  t.assign(options.loadaddr.segm,impaddr2);
		  scheduler.addtask(dis_datadword,priority_data,t,NULL);
		  j++;
		}
		t.assign(options.loadaddr.segm,
		  peh->image_base+peh->importtable_rva+j*sizeof(struct peimportdirentry));
		scheduler.addtask(dis_datadword,priority_data,t,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+4,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+8,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+12,NULL);
		scheduler.addtask(dis_datadword,priority_data,t+16,NULL);
	 }
	 if(pdata[i].rva==peh->tls_rva)
      peobjdone=true; // tls info
	 if((pdata[i].rva==peh->resourcetable_rva) // resource info
		||((peh->resourcetable_rva>pdata[i].rva)&&(peh->resourcetable_rva<pdata[i].rva+pdata[i].phys_size)))
	 { // RESOURCE_DATA;
		if((pdata[i].phys_size)&&(options.loadresources))
		{ resdir=(perestable *)&fbuff[pdata[i].phys_offset+peh->resourcetable_rva-pdata[i].rva];
		  pdatarva=peh->resourcetable_rva;  // bugfix  build 14
		  rawdata=(byte *)resdir;
		  numitems=resdir->numnames+resdir->numids;
		  rentry=(struct perestableentry *)(resdir+1);
		  while(numitems)
		  { if((rentry->id)&0x80000000)
			 { impname=rawdata+((rentry->id)&0x7fffffff);
				clen=((word *)impname)[0];
				WideCharToMultiByte(CP_ACP,0,(const wchar_t *)(impname+2),clen,impbuff,100,NULL,NULL);
				impbuff[clen]=0;
			 }
			 else
			 { switch(rentry->id)
				{ case 1:
					 strcpy(impbuff,"Cursor");
					 break;
				  case 2:
					 strcpy(impbuff,"Bitmap");
					 break;
				  case 3:
					 strcpy(impbuff,"Icon");
					 break;
				  case 4:
					 strcpy(impbuff,"Menu");
					 break;
				  case 5:
					 strcpy(impbuff,"Dialog");
					 break;
				  case 6:
					 strcpy(impbuff,"String Table");
					 break;
				  case 7:
					 strcpy(impbuff,"Font Directory");
					 break;
				  case 8:
					 strcpy(impbuff,"Font");
					 break;
				  case 9:
					 strcpy(impbuff,"Accelerators");
					 break;
				  case 10:
					 strcpy(impbuff,"Unformatted Resource Data");
					 break;
				  case 11:
					 strcpy(impbuff,"Message Table");
					 break;
				  case 12:
					 strcpy(impbuff,"Group Cursor");
					 break;
				  case 14:
					 strcpy(impbuff,"Group Icon");
					 break;
				  case 16:
					 strcpy(impbuff,"Version Information");
					 break;
				  case 0x2002:
					 strcpy(impbuff,"New Bitmap");
					 break;
				  case 0x2004:
					 strcpy(impbuff,"New Menu");
					 break;
				  case 0x2005:
					 strcpy(impbuff,"New Dialog");
					 break;
				  default:
					 strcpy(impbuff,"User Defined Id:");
					 numtmp=rentry->id&0x7fffffff;
					 wsprintf(inum,"%02lx",numtmp);
					 strcat(impbuff,inum);
					 break;
				}
			 }
			 if(rentry->offset&0x80000000)
			 { subdirsummary(rawdata+((rentry->offset)&0x7fffffff),impbuff,peh->image_base,rentry->id);
			 }
			 else
			 { leafnodesummary(rawdata+((rentry->offset)&0x7fffffff),impbuff,peh->image_base,0);
			 }
			 rentry++;
			 numitems--;
		  }
		}
		if(pdata[i].rva==peh->resourcetable_rva)
        peobjdone=true;
	 }
	 if(pdata[i].rva==peh->fixuptable_rva) // fixup info
	 { per=(perelocheader *)&fbuff[pdata[i].phys_offset];
		while(per->rva)
		{ rdata=(word *)per+sizeof(perelocheader)/2;
		  numrelocs=(per->len-sizeof(perelocheader))/2;
		  while((numrelocs)&&(rdata[0]))
		  { t.assign(options.loadaddr.segm,((dword)(rdata[0])&0x0fff)+per->rva+peh->image_base);
			 reloc.addreloc(t,RELOC_NONE);
			 rdata++;
			 numrelocs--;
		  }
        per=(perelocheader *)((byte *)(per)+per->len);
		}
		peobjdone=true;
	 }
	 if(pdata[i].rva==peh->debugtable_rva) // debug info
	 { // DEBUG_DATA;
		if((pdata[i].phys_size)&&(options.loaddebug))
		{ sseg.offs=pdata[i].rva+peh->image_base;
		  dta.addseg(sseg,pdata[i].phys_size,&fbuff[pdata[i].phys_offset],debugdata,NULL);
		}
		peobjdone=true;
	 }
	 if((pdata[i].obj_flags&0x40)&&(!(pdata[i].obj_flags&0x20))&&(!peobjdone))
	 { // INIT_DATA;
		if((pdata[i].phys_size)&&(options.loaddata))
		{ sseg.offs=pdata[i].rva+peh->image_base;
		  dta.addseg(sseg,pdata[i].phys_size,&fbuff[pdata[i].phys_offset],data32,NULL);
		}
		if((pdata[i].virt_size>pdata[i].phys_size)&&(options.loaddata))
		{ sseg.offs=pdata[i].rva+peh->image_base+pdata[i].phys_size;
		  uinit=new byte[pdata[i].virt_size-pdata[i].phys_size];
		  for(j=0;j<pdata[i].virt_size-pdata[i].phys_size;j++)
          uinit[j]=0;
		  dta.addseg(sseg,pdata[i].virt_size-pdata[i].phys_size,uinit,uninitdata,NULL);
		}
	 }
	 else if((pdata[i].obj_flags&0x80)&&(!peobjdone))
	 { // UNINIT_DATA;
		if(options.loaddata)
		{ sseg.offs=pdata[i].rva+peh->image_base;
		  uinit=new byte[pdata[i].virt_size];
		  for(j=0;j<pdata[i].virt_size;j++)
          uinit[j]=0;
		  dta.addseg(sseg,pdata[i].virt_size,uinit,uninitdata,NULL);
		}
	 }
	 else if(!peobjdone)
	 { // CODE_DATA;
		if(pdata[i].phys_size)
		{ sseg.offs=pdata[i].rva+peh->image_base;
		  dta.addseg(sseg,pdata[i].phys_size,&fbuff[pdata[i].phys_offset],code32,NULL);
		  dta.possibleentrycode(sseg);
		}
	 }
    // default start addr=first seg, in the case of no entry point
    // (eg some dll files). added version 2.20
    if(!start_addr.segm)
    { start_addr.assign(options.loadaddr.segm,pdata[i].rva+peh->image_base);
      dio.setcuraddr(start_addr);
    }
  }
  start_addr.assign(options.loadaddr.segm,peh->entrypoint_rva+peh->image_base);
  options.oep=start_addr;
  dio.setcuraddr(start_addr);
  scheduler.addtask(dis_code,priority_definitecode,start_addr,NULL);
  scheduler.addtask(nameloc,priority_nameloc,start_addr,"start");
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* mzcmp                                                                 *
* - short comparison routine used in a qsort of relocations in dos mz   *
*   executable files                                                    *
************************************************************************/
int mzcmp(const void *a1,const void *a2)
{ if(((word *)a1)[0]<((word *)a2)[0])
    return -1;
  if(((word *)a1)[0]>((word *)a2)[0])
    return 1;
  return 0;
}

/************************************************************************
* readmzfile                                                            *
* - standard dos mz-executable file reader.                             *
* - fairly basic at the moment, as it is simply a load, and a better    *
*   analysis of d_seg is required (in fact Borg needs more work on      *
*   segmentation all round.                                             *
************************************************************************/
void fileloader::readmzfile(dword fsize)
{ mzheader *mzh;
  dword fs;
  byte *roffs;
  byte *poffs;
  word nrelocs,nr;
  word *ritem;
  word *rchange;
  word *rtable;
  lptr sseg,tseg;  // current segment limits
  lptr ip;
  dword saddr,taddr,ipaddr;
  options.loadaddr.offs=0;
  mzh=(mzheader *)fbuff;
  fs=(mzh->numpages-1)*512L+mzh->numbytes;
  if(fs>fsize)
    fs=fsize;
  fs-=mzh->headersize*16L;
  roffs=fbuff+mzh->relocoffs;
  poffs=fbuff+mzh->headersize*16L;
  nrelocs=mzh->numrelocs;
  if(nrelocs)
    rtable=new word[nrelocs];
  if(!nrelocs)
  { MessageBox(mainwindow,"Relocation table is empty\nThis file is probably packed"
	 "\nBorg will not be able to create the segments properly","Borg Warning",MB_OK|MB_ICONEXCLAMATION);
  }
  while(nrelocs)
  { ritem=(word *)(&roffs[(mzh->numrelocs-nrelocs)*4L]);
	 if((ritem[0])||(ritem[1]))
	 { rchange=(word *)(&poffs[ritem[0]+ritem[1]*16L]);
		rchange[0]+=(word)options.loadaddr.segm;
		rtable[mzh->numrelocs-nrelocs]=rchange[0];
	 }
	 else
      rtable[mzh->numrelocs-nrelocs]=options.loadaddr.segm;
	 nrelocs--;
  }
  qsort(rtable,mzh->numrelocs,2,mzcmp);
  sseg=options.loadaddr;
  options.dseg=options.loadaddr.segm;   // need to look for better value for dseg later
  ip=options.loadaddr;
  ipaddr=(((word)mzh->csip)+((word)(mzh->csip/0x10000L)+options.loadaddr.segm)*16L+options.loadaddr.offs)&0xfffff;
  for(nrelocs=0;nrelocs<mzh->numrelocs;nrelocs++)
  { if(rtable[nrelocs]!=sseg.segm)
	 { tseg.assign(rtable[nrelocs],0);
		saddr=sseg.segm*16L+sseg.offs;
		taddr=tseg.segm*16L+tseg.offs;
		if((ipaddr>=saddr)&&(ipaddr<taddr))
		{ ip.assign(sseg.segm,ipaddr-ip.segm*16L);
		}
		if((saddr<taddr)&&(sseg.segm>=options.loadaddr.segm))
		{ dta.addseg(sseg,taddr-saddr,fbuff+mzh->headersize*16
			 +(sseg.segm-options.loadaddr.segm)*16L,code16,NULL);
		  dta.possibleentrycode(sseg);
		  // go through the reloc items, check if any lie in the seg
		  // if they do - add to reloc entries.
		  for(nr=0;nr<mzh->numrelocs;nr++)
		  { ritem=(word *)(&roffs[(mzh->numrelocs-nr)*4L]);
			 if((ritem[0])||(ritem[1]))
			 { rchange=(word *)(&poffs[ritem[0]+ritem[1]*16L]);
				if(((byte *)rchange>=fbuff+mzh->headersize*16+(sseg.segm-options.loadaddr.segm)*16L)
				  &&((byte *)rchange<fbuff+mzh->headersize*16+(sseg.segm-options.loadaddr.segm)*16L+taddr-saddr))
				  reloc.addreloc(sseg+(dword)((byte *)rchange-
					 (fbuff+mzh->headersize*16+(sseg.segm-options.loadaddr.segm)*16L)),RELOC_SEG);
			 }
		  }
		  sseg.segm=tseg.segm;
		}
	 }
  }
  if((sseg.segm-options.loadaddr.segm)*16UL<fs)
  { saddr=sseg.segm*16L+sseg.offs;
	 if(ipaddr>=saddr)
	 { ip.assign(sseg.segm,ipaddr-ip.segm*16L);
	 }
	 dta.addseg(sseg,fs-(sseg.segm-options.loadaddr.segm)*16L,
		fbuff+mzh->headersize*16+(sseg.segm-options.loadaddr.segm)*16L,code16,NULL);
	 for(nr=0;nr<mzh->numrelocs;nr++)
	 { ritem=(word *)(&roffs[(mzh->numrelocs-nr)*4L]);
		if((ritem[0])||(ritem[1]))
		{ rchange=(word *)(&poffs[ritem[0]+ritem[1]*16L]);
		  if(((byte *)rchange>=fbuff+mzh->headersize*16+(sseg.segm-options.loadaddr.segm)*16L)
			 &&((byte *)rchange<fbuff+mzh->headersize*16+fs))
			 reloc.addreloc(sseg+(dword)((byte *)rchange-
				(fbuff+mzh->headersize*16+(sseg.segm-options.loadaddr.segm)*16L)),RELOC_SEG);
		}
	 }
	 dta.possibleentrycode(sseg);
  }
  // need to search for dseg better value.
  options.mode16=true;
  options.mode32=false;
  dio.setcuraddr(ip);
  scheduler.addtask(dis_code,priority_definitecode,ip,NULL);
  scheduler.addtask(nameloc,priority_nameloc,ip,"start");
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
  if(mzh->numrelocs)
    delete rtable;
}

/************************************************************************
* readlefile                                                            *
* - to be written yet                                                   *
************************************************************************/
void fileloader::readlefile(void)
{
}

/************************************************************************
* readnefile                                                            *
* - NE = new executable = old windows 16-bit format.                    *
* - this is partially written but needs much more work on imports,      *
*   exports, etc                                                        *
************************************************************************/
void fileloader::readnefile(dword neoffs)
{ neheader *neh;
  byte *nestart,*importnames,*modoffsets;
  word nsegs;
  nesegtable *nesegt;
  int i,j,k;
  lptr sseg,iaddr,inum;
  dword slen,soffs;
  word numrelocs;
  word *stable;
  nesegtablereloc *reloctable;
  char iname[80];
  options.dseg=options.loadaddr.segm;
  sseg.assign(options.loadaddr.segm,0);
  nestart=&fbuff[neoffs];
  neh=(neheader *)nestart;
  if(!neh->csip)
  { MessageBox(mainwindow,"No entry point to executable - assume that Executable"
		"is a resource only\nUse a resource viewer","Borg Warning",MB_OK|MB_ICONEXCLAMATION);
	 CloseHandle(efile);
	 efile=INVALID_HANDLE_VALUE;
	 exetype=0;
	 return;
  }
  nsegs=neh->numsegs;
  stable=new word[nsegs];
  nesegt=(nesegtable *)(nestart+neh->offs_segments);
  importnames=(byte *)(nestart+neh->offs_imports);
  modoffsets=(byte *)(nestart+neh->offs_module);
  // add segments
  for(i=0;i<nsegs;i++)
  { slen=nesegt[i].seglength;
	 if(!slen)
      slen=0x10000;
	 soffs=nesegt[i].sectoroffs;
    // added uninit data borg 2.20
	 if(soffs)
	 { if(nesegt[i].segflags&1)
		{ dta.addseg(sseg,slen,&fbuff[soffs<<(dword)neh->shiftcount],data16,NULL);
		  options.dseg=sseg.segm;
		}
		else
		{ dta.addseg(sseg,slen,&fbuff[soffs<<(dword)neh->shiftcount],code16,NULL);
		  dta.possibleentrycode(sseg);
		}
	 }
	 else
      dta.addseg(sseg,slen,NULL,uninitdata,NULL); // uninit data
	 stable[i]=sseg.segm;
	 sseg.segm+=(word)((slen+15)/16L);
  }
  // relocate data
  // approach to imports:
  // - start with a new segment 0xffff, to be created later, size 0.
  // - for each import, if its an ordinal add it at the current addr in the import segment,
  // - and increase the size of the segment, name it = name+ordinal num
  // - otherwise name=importnames table name, check for if it is already an import
  // - and only add if necessary.
  // - finally create the segment at the end.
  iaddr.segm=0xffff;
  inum.segm=0xffff;
  iaddr.offs=0;
  for(i=0;i<nsegs;i++)
  { slen=nesegt[i].seglength;
	 if(!slen)
      slen=0x10000;
	 soffs=nesegt[i].sectoroffs;
	 if((nesegt[i].segflags&100)&&(soffs))
	 { // reloc data present
		numrelocs=((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+slen])[0];
		reloctable=(nesegtablereloc *)&fbuff[(soffs<<(dword)neh->shiftcount)+slen+2];
		for(j=0;j<numrelocs;j++)
		{ switch(reloctable[j].reloctype)
		  { case 0:      //low byte
				break;
			 case 2:      //16bit selector
				if((!reloctable[j].relocsort)&&(reloctable[j].index<0xff))
				{ ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs])[0]=stable[reloctable[j].index-1];
				}
				break;
			 case 3:      //32bit pointer
				if((!reloctable[j].relocsort)&&(reloctable[j].index<0xff))
				{ ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs+2])[0]=stable[reloctable[j].index-1];
				  ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs])[0]=reloctable[j].indexoffs;
				}
            else if(reloctable[j].relocsort==2) // import by name
            { for(k=0;(k<importnames[reloctable[j].indexoffs])&&(k<79);k++)
              { iname[k]=importnames[reloctable[j].indexoffs+1+k];
                iname[k+1]=0;
              }
              inum.offs=import.getoffsfromname(iname);
              if(!inum.offs)
              { iaddr++;
                import.addname(iaddr,iname);
                inum.offs=iaddr.offs;
              }
              ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs+2])[0]=inum.segm;
				  ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs])[0]=(word)inum.offs;
            }
				break;
			 case 5:      //16bit offset
				if((!reloctable[j].relocsort)&&(reloctable[j].index<0xff))
				{ ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs])[0]=reloctable[j].indexoffs;
				}
				break;
			 case 11:     //48bit pointer
				if((!reloctable[j].relocsort)&&(reloctable[j].index<0xff))
				{ ((word *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs+4])[0]=stable[reloctable[j].index-1];
				  ((dword *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs])[0]=reloctable[j].indexoffs;
				}
				break;
			 case 13:     //32bit offset
				if((!reloctable[j].relocsort)&&(reloctable[j].index<0xff))
				{ ((dword *)&fbuff[(soffs<<(dword)neh->shiftcount)+reloctable[j].segm_offs])[0]=reloctable[j].indexoffs;
				}
				break;
			 default:
				break;
		  }
		}
	 }
  }
  inum.offs=0;
  if(iaddr.offs)
    dta.addseg(inum,iaddr.offs+1,NULL,uninitdata,"Import Segment <Borg>");
  // set up disassembly
  options.loadaddr.assign(stable[(neh->csip>>16)-1],neh->csip&0xffff);
  dio.setcuraddr(options.loadaddr);
  scheduler.addtask(dis_code,priority_definitecode,options.loadaddr,NULL);
  scheduler.addtask(nameloc,priority_nameloc,options.loadaddr,"start");
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
  delete stable;
}

/************************************************************************
* reados2file                                                           *
* - not written yet
************************************************************************/
void fileloader::reados2file(void)
{
}

/************************************************************************
* readbinfile                                                           *
* - reads a file as a flat binary file, so in effect we can load more   *
*   or less anything and do some analysis                               *
************************************************************************/
void fileloader::readbinfile(dword fsize)
{ options.mode32=!options.mode16;
  options.dseg=options.loadaddr.segm;
  dta.addseg(options.loadaddr,fsize,fbuff,options.mode32 ? code32:code16,NULL);
  dta.possibleentrycode(options.loadaddr);
  dio.setcuraddr(options.loadaddr);
  scheduler.addtask(dis_code,priority_definitecode,options.loadaddr,NULL);
  scheduler.addtask(nameloc,priority_nameloc,options.loadaddr,"start");
  scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* getexetype                                                            *
* - returns the exe type, external class interface function             *
************************************************************************/
int fileloader::getexetype(void)
{ return exetype;
}

/************************************************************************
* setexetype                                                            *
* - sets the exe type, external class interface function                *
************************************************************************/
void fileloader::setexetype(int etype)
{ exetype=etype;
}

/************************************************************************
* subdirsummary                                                         *
* - this is part of the resource analysis for PE files. Resources are   *
*   held in a tree type format consisting of subdirs and leafnodes.     *
************************************************************************/
void fileloader::subdirsummary(byte *data,char *impname,dword image_base,dword rtype)
{ struct perestable *resdir;
  struct perestableentry *rentry;
  unsigned char *name;
  char nbuff[100],nbuff2[100],inum[10];
  int clen;
  dword numtmp;
  unsigned long numitems;
  resdir=(struct perestable *)data;
  numitems=resdir->numnames+resdir->numids;
  rentry=(struct perestableentry *)(resdir+1);
  while(numitems)
  { if(rentry->id&0x80000000)
	 { name=rawdata+((rentry->id)&0x7fffffff);
		clen=((short int *)name)[0];
		WideCharToMultiByte(CP_ACP,0,(const wchar_t *)(name+2),clen,nbuff,100,NULL,NULL);
		nbuff[clen]=0;
		if(impname!=NULL)
		{ strcpy(nbuff2,nbuff);
		  strcpy(nbuff,impname);
		  strcat(nbuff," ");
		  strcat(nbuff,nbuff2);
		}
	 }
	 else
	 { numtmp=rentry->id&0x7fffffff;
		wsprintf(inum,"%02lx",numtmp);
		strcpy(nbuff,impname);
		strcat(nbuff," Id:");
		strcat(nbuff,inum);
	 }
	 if(rentry->offset&0x80000000)
      leaf2summary(rawdata+((rentry->offset)&0x7fffffff),nbuff,image_base,rtype);
	 else
      leafnodesummary(rawdata+((rentry->offset)&0x7fffffff),nbuff,image_base,rtype);
	 rentry++;
	 numitems--;
  }
}

/************************************************************************
* leaf2summary                                                          *
* - PE resource analysis of leaf nodes                                  *
************************************************************************/
void fileloader::leaf2summary(byte *data,char *name,dword image_base,dword rtype)
{ struct perestable *resdir;
  struct perestableentry *rentry;
  unsigned long numitems;
  resdir=(struct perestable *)data;
  numitems=resdir->numnames+resdir->numids;
  rentry=(struct perestableentry *)(resdir+1);
  while(numitems)
  { leafnodesummary(rawdata+((rentry->offset)&0x7fffffff),name,image_base,rtype);
	 rentry++;
	 numitems--;
  }
}

/************************************************************************
* leafnodesummary                                                       *
* - analysis of a leaf node in a PE resource table                      *
* - detailed analysis of dialogs and string tables is done at the       *
*   moment                                                              *
************************************************************************/
void fileloader::leafnodesummary(byte *data,char *resname,dword image_base,dword rtype)
{ struct peleafnode *leaf;
  lptr t;
  leaf=(struct peleafnode *)data;
  t.assign(options.loadaddr.segm,leaf->datarva+image_base);
  // bugfix to third arg - build 14
  dta.addseg(t,leaf->size,&rawdata[leaf->datarva-pdatarva],resourcedata,resname);
  switch(rtype)
  { case 5: // dialog
      scheduler.addtask(dis_dialog,priority_data,t,resname);
      break;
    case 6: // stringtable
      scheduler.addtask(dis_stringtable,priority_data,t,resname);
      break;
    default:
      break;
  }
}


