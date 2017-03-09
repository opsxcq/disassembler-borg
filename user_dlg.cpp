/************************************************************************
* dlg_view.cpp                                                          *
* - the functions here are viewers for various lists within Borg, for   *
*   example exports,imports and xrefs Dialog box viewers, along with    *
*   any calling functions which stop/start the secondary thread.        *
* - Extracted from various classes v2.22                                *
* - All routines in here when entered from outside should stop the      *
*   secondary thread and restart it on exit.                            *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "dasm.h"
#include "schedule.h"
#include "xref.h"
#include "range.h"
#include "gname.h"
#include "disio.h"
#include "data.h"
#include "disasm.h"
#include "debug.h"
#include "exeload.h"

/************************************************************************
* global variables                                                      *
* nme is used to hold a name entered in a dialog                        *
************************************************************************/
char *nme;

/************************************************************************
* forward declarations                                                  *
************************************************************************/
BOOL CALLBACK exportsbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK importsbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK namesbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK getnamebox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK blockbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK xrefsbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK segbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK getcommentbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK getjaddrbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK getoepbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
* exportsviewer                                                         *
* - stops the thread and displays the exports viewer dialog.            *
************************************************************************/
void exportsviewer(void)
{ scheduler.stopthread();
  if(!expt.numlistitems())
  { MessageBox(mainwindow,"There are no exports in the list","Borg Message",MB_OK);
  }
  else
  { DialogBox(hInst,MAKEINTRESOURCE(Exports_Viewer),mainwindow,(DLGPROC)exportsbox);
  }
  scheduler.continuethread();
}

/************************************************************************
* exportsbox                                                            *
* - this is the exports viewer dialog box. It is simpler than the names *
*   class dialog box, featuring only a jump to option. As for the names *
*   class a request is added to the scheduler for any jump and the      *
*   dialog box exits. the main code is for filling the initial list box *
*   and for displaying a new address when the selection is changed      *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK exportsbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static char nseg[20],noffs[20];
  static gnameitem *t;
  dword st;
  dword i;
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDC_OKBUTTON:
				EndDialog(hdwnd,NULL);
				return true;
			 case IDC_JUMPTOBUTTON:
				scheduler.addtask(user_jumptoaddr,priority_userrequest,t->addr,NULL);
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		  switch(HIWORD(wParam))
		  { case LBN_SELCHANGE:
				i=SendDlgItemMessage(hdwnd,IDC_EXPORTSLISTBOX,LB_GETCURSEL,0,0)+1;
				expt.resetiterator();
				while(i)
				{ t=expt.nextiterator();
				  i--;
				}
				st=t->addr.segm;
				wsprintf(nseg,"0x%lx",st);
				wsprintf(noffs,"0x%lx",t->addr.offs);
				SendDlgItemMessage(hdwnd,EXPORTS_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)nseg);
				SendDlgItemMessage(hdwnd,EXPORTS_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)noffs);
            return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		expt.resetiterator();
		for(i=0;i<expt.numlistitems();i++)
		{ t=expt.nextiterator();
		  SendDlgItemMessage(hdwnd,IDC_EXPORTSLISTBOX,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)t->name);
		}
		SendDlgItemMessage(hdwnd,IDC_EXPORTSLISTBOX,LB_SETCURSEL,0,0);
		expt.resetiterator();
		t=expt.nextiterator();
		st=t->addr.segm;
		wsprintf(nseg,"0x%lx",st);
		wsprintf(noffs,"0x%lx",t->addr.offs);
		SendDlgItemMessage(hdwnd,EXPORTS_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)nseg);
		SendDlgItemMessage(hdwnd,EXPORTS_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)noffs);
		SetFocus(GetDlgItem(hdwnd,IDC_OKBUTTON));
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
* importsviewer                                                         *
* - stops the thread and displays the imports viewer dialog.            *
************************************************************************/
void importsviewer(void)
{ scheduler.stopthread();
  if(!import.numlistitems())
  { MessageBox(mainwindow,"There are no imports in the list","Borg Message",MB_OK);
  }
  else
  { DialogBox(hInst,MAKEINTRESOURCE(Imports_Viewer),mainwindow,(DLGPROC)importsbox);
  }
  scheduler.continuethread();
}

