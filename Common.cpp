/************************************************************************
*                   common.cpp                                          *
* This includes the lptr class, and any other common functions that     *
* are needed.                                                           *
* The lptr class is a class of 'long pointers' which contain a segment  *
* and an offset value.                                                  *
* I have defined some arithmetic on these which make coding in Borg     *
* easier.                                                               *
* Within Borg the seg/offset values should be well defined, ie unique.  *
* This means we do not have to test for seg1:offs1==seg2:offs2 where    *
* seg1!=seg2. Borg addresses should all be individual. [Of course this  *
* may make some coding of DOS exe's harder later on, but I dont believe *
* it will be a great burden.                                            *
************************************************************************/

#include <windows.h>

#include "common.h"
#include "dasm.h"
#include "resource.h"

/************************************************************************
* nlptr                                                                 *
* - this is a predefined null pointer                                   *
************************************************************************/
const lptr nlptr(0,0);

/************************************************************************
* lptr constructor with values                                          *
* - the standard constructor and destructor are defined in the header   *
*   as null functions. This constructor allows for declarations like    *
*   the nlptr declaration above                                         *
************************************************************************/
lptr::lptr(word seg,dword off)
{ segm=seg;
  offs=off;
}

/************************************************************************
* assign                                                                *
* - this is an assign function for the lptr, to give it a specific      *
*   segment and offset combination.                                     *
************************************************************************/
void lptr::assign(word seg,dword off)
{ segm=seg;
  offs=off;
}

/************************************************************************
* between                                                               *
* - returns true if loc >= lwb and loc <= upb                           *
************************************************************************/
bool lptr::between(const lptr& lwb,const lptr& upb)
{ return ((*this>=lwb) && (*this<=upb));
}

/************************************************************************
* lptr == operator                                                      *
************************************************************************/
bool lptr::operator==(const lptr& loc2)
{ return ((segm==loc2.segm)&&(offs==loc2.offs));
}

/************************************************************************
* lptr <= operator                                                      *
* - this overload of <= is well defined given the constraints we have   *
*   placed on the lptr class. It allows simplification of code in many  *
*   of the lists and databases.                                         *
************************************************************************/
bool lptr::operator<=(const lptr& loc2)
{ if(segm!=loc2.segm)
    return (segm<=loc2.segm);
  return (offs<=loc2.offs);
}

/************************************************************************
* lptr >= operator                                                      *
* - this overload of >= is well defined given the constraints we have   *
*   placed on the lptr class. It allows simplification of code in many  *
*   of the lists and databases.                                         *
************************************************************************/
bool lptr::operator>=(const lptr& loc2)
{ if(segm!=loc2.segm)
    return (segm>=loc2.segm);
  return (offs>=loc2.offs);
}

/************************************************************************
* lptr < operator                                                       *
* - this overload of < is well defined given the constraints we have    *
*   placed on the lptr class. It allows simplification of code in many  *
*   of the lists and databases.                                         *
************************************************************************/
bool lptr::operator<(const lptr& loc2)
{ if(segm!=loc2.segm)
    return (segm<loc2.segm);
  return (offs<loc2.offs);
}

/************************************************************************
* lptr > operator                                                       *
* - this overload of > is well defined given the constraints we have    *
*   placed on the lptr class. It allows simplification of code in many  *
*   of the lists and databases.                                         *
************************************************************************/
bool lptr::operator>(const lptr& loc2)
{ if(segm!=loc2.segm)
    return (segm>loc2.segm);
  return (offs>loc2.offs);
}

/************************************************************************
* lptr != operator                                                      *
* - this overload of != is well defined given the constraints we have   *
*   placed on the lptr class. It allows simplification of code in many  *
*   of the lists and databases.                                         *
************************************************************************/
bool lptr::operator!=(const lptr& loc2)
{ return ((segm!=loc2.segm)||(offs!=loc2.offs));
}

/************************************************************************
* lptr + dword operator                                                 *
* - this has been defined to allowed an expression like lptr r+1 to be  *
*   used. It simply adds the dword to the offset of the lptr, and does  *
*   not change the segment at all                                       *
************************************************************************/
lptr lptr::operator+(dword offs2)
{ lptr tmp;
  tmp.segm=segm;
  tmp.offs=offs+offs2;
  return tmp;
}

/************************************************************************
* lptr ++ operator                                                      *
* - as for + dword we can define ++ similarly                           *
* - note that in these functions offsets can wrap around but generally  *
*   this should not be a problem for us                                 *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
// increment lptr.
lptr& lptr::operator++(int x)
{ offs++;
  return *this;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* lptr += operator                                                      *
* - An overload of +=, as for + and ++                                  *
************************************************************************/
lptr& lptr::operator+=(dword offs2)
{ offs+=offs2;
  return *this;
}

/************************************************************************
* lptr - (dword) operator                                               *
* - If we can add a dword then we take one away also....                *
************************************************************************/
lptr lptr::operator-(dword offs2)
{ lptr tmp;
  tmp.segm=segm;
  tmp.offs=offs-offs2;
  return tmp;
}

