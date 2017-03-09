/************************************************************************
*                  dlg_ldopt.cpp                                        *
* Contains the dialog routines for the load file dialogboxes            *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "exeload.h"
#include "proctab.h"
#include "dasm.h"
#include "disasm.h"
#include "help.h"
#include "debug.h"

/************************************************************************
* forward declarations                                                  *
************************************************************************/
BOOL CALLBACK checktypebox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK moreoptions(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
* checktypebox                                                          *
* - after a file has been chosen to load and before the file is loaded  *
*   this is displayed for the user to set options for analysis, file    *
*   type, etc.                                                          *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
//dialog proc for verifying type and
// initial options
BOOL CALLBACK checktypebox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ char segtext[20],offstext[20];
  static int exetype;
  int i;
  dword segd;
  switch(message)
  { case WM_INITDIALOG:
		exetype=floader.getexetype();
		i=0;
		options.loadaddr.segm=0x1000;
		options.loadaddr.offs=0x00;
		switch(exetype)
		{ case NE_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"NE Executable");
			 CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			 options.processor=PROC_80486;
			 options.mode16=true;
			 break;
		  case COM_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"COM File");
			 CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			 options.processor=PROC_80386;
			 options.mode16=true;
			 options.loadaddr.offs=0x100;
			 break;
		  case SYS_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"SYS File");
			 CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			 options.processor=PROC_80386;
			 options.mode16=true;
			 options.loadaddr.offs=0x00;
			 break;
		  case PE_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"PE Executable");
			 CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			 options.processor=PROC_PENTIUM;
			 options.mode16=false;
			 break;
		  case OS2_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"OS2 Executable");
			 CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			 options.processor=PROC_PENTIUM;
			 options.mode16=false;
			 break;
		  case LE_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"LE Executable");
			 CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			 options.processor=PROC_80486;
			 options.mode16=false;
			 break;
		  case MZ_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"COM File");
			 CheckDlgButton(hdwnd,IDC_DOSBUTTON,true);
			 options.processor=PROC_80386;
			 options.mode16=true;
			 break;
		  default:
		  case BIN_EXE:
			 SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"COM File");
			 CheckDlgButton(hdwnd,IDC_BINBUTTON,true);
			 options.processor=PROC_8086;
			 options.mode16=true;
			 break;
		}
		options.mode32=!options.mode16;
		CheckDlgButton(hdwnd,load_debug,options.loaddebug);
		CheckDlgButton(hdwnd,demangle_names,options.demangle);
		CheckDlgButton(hdwnd,IDC_16DASM,options.mode16);
		CheckDlgButton(hdwnd,IDC_32DASM,options.mode32);
		CheckDlgButton(hdwnd,IDC_LOADDATA,options.loaddata);
		CheckDlgButton(hdwnd,IDC_LOADRESOURCES,options.loadresources);
		while(procnames[i].num)
		{ SendDlgItemMessage(hdwnd,IDC_LISTBOX1,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)procnames[i].name);
		  if(options.processor==procnames[i].num)
			 SendDlgItemMessage(hdwnd,IDC_LISTBOX1,LB_SETCURSEL,i,0);
		  i++;
		}
		segd=options.loadaddr.segm;
		wsprintf(segtext,"%x",segd);
		wsprintf(offstext,"%lx",options.loadaddr.offs);
		SendDlgItemMessage(hdwnd,IDC_SEGEDIT,WM_SETTEXT,0,(LPARAM)segtext);
		SendDlgItemMessage(hdwnd,IDC_OFFSEDIT,WM_SETTEXT,0,(LPARAM)offstext);
		return false;
	 case WM_COMMAND:
		switch(LOWORD(wParam))
		{ case IDOK:
			 if(!IsDlgButtonChecked(hdwnd,IDC_DEFAULTBUTTON))
			 { if(IsDlgButtonChecked(hdwnd,IDC_DOSBUTTON))
              floader.setexetype(MZ_EXE);
				else
              floader.setexetype(BIN_EXE);
			 }
			 else if((exetype==BIN_EXE)||(exetype==MZ_EXE))
            floader.setexetype(COM_EXE);
			 options.processor=procnames[SendDlgItemMessage(hdwnd,IDC_LISTBOX1,LB_GETCURSEL,0,0)].num;
			 EndDialog(hdwnd,NULL);
			 return true;
		  case IDC_SEGEDIT:
			 if(HIWORD(wParam)==EN_CHANGE)
			 { SendDlgItemMessage(hdwnd,IDC_SEGEDIT,WM_GETTEXT,(WPARAM)18,(LPARAM)segtext);
				sscanf(segtext,"%x",&options.loadaddr.segm);
			 }
			 return true;
		  case IDC_OFFSEDIT:
			 if(HIWORD(wParam)==EN_CHANGE)
			 { SendDlgItemMessage(hdwnd,IDC_OFFSEDIT,WM_GETTEXT,(WPARAM)18,(LPARAM)offstext);
				sscanf(offstext,"%lx",&options.loadaddr.offs);
			 }
			 return true;
		  case IDC_HELPBUTTON1:
			 DialogBox(hInst,MAKEINTRESOURCE(HELPDIALOG_1),hdwnd,(DLGPROC)helpbox1);
			 return true;
		  case more_options:
			 DialogBox(hInst,MAKEINTRESOURCE(Advanced_Options),hdwnd,(DLGPROC)moreoptions);
			 return true;
		  case load_debug:
			 options.loaddebug=!options.loaddebug;
			 CheckDlgButton(hdwnd,load_debug,options.loaddebug);
			 return true;
		  case demangle_names:
			 options.demangle=!options.demangle;
			 CheckDlgButton(hdwnd,demangle_names,options.demangle);
			 return true;
		  case IDC_16DASM:
			 options.mode16=!options.mode16;
			 CheckDlgButton(hdwnd,IDC_16DASM,options.mode16);
			 return true;
		  case IDC_32DASM:
			 options.mode32=!options.mode32;
			 CheckDlgButton(hdwnd,IDC_32DASM,options.mode32);
			 return true;
		  case IDC_LOADDATA:
			 options.loaddata=!options.loaddata;
			 CheckDlgButton(hdwnd,IDC_LOADDATA,options.loaddata);
			 return true;
		  case IDC_LOADRESOURCES:
			 options.loadresources=!options.loadresources;
			 CheckDlgButton(hdwnd,IDC_LOADRESOURCES,options.loadresources);
			 return true;
        default:
          break;
		}
  }
  return false;
}

