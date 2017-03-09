/************************************************************************
* user_fn.cpp                                                           *
* - the functions here are various user dialogs and directly callable   *
*   routines, these are all primary thread routines                     *
* - Extracted from various classes v2.22                                *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "dasm.h"
#include "schedule.h"
#include "decrypt.h"
#include "resource.h"
#include "range.h"
#include "debug.h"

/************************************************************************
* forward declarations                                                  *
************************************************************************/
BOOL CALLBACK decbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
* global variables                                                      *
* - save state of decryption dialog, etc.....                           *
************************************************************************/
dectype lastdec=decxor;
ditemtype lastditem=decbyte;
char lastvalue[20]={'\0'},last_seg[20]={'\0'},lastoffset[20]={'\0'};
bool patchexe=false;

/************************************************************************
* shutbox                                                               *
* - this is a shutdown warning box if Borg is having difficulty         *
*   quitting. It just puts up a 'shutting down' message for a couple of *
*   seconds                                                             *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK shutbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ return false;
}

#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* choosecolour                                                          *
* - a small dialog box for colour choice (standard dialog) when setting *
*   background or text colours                                          *
************************************************************************/
COLORREF choosecolour(COLORREF cr)
{  CHOOSECOLOR cc;
   COLORREF crCustColors[16];
   cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.hwndOwner=mainwindow;
   cc.hInstance=NULL;
   cc.rgbResult=cr;
   cc.lpCustColors=crCustColors;
   cc.Flags=CC_RGBINIT|CC_FULLOPEN;
   cc.lCustData=0;
   cc.lpfnHook=0;
   cc.lpTemplateName=0;
   if(ChooseColor(&cc))
     return cc.rgbResult;
   return cr;
}

/************************************************************************
* dialg                                                                 *
* - we stop the thread whilst displaying the decryptor dialog and doing *
*   the patch                                                           *
************************************************************************/
void decrypterdialog(void)
{ scheduler.stopthread();
  if(blk.checkblock())
    DialogBox(hInst,MAKEINTRESOURCE(Decrypt_Dialog),mainwindow,(DLGPROC)decbox);
  scheduler.continuethread();
}

/************************************************************************
* decbox                                                                *
* - the decryptor dialog, it only allows patching if the file is not    *
*   readonly, and adds the decryptor to the list and calls the process  *
*   and patch functions.                                                *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK decbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ dword dec_id,d_val;
  lptr d_adr;
  switch(message)
  { case WM_COMMAND:
		{	switch(wParam)
		  { case IDOK:
				if(SendDlgItemMessage(hdwnd,idc_xor,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastdec=decxor;
				else if(SendDlgItemMessage(hdwnd,idc_mul,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastdec=decmul;
				else if(SendDlgItemMessage(hdwnd,idc_add,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastdec=decadd;
				else if(SendDlgItemMessage(hdwnd,idc_sub,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastdec=decsub;
				else if(SendDlgItemMessage(hdwnd,idc_rot,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastdec=decrot;
				else if(SendDlgItemMessage(hdwnd,idc_xadd,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastdec=decxadd;
            else
              lastdec=decnull;
				if(SendDlgItemMessage(hdwnd,idc_byte,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastditem=decbyte;
				else if(SendDlgItemMessage(hdwnd,idc_word,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastditem=decword;
				else if(SendDlgItemMessage(hdwnd,idc_dword,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastditem=decdword;
				else if(SendDlgItemMessage(hdwnd,idc_array,BM_GETCHECK,(WPARAM)0,(LPARAM)0))
				  lastditem=decarray;
            else
              lastditem=decbyte;
				SendDlgItemMessage(hdwnd,idc_value,WM_GETTEXT,(WPARAM)18,(LPARAM)lastvalue);
				SendDlgItemMessage(hdwnd,idc_arrayseg,WM_GETTEXT,(WPARAM)18,(LPARAM)last_seg);
				SendDlgItemMessage(hdwnd,idc_arrayoffset,WM_GETTEXT,(WPARAM)18,(LPARAM)lastoffset);
				if(IsDlgButtonChecked(hdwnd,idc_applytoexe))
              patchexe=true;
            else
              patchexe=false;
				sscanf(lastvalue,"%lx",&d_val);
				sscanf(last_seg,"%lx",&d_adr.segm);
				sscanf(lastoffset,"%lx",&d_adr.offs);
            if((options.readonly)&&(patchexe))
            { patchexe=false;
              MessageBox(mainwindow,"File opened readonly - unable to patch","Borg Message",MB_OK);
            }
            dec_id=decrypter.add_decrypted(blk.top,blk.bottom,lastdec,lastditem,d_val,d_adr,patchexe);
            decrypter.process_dec(dec_id);
            if(patchexe)
              decrypter.exepatch(dec_id);
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
      switch(lastdec)
      { case decxor:
			 SendDlgItemMessage(hdwnd,idc_xor,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decmul:
			 SendDlgItemMessage(hdwnd,idc_mul,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decadd:
			 SendDlgItemMessage(hdwnd,idc_add,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decsub:
          SendDlgItemMessage(hdwnd,idc_sub,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decrot:
			 SendDlgItemMessage(hdwnd,idc_rot,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decxadd:
			 SendDlgItemMessage(hdwnd,idc_xadd,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        default:
			 SendDlgItemMessage(hdwnd,idc_xor,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
      }
      switch(lastditem)
      { case decbyte:
			 SendDlgItemMessage(hdwnd,idc_byte,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decword:
			 SendDlgItemMessage(hdwnd,idc_word,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decdword:
			 SendDlgItemMessage(hdwnd,idc_dword,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        case decarray:
			 SendDlgItemMessage(hdwnd,idc_array,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
        default:
			 SendDlgItemMessage(hdwnd,idc_byte,BM_SETCHECK,(WPARAM)1,(LPARAM)0);
          break;
      }
		SendDlgItemMessage(hdwnd,idc_value,WM_SETTEXT,(WPARAM)0,(LPARAM)lastvalue);
		SendDlgItemMessage(hdwnd,idc_arrayseg,WM_SETTEXT,(WPARAM)0,(LPARAM)last_seg);
		SendDlgItemMessage(hdwnd,idc_arrayoffset,WM_SETTEXT,(WPARAM)0,(LPARAM)lastoffset);
      CheckDlgButton(hdwnd,idc_applytoexe,patchexe);
		SetFocus(GetDlgItem(hdwnd,idc_value));
		return false;
    default:
      break;
  }
  return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

