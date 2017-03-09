/************************************************************************
* registry.cpp                                                          *
* - functions for saving and loading Borgs settings in the registry     *
************************************************************************/

#include <windows.h>

#include "dasm.h"

char reg_borg[]="Software\\Borg";

/************************************************************************
* save_reg_entries                                                      *
* - Saves Borgs settings to the registry :)                             *
* - these are just colours, fonts, version, directory and window status *
************************************************************************/
void save_reg_entries(void)
{  HKEY regkey;
	DWORD disposition;
   DWORD rver,dsize;
   COLORREF cl;
	char cdir[MAX_PATH];
	if(RegCreateKeyEx(HKEY_CURRENT_USER,reg_borg,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
   	NULL,&regkey,&disposition)!=ERROR_SUCCESS)
     return;
   // now write values out.......
   dsize=sizeof(rver);
   rver=BORG_VER;
   RegSetValueEx(regkey,"Version",0,REG_DWORD,(LPBYTE)&rver,dsize);
   GetCurrentDirectory(MAX_PATH,cdir);
	dsize=strlen(cdir)+1;
   RegSetValueEx(regkey,"Curdir",0,REG_SZ,(LPBYTE)cdir,dsize);
   dsize=sizeof(options.winmax);
   RegSetValueEx(regkey,"Winmax",0,REG_DWORD,(LPBYTE)&options.winmax,dsize);
   dsize=sizeof(COLORREF);
   cl=options.bgcolor;
   RegSetValueEx(regkey,"BackgroundColor",0,REG_DWORD,(LPBYTE)&cl,dsize);
   cl=options.highcolor;
   RegSetValueEx(regkey,"HighlightColor",0,REG_DWORD,(LPBYTE)&cl,dsize);
   cl=options.textcolor;
   RegSetValueEx(regkey,"TextColor",0,REG_DWORD,(LPBYTE)&cl,dsize);
   cl=options.font;
   RegSetValueEx(regkey,"Font",0,REG_DWORD,(LPBYTE)&cl,dsize);
   RegCloseKey(regkey);
}

/************************************************************************
* load_reg_entries                                                      *
* - Loads Borgs saved settings from the registry :)                     *
* - these are just colours, fonts, version, directory and window status *
* - settings are then restored                                          *
************************************************************************/
void load_reg_entries(void)
{  HKEY regkey;
	DWORD disposition;
   DWORD rver,dsize;
   COLORREF cl;
   char cdir[MAX_PATH];
	if(RegCreateKeyEx(HKEY_CURRENT_USER,reg_borg,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
   	NULL,&regkey,&disposition)!=ERROR_SUCCESS)
     return;
   if(disposition==REG_CREATED_NEW_KEY)
   { RegCloseKey(regkey);
     return;
   }
   dsize=sizeof(rver);
   rver=0;
   RegQueryValueEx(regkey,"Version",NULL,NULL,(LPBYTE)&rver,&dsize);
   // now read values in.......
   cdir[0]=0;
   dsize=MAX_PATH;
   RegQueryValueEx(regkey,"Curdir",NULL,NULL,(LPBYTE)cdir,&dsize);
   if(cdir[0])
     SetCurrentDirectory(cdir);
   dsize=sizeof(options.winmax);
   RegQueryValueEx(regkey,"Winmax",NULL,NULL,(LPBYTE)&options.winmax,&dsize);
   if(options.winmax)
   { PostMessage(mainwindow,WM_MAXITOUT,0,0);
   }
   dsize=sizeof(cl);
   if(RegQueryValueEx(regkey,"BackgroundColor",NULL,NULL,(LPBYTE)&cl,&dsize)==ERROR_SUCCESS)
   	options.bgcolor=cl;
   if(RegQueryValueEx(regkey,"HighlightColor",NULL,NULL,(LPBYTE)&cl,&dsize)==ERROR_SUCCESS)
   	options.highcolor=cl;
   if(RegQueryValueEx(regkey,"TextColor",NULL,NULL,(LPBYTE)&cl,&dsize)==ERROR_SUCCESS)
   	options.textcolor=cl;
   if(RegQueryValueEx(regkey,"Font",NULL,NULL,(LPBYTE)&cl,&dsize)==ERROR_SUCCESS)
   {	options.font=(fontselection)cl;
   }
   RegCloseKey(regkey);
   if(rver!=BORG_VER)
   { RegCloseKey(regkey);
     RegDeleteKey(HKEY_CURRENT_USER,reg_borg);
     save_reg_entries();
     return;
   }
}