#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* moreoptions                                                           *
* - advanced loading options                                            *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK moreoptions(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDOK:
				options.codedetect=0;
				if(IsDlgButtonChecked(hdwnd,advanced_pushbp))
              options.codedetect|=CD_PUSHBP;
				if(IsDlgButtonChecked(hdwnd,advanced_aggressive))
              options.codedetect|=CD_AGGRESSIVE;
				if(IsDlgButtonChecked(hdwnd,advanced_enter))
              options.codedetect|=CD_ENTER;
				if(IsDlgButtonChecked(hdwnd,advanced_movbx))
              options.codedetect|=CD_MOVBX;
				if(IsDlgButtonChecked(hdwnd,advanced_moveax))
              options.codedetect|=CD_MOVEAX;
				if(IsDlgButtonChecked(hdwnd,advanced_eaxfromesp))
              options.codedetect|=CD_EAXFROMESP;
				EndDialog(hdwnd,NULL);
				return true;
			 default:
				break;
		  }
		}
		break;
	 case WM_INITDIALOG:
		CheckDlgButton(hdwnd,advanced_pushbp,options.codedetect&CD_PUSHBP);
		CheckDlgButton(hdwnd,advanced_aggressive,options.codedetect&CD_AGGRESSIVE);
		CheckDlgButton(hdwnd,advanced_enter,options.codedetect&CD_ENTER);
		CheckDlgButton(hdwnd,advanced_movbx,options.codedetect&CD_MOVBX);
		CheckDlgButton(hdwnd,advanced_moveax,options.codedetect&CD_MOVEAX);
		CheckDlgButton(hdwnd,advanced_eaxfromesp,options.codedetect&CD_EAXFROMESP);
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
* load                                                                  *
* - checks file header info, identifies the possible types of files,    *
*   gets the users file loading options and calls the specific exe      *
*   format loading routines.                                            *
************************************************************************/
bool loadexefile(char *fname)
{ char mzhead[2],exthead[2];
  dword pe_offset;
  dword num;
  dword fsize;
  if(floader.efile!=INVALID_HANDLE_VALUE)
    return false;

  // just grab the file size first
  floader.efile=CreateFile(fname,GENERIC_READ,1,NULL,OPEN_EXISTING,0,NULL);
  fsize=GetFileSize(floader.efile,NULL);
  CloseHandle(floader.efile);
  if(!fsize)
  { MessageBox(mainwindow,"File appears to be of zero length ???",
      "Borg Message",MB_OK);
    return false;
  }

  floader.efile=CreateFile(fname,GENERIC_READ|GENERIC_WRITE,1,NULL,OPEN_EXISTING,0,NULL);
  if(floader.efile==INVALID_HANDLE_VALUE)
  { floader.efile=CreateFile(fname,GENERIC_READ,1,NULL,OPEN_EXISTING,0,NULL);
    if(floader.efile==INVALID_HANDLE_VALUE)
      return false;
    options.readonly=true;
    MessageBox(mainwindow,"Couldn't obtain write permission to file\nFile opened readonly - will not be able to apply any patches",
      "Borg Message",MB_OK);
  }
  if(GetFileType(floader.efile)!=FILE_TYPE_DISK)
    return false;
  floader.exetype=BIN_EXE;
  if(ReadFile(floader.efile,mzhead,2,&num,NULL))
  { if((num==2)&&(((mzhead[0]=='M')&&(mzhead[1]=='Z'))||
		 ((mzhead[0]=='Z')&&(mzhead[1]=='M'))))
	 { SetFilePointer(floader.efile,0x3c,NULL,FILE_BEGIN);
		if(ReadFile(floader.efile,&pe_offset,4,&num,NULL))
		  SetFilePointer(floader.efile,pe_offset,NULL,FILE_BEGIN);
		if(ReadFile(floader.efile,exthead,2,&num,NULL))
		{ if(((short int *)exthead)[0]==0x4550)
          floader.exetype=PE_EXE;
		  else if(((short int *)exthead)[0]==0x454e)
          floader.exetype=NE_EXE;
		  else if(((short int *)exthead)[0]==0x454c)
          floader.exetype=LE_EXE;
		  else if(((short int *)exthead)[0]==0x584c)
          floader.exetype=OS2_EXE;
		  else
          floader.exetype=MZ_EXE;
		}
	 }
	 else
	 { if(strlen(fname)>3)
		{ if(!lstrcmpi(fname+strlen(fname)-3,"com"))
		  { SetFilePointer(floader.efile,0,NULL,FILE_BEGIN);
			 floader.exetype=COM_EXE;
		  }
		  else if(!lstrcmpi(fname+strlen(fname)-3,"sys"))
		  { SetFilePointer(floader.efile,0,NULL,FILE_BEGIN);
			 floader.exetype=SYS_EXE;
		  }
		}
	 }
  }
  floader.fbuff=new byte[fsize];
  SetFilePointer(floader.efile,0x00,NULL,FILE_BEGIN);
  ReadFile(floader.efile,floader.fbuff,fsize,&num,NULL);
  DialogBox(hInst,MAKEINTRESOURCE(D_checktype),mainwindow,(DLGPROC)checktypebox);
  if(!options.loadaddr.segm)
  { options.loadaddr.segm=0x1000;
	 MessageBox(mainwindow,"Sorry - Can't use a zero segment base.\nSegment Base has been set to 0x1000"
		,"Borg Message",MB_OK);
  }
  dsm.dissettable();
  switch(floader.exetype)
  { case BIN_EXE:
		floader.readbinfile(fsize);
		break;
	 case PE_EXE:
		floader.readpefile(pe_offset);
		break;
	 case MZ_EXE:
		floader.readmzfile(fsize);
		break;
	 case OS2_EXE:
		floader.reados2file();
		CloseHandle(floader.efile);
		floader.efile=INVALID_HANDLE_VALUE;
		floader.exetype=0;
		return false; // at the moment;
	 case COM_EXE:
		floader.readcomfile(fsize);
		break;
	 case SYS_EXE:
		floader.readsysfile(fsize);
		break;
	 case LE_EXE:
		floader.readlefile();
		CloseHandle(floader.efile);
		floader.efile=INVALID_HANDLE_VALUE;
		floader.exetype=0;
		return false; // at the moment;
	 case NE_EXE:
		floader.readnefile(pe_offset);
		break;
	 default:
		CloseHandle(floader.efile);
		floader.efile=INVALID_HANDLE_VALUE;
		floader.exetype=0;
		return false;
  }
  return true;
}

/************************************************************************
* newfile                                                               *
* - handles selecting a new file and its messages, using the standard   *
*   routine GetOpenFileName                                             *
* - starts up the secondary thread when the file is loaded              *
************************************************************************/
bool newfile(void)
{ // factor of 2 added for nt unicode
  getfiletoload(current_exe_name);
  if(current_exe_name[0])
  { if(loadexefile(current_exe_name))
	 { StatusMessage("File Opened");
		strcat(winname," : ");
		strcat(winname,current_exe_name);
		SetWindowText(mainwindow,winname);
		InThread=true;
		ThreadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread,0,0,&ThreadId);
		changemenus();
	 }
	 else
		MessageBox(mainwindow,"File open failed ?",program_name,MB_OK|MB_ICONEXCLAMATION);
  }
  return 0;
}