/************************************************************************
* importsbox                                                            *
* - this is the imports viewer dialog box, it is similar to the exports *
*   and names dialog, although simpler since there is only an ok button *
*   Most of the code is for filling the list box and displaying info    *
*   when an item is selected                                            *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK importsbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static char nseg[20],noffs[20];
  static gnameitem *t;
  dword st;
  dword i;
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDC_OKBUTTON:
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		  switch(HIWORD(wParam))
		  { case LBN_SELCHANGE:
				i=SendDlgItemMessage(hdwnd,IDC_IMPORTSLISTBOX,LB_GETCURSEL,0,0)+1;
				import.resetiterator();
				while(i)
				{ t=import.nextiterator();
				  i--;
				}
				st=t->addr.segm;
				wsprintf(nseg,"0x%lx",st);
				wsprintf(noffs,"0x%lx",t->addr.offs);
				SendDlgItemMessage(hdwnd,IMPORTS_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)nseg);
				SendDlgItemMessage(hdwnd,IMPORTS_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)noffs);
				return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		import.resetiterator();
		for(i=0;i<import.numlistitems();i++)
		{ t=import.nextiterator();
		  SendDlgItemMessage(hdwnd,IDC_IMPORTSLISTBOX,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)t->name);
		}
		SendDlgItemMessage(hdwnd,IDC_IMPORTSLISTBOX,LB_SETCURSEL,0,0);
		import.resetiterator();
		t=import.nextiterator();
		st=t->addr.segm;
		wsprintf(nseg,"0x%lx",st);
		wsprintf(noffs,"0x%lx",t->addr.offs);
		SendDlgItemMessage(hdwnd,IMPORTS_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)nseg);
		SendDlgItemMessage(hdwnd,IMPORTS_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)noffs);
		SetFocus(GetDlgItem(hdwnd,IDC_OKBUTTON));
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
* namelocation                                                          *
* - this calls the user dialog for a name to be entered for the current *
*   location, and names it                                              *
************************************************************************/
void namelocation(void)
{ lptr loc;
  scheduler.stopthread();
  nme=NULL;
  DialogBox(hInst,MAKEINTRESOURCE(Get_Name),mainwindow,(DLGPROC)getnamebox);
  if(nme!=NULL)
  { dio.findcurrentaddr(&loc);
    scheduler.addtask(namecurloc,priority_userrequest,loc,nme);
  }
  scheduler.continuethread();
}

/************************************************************************
* namesviewer                                                           *
* - this controls the display of the names viewer dialog box. names are *
*   viewed in the dialog box in location order.                         *
************************************************************************/
void namesviewer(void)
{ scheduler.stopthread();
  if(!name.numlistitems())
  { MessageBox(mainwindow,"There are no names in the list","Borg Message",MB_OK);
  }
  else
  { DialogBox(hInst,MAKEINTRESOURCE(Names_Viewer),mainwindow,(DLGPROC)namesbox);
  }
  scheduler.continuethread();
}

