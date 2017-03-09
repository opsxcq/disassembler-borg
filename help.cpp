/************************************************************************
* help.cpp                                                              *
* - functions for help dialogs                                          *
************************************************************************/
#include <windows.h>

#include "resource.h"
#include "common.h"

#ifdef __BORLANDC__
#pragma warn -par
#endif
/************************************************************************
* helpshortcuts                                                         *
* - the shortcuts help dialog box                                       *
* - simply a text summary of the shortcut keys in Borg                  *
************************************************************************/
BOOL CALLBACK helpshortcuts(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ switch(message)
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
		SetFocus(GetDlgItem(hdwnd,IDOK));
		return false;
	 default:
		break;
  }
  return false;
}

/************************************************************************
* habox                                                                 *
* - actually the 'Help -> About' dialog box                             *
************************************************************************/
BOOL CALLBACK habox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ switch(message)
  { case WM_COMMAND:
		switch(wParam)
		{ case idc_email:
        	 ShellExecute(NULL,"open","mailto:cronos@ntlworld.com",NULL,NULL,SW_SHOWNORMAL);
          return true;
        case idc_website:
        	 ShellExecute(NULL,"open","http://www.cronos.cc/",NULL,NULL,SW_SHOWNORMAL);
          return true;
        case IDC_BUTTON1:
			 EndDialog(hdwnd,NULL);
			 return true;
        default:
          break;
		}
		break;
	 case WM_INITDIALOG:
      CenterWindow(hdwnd);
		SetFocus(GetDlgItem(hdwnd,IDC_BUTTON1));
		return false;
    default:
      break;
  }
  return false;
}

/************************************************************************
* helpbox1                                                              *
* - this is a file_open_options help box which gives a few helping      *
*   hints on the options available.                                     *
************************************************************************/
BOOL CALLBACK helpbox1(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{ switch(message)
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
		return false;
	 default:
		break;
  }
  return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

