/************************************************************************
*                 mainwind.cpp                                          *
* This is one of a few files which contain some old code, a lot of      *
* hacks, a lot of hastily stuffed lines and variables, etc, and is in   *
* in need of an overhaul. Its also why I left commenting it until the   *
* last four files (disasm,fileload,dasm and this one!). Whilst making   *
* these comments I have taken a small opportunity to tidy it a little.  *
* This file controls output to the main window. It keeps a small buffer *
* of disassembled lines, and it uses this buffer to repaint the main    *
* window when WM_PAINT requests are received.                           *
* Whenever the disassembly is changed or the user scrolls there is a    *
* windowupdate request sent to the scheduler with a high priority.      *
* When the windowupdate is processed it is passed to the windowupdate   *
* or scroller function in disio. disio will regenerate the buffer as    *
* required and then invalidate the main window so that a repaint is     *
* done.                                                                 *
* The functions here include basic formatted output to the buffer as    *
* well as the window repaints. Repaints will not be done while the      *
* buffer is being updated, but will wait for the buffer update to       *
* finish and then do the repaint.                                       *
* The functions also use extensive critical sections which ensure that  *
* we do not start clearing the buffer during a window update, etc       *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "mainwind.h"
#include "dasm.h"
#include "debug.h"

/************************************************************************
* global variables                                                      *
* - the MainBuff is the buffered output used for repainting             *
************************************************************************/
UINT lastline=0;
volatile bool bufferready=true;
char MainBuff[buffer_lines*max_length+1000]; // lets hope it doesn't overflow
                                             // wvsprintf sucks!
UINT nScreenRows;
UINT usersel;  // the line which is the current user selection. (line number).
unsigned int rrr=0,cyc;
int hpos=0;
bool userselonscreen=false;

/************************************************************************
* horizscroll                                                           *
* - hpos keeps track of the horizontal scroll which determines where    *
*   from each buffer line we start printing the output                  *
************************************************************************/
void horizscroll(int amount)
{ EnterCriticalSection(&cs);
  hpos+=amount;
  if(hpos<0)
    hpos=0;  // max-size checked in dopaint.
  LeaveCriticalSection(&cs);
  InvalidateRect(mainwindow,NULL,true);
}

/************************************************************************
* horizscrollto                                                         *
* - this is used when the horizontal scrollbar control is dragged and   *
*   dropped to change the hpos offset to the new place (maximum         *
*   horizontal size is fixed in Borg)                                   *
************************************************************************/
void horizscrollto(int place)
{ EnterCriticalSection(&cs);
  hpos=place;
  LeaveCriticalSection(&cs);
  InvalidateRect(mainwindow,NULL,true);
}

/************************************************************************
* ClearBuff                                                             *
* - This should be called before each reworking of the buffer. It       *
*   clears the buffer and stops any repainting from taking place.       *
* - It also resets the line pointer to the start of the buffer.         *
************************************************************************/
void ClearBuff(void)
{ int i;
  EnterCriticalSection(&cs);
  for(i=0;i<buffer_lines;i++)
  { MainBuff[i*max_length]=0;
  }
  lastline=0;
  bufferready=false;
  LeaveCriticalSection(&cs);
}

/************************************************************************
* DoneBuff                                                              *
* - This should be called after a reworking of the buffer. It reenables *
*   window repainting.                                                  *
************************************************************************/
void DoneBuff(void)
{ EnterCriticalSection(&cs);
  bufferready=true;
  LeaveCriticalSection(&cs);
}

/************************************************************************
* PrintBuff                                                             *
* - This is the printf of the buffer and is similar to wvsprintf but    *
*   output is to the main buffer. Note that the line pointer is moved   *
*   on after a call, so we move to the next line automatically.         *
************************************************************************/
// adds next line to the buffer.
void PrintBuff(char *szFormat,...)
{ EnterCriticalSection(&cs);
  va_list vaArgs;
  va_start(vaArgs,szFormat);
  if(lastline<buffer_lines-1)
  { wvsprintf(&MainBuff[lastline*max_length],szFormat,vaArgs);
  }
  va_end(vaArgs);
  if(lastline<buffer_lines-1)
  { lastline++;
  }
  // just zero end of buffer.....in case.
  MainBuff[buffer_lines*max_length]=0;
  LeaveCriticalSection(&cs);
}

