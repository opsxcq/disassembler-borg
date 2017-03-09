/************************************************************************
*                   debug.cpp                                           *
* NB change the path and name of the debugfile in debug.h before using  *
* the debug functions. To enable debug functions change the commented   *
* line in the debug.f header file. Warning - debug files can get very   *
* big quite quickly. I only use them for hard to finc bugs.             *
* One alternative is just to call the DebugMessage function at any      *
* point without using #ifdef DEBUG.... and then you'll get a smaller    *
* file referencing the area you are interested in. (Or just use         *
* Messageboxes for real small bugfinding.... Anyway, the functions are  *
* there should they be required. Eugen Polukhin had suggested using a   *
* second screen for the debug messages, with the ability to switch      *
* between the two with a keypress. It looked good, but in reality it's  *
* not as useful because most bugs I would use this for are fatal...     *
************************************************************************/

#include <windows.h>
#include "debug.h"
#include "dasm.h"

/************************************************************************
* DebugMessage                                                          *
* - this is the only function. It opens the debug file if it isnt open  *
*   already - see the debug.h file for the full name of it, as I have   *
*   it set. If you use this function then you'll need to change it.     *
* - DebugMessages are appended to the debug file.                       *
* - The function can be used as wvsprintf is used, with any number of   *
*   arguments. See the scheduler for many examples of its use           *
************************************************************************/
void DebugMessage(char *szFormat,...)
{ char DebugBuff[200];
  SECURITY_ATTRIBUTES securityatt;
  HANDLE efile;
  dword num;
  va_list vaArgs;
  va_start(vaArgs,szFormat);
  wvsprintf(DebugBuff,szFormat,vaArgs);
  va_end(vaArgs);
  securityatt.nLength=sizeof(SECURITY_ATTRIBUTES);
  securityatt.lpSecurityDescriptor=NULL;
  securityatt.bInheritHandle=false;
  efile=CreateFile(DEBUGFILE,GENERIC_WRITE,0,&securityatt,OPEN_ALWAYS,0,NULL);
  if(efile==INVALID_HANDLE_VALUE)
  { MessageBox(mainwindow,"Debug File Creation Failed","Borg Disassembler Alert",MB_OK);
	 return;
  }
  SetFilePointer(efile,0,0,FILE_END);
  WriteFile(efile,DebugBuff,strlen(DebugBuff),&num,NULL);
  WriteFile(efile,"\r\n",2,&num,NULL);
  CloseHandle(efile);
}

/************************************************************************
* ShowLastError                                                         *
* - Can be called to show windows error messages, decoded               *
************************************************************************/
void ShowLastError(void)
{ char *lpMsgBuf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
  );
  MessageBox(NULL,lpMsgBuf,"Borg Error Message",MB_OK);
}