/************************************************************************
* namesbox                                                              *
* - the dialog box for the names list.                                  *
* - the list is a simple location order of names, which is the same as  *
*   the underlying list class ordering                                  *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK namesbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static char nseg[20],noffs[20];
  static gnameitem *t;
  dword st;
  dword i;
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDC_OKBUTTON:
				EndDialog(hdwnd,NULL);
				return true;
			 case IDC_JUMPTOBUTTON:
				scheduler.addtask(user_jumptoaddr,priority_userrequest,t->addr,NULL);
				EndDialog(hdwnd,NULL);
				return true;
			 case NAMES_DELETE:
				name.delname(t->addr);
            scheduler.addtask(user_repeatnameview,priority_userrequest,nlptr,NULL);
				EndDialog(hdwnd,NULL);
				return true;
			 case NAMES_RENAME:
				nme=NULL;
				DialogBox(hInst,MAKEINTRESOURCE(Get_Name),hdwnd,(DLGPROC)getnamebox);
				if(nme!=NULL)
              scheduler.addtask(namecurloc,priority_userrequest,t->addr,nme);
            scheduler.addtask(user_repeatnameview,priority_userrequest,nlptr,NULL);
				EndDialog(hdwnd,NULL);
            return true;
			 default:
				break;
		  }
		  switch(HIWORD(wParam))
		  { case LBN_SELCHANGE:
				i=SendDlgItemMessage(hdwnd,IDC_NAMESLISTBOX,LB_GETCURSEL,0,0)+1;
				name.resetiterator();
				while(i)
				{ t=name.nextiterator();
				  i--;
				}
				st=t->addr.segm;
				wsprintf(nseg,"0x%lx",st);
				wsprintf(noffs,"0x%lx",t->addr.offs);
				SendDlgItemMessage(hdwnd,NAMES_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)nseg);
				SendDlgItemMessage(hdwnd,NAMES_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)noffs);
				return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		name.resetiterator();
		for(i=0;i<name.numlistitems();i++)
		{ t=name.nextiterator();
		  SendDlgItemMessage(hdwnd,IDC_NAMESLISTBOX,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)t->name);
		}
		SendDlgItemMessage(hdwnd,IDC_NAMESLISTBOX,LB_SETCURSEL,0,0);
		name.resetiterator();
		t=name.nextiterator();
		st=t->addr.segm;
		wsprintf(nseg,"0x%lx",st);
		wsprintf(noffs,"0x%lx",t->addr.offs);
		SendDlgItemMessage(hdwnd,NAMES_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)nseg);
		SendDlgItemMessage(hdwnd,NAMES_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)noffs);
		SetFocus(GetDlgItem(hdwnd,IDC_OKBUTTON));
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
* getnamebox                                                            *
* - this is a small dialog for the input of name for a location. the    *
*   name is stored (pointer) in the global variable nme for the caller  *
*   to process                                                          *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK getnamebox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDOK:
				EndDialog(hdwnd,NULL);
				nme=new char[GNAME_MAXLEN+1];
				SendDlgItemMessage(hdwnd,IDC_NAMEEDIT,WM_GETTEXT,(WPARAM)GNAME_MAXLEN,(LPARAM)nme);
				return true;
			 case IDCANCEL:
				EndDialog(hdwnd,NULL);
				nme=NULL;
				return true;
          default:
            break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		SetFocus(GetDlgItem(hdwnd,IDC_NAMEEDIT));
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
* blockview                                                             *
* - this stops the secondary thread and puts up the dialog box for      *
*   viewing the extents of the block                                    *
************************************************************************/
void blockview(void)
{ scheduler.stopthread();
  DialogBox(hInst,MAKEINTRESOURCE(Block_Dialog),mainwindow,(DLGPROC)blockbox);
  scheduler.continuethread();
}

