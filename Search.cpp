/************************************************************************
*                 search.cpp                                            *
* These are the functions which handle searching. I still have more     *
* work to do on searching (particularly regarding strings - unicode,    *
* etc), but the basic functionality is there. Other ideas for the       *
* future include wildcard searching. (Byte wildcard searching would be  *
* nice :))                                                              *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "dasm.h"
#include "schedule.h"
#include "srch.h"
#include "data.h"
#include "disasm.h"
#include "debug.h"
#include "resource.h"

enum search_type {SEARCH_STR=1,SEARCH_HEX,SEARCH_DEC,SEARCH_BYTES};

#define MAX_SEARCHLEN 200

/************************************************************************
* global variables                                                      *
* - these just save the search dialog status, and allow us to fill in   *
*   the old options in the dialog box, and also do a second search, etc *
************************************************************************/
char oldsearchtext[MAX_SEARCHLEN+1]="";
search_type lastsearchtype=SEARCH_STR;
bool lastfromstart=false;

/************************************************************************
* forward declarations                                                  *
************************************************************************/
BOOL CALLBACK searchbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK searchingbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
* searchengine                                                          *
* - simply stops the secondary thread, puts up the search dialog box,   *
*   and restarts the thread again at the end                            *
************************************************************************/
void searchengine(void)
{ scheduler.stopthread();
  DialogBox(hInst,MAKEINTRESOURCE(Search_Dialog),mainwindow,(DLGPROC)searchbox);
  scheduler.continuethread();
}

/************************************************************************
* parsestring                                                           *
* - parses a string into from the input text into the search array      *
************************************************************************/
int parsestring(byte *match,char *srchtext)
{ strcpy((char *)match,srchtext);
  return strlen(srchtext);
}

/************************************************************************
* parsehex                                                              *
* - parses a string for a hex value and puts it in the search array     *
************************************************************************/
int parsehex(byte *match,char *srchtext)
{ int matchlen;
  dword mtch;
  sscanf(srchtext,"%x",&mtch);
  if(mtch<256)
  { matchlen=1;
    match[0]=(byte)mtch;
  }
  else if(mtch<65536)
  { matchlen=2;
    match[0]=(byte)(mtch&0xff);
    match[1]=(byte)(mtch/0x100);
  }
  else
  { matchlen=4;
    match[0]=(byte)(mtch&0xff);
    match[1]=(byte)(mtch/0x100);
    match[2]=(byte)(mtch/0x10000);
    match[3]=(byte)(mtch/0x1000000);
  }
  return matchlen;
}

/************************************************************************
* parsedecimal                                                          *
* - parses a string for a decimal value and puts it in the search array *
************************************************************************/
int parsedec(byte *match,char *srchtext)
{ int matchlen;
  dword mtch;
  sscanf(srchtext,"%d",&mtch);
  if(mtch<256)
  { matchlen=1;
    match[0]=(byte)mtch;
  }
  else if(mtch<65536)
  { matchlen=2;
    match[0]=(byte)(mtch&0xff);
    match[1]=(byte)(mtch/0x100);
  }
  else
  { matchlen=4;
    match[0]=(byte)(mtch&0xff);
    match[1]=(byte)(mtch/0x100);
    match[2]=(byte)(mtch/0x10000);
    match[3]=(byte)(mtch/0x1000000);
  }
  return matchlen;
}

/************************************************************************
* parsebytes                                                            *
* - parses a string for a series of bytes and puts them in the search   *
*   array                                                               *
************************************************************************/
int parsebytes(byte *match,char *srchtext)
{ int matchlen,i;
  byte tmpbyte;
  matchlen=strlen(srchtext)/2;
  for(i=0;i<matchlen;i++)
  { if(srchtext[i*2]>='a')
      tmpbyte=(byte)((srchtext[i*2]-'a'+10));
	 else
      tmpbyte=(byte)((srchtext[i*2]-'0'));
	 if(srchtext[i*2+1]>='a')
      tmpbyte=(byte)(tmpbyte*16+(srchtext[i*2+1]-'a'+10));
	 else
      tmpbyte=(byte)(tmpbyte*16+(srchtext[i*2+1]-'0'));
    match[i]=tmpbyte;
  }
  return matchlen;
}