/************************************************************************
* LastPrintBuffEpos                                                     *
* - Often we use PrintBuff followed by LastPrintBuff to construct a     *
*   line of output a piece at a time. This function provides basic      *
*   formatting by allowing us to set the cursor position on the last    *
*   line printed, by adding spaces until the position.                  *
************************************************************************/
// adds blanks to previous line to set position
void LastPrintBuffEpos(UINT xpos)
{ UINT i;
  int spos;
  EnterCriticalSection(&cs);
  if(lastline)
  { spos=(lastline-1)*max_length;
	 i=strlen(&MainBuff[spos]);
	 while(i<xpos)
	 { MainBuff[spos+i]=' ';
		MainBuff[spos+i+1]=0;
		i++;
	 }
  }
  LeaveCriticalSection(&cs);
}

/************************************************************************
* LastPrintBuffHexValue                                                 *
* - Same as LastPrintBuff, but prints num only, in hex. It prints a     *
*   leading zero where the leading char is alpha.                       *
************************************************************************/
void LastPrintBuffHexValue(byte num)
{ char tstr[20];
  sprintf(tstr,"%02xh",num);
  if((tstr[0]>='a')&&(tstr[0]<='f'))
    LastPrintBuff("0");
  LastPrintBuff("%02xh",num);
}

/************************************************************************
* LastPrintBuffHexValue                                                 *
* - Same as LastPrintBuff, but prints num only, in hex. It prints a     *
*   leading zero where the leading char is alpha.                       *
************************************************************************/
void LastPrintBuffLongHexValue(dword num)
{ char tstr[20];
  sprintf(tstr,"%02lxh",num);
  if((tstr[0]>='a')&&(tstr[0]<='f'))
    LastPrintBuff("0");
  LastPrintBuff("%02lxh",num);
}

/************************************************************************
* LastPrintBuff                                                         *
* - This is the same as PrintBuff except that instead of printing a new *
*   line it goes back to the last line and adds more to the end of it   *
* - So a set of calls tends to look like PrintBuff, LastPrintBuffEPos,  *
*   LastPrintBuff, LastPrintBuffEPos, LastPrintBuff, PrintBuff, etc     *
************************************************************************/
void LastPrintBuff(char *szFormat,...)
{ int spos;
  EnterCriticalSection(&cs);
  va_list vaArgs;
  va_start(vaArgs,szFormat); 
  if(lastline)
  { spos=(lastline-1)*max_length;
    if(strlen(&MainBuff[spos])<max_length)
	   wvsprintf(&MainBuff[spos+strlen(&MainBuff[spos])],szFormat,vaArgs);
  }
  MainBuff[buffer_lines*max_length]=0;
  va_end(vaArgs);
  LeaveCriticalSection(&cs);
}

/************************************************************************
* DumpBuff                                                              *
* - Now this is a real hack. Instead of writing proper file IO routines *
*   I simply write a buffer full at a time, and dump each to a file.... *
*   Then when I've written the file out I regenerate the buffer for the *
*   display again....... to be rewritten......                          *
************************************************************************/
void DumpBuff(HANDLE efile)
{ dword num;
  unsigned int i;
  for(i=0;i<lastline;i++)
  { WriteFile(efile,&MainBuff[i*max_length],lstrlen(&MainBuff[i*max_length]),&num,NULL);
	 WriteFile(efile,"\r\n",2,&num,NULL);
  }
}

