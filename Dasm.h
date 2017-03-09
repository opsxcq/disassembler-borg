//					DASM.H

#ifndef dasmh
#define dasmh

#include "common.h"

#define WM_MAXITOUT (WM_USER+100)
#define WM_REPEATNAMEVIEW (WM_USER+101)
#define WM_REPEATXREFVIEW (WM_USER+102)

#define BORG_VER 228

#define CD_PUSHBP  1
#define CD_ENTER   2
#define CD_MOVBX   4
#define CD_AGGRESSIVE 8
#define CD_EAXFROMESP 16
#define CD_MOVEAX 32
#define VERTSCROLLRANGE 16000

enum fontselection {ansifont=1,systemfont,courierfont,courierfont10,courierfont12};

// nb changing this options structure will change the file database structure
struct globaloptions
{ bool loaddebug;
  bool mode16,mode32;
  bool loaddata,loadresources;
  bool demangle;
  bool cfa;
  dword processor;
  lptr loadaddr,oep;
  word dseg;
  word codedetect;
  COLORREF bgcolor,textcolor,highcolor;
  fontselection font;
  bool readonly;                           // file readonly
  bool winmax;                             // Borg window is maximised
};

void StatusMessageNItems(dword nolistitems);
void changemenus(void);
void StatusMessage(char *msg);
void Thread(PVOID pvoid);

extern CRITICAL_SECTION cs;
extern HINSTANCE hInst;
extern HWND mainwindow;
extern globaloptions options;
extern RECT mainwnd;
extern volatile bool KillThread;
extern char winname[];
extern HFONT cf;
extern HANDLE ThreadHandle;
extern DWORD ThreadId;
extern volatile bool InThread;
extern char program_name[];
extern char current_exe_name[];

#define Status_SetText(hwnd, iPart, uType, szText) \
	 PostMessage((hwnd), SB_SETTEXT, (WPARAM) (iPart | uType), (LPARAM) (LPSTR) szText)

#endif