/************************************************************************
* lptr1 - lptr2 operator                                                *
* - I have overloaded - in two ways. The first, above, takes a dword    *
*   from an operator, by taking it from the lptrs offset. This overload *
*   allows the difference in two operators to be taken, and returns a   *
*   dword which is the difference in offsets. Now you should only       *
*   really be interested in this if the segments are the same. I found  *
*   it useful to make this definition because it greatly simplifies     *
*   code elsewhere. Note that with some complex expressions you need to *
*   force operator precedence, for example dword+(loc1-loc2)+dword2     *
*   type expressions need the brackets for force the - to be performed  *
*   first.                                                              *
************************************************************************/
dword lptr::operator-(lptr& loc2)
{ return offs-loc2.offs;
}

/************************************************************************
* Basic Support Functions                                               *
************************************************************************/

/************************************************************************
* cleanstring                                                           *
* - moved out of the disasm class build 211.                            *
* - simply changes any strange characters in a string into an           *
*   underscore, used for generating automatic names from disassembled   *
*   strings.                                                            *
************************************************************************/
void cleanstring(char *str)
{ unsigned int i;
  for(i=0;i<strlen(str);i++)
  { if(!isalnum(str[i]))
      str[i]='_';
  }
}

/************************************************************************
* CenterWindow                                                          *
* - centers a window within its client area, used by Dialog functions   *
************************************************************************/
void CenterWindow(HWND hdwnd)
{ RECT drect,prect;
  HWND parent;
  parent=GetParent(hdwnd);
  GetWindowRect(parent,&prect);
  GetWindowRect(hdwnd,&drect);
  MoveWindow(hdwnd,((prect.right+prect.left)-(drect.right-drect.left))/2,
	 ((prect.bottom+prect.top)-(drect.bottom-drect.top))/2,drect.right-drect.left,
    drect.bottom-drect.top,true);
}

/************************************************************************
* demangle                                                              *
* - this is a general string function. Name damangling is currently not *
*   very good, and I have some old Borland source code which could      *
*   improve this greatly......... just need to dig it out, rework it    *
*   for Borg, and put it in here now......                              *
************************************************************************/
void demangle(char **nme)
{ unsigned char buff[256],buff2[256],*name;
  unsigned int namelen,i,j,k,atcount,bpoint;
  bool brac,pointer,rpointer;
  if(!options.demangle)
    return;
  atcount=0;
  i=0;
  j=0;
  bpoint=0;
  brac=false;
  name=(unsigned char *)(*nme);
  while(name[i])
  { if(name[i]=='@')
	 { atcount++;
		if(atcount>1)
		{ buff[j++]=':';
		  buff[j++]=':';
		}
	 }
	 else if((name[i]=='$')&&(name[i+1]=='q')&&(!brac))
	 { brac=true;
		bpoint=j;
		buff[j++]='(';
		i+=1;
	 }
	 else if((name[i]=='$')&&(name[i+1]=='x')&&(name[i+2]=='q')&&(!brac))
	 { brac=true;
		bpoint=j;
		buff[j++]='(';
		i+=2;
	 }
	 else if(!strnicmp((char *)&name[i],"$bctr",5))
	 { k=0;
		while((buff[k]!=':')&&(k<20))
		{ buff[j++]=buff[k++];
		}
		i+=4;
	 }
	 else if(!strnicmp((char *)&name[i],"$bdtr",5))
	 { k=0;
		buff[j++]='~';
		while((buff[k]!=':')&&(k<20))
		{ buff[j++]=buff[k++];
		}
		i+=4;
	 }
	 else if(!strnicmp((char *)&name[i],"$bdla",5))
	 { strcpy((char *)&buff[j],"delete");
		j+=6;
		i+=4;
	 }
	 else if(!strnicmp((char *)&name[i],"$bnwa",5))
	 { strcpy((char *)&buff[j],"new");
		j+=3;
		i+=4;
	 }
	 else if(!strnicmp((char *)&name[i],"$bdele",6))
	 { strcpy((char *)&buff[j],"delete");
		j+=6;
		i+=5;
	 }
	 else if(!strnicmp((char *)&name[i],"$bnew",5))
	 { strcpy((char *)&buff[j],"new");
		j+=5;
		i+=4;
	 }
	 else
      buff[j++]=name[i];
	 i++;
  }
  if(brac)
    buff[j++]=')';
  buff[j]=0;
  strcpy((char *)buff2,(char *)buff);
  if(brac)
  { j=bpoint;
	 k=bpoint;
	 i=0;
	 pointer=false;
	 rpointer=false;
	 while(buff[j])
	 { if((buff[j]=='p')&&(!i))
		{ pointer=true;
		  j++;
		}
		if((buff[j]=='r')&&(!i))
		{ rpointer=true;
		  j++;
		}
		else if((buff[j]=='i')&&(!i))
		{ i=1;
		  strcpy((char *)&buff2[k],"int");
		  k+=3;
		  j+=1;
		}
		else if((buff[j]=='c')&&(!i))
		{ i=1;
		  strcpy((char *)&buff2[k],"char");
		  k+=4;
		  j+=1;
		}
		else if((buff[j]=='v')&&(!i))
		{ i=1;
		  strcpy((char *)&buff2[k],"void");
		  k+=4;
		  j+=1;
		}
		else if((buff[j]=='l')&&(!i))
		{ i=1;
		  strcpy((char *)&buff2[k],"long");
		  k+=4;
		  j+=1;
		}
		else if((buff[j]=='u')&&(buff[j+1]=='i')&&(!i))
		{ i=1;
		  strcpy((char *)&buff2[k],"uint");
		  k+=4;
		  j+=2;
		}
		else if((buff[j]==':')&&(buff[j+1]==':')&&(i))
		{ strcpy((char *)&buff2[k],"::");
		  k+=2;
		  j+=2;
		}
		else if((buff[j]=='t')&&((buff[j+1]>='0')&&(buff[j+1]<='9'))&&(!i))
		{ buff2[k++]=buff[j++];
		  buff2[k++]=buff[j++];
		  i=1;
		}
		else if(((buff[j]>='0')&&(buff[j]<='9'))&&(!i))
		{ i=buff[j]-'0';
		  j++;
		  if((buff[j]>='0')&&(buff[j]<='9'))
		  { i=i*10+buff[j]-'0';
			 j++;
		  }
		  i++;
		}
		else
        buff2[k++]=buff[j++];
		if(i)
		{ i--;
		  if(!i)
		  { if((pointer)&&(buff2[k-1]!=')'))
            buff2[k++]='*';
			 if((rpointer)&&(buff2[k-1]!=')'))
            buff2[k++]='&';
			 if((buff[j]!=')')&&(buff2[k-1]!=')'))
				buff2[k++]=',';
			 pointer=false;
			 rpointer=false;
		  }
		}
	 }
	 buff2[k]=0;
  }
  namelen=strlen((char *)buff2);
  //BugFix Build 15 nme-> *nme.
  delete *nme;
  name=new unsigned char[namelen+1];
  strcpy((char *)name,(char *)buff2);
  *nme=(char *)name;
}