/************************************************************************
* dosearch                                                              *
* - main search routine ripped from other routines in previous versions *
************************************************************************/
void dosearch(HWND hdwnd,search_type searchtype,bool fromstart,lptr s_seg)
{ bool found;                  // did we find it ?
  int matchlen;                // length of string to try and match
  int i;                       // loop counter
  HWND sbox;                   // 'searching' box displayed during search
  dsegitem *srchseg;           // current segment of search
  byte match[MAX_SEARCHLEN+1]; // what we're looking for.... filled in by parse routine
  byte *segmtch;               // segment data pointer.... want segmtch[x]==match[x]
  sbox=CreateDialog(hInst,MAKEINTRESOURCE(S_Box),hdwnd,(DLGPROC)searchingbox);
  switch(searchtype)
  { case SEARCH_STR:
      matchlen=parsestring(match,oldsearchtext);
      break;
    case SEARCH_HEX:
      matchlen=parsehex(match,oldsearchtext);
		break;
    case SEARCH_DEC:
      matchlen=parsedec(match,oldsearchtext);
		break;
    case SEARCH_BYTES:
      matchlen=parsebytes(match,oldsearchtext);
		break;
    default:
      MessageBox(hdwnd,"Internal Error:Search Unknown Option","Borg",MB_OK);
      return;
  }
  // string->data,
  // for each seg do .....
  found=false;
  if(fromstart)
  { dta.resetiterator();
	 srchseg=dta.nextiterator();
  }
  else
  {  srchseg=dta.findseg(s_seg);
  }
  lastfromstart=fromstart;
  while(srchseg!=NULL)
  { s_seg.segm=srchseg->addr.segm;
    if(fromstart)
      s_seg=srchseg->addr;
    fromstart=false;
	 if(s_seg<srchseg->addr)
      s_seg=srchseg->addr;
	 while(s_seg<=srchseg->addr+srchseg->size-matchlen)
	 { segmtch=srchseg->data+(s_seg-srchseg->addr);
		found=true;
		for(i=0;i<matchlen;i++)
		{ if(segmtch[i]!=match[i])
		  { found=false;
		    break;
		  }
      }
		if(found)
        break;
		s_seg++;
    }
	 if(found)
      break;
	 s_seg.offs=0;
	 srchseg=dta.nextiterator();
  }
  if(found)
    scheduler.addtask(user_jumptoaddr,priority_userrequest,s_seg,NULL);
  DestroyWindow(sbox);
}
/************************************************************************
* searchmore                                                            *
* - Rewritten v2.22...... just calls the search function with search    *
*   again now.                                                          *
************************************************************************/
void searchmore(void)
{ lptr s_seg;
  if(!strlen(oldsearchtext))
  { searchengine();
  	 return;
  }
  scheduler.stopthread();
  dio.findcurrentaddr(&s_seg);
  s_seg+=dsm.getlength(s_seg);
  dosearch(mainwindow,lastsearchtype,false,s_seg);
  scheduler.continuethread();
}

/************************************************************************
* searchbox                                                             *
* - this is the main search dialog box.                                 *
* - it performs the search when we press ok, and the state of controls  *
*   is saved to global variables                                        *
* - search function separated out v2.22                                 *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK searchbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ search_type searchtype;
  lptr s_seg;
  bool fromstart;  // added plus code, bug fix build 14
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDOK:
				if(SendDlgItemMessage(hdwnd,search_string,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  searchtype=SEARCH_STR;
				else if(SendDlgItemMessage(hdwnd,search_hex,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  searchtype=SEARCH_HEX;
				else if(SendDlgItemMessage(hdwnd,search_decimal,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  searchtype=SEARCH_DEC;
				else
              searchtype=SEARCH_BYTES;
				if(SendDlgItemMessage(hdwnd,search_fromstart,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				{ s_seg=options.loadaddr;
				  fromstart=true;
				}
				else
				{ dio.findcurrentaddr(&s_seg);
				  s_seg+=dsm.getlength(s_seg);
				  fromstart=false;
				}
				SendDlgItemMessage(hdwnd,search_edit,WM_GETTEXT,(WPARAM)18,(LPARAM)oldsearchtext);
            dosearch(hdwnd,searchtype,fromstart,s_seg);
            lastsearchtype=searchtype;
				EndDialog(hdwnd,NULL);
				return true;
			 case IDCANCEL:
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
      fromstart=lastfromstart;
      searchtype=lastsearchtype;
	   if(searchtype==SEARCH_STR)
		  SendDlgItemMessage(hdwnd,search_string,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
	   else if(searchtype==SEARCH_HEX)
		  SendDlgItemMessage(hdwnd,search_hex,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
	   else if(searchtype==SEARCH_DEC)
		  SendDlgItemMessage(hdwnd,search_decimal,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
	   else
		  SendDlgItemMessage(hdwnd,search_bytes,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
      if(fromstart)
		  SendDlgItemMessage(hdwnd,search_fromstart,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
      else
		  SendDlgItemMessage(hdwnd,search_fromcurr,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
		SendDlgItemMessage(hdwnd,search_edit,WM_SETTEXT,(WPARAM)0,(LPARAM)oldsearchtext);
		SetFocus(GetDlgItem(hdwnd,search_edit));
		return false;
    default:
      break;
  }
  return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* searchingbox                                                          *
* - this is simply a small dialog box with only the text message        *
*   'searching' which is displayed while the search takes place. Note   *
*   that while a search is being done we are in the primary thread and  *
*   that the secondary thread is stopped, so nothing else happens       *
*   within Borg at all.                                                 *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK searchingbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ switch(message)
  { case WM_INITDIALOG:
      CenterWindow(hdwnd);
		return false;
    default:
      break;
  }
  return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif