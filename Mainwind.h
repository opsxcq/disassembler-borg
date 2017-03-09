//					mainwind.h

#ifndef mainwind_h
#define mainwind_h

#include "common.h"

#define buffer_lines 60
#define max_length 200
#define max_stringprint (max_length-60)

void ClearBuff(void);
void DoneBuff(void);
void PrintBuff(char *szFormat,...);
void LastPrintBuffEpos(UINT xpos);
void LastPrintBuff(char *szFormat,...);
void LastPrintBuffHexValue(byte num);
void LastPrintBuffLongHexValue(dword num);
void DoPaint(HWND hWnd,int cxChar,int cyChar);
void PaintBack(HWND hWnd);
void DumpBuff(HANDLE efile);
void horizscroll(int amount);
void horizscrollto(int place);

extern UINT nScreenRows;
extern UINT usersel;  // user selection. (line number).
extern unsigned int rrr,cyc;
extern bool userselonscreen;

#endif