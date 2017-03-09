//						proctab.h
//

#ifndef proctab_h
#define proctab_h

#include "common.h"

//processor types
#define PROC_8086     0x0001
#define PROC_80286    0x0002
#define PROC_80386    0x0004
#define PROC_80486    0x0008
#define PROC_PENTIUM  0x0010
#define PROC_PENTMMX  0x0020
#define PROC_PENTIUM2 0x0080
#define PROC_Z80      0x0100
#define PROC_PENTIUMPRO  0x0200
#define PROC_ALL      0xffff

//processor macros
#define PROC_FROMPENTIUM2 PROC_PENTIUM2
#define PROC_FROMPENTMMX PROC_PENTMMX|PROC_PENTIUM2
#define PROC_FROMPENTPRO PROC_PENTIUMPRO|PROC_FROMPENTMMX
#define PROC_FROMPENTIUM PROC_PENTIUM|PROC_FROMPENTPRO
#define PROC_FROM80486   PROC_80486|PROC_FROMPENTIUM
#define PROC_FROM80386   PROC_80386|PROC_FROM80486
#define PROC_FROM80286   PROC_80286|PROC_FROM80386
#define PROC_FROM8086    PROC_8086|PROC_FROM80286

//flags
#define FLAGS_MODRM      0x00001  //contains mod r/m byte
#define FLAGS_8BIT       0x00002  //force 8-bit arguments
#define FLAGS_16BIT      0x00004  //force 16-bit arguments
#define FLAGS_32BIT      0x00008  //force 32-bit arguments
#define FLAGS_REAL       0x00010  //real mode only
#define FLAGS_PMODE      0x00020  //protected mode only
#define FLAGS_PREFIX     0x00040  //for lock and rep prefix
#define FLAGS_MMX        0x00080  //mmx instruction/registers
#define FLAGS_FPU        0x00100  //fpu instruction/registers
#define FLAGS_CJMP       0x00200  //codeflow - conditional jump
#define FLAGS_JMP        0x00400  //codeflow - jump
#define FLAGS_IJMP       0x00800  //codeflow - indexed jump
#define FLAGS_CALL       0x01000  //codeflow - call
#define FLAGS_ICALL      0x02000  //codeflow - indexed call
#define FLAGS_RET        0x04000  //codeflow - return
#define FLAGS_SEGPREFIX  0x08000  //segment prefix
#define FLAGS_OPERPREFIX 0x10000  //operand prefix
#define FLAGS_ADDRPREFIX 0x20000  //address prefix
#define FLAGS_OMODE16    0x40000  //16-bit operand mode only
#define FLAGS_OMODE32    0x80000  //32-bit operand mode only

//Z80 flags
#define FLAGS_INDEXREG   0x00001

#define TABLE_MAIN 1
#define TABLE_EXT 2
#define TABLE_EXT2 3

enum argtype {ARG_REG=1,ARG_IMM,ARG_NONE,ARG_MODRM,ARG_REG_AX,ARG_REG_ES,ARG_REG_CS,
  ARG_REG_SS,ARG_REG_DS,ARG_REG_FS,ARG_REG_GS,ARG_REG_BX,ARG_REG_CX,ARG_REG_DX,
  ARG_REG_SP,ARG_REG_BP,ARG_REG_SI,ARG_REG_DI,ARG_IMM8,ARG_RELIMM8,ARG_FADDR,ARG_REG_AL,
  ARG_MEMLOC,ARG_SREG,ARG_RELIMM,ARG_16REG_DX,ARG_REG_CL,ARG_REG_DL,ARG_REG_BL,ARG_REG_AH,
  ARG_REG_CH,ARG_REG_DH,ARG_REG_BH,ARG_MODREG,ARG_CREG,ARG_DREG,ARG_TREG_67,ARG_TREG,
  ARG_MREG,ARG_MMXMODRM,ARG_MODRM8,ARG_IMM_1,ARG_MODRM_FPTR,ARG_MODRM_S,ARG_MODRMM512,
  ARG_MODRMQ,ARG_MODRM_SREAL,ARG_REG_ST0,ARG_FREG,ARG_MODRM_PTR,ARG_MODRM_WORD,ARG_MODRM_SINT,
  ARG_MODRM_EREAL,ARG_MODRM_DREAL,ARG_MODRM_WINT,ARG_MODRM_LINT,ARG_REG_BC,ARG_REG_DE,
  ARG_REG_HL,ARG_REG_DE_IND,ARG_REG_HL_IND,ARG_REG_BC_IND,ARG_REG_SP_IND,ARG_REG_A,
  ARG_REG_B,ARG_REG_C,ARG_REG_D,ARG_REG_E,ARG_REG_H,ARG_REG_L,ARG_IMM16,ARG_REG_AF,
  ARG_REG_AF2,ARG_MEMLOC16,ARG_IMM8_IND,ARG_BIT,ARG_REG_IX,ARG_REG_IX_IND,ARG_REG_IY,
  ARG_REG_IY_IND,ARG_REG_C_IND,ARG_REG_I,ARG_REG_R,ARG_IMM16_A,ARG_MODRM16,ARG_SIMM8,
  ARG_IMM32,ARG_STRING,ARG_MODRM_BCD,ARG_PSTRING,ARG_DOSSTRING,ARG_CUNICODESTRING,
  ARG_PUNICODESTRING,ARG_NONEBYTE,ARG_XREG,ARG_XMMMODRM,ARG_IMM_SINGLE,ARG_IMM_DOUBLE,
  ARG_IMM_LONGDOUBLE};

#pragma pack(push,pack_save,1)
struct asminstdata         //Asm Instructions data
{ char *name;              //eg nop,NULL=subtable/undefined
  byte instbyte;           //   0x90/subtable number
  word processor;          //   8086|386|486|pentium,etc bitwise flags
  dword flags;             //   mod r/m,8/16/32 bit
  argtype arg1,arg2,arg3;  //   argtypes=reg/none/immediate,etc
  dword uniqueid;          //   unique id for reconstructing saved databases
};

struct asmtable            //Assembly instruction tables
{ asminstdata *table;      //Pointer to table of instruction encodings
  byte type;               // type - main table/extension
  byte extnum,extnum2;     // bytes= first bytes of instruction
  byte divisor;            // number to divide by for look up
  byte mask;               // bit mask for look up
  byte minlim,maxlim;      // limits on min/max entries.
  byte modrmpos;           // modrm byte position plus
};
#pragma pack(pop,pack_save,1)

extern asmtable tables86[],tablesz80[];

struct proctable
{ dword num;
  char *name;
  asmtable *tab;
};

extern asminstdata asmdword[],asmword[],asmstr[],asm_fp[];
extern proctable procnames[];
extern char *reg32ascii[];
extern char *reg16ascii[];
extern char *reg8ascii[];
extern char *regind16ascii[];
extern char *regfascii[];
extern char *regmascii[];
extern char *regxascii[];
extern char *regsascii[];
extern char *regcascii[];
extern char *regdascii[];
extern char *regtascii[];
extern char *regzascii[];
extern asminstdata *reconstruct[];
#endif