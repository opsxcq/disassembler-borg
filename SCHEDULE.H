//					schedule.h
//

#ifndef schedule_h
#define schedule_h

#include "list.h"
#include "common.h"

enum tasktype {tasktype_null=0,dis_code=1,dis_dataword,dis_datadword,dis_datastring,nameloc,windowupdate,
  scrolling,user_makecode,user_undefineline,user_undefinelines,user_jumpto,user_jumptoarg2,user_jumptoaddr,
  user_makedword,user_makeword,user_makestring,user_pascalstring,dis_datapstring,namecurloc,
  dis_segheader,user_jumpback,dis_jumptable,user_ucstring,user_upstring,user_dosstring,
  user_generalstring,dis_dataucstring,dis_dataupstring,dis_datadosstring,dis_datageneralstring,
  dis_xref,dis_import,dis_ordimport,dis_export,dis_ordexport,hscroll,user_argoverdec,user_argoverhex,
  user_argoverchar,dis_argoverdec,dis_argoverhex,dis_argoverchar,quitborg,user_argoveroffsetdseg,
  dis_argoveroffsetdseg,dis_datadsoffword,seek_code,vthumbposition,hthumbposition,dis_exportcode,
  user_undefinelines_long,user_argnegate,user_marktopblock,user_markbottomblock,user_undefineblock,
  dis_stringtable,user_makesingle,user_makedouble,user_makelongdouble,user_argsingle,dis_dialog,
  user_addcomment,user_delcomment,user_repeatxrefview,user_repeatnameview,user_delxref};

enum priority {priority_null=0,priority_quit=1,priority_window,priority_userrequest,priority_import,priority_export,
  priority_xref,priority_continuation,priority_nameloc,priority_segheader,priority_definitecode,
  priority_data,priority_possiblecode,priority_aggressivesearch};

// our internal implementation of the taskitem.
// we have type and priority
// and for each we can also keep an addr, a text string and a number
struct taskitem
{ tasktype ttype;
  priority p;
  lptr addr;
  char *comment;
  dword tnum;
};

class schedule: private slist <taskitem *>
{ private:
	 volatile bool threadpause;
	 volatile bool threadstopped;

  public:
	 schedule();
	 ~schedule();
	 void addtask(tasktype ttype,priority p,lptr loc,char *comment);
    bool process(void);
	 dword sizelist(void);
	 void stopthread(void);
    void continuethread(void);
    // virtual functions from slist
    int compare(taskitem *i,taskitem *j);
    void delfunc(taskitem *i);
};

extern schedule scheduler;

#endif