/************************************************************************
* blockbox                                                              *
* - this is the dialog which just shows the extents of the current      *
*   block                                                               *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK blockbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static char s1[30],s2[30],s3[40];
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDOK:
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
      wsprintf(s1,"%04x:%08lxh",blk.top.segm,blk.top.offs);
      wsprintf(s2,"%04x:%08lxh",blk.bottom.segm,blk.bottom.offs);
		SendDlgItemMessage(hdwnd,Text_Top,WM_SETTEXT,0,(LPARAM)(LPCTSTR)s1);
		SendDlgItemMessage(hdwnd,Text_Bottom,WM_SETTEXT,0,(LPARAM)(LPCTSTR)s2);
      if(blk.top==nlptr)
        strcpy(s3,"Top not set");
      else if(blk.bottom==nlptr)
        strcpy(s3,"Bottom not set");
      else if(blk.top>blk.bottom)
        strcpy(s3,"Range is empty");
      else
        strcpy(s3,"Range set");
		SendDlgItemMessage(hdwnd,Text_Status,WM_SETTEXT,0,(LPARAM)(LPCTSTR)s3);
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
* the xrefs viewer - stops the thread and continues it after            *
* displaying the dialog box                                             *
************************************************************************/
void xrefsviewer(void)
{ xrefitem *findit,xtmp;
  scheduler.stopthread();
  dio.findcurrentaddr(&xtmp.addr);
  xtmp.refby=nlptr;
  findit=xrefs.findnext(&xtmp);
  if(findit==NULL)
  { MessageBox(mainwindow,"Unable to find any xrefs for the location","Borg Message",MB_OK);
  }
  else if(findit->addr!=xtmp.addr)
  { MessageBox(mainwindow,"There are no xrefs for the current location in the list","Borg Message",MB_OK);
  }
  else
  { DialogBox((struct HINSTANCE__ *)hInst,MAKEINTRESOURCE(Xrefs_Viewer),mainwindow,(DLGPROC)xrefsbox);
  }
  scheduler.continuethread();
}

/************************************************************************
* dialog box controls and workings - most message processing is         *
* standardised across Borg (colorchanges, etc)                          *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK xrefsbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static char nseg[20],noffs[20];
  static xrefitem *currentsel,xtmp,*vt;
  static int numberofitems;
  dword st;
  dword i;
  switch(message)
  { case WM_COMMAND:
		{ switch(wParam)
		  { case IDC_OKBUTTON:
				EndDialog(hdwnd,NULL);
				return true;
			 case IDC_JUMPTOBUTTON:
				scheduler.addtask(user_jumptoaddr,priority_userrequest,currentsel->refby,NULL);
				EndDialog(hdwnd,NULL);
				return true;
			 case XREFS_DELETE:
            scheduler.addtask(user_delxref,priority_userrequest,currentsel->refby,NULL);
	         scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
            if(numberofitems>1)
              scheduler.addtask(user_repeatxrefview,priority_userrequest,nlptr,NULL);
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		  switch(HIWORD(wParam))
		  { case LBN_SELCHANGE:
				i=SendDlgItemMessage(hdwnd,IDC_XREFSLISTBOX,LB_GETCURSEL,0,0)+1;
				xrefs.findnext(&xtmp);
				while(i)
				{ vt=xrefs.nextiterator();
				  i--;
				}
				currentsel=vt;
				return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		dio.findcurrentaddr(&xtmp.addr);
		xtmp.refby=nlptr;
		xrefs.findnext(&xtmp);
		vt=xrefs.nextiterator();
		currentsel=vt;
      numberofitems=0;
      if(vt!=NULL)
		{ while(vt->addr==xtmp.addr)
		  { st=vt->refby.segm;
		    wsprintf(nseg,"0x%lx",st);
		    wsprintf(noffs,"0x%lx",vt->refby.offs);
		    strcat(nseg,":");
		    strcat(nseg,noffs);
          numberofitems++;
		    SendDlgItemMessage(hdwnd,IDC_XREFSLISTBOX,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)nseg);
		    vt=xrefs.nextiterator();
          if(vt==NULL)
            break;
		  }
      }
		SendDlgItemMessage(hdwnd,IDC_XREFSLISTBOX,LB_SETCURSEL,0,0);
		SetFocus(GetDlgItem(hdwnd,IDC_OKBUTTON));
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
* segviewer                                                             *
* - stops the secondary thread, and calls the dialog box for viewing    *
*   the segments, then restarts the thread when the dialog box is done. *
************************************************************************/
void segviewer(void)
{ scheduler.stopthread();
  DialogBox(hInst,MAKEINTRESOURCE(Seg_Viewer),mainwindow,(DLGPROC)segbox);
  scheduler.continuethread();
}

