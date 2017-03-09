//					exeload.h
//

#ifndef exeload_h
#define exeload_h

#include "common.h"

#define PE_EXE 1
#define MZ_EXE 2
#define OS2_EXE 3
#define COM_EXE 5
#define NE_EXE 6
#define SYS_EXE 7
#define LE_EXE 8
#define BIN_EXE 9

#pragma pack(push,pack_save,1)

struct mzheader
{ word sig;
  word numbytes,numpages;
  word numrelocs,headersize;
  word minpara,maxpara;
  word initialss,initialsp;
  word csum;
  dword csip;
  word relocoffs;
  word ovlnum;
};

struct neheader
{ word sig;
  word linkerver;
  word entryoffs,entrylen;
  dword filecrc;
  word contentflags;
  word dsnum;
  word heapsize,stacksize;
  dword csip,sssp;
  word numsegs,nummodules;
  word nonresnamesize;
  word offs_segments,offs_resources,offs_resnames,offs_module,offs_imports;
  dword nonresnametable;
  word movableentries;
  word shiftcount;
  word numresources;
  byte targetos,os_info;
  word fastloadoffs,fastloadlen;
  word mincodeswapareasize,winver;
};

struct nesegtable
{ word sectoroffs;
  word seglength;
  word segflags;
  word minalloc;
};

struct nesegtablereloc
{ byte reloctype,relocsort;
  word segm_offs;
  word index,indexoffs;
};

struct peheader
{ dword sigbytes; //+0
  word cputype,objects;
  dword timedatestamp;
  unsigned long reserveda[2];
  unsigned short int nt_hdr_size,flags;
  // optional header
  unsigned short int reserved; // +24
  unsigned char lmajor,lminor;
  unsigned long reserved1[3];
  unsigned long entrypoint_rva; // +40
  unsigned long reserved2[2];
  unsigned long image_base; // +52
  unsigned long objectalign;
  unsigned long filealign;
  unsigned short int osmajor,osminor;
  unsigned short int usermajor,userminor;
  unsigned short int subsysmajor,subsysminor;
  unsigned long reserved3;
  unsigned long imagesize; // +80
  unsigned long headersize;
  unsigned long filechecksum;
  unsigned short int subsystem,dllflags;
  unsigned long stackreserve,stackcommit; // +96
  unsigned long heapreserve,heapcommit;
  unsigned long reserved4;
  unsigned long numintitems; // +116
  unsigned long exporttable_rva,export_datasize; // +120
  unsigned long importtable_rva,import_datasize; // +128
  unsigned long resourcetable_rva,resource_datasize;
  unsigned long exceptiontable_rva,exception_datasize;
  unsigned long securitytable_rva,security_datasize;
  unsigned long fixuptable_rva,fixup_datasize;
  unsigned long debugtable_rva,debug_directory;
  unsigned long imagedesc_rva,imagedesc_datasize;
  unsigned long machspecific_rva,machspecific_datasize;
  unsigned long tls_rva,tls_datasize;
};

struct peobjdata
{ char name[8];
  unsigned long virt_size,rva;
  unsigned long phys_size,phys_offset;
  unsigned long reserved[3],obj_flags;
};

struct peimportdirentry
{ dword originalthunkrva;
  dword timedatestamp;
  dword forwarder;
  dword namerva;
  dword firstthunkrva;
};

struct peexportdirentry
{ dword characteristics;
  dword timedatestamp;
  word majver,minver;
  dword namerva;
  dword base;
  dword numfunctions;
  dword numnames;
  dword funcaddrrva,nameaddrrva,ordsaddrrva;
};

struct perestable
{ dword flags;
  dword timedatestamp;
  word majver,minver;
  word numnames,numids;
};

struct peleafnode
{ dword datarva;
  dword size;
  dword codepage;
  dword reserved;
};

struct perestableentry
{ dword id;
  dword offset;
};

struct perelocheader
{ dword rva;
  dword len;
};

BOOL CALLBACK savemessbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

//loads file and sets up objects using data.cpp
class fileloader
{ public:
	int exetype;
	HANDLE efile;
	byte *fbuff;
  private:
	byte *rawdata;
    // added build 14 bugfix
    dword pdatarva;
	// moved here 2.28 so we can access it later
	// for changing oep, etc
    peheader *peh;
  
  public:
	fileloader(void);
	~fileloader(void);
	int getexetype(void);
	void setexetype(int etype);
    dword fileoffset(lptr loc);
    void patchfile(dword file_offs,dword num,byte *dat);
    void patchoep(void);
    void reloadfile(dword file_offs,dword num,byte *dat);
	void readcomfile(dword fsize);
	void readsysfile(dword fsize);
	void readpefile(dword offs);
	void readmzfile(dword fsize);
	void readlefile(void);
	void readnefile(dword offs);
	void reados2file(void);
	void readbinfile(dword fsize);

  private:
	void subdirsummary(byte *data,char *impname,dword image_base,dword rtype);
	void leaf2summary(byte *data,char *name,dword image_base,dword rtype);
	void leafnodesummary(byte *data,char *resname,dword image_base,dword rtype);
};

#pragma pack(pop,pack_save)

extern class fileloader floader;

#endif
