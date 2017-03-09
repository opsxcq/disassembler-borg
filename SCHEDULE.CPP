/************************************************************************
*              schedule.cpp                                             *
* Main task scheduler for Borg. This class is central to the workings   *
* of the secondary thread, and to the calls which are passed back and   *
* forth to the secondary thread. When Borg decides it is going to do    *
* something it is added to a list of things to do, along with a         *
* priority for doing it. Like updating the main window is more          *
* important than naming a location. The scheduler then manages the      *
* queue of items to be processed, and calls each in turn. The queue can *
* be temporarily stopped for low priority items (lower than userrequest *
* priority), this is to enable many of the primary thread dialog boxes  *
* to work whilst maintaining window updates and changes                 *
* Accessing the task list should be covered by critical sections, since *
* either thread can access the list. The other place that critical      *
* sections are heavily used is in the display of the main window, and   *
* the buffer generation for the main window.                            *
************************************************************************/

#include <windows.h>

#include "schedule.h"
#include "xref.h"
#include "gname.h"
#include "disasm.h"
#include "data.h"
#include "dasm.h"
#include "mainwind.h"
#include "range.h"
#include "debug.h"

/************************************************************************
* constructor                                                           *
* - simply sets up the thread pause/stop request status                 *
************************************************************************/
schedule::schedule()
{ threadpause=false;
  threadstopped=true;
}

/************************************************************************
* destructor                                                            *
* - delete any tasks left in queue                                      *
************************************************************************/
schedule::~schedule()
{ taskitem *task;
  // need to delete any remaining tasks
  while((task=processqueue())!=NULL)
  { if(task->comment!=NULL)
      delete task->comment;
	delete task;
  } 
}

/************************************************************************
* addtask                                                               *
* - this adds a task to the queue. This function has to be very careful *
*   as it can potentially be called by either thread at any time, and   *
*   so could be called from both threads at almost the same time.       *
*   So there is a critical section for manipulating the queue of tasks. *
* - added tasktype tt and moved tnumt whilst commenting this function   *
*   Borg v2.21 for safety!                                              *
* - tasks now maintain comment memory v 2.28                            *
************************************************************************/
void schedule::addtask(tasktype ttype,priority p,lptr loc,char *comment)
{ taskitem *titem;
  static dword tnumt=0;
  tasktype tt;
  // limit window updates added to queue - could be called many times by disasm.
  if(ttype==windowupdate)
  { EnterCriticalSection(&cs);
	titem=peekfirst();
	if(titem!=NULL)
	  tt=titem->ttype;
    else
      tt=tasktype_null; // zero
	LeaveCriticalSection(&cs);
	if(tt==windowupdate)
      return;
  }
  titem=new taskitem;
  titem->ttype=ttype;
  titem->p=p;
  titem->addr=loc;
  if(comment!=NULL)
  { titem->comment=new char[strlen(comment)+1];
    strcpy(titem->comment,comment);
  }
  else
    titem->comment=NULL;
  EnterCriticalSection(&cs);
  titem->tnum=tnumt;
  tnumt++;
  addto(titem);
  LeaveCriticalSection(&cs);
}

/************************************************************************
* compare function                                                      *
* - the task list is stored in priority order, and then using a uid     *
*   generated during the add function                                   *
************************************************************************/
int schedule::compare(taskitem *i,taskitem *j)
{ if(i->p==j->p)
  { if(i->tnum==j->tnum)
      return 0;
	 if(i->tnum>j->tnum)
		return 1;
	 return -1;
  }
  if(i->p>j->p)
	 return 1;
  return -1;
}

/************************************************************************
* delete function                                                       *
* - overrides the standard listitem deletion function                   *
************************************************************************/
void schedule::delfunc(taskitem *i)
{ if(i->comment!=NULL)
    delete i->comment;
  delete i;
}