/************************************************************************
* segbox                                                                *
* - this is the segment viewer dialog box which shows the segments, and *
*   details of them as they are clicked on. It allows jumping to the    *
*   segments as well. Background analysis is halted when calling this   *
*   particularly because iterators are used, and they there is only one *
*   iterator for the segment list                                       *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK segbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static string *segt;
  static char sstart[20],send[20],ssize[20],stype[20];
  static dsegitem *t;
  dword st;
  dword i;
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDC_OKBUTTON:
				EndDialog(hdwnd,NULL);
				return true;
			 case IDC_JUMPTOBUTTON:
				scheduler.addtask(user_jumptoaddr,priority_userrequest,t->addr,NULL);
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		  switch(HIWORD(wParam))
		  { case LBN_SELCHANGE:
				i=SendDlgItemMessage(hdwnd,IDC_SEGLISTBOX,LB_GETCURSEL,0,0);
				dta.resetiterator();
				t=dta.nextiterator();
				while(i)
				{ t=dta.nextiterator();
				  i--;
				}
				wsprintf(sstart,"0x%lx",t->addr.offs);
				st=t->addr.offs+t->size-1;
				wsprintf(send,"0x%lx",st);
				wsprintf(ssize,"0x%lx",t->size);
				switch(t->typ)
				{ case code16:
					 strcpy(stype,"16-bit Code");
					 break;
				  case code32:
					 strcpy(stype,"32-bit Code");
					 break;
				  case data16:
					 strcpy(stype,"16-bit Data");
					 break;
				  case data32:
					 strcpy(stype,"32-bit Data");
					 break;
				  case uninitdata:
					 strcpy(stype,"Uninit Data");
					 break;
				  case debugdata:
					 strcpy(stype,"Debug Data");
					 break;
				  case resourcedata:
					 strcpy(stype,"Resource Data");
					 break;
				  default:
					 strcpy(stype,"Unknown");
					 break;
				}
				SendDlgItemMessage(hdwnd,SEG_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)sstart);
				SendDlgItemMessage(hdwnd,SEG_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)send);
				SendDlgItemMessage(hdwnd,SEG_TEXTSIZE,WM_SETTEXT,0,(LPARAM)(LPCTSTR)ssize);
				SendDlgItemMessage(hdwnd,SEG_TEXTTYPE,WM_SETTEXT,0,(LPARAM)(LPCTSTR)stype);
				SendDlgItemMessage(hdwnd,IDC_SEGNAMETEXT,WM_SETTEXT,0,(LPARAM)(LPCTSTR)t->name);
            return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		segt=new string[dta.numlistitems()];
		dta.resetiterator();
		for(i=0;i<dta.numlistitems();i++)
		{ segt[i]=new char[20];
		  t=dta.nextiterator();
		  st=t->addr.segm;
		  wsprintf(segt[i],"0x%x",st);
		  st=t->addr.offs;
		  wsprintf(segt[i]+strlen(segt[i]),":0x%04lx",st);
		  SendDlgItemMessage(hdwnd,IDC_SEGLISTBOX,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)segt[i]);
		}
		SendDlgItemMessage(hdwnd,IDC_SEGLISTBOX,LB_SETCURSEL,0,0);
		dta.resetiterator();
		t=dta.nextiterator();
		wsprintf(sstart,"0x%lx",t->addr.offs);
		st=t->addr.offs+t->size-1;
		wsprintf(send,"0x%lx",st);
		wsprintf(ssize,"0x%lx",t->size);
		switch(t->typ)
		{ case code16:
			 strcpy(stype,"16-bit Code");
			 break;
		  case code32:
			 strcpy(stype,"32-bit Code");
			 break;
		  case data16:
			 strcpy(stype,"16-bit Data");
			 break;
		  case data32:
			 strcpy(stype,"32-bit Data");
			 break;
		  case uninitdata:
			 strcpy(stype,"Uninit Data");
			 break;
		  case debugdata:
			 strcpy(stype,"Debug Data");
			 break;
		  case resourcedata:
			 strcpy(stype,"Resource Data");
			 break;
		  default:
			 strcpy(stype,"Unknown");
			 break;
		}
		SendDlgItemMessage(hdwnd,SEG_TEXTSTART,WM_SETTEXT,0,(LPARAM)(LPCTSTR)sstart);
		SendDlgItemMessage(hdwnd,SEG_TEXTEND,WM_SETTEXT,0,(LPARAM)(LPCTSTR)send);
		SendDlgItemMessage(hdwnd,SEG_TEXTSIZE,WM_SETTEXT,0,(LPARAM)(LPCTSTR)ssize);
		SendDlgItemMessage(hdwnd,SEG_TEXTTYPE,WM_SETTEXT,0,(LPARAM)(LPCTSTR)stype);
		SendDlgItemMessage(hdwnd,IDC_SEGNAMETEXT,WM_SETTEXT,0,(LPARAM)(LPCTSTR)t->name);
		SetFocus(GetDlgItem(hdwnd,IDC_OKBUTTON));
		return false;
	 case WM_DESTROY:
		for(i=0;i<dta.numlistitems();i++)
		{ SendDlgItemMessage(hdwnd,IDC_SEGLISTBOX,LB_DELETESTRING,(WPARAM)(dta.numlistitems()-i-1),0);
		  delete segt[dta.numlistitems()-i-1];
		}
		delete segt;
		return true;
    default:
      break;
  }
  return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* changeoep                                                             *
* - stops the thread and gets an address from the user for the new oep  *
************************************************************************/
void changeoep(void)
{ if(floader.exetype!=PE_EXE)
  { MessageBox(mainwindow,"Can only change the oep for PE files","Borg",MB_OK);
    return;
  }
  scheduler.stopthread();
  DialogBox(hInst,MAKEINTRESOURCE(OEP_Editor),mainwindow,(DLGPROC)getoepbox);
  scheduler.continuethread();
}

