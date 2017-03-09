//						disfuncs.h
//
// common disasm funcs for the disasm and disio classes
// to resolve mutual inclusion problems and forward declarations of
// class functions during splitting of classes :(

#ifndef disfuncs_h
#define disfuncs_h

#define BYTEPOS 14
#define ASMPOS 31
#define ARGPOS 38
#define COMMENTPOS 50
#define COMMENT_MAXLEN 100

#define CALLSTACKSIZE 100

enum dsmitemtype {dsmnull=0,dsmsegheader,dsmxref=5,dsmcomment,dsmnameloc,dsmcode};
enum byteoverride {over_null=0,over_decimal,over_char,over_dsoffset,over_single};

struct dsmitem
{ // these items should be dword size
  lptr addr;
  void *tptr;                     // string pointer for comment
								  // asminstdata ptr for code
  dword flags;
  byte *data;                     // data ptr

  // these items should be word size
  word length;

  // and these items should be just bytesize for efficiency
  dsmitemtype type;
  byte modrm;              // length, offset to  modrm byte
  bool mode32;
  byteoverride override;
  byte displayflags;
};

#pragma pack(push,pack_save,1)
struct dsmitemsave
{ lptr addr;
  dword tptroffset;
  dsmitemtype type;
  word length;
  byte modrm;
  bool mode32;
  dword fileoffset;
  byteoverride override;
  dword flags;
  byte displayflags;
};
#pragma pack(pop,pack_save)

#define DSMITEM_ARG1(x) (((asminstdata *)x->tptr)->arg1)
#define DSMITEM_ARG2(x) (((asminstdata *)x->tptr)->arg2)
#define DSMITEM_ARG3(x) (((asminstdata *)x->tptr)->arg3)
#define DSMITEM_NAME(x) (((asminstdata *)x->tptr)->name)

#endif
