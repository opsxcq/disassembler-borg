//						disasm.h
//

#ifndef disasm_h
#define disasm_h

#include "list.h"
#include "common.h"
#include "proctab.h"
#include "stacks.h"
#include "disfuncs.h"

#include "disio.h"

class disasm: public slist <dsmitem *>
{ private:
	asmtable *itable;
	dsmitem *lastdis;
  public:
    int itables,jtables,irefs; // number of jump tables detected

  public:
	disasm();
	~disasm();
	void dissettable(void);
	void discomment(lptr loc,dsmitemtype typ,byte *comment);
    void addcomment(lptr loc,char *comment);
	void delcomment(lptr loc,dsmitemtype typ);
	void disblock(lptr loc);
	void disexportblock(lptr loc);
	void disxref(lptr loc);
	void disjumptable(lptr loc);
    void disstringtable(lptr loc,char *basename);
    void disdialog(lptr loc,char *basename);
    void disautocomment(lptr loc,dsmitemtype typ,byte *comment);
    int disname_or_ordinal(lptr loc,bool ctrl_comment);
	void disdataword(lptr loc);
    void disdatadword(lptr loc);
    void disdatasingle(lptr loc);
    void disdatadouble(lptr loc);
    void disdatalongdouble(lptr loc);
    void disdatadsoffword(lptr loc);
	void disdatastring(lptr loc);
	void disdatapstring(lptr loc);
	void disdataucstring(lptr loc);
	void disdataupstring(lptr loc);
	void disdatadosstring(lptr loc);
	void disdatageneralstring(lptr loc);
	void disargoverdec(lptr loc);
	void disargoversingle(lptr loc);
	void disargnegate(lptr loc);
	void disargoverhex(lptr loc);
	void disargoverchar(lptr loc);
	void disargoveroffsetdseg(lptr loc);
    void codeseek(lptr loc);
	bool oktoname(lptr loc);
	void undefineline(void);
	void undefinelines(void);
	void undefinelines_long(void);
    void undefineblock(lptr ufrom,lptr uto);
	int getlength(lptr loc);
	dsmitem *nextiter_code(dsmitem *tblock);
    // virtual functions from slist
    int compare(dsmitem *i,dsmitem *j);
    void delfunc(dsmitem *i);

  private:
	dsmitem *decodeinst(byte *ibuff,byte *mcode,lptr loc,byte tabtype,bool omode32,int depth);
	byte arglength(argtype a,byte modrmbyte,byte sibbyte,dword flgs,bool omode32);
	bool checkvalid(dsmitem *newdsm);
	bool interpretmod(byte *data,dword *toffs,byte *indexreg,byte *indexreg2,byte *indexamount,int *numjumps);
	void setcodeoverride(lptr loc,byteoverride typ);
	void initnewdsm(dsmitem *newdsm,lptr loc,dsmitemtype typ);
	void disdata(lptr loc,asminstdata *asmwd,byte len,byteoverride overr);
    bool dsmitem_contains_loc(dsmitem *d,lptr loc);
    dsmitem *dsmfindaddrtype(lptr loc,dsmitemtype typ);
};

extern disasm dsm;

#endif