/************************************************************************
* getoepbox                                                             *
* - this is the small dialog box for getting an address from the user.  *
* - it will change the program entry point to that address and patch    *
*   the program if necessary. The current viewing address is also       *
*   changed to the new address.                                         *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK getoepbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static lptr loc;
  char newaddr[80];
  bool patchit;
  switch(message)
  { case WM_COMMAND:
	  switch(wParam)
	  { case IDOK:
		  EndDialog(hdwnd,NULL);
          SendDlgItemMessage(hdwnd,IDC_OEPEDIT,WM_GETTEXT,(WPARAM)80,(LPARAM)newaddr);
		  dio.findcurrentaddr(&loc);
          if(strlen(newaddr))
          { if(sscanf(newaddr,"%x",&loc.offs)==1)
		      if(dta.findseg(loc)!=NULL)
		      { dio.savecuraddr();
			    dio.setcuraddr(loc);
			    scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
		      }
		  }
		  /* apply new start point */
          scheduler.addtask(nameloc,priority_userrequest,options.oep,"");
		  options.oep=loc;
		  scheduler.addtask(nameloc,priority_userrequest,options.oep,"start");
		  if(SendDlgItemMessage(hdwnd,IDC_PATCHOEP,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
		    patchit=true;
		  else 
		    patchit=false;
		  if(options.readonly&&patchit)
		  { MessageBox(hdwnd,"Program is opened read only, can't patch it, sorry",
		       "Borg - Change OEP",MB_OK);
		    patchit=false;
		  }
  		  if(patchit)
		  { /* and patch executable */
		    floader.patchoep();
		  }
		  return true;
		case IDCANCEL:
		  EndDialog(hdwnd,NULL);
		  return true;
        default:
          break;
	  }
	  break;
	case WM_INITDIALOG:
      CenterWindow(hdwnd);
      SetFocus(GetDlgItem(hdwnd,IDC_OEPEDIT));
	  if(!options.readonly)
        SendDlgItemMessage(hdwnd,IDC_PATCHOEP,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
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
* jumpto                                                                *
* - stops the thread and gets an address from the user to be jumped to  *
************************************************************************/
void jumpto(void)
{ scheduler.stopthread();
  DialogBox(hInst,MAKEINTRESOURCE(Jaddr_Editor),mainwindow,(DLGPROC)getjaddrbox);
  scheduler.continuethread();
}

/************************************************************************
* getjaddrbox                                                           *
* - this is the small dialog box for getting an address from the user.  *
* - it changes the current viewing location to that address.            *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK getjaddrbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static lptr loc;
  char newaddr[80];
  switch(message)
  { case WM_COMMAND:
	  switch(wParam)
	  { case IDOK:
		  EndDialog(hdwnd,NULL);
          SendDlgItemMessage(hdwnd,IDC_JADDREDIT,WM_GETTEXT,(WPARAM)80,(LPARAM)newaddr);
		  dio.findcurrentaddr(&loc);
          if(strlen(newaddr))
          { if(sscanf(newaddr,"%x",&loc.offs)==1)
		      if(dta.findseg(loc)!=NULL)
		      { dio.savecuraddr();
			    dio.setcuraddr(loc);
			    scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
		      }
		  }
		  return true;
		case IDCANCEL:
		  EndDialog(hdwnd,NULL);
		  return true;
        default:
          break;
	  }
	  break;
	case WM_INITDIALOG:
      CenterWindow(hdwnd);
      SetFocus(GetDlgItem(hdwnd,IDC_JADDREDIT));
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
* getcomment                                                            *
* - stops the thread and gets a comment from the user to be added to    *
*   the disassembly database, and posts a windowupdate request.         *
************************************************************************/
void getcomment(void)
{ scheduler.stopthread();
  DialogBox(hInst,MAKEINTRESOURCE(Comment_Editor),mainwindow,(DLGPROC)getcommentbox);
  scheduler.continuethread();
}

/************************************************************************
* getcommentbox                                                         *
* - this is the small dialog box for getting a comment from the user.   *
* - it determines the current address, and obtains a comment, adding it *
*   to the database, and deleting any previous comment.                 *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK getcommentbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ static lptr loc;
  dsmitem titem,*tblock;
  char newcomment[80];
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDOK:
				EndDialog(hdwnd,NULL);
            	SendDlgItemMessage(hdwnd,IDC_COMMENTEDIT,WM_GETTEXT,(WPARAM)80,(LPARAM)newcomment);
            scheduler.addtask(user_delcomment,priority_userrequest,loc,NULL);
            if(strlen(newcomment))
              scheduler.addtask(user_addcomment,priority_userrequest,loc,newcomment);
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
      // need to get any initial comments and stuff into edit box.
      dio.findcurrentaddr(&loc);
		titem.addr=loc;
  		titem.type=dsmnull;
      dsm.findnext(&titem);
  		tblock=dsm.nextiterator();
      if(tblock!=NULL)
        	while(tblock->addr==loc)
         {  if(tblock->type==dsmcomment)
           	{  SendDlgItemMessage(hdwnd,IDC_COMMENTEDIT,WM_SETTEXT,(WPARAM)0,(LPARAM)tblock->tptr);
            	break;
            }
            tblock=dsm.nextiterator();
            if(tblock==NULL)
              break;
         }
		SetFocus(GetDlgItem(hdwnd,IDC_COMMENTEDIT));
		return false;
    default:
      break;
  }
  return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif
