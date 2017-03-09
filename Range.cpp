/************************************************************************
*              range.cpp                                                *
* This range class started out as just that - a range class for pairs   *
* of lptr's, but ended up as a class for defining a block more than a   *
* range. The block is set by the use in Borg, and then the block can be *
* undefined, exported as txt/asm, decrypted, etc. All of this was added *
* in Borg 2.15                                                          *
************************************************************************/

#include <windows.h>

#include "range.h"
#include "disio.h"
#include "dasm.h"
#include "disasm.h"
#include "debug.h"

/************************************************************************
* constructor                                                           *
* - sets the top and bottom of the range to the null pointer            *
************************************************************************/
range::range()
{ top=nlptr;
  bottom=nlptr;
}

/************************************************************************
* destructor                                                            *
* - currently null                                                      *
************************************************************************/
range::~range()
{
}

/************************************************************************
* checkblock                                                            *
* - This just checks that the top and bottom of the block have been set *
*   and returns true if they have, otherwise puts up a messagebox       *
************************************************************************/
bool range::checkblock(void)
{ if(top==nlptr)
  { MessageBox(mainwindow,"Set top of block first","Borg Disassembler",MB_OK);
    return false;
  }
  if(bottom==nlptr)
  { MessageBox(mainwindow,"Set bottom of block first","Borg Disassembler",MB_OK);
    return false;
  }
  if(top>bottom)
  { MessageBox(mainwindow,"Block empty ?","Borg Disassembler",MB_OK);
    return false;
  }
  return true;
}

/************************************************************************
* undefine                                                              *
* - this undefines a block if the block has been set                    *
************************************************************************/
void range::undefine(void)
{ if(!checkblock())
    return;
  dsm.undefineblock(top,bottom);
}

/************************************************************************
* settop                                                                *
* - sets the top of the block to the current line                       *
************************************************************************/
void range::settop(void)
{ dio.findcurrentaddr(&top);
  MessageBox(mainwindow,"Top marked","Borg Disassembler",MB_OK);
}

/************************************************************************
* setbottom                                                             *
* - sets the bottom of the block to the current line                    *
************************************************************************/
void range::setbottom(void)
{ dio.findcurrentaddr(&bottom);
  MessageBox(mainwindow,"Bottom marked","Borg Disassembler",MB_OK);
}