/************************************************************************
* DoPaint                                                               *
* - This is the main painting routine. If the program is quitting then  *
*   it returns (we dont want thread clashes due to critical sections    *
*   here and theres no point to repainting when we're exitting).        *
* - If the buffer is not ready then we wait, and go to sleep.           *
* - Otherwise the routine paints the screen from the buffer, using the  *
*   selected font and colours                                           *
************************************************************************/
void DoPaint(HWND hWnd,int cxChar,int cyChar)
{ HDC         hDC;   // handle for the display device
  PAINTSTRUCT ps;    // holds PAINT information
  UINT        nI;
  RECT rRect;
  int startpt,sn;
  char sl[300];

  cyc=cyChar;
  while(!bufferready)
    Sleep(0);                   // wait if filling buffer
  if(KillThread)
    return;
  if(!lastline)
  { PaintBack(hWnd);
  	 return;
  }
  EnterCriticalSection(&cs);
  memset(&ps, 0x00, sizeof(PAINTSTRUCT));
  hDC = BeginPaint(hWnd, &ps);
  // Included as the background is not a pure color
  //SetBkMode(hDC,TRANSPARENT);
  switch(options.font)
  { case courierfont:
  	 case courierfont10:
  	 case courierfont12:
  		if(cf==NULL)
        SelectObject(hDC,GetStockObject(ANSI_FIXED_FONT));
      else
        SelectObject(hDC,cf);
      break;
    case ansifont:
  		SelectObject(hDC,GetStockObject(ANSI_FIXED_FONT));
      break;
    case systemfont:
  		SelectObject(hDC,GetStockObject(SYSTEM_FIXED_FONT));
      break;
    default:
  		SelectObject(hDC,GetStockObject(ANSI_FIXED_FONT));
      break;
  }
  GetClientRect(hWnd,&rRect);
  if(hpos>max_length-(rRect.right/cxChar))
    hpos=max_length-rRect.right/cxChar;
  if(rrr!=(unsigned int)rRect.right)
  { rrr=(unsigned int)rRect.right;
	 SetScrollRange(hWnd,SB_HORZ,0,max_length-rRect.right/cxChar,true);
  }
  SetScrollPos(hWnd,SB_HORZ,hpos,true);
  nScreenRows = rRect.bottom/cyChar;
  ShowScrollBar (hWnd, SB_VERT, true);
  ShowScrollBar (hWnd, SB_HORZ, max_length>(rRect.right/cxChar));
  startpt=0;
  SetTextColor(hDC,options.textcolor);
  for(nI=startpt;nI<lastline;nI++)
  { if((userselonscreen)&&(nI==usersel))
      SetBkColor(hDC,options.highcolor);
  	 else
      SetBkColor(hDC,options.bgcolor);
    strcpy(sl,&MainBuff[nI*max_length]);
    sn=strlen(sl);
	if(sn<max_length)
      memset(&sl[sn],' ',max_length-sn);
    sl[max_length]=0;
  	 TabbedTextOut(hDC,2-hpos*cxChar,nI*cyChar,sl,
		max_length,0,NULL,2-hpos*cxChar);
  }
  for(nI=lastline;nI<nScreenRows+1;nI++)
  { if((userselonscreen)&&(nI==usersel))
      SetBkColor(hDC,options.highcolor);
  	 else
      SetBkColor(hDC,options.bgcolor);
    memset(sl,' ',max_length);
    sl[max_length]=0;
  	 TabbedTextOut(hDC,2-hpos*cxChar,nI*cyChar,sl,
		max_length,0,NULL,2-hpos*cxChar);
  }
  // Inform Windows painting is complete
  EndPaint(hWnd,&ps);
  LeaveCriticalSection(&cs);
}

/************************************************************************
* PaintBack                                                             *
* - This is the routine for painting when there is no file loaded. It   *
*   simply paints the background in our selected colour.                *
* - Yet another quick hack, from the above routine.... looks ok though  *
************************************************************************/
void PaintBack(HWND hWnd)
{ HDC         hDC;   // handle for the display device
  PAINTSTRUCT ps;    // holds PAINT information
  RECT rRect;

  EnterCriticalSection(&cs);
  memset(&ps, 0x00, sizeof(PAINTSTRUCT));
  hDC = BeginPaint(hWnd, &ps);
  GetClientRect(hWnd,&rRect);
  ShowScrollBar (hWnd, SB_VERT, true);
  FillRect(hDC, &rRect, (HBRUSH) CreateSolidBrush(options.bgcolor));
  EndPaint(hWnd,&ps);
  LeaveCriticalSection(&cs);
}