/************************************************************************
* init_ofn                                                              *
* - initialises the OPENFILENAME struct used in calls to common dialogs *
*   callers can then change options further as needed                   *
************************************************************************/
void init_ofn(OPENFILENAME *ofn)
{ int cbString,i;
  char chReplace;
  static char szDirName[MAX_PATH*2];
  static char szFilesave[MAX_PATH*2];
  static char szFilter[MAX_PATH];
  static char szFileTitle[MAX_PATH*2];

  GetCurrentDirectory(MAX_PATH,szDirName);
  szFilesave[0]=0;
  cbString=LoadString(hInst,IDS_FILTERSTRING,szFilter,sizeof(szFilter));
  chReplace=szFilter[cbString-1];
  for(i=0;szFilter[i]!=0;i++)
  { if(szFilter[i]==chReplace)
      szFilter[i]=0;
  }
  ofn->lStructSize=sizeof(OPENFILENAME);
  ofn->hwndOwner=mainwindow;
  ofn->hInstance=NULL;
  ofn->lpstrFilter=szFilter;
  ofn->lpstrCustomFilter=NULL;
  ofn->nMaxCustFilter=0;
  ofn->nFilterIndex=1;
  ofn->lpstrFile=szFilesave;
  ofn->nMaxFile=sizeof(szFilesave)/2;
  ofn->lpstrFileTitle=szFileTitle;
  ofn->nMaxFileTitle=sizeof(szFileTitle)/2;
  ofn->lpstrInitialDir=szDirName;
  ofn->lpstrTitle="Borg Disassembler - Select File";
  ofn->Flags=OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES
				|OFN_EXPLORER;
  ofn->nFileOffset=0;
  ofn->nFileExtension=NULL;
  ofn->lpstrDefExt=NULL;
  ofn->lCustData=0;
  ofn->lpfnHook=NULL;
  ofn->lpTemplateName=NULL;
}

/************************************************************************
* getfiletoload                                                         *
* - obtains a filename for the file to be loaded                        *
*   used for new files to load, and load database file                  *
*   sets fname to the filename                                          *
************************************************************************/
void getfiletoload(char *fname)
{ OPENFILENAME ofn;

  init_ofn(&ofn);
  ofn.Flags|=OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);
  strcpy(fname,ofn.lpstrFile);
}

/************************************************************************
* getfiletosave                                                         *
* - obtains a filename for the file to be saved                         *
*   sets fname to the filename                                          *
************************************************************************/
void getfiletosave(char *fname)
{ OPENFILENAME ofn;

  init_ofn(&ofn);
  GetSaveFileName(&ofn);
  strcpy(fname,ofn.lpstrFile);
}