/************************************************************************
* process                                                               *
* - here we take the item at the front of the task queue and process it *
* - if the queue has been requested to hold then we only process high   *
*   priority items                                                      *
* - more thread safety code added v 2.21 during commenting and closer   *
*   examination                                                         *
************************************************************************/
bool schedule::process(void)
{ taskitem *task;
  bool done;
  bool procdany;
  priority q;
  done=false;
  procdany=false;
  threadstopped=false;
  do
  { // our checks for pausing the threads must be thread safe
    // accessing the task list must be in a critical section
    EnterCriticalSection(&cs);
	 task=peekfirst();
    if(task!=NULL)
      q=task->p;
    else
      q=priority_null; // zero
	 LeaveCriticalSection(&cs);
    if(q>priority_userrequest)
	 { while(threadpause)
	   { threadstopped=true;
	 	  EnterCriticalSection(&cs);
		  task=peekfirst();
		  if(task!=NULL)
          q=task->p;
        else
          q=priority_null; // zero
		  LeaveCriticalSection(&cs);
		  if(q<priority_userrequest)
		  	  break;
		  Sleep(0);
      }
    }
	 threadstopped=false;
	 EnterCriticalSection(&cs);
    // note: processqueue detaches the task from the list and gives it to us
	 task=processqueue();
	 LeaveCriticalSection(&cs);
	 if(task==NULL)
      done=true;
	 else
	 { StatusMessageNItems(numlistitems());
		procdany=true;
		switch(task->ttype)
		{ case dis_code:
#ifdef DEBUG
			 DebugMessage("Scheduler:Disassembling Code at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disblock(task->addr);
			 break;
		  case dis_exportcode:
#ifdef DEBUG
			 DebugMessage("Scheduler:Disassembling Export Code at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disexportblock(task->addr);
			 break;
		  case user_makecode:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make Code");
#endif
			 dio.makecode();
			 break;
		  case user_undefineline:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Undefine Line");
#endif
			 dsm.undefineline();
			 break;
		  case user_undefinelines:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Undefine Lines");
#endif
			 dsm.undefinelines();
			 break;
		  case user_undefinelines_long:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Undefine Lines Long");
#endif
			 dsm.undefinelines_long();
			 break;
		  case user_jumpback:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Jump Back");
#endif
			 dio.jumpback();
			 break;
		  case user_jumpto:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Jump To");
#endif
			 dio.jumpto(true);
			 break;
		  case user_jumptoarg2:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Jump To (arg2)");
#endif
			 dio.jumpto(false);
			 break;
		  case user_jumptoaddr:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Jump To Addr");
#endif
			 dio.savecuraddr();
			 dio.setcuraddr(task->addr);
			 break;
		  case windowupdate:
			 dio.updatewindow();
			 break;
		  case scrolling:
			 dio.scroller(task->addr.offs);
			 break;
        case vthumbposition:
        	 dio.vertsetpos(task->addr.offs);
          break;
        case hthumbposition:
        	 horizscrollto(task->addr.offs);
          break;
		  case hscroll:                 // amount
			 horizscroll(task->addr.offs);
			 break;
		  case user_makedword:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make dword");
#endif
			 dio.makedword();
			 break;
		  case user_makesingle:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make single real");
#endif
			 dio.makesingle();
			 break;
		  case user_makedouble:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make double real");
#endif
			 dio.makedouble();
			 break;
		  case user_makelongdouble:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make long double real");
#endif
			 dio.makelongdouble();
			 break;
		  case user_makeword:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make Word");
#endif
			 dio.makeword();
			 break;
		  case user_makestring:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Make String");
#endif
			 dio.makestring();
			 break;
		  case user_pascalstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Pascal String");
#endif
			 dio.pascalstring();
			 break;
		  case user_ucstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Unicode C String");
#endif
			 dio.ucstring();
			 break;
		  case user_upstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Unicode Pascal String");
#endif
			 dio.upstring();
			 break;
		  case user_dosstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:User DOS String");
#endif
			 dio.dosstring();
			 break;
		  case user_generalstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:User General String");
#endif
			 dio.generalstring();
			 break;
		  case user_argoverdec:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Argument Override Decimal");
#endif
			 dio.argoverdec();
			 break;
		  case user_argsingle:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Argument Override Single Real");
#endif
			 dio.argoversingle();
			 break;
		  case user_argoverhex:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Argument Override Hex");
#endif
			 dio.argoverhex();
			 break;
		  case user_argoverchar:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Argument Override Char");
#endif
			 dio.argoverchar();
			 break;
		  case user_argoveroffsetdseg:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Argument Override Offset Dseg");
#endif
			 dio.argoveroffsetdseg();
			 break;
        case user_argnegate:
#ifdef DEBUG
			 DebugMessage("Scheduler:User Argument Negate");
#endif
			 dio.arg_negate();
			 break;
		  case dis_dialog:
#ifdef DEBUG
			 DebugMessage("Scheduler:Decoding Dialog at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdialog(task->addr,task->comment);
			 break;
		  case dis_stringtable:
#ifdef DEBUG
			 DebugMessage("Scheduler:Decoding Stringtable at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disstringtable(task->addr,task->comment);
			 break;
		  case dis_dataword:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making word at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdataword(task->addr);
			 break;
		  case dis_datadword:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making dword at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdatadword(task->addr);
			 break;
		  case dis_datadsoffword:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making dword at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdatadsoffword(task->addr);
			 break;
		  case dis_datastring:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making string at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdatastring(task->addr);
			 break;
		  case dis_datapstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making Pascal string at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdatapstring(task->addr);
			 break;
		  case dis_dataucstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making unicode c string at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdataucstring(task->addr);
			 break;
		  case dis_dataupstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making unicode pascal string at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdataupstring(task->addr);
			 break;
		  case dis_datadosstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making DOS string at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdatadosstring(task->addr);
			 break;
		  case dis_datageneralstring:
#ifdef DEBUG
			 DebugMessage("Scheduler:Making general string at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disdatageneralstring(task->addr);
			 break;
		  case dis_argoverdec:
#ifdef DEBUG
			 DebugMessage("Scheduler:Argument Override Decimal at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disargoverdec(task->addr);
			 break;
		  case dis_argoverhex:
#ifdef DEBUG
			 DebugMessage("Scheduler:Argument Override Hex at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disargoverhex(task->addr);
			 break;
		  case dis_argoverchar:
#ifdef DEBUG
			 DebugMessage("Scheduler:Argument Override Char at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disargoverchar(task->addr);
			 break;
		  case dis_argoveroffsetdseg:
#ifdef DEBUG
			 DebugMessage("Scheduler:Argument Override Offset Dseg at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disargoveroffsetdseg(task->addr);
			 break;
		  case nameloc:
#ifdef DEBUG
			 DebugMessage("Scheduler:Naming : %04lx:%04lx to %s",task->addr.segm,task->addr.offs,task->comment);
#endif
			 name.addname(task->addr,task->comment);
			 break;
		  case dis_xref:
#ifdef DEBUG
			 DebugMessage("Scheduler:Adding Xref at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disxref(task->addr);
			 break;
		  case namecurloc:
#ifdef DEBUG
			 DebugMessage("Scheduler:Naming : %04lx:%04lx to %s",task->addr.segm,task->addr.offs,task->comment);
#endif
			 name.addname(task->addr,task->comment);
			 break;
		  case dis_segheader:
#ifdef DEBUG
			 DebugMessage("Scheduler:Segheader at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dta.segheader(task->addr);
			 break;
		  case dis_jumptable:
#ifdef DEBUG
			 DebugMessage("Scheduler:Disassembling Jumptable at : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
			 dsm.disjumptable(task->addr);
			 break;
		  case dis_ordimport:
#ifdef DEBUG
			 DebugMessage("Scheduler:Import : %04lx:%04lx to %s",task->addr.segm,task->addr.offs,task->comment);
#endif
			 import.addname(task->addr,task->comment);
			 break;
		  case dis_import:
#ifdef DEBUG
			 DebugMessage("Scheduler:Import : %04lx:%04lx to %s",task->addr.segm,task->addr.offs,task->comment);
#endif
			 import.addname(task->addr,task->comment);
			 break;
		  case dis_ordexport:
#ifdef DEBUG
			 DebugMessage("Scheduler:Export : %04lx:%04lx to %s",task->addr.segm,task->addr.offs,task->comment);
#endif
			 expt.addname(task->addr,task->comment);
			 break;
		  case dis_export:
#ifdef DEBUG
			 DebugMessage("Scheduler:Export : %04lx:%04lx to %s",task->addr.segm,task->addr.offs,task->comment);
#endif
			 expt.addname(task->addr,task->comment);
			 break;
        case seek_code:
#ifdef DEBUG
			 DebugMessage("Scheduler:Seek_Code : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
          dsm.codeseek(task->addr);
          break;
        case user_delcomment:
#ifdef DEBUG
			 DebugMessage("Scheduler:Delete_Comment : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
		    dsm.delcomment(task->addr,dsmcomment);
          break;
        case user_addcomment:
#ifdef DEBUG
			 DebugMessage("Scheduler:Add_Comment : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
          dsm.discomment(task->addr,dsmcomment,(unsigned char *)task->comment);
          break;
        case user_delxref:
#ifdef DEBUG
			 DebugMessage("Scheduler:User_Del_Xref : %04lx:%04lx",task->addr.segm,task->addr.offs);
#endif
          xrefs.userdel(task->addr);
          break;
        case user_repeatxrefview:
          PostMessage(mainwindow,WM_REPEATXREFVIEW,(WPARAM)0,(LPARAM)0);
          break;
        case user_repeatnameview:
          PostMessage(mainwindow,WM_REPEATNAMEVIEW,(WPARAM)0,(LPARAM)0);
          break;
		case user_undefineblock:
          blk.undefine();
          break;
        case user_marktopblock:
          blk.settop();
          break;
        case user_markbottomblock:
          blk.setbottom();
          break;
		case quitborg:
          threadstopped=true;
          return true;
		default:
			 break;
		}
      // done with it, now delete it
	    if(task->comment!=NULL)
		  delete task->comment;
		delete task;
	 }
	 if(KillThread)
      return true;
  } while(!done);
  // it is important to mark the thread as stopped if we leave here
  // so that the main thread can know if it is safe to quit
  threadstopped=true;
  return procdany;
}

/************************************************************************
* sizelist                                                              *
* - this simply returns number of tasks left to process                 *
************************************************************************/
dword schedule::sizelist(void)
{ return numlistitems();
}

/************************************************************************
* stopthread                                                            *
* - pausing and continuing the secondary thread has not been without    *
*   its headaches in early versions. Here I simply set threadpause and  *
*   wait until threadstopped is set to true. Having finally sorted out  *
*   most thread issues and critical sections, etc I have more or less   *
*   ended up with the simplest code back in here. Many problems have    *
*   been due to clashes of critical section code, and pausing at the    *
*   wrong time, or exitting and waiting for a thread to finish when it  *
*   was waiting for a critical section, etc. Anyway, hopefully these    *
*   issues are now resolved and this simple code will endure for a bit  *
* - further simplified whilst commenting and analysing, Borg v2.21      *
* - Note that threadpause and threadstopped are both declared as        *
*   volatile variables.                                                 *
************************************************************************/
void schedule::stopthread(void)
{ threadpause=true;
  while(!threadstopped)
    Sleep(0);
}

/************************************************************************
* continuethread                                                        *
* - simply sets threadpause to false again                              *
* - we dont need to wait around for verification that it has continued  *
************************************************************************/
void schedule::continuethread(void)
{ threadpause=false;
}

