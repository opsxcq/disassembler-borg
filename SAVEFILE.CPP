/************************************************************************
*                   savefile.cpp                                        *
* File I/O for saving to and reading from database files only.          *
* Added in version 2.11. Compression is implemented based on rle, but   *
* it is nibble based (2*nibbles=byte) rather than byte based. I decided *
* on this after closer examination of the database files. It results in *
* around a 30% reduction to the database file size.                     *
************************************************************************/

#include <windows.h>

#include "savefile.h"
#include "dasm.h"
#include "debug.h"

/************************************************************************
* Constructor - sets up some variables used in the compression and save *
************************************************************************/
savefile::savefile()
{	sfile=INVALID_HANDLE_VALUE;
	rbufflen=0;
   rbuffptr=0;
   rbhigh=true;
   rlecount=0;
   rlemode=false;
   rlestart=true;
}

/************************************************************************
* Destructor - closes the database file if it is still open             *
************************************************************************/
savefile::~savefile()
{  sclose();
}

/************************************************************************
* sopen                                                                 *
* - opens the database file and returns true on success                 *
************************************************************************/
bool savefile::sopen(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,
			DWORD dwCreationDistribution,DWORD dwFlagsAndAttributes)
{	sfile=CreateFile(lpFileName,dwDesiredAccess,dwShareMode,NULL,
				dwCreationDistribution,dwFlagsAndAttributes,NULL);
   if(sfile==INVALID_HANDLE_VALUE)
   { MessageBox(mainwindow,"File open failed ?",lpFileName,MB_OK|MB_ICONEXCLAMATION);
	  return false;
   }
   if(GetFileType(sfile)!=FILE_TYPE_DISK)
   { MessageBox(mainwindow,"File open failed ?",lpFileName,MB_OK|MB_ICONEXCLAMATION);
	  sclose();
	  return false;
   }
	return true;
}

/************************************************************************
* sclose                                                                *
* - closes the database file if still open                              *
************************************************************************/
void savefile::sclose(void)
{ if(sfile!=INVALID_HANDLE_VALUE)
    CloseHandle(sfile);
}

/************************************************************************
* getnibble                                                             *
* - This function sets n to the next nibble from the file, it uses      *
*   buffering and reads from the file as required                       *
************************************************************************/
bool savefile::getnibble(byte *n)
{  BOOL rval;
	if(rbuffptr<rbufflen)
	{	if(rbhigh)
   	{	(*n)=(byte)(rbuff[rbuffptr]>>4);
      	rbhigh=false;
      	return true;
      }
      (*n)=(byte)(rbuff[rbuffptr]&0x0f);
      rbhigh=true;
      rbuffptr++;
      return true;
   }
   else
   {  rval=ReadFile(sfile,rbuff,RBUFF_MAXLEN,&rbufflen,NULL);
   	  rbhigh=true;
      rbuffptr=0;
      if((rval)&&(rbufflen))
        return getnibble(n);
      return false;
   }
}

/************************************************************************
* getrlenibble                                                          *
* - this function sets n to the next nibble from the file, taking into  *
*   account rle encoding. So this returns the next uncompressed nibble  *
* - rle encoding is:                                                    *
*   rle_code count nibble                                               *
*   count is number-1 (so it can encode from 2 to 16 nibbles)           *
*   or rle_code 0 signifies a nibble equal to rle_code                  *
* - note that rle_code is a constant specified in the savefile.h which  *
*   indicates an rle encoding, and is currently 0x0f. Do not set this   *
*   constant to 0 as this would be inefficient....                      *
************************************************************************/
bool savefile::getrlenibble(byte *n)
{	if(rlemode)
	{	rlecount--;
   	if(!rlecount)
         rlemode=false;
      (*n)=(byte)(rlebyte);
      return true;
   }
   if(!getnibble(&rlebyte))
      return false;
   if(rlebyte==rle_code)
   {	if(!getnibble(&rlebyte))
         return false;
   	if(rlebyte)
      {	rlecount=rlebyte;
      	rlemode=true;
         if(!getnibble(&rlebyte))
            return false;
   	   (*n)=rlebyte;
         return true;
      }
   	(*n)=rle_code;
	   return true;
   }
   (*n)=rlebyte;
   return true;
}

/************************************************************************
* putnibble                                                             *
* - The opposite function to getnibble, it writes one nibble to the     *
*   file using buffering                                                *
************************************************************************/
bool savefile::putnibble(byte n)
{	BOOL rval;
	dword num;
	if(rbuffptr<RBUFF_MAXLEN)
   {	if(rbhigh)
   	{	rbuff[rbuffptr]=(byte)((n&0x0f)<<4);
      	rbhigh=false;
         rbufflen++;
         return true;
      }
      rbuff[rbuffptr]+=(byte)(n&0x0f);
      rbhigh=true;
      rbuffptr++;
      return true;
   }
   else
   {  rval=WriteFile(sfile,rbuff,RBUFF_MAXLEN,&num,NULL);
   	rbhigh=true;
      rbuffptr=0;
      rbufflen=0;
      if(rval)
         return putnibble(n);
      return false;
   }
}

/************************************************************************
* flushnibble                                                           *
* - A necessity of buffered writing, this flushes the remainder of the  *
*   buffer, writing it out to file                                      *
************************************************************************/
bool savefile::flushnibble(void)
{	dword num;
 	if(rbufflen)
		return WriteFile(sfile,rbuff,rbufflen,&num,NULL)!=0;
	return true;
}

/************************************************************************
* putrlenibble                                                          *
* - This is the opposite function to getrlenibble. It writes nibbles to *
*   file whilst performing the compression. The rle encoding happens    *
*   here and when nibbles are ready to be written the putnibble         *
*   function is called                                                  *
************************************************************************/
bool savefile::putrlenibble(byte n)
{  if(rlestart)
	{	rlestart=false;
   	rlebyte=n;
      return true;
   }
	if(rlemode)
	{	if((rlebyte==n)&&(rlecount<0x0f))
   	{	rlecount++;
      	return true;
      }
      if(!putnibble(rle_code))
         return false;
      if(!putnibble(rlecount))
         return false;
      if(!putnibble(rlebyte))
         return false;
      rlemode=false;
      rlebyte=n;
      return true;
   }
   if(rlebyte==n)
   {	rlemode=true;
   	rlecount=1;
      return true;
   }
   if(!putnibble(rlebyte))
      return false;
   if(rlebyte==rle_code)
   	if(!putnibble(0))
         return false;
   rlebyte=n;
   return true;
}

/************************************************************************
* flushrlenibble                                                        *
* - This flushes any partial rle at the end of a file and forces it to  *
*   the putnibble function                                              *
************************************************************************/
bool savefile::flushrlenibble(void)
{  if(rlemode)
   {  if(!putnibble(rle_code))
         return false;
      if(!putnibble(rlecount))
         return false;
      if(!putnibble(rlebyte))
         return false;
   }
	else
   {	if(!putnibble(rlebyte))
         return false;
		if(rlebyte==rle_code)
   		if(!putnibble(0))
            return false;
   }
   return true;
}

/************************************************************************
* flushfilewrite                                                        *
* - The function to flush writing which should be called at the end of  *
*   the save. It flushes any partial encoding and then flushes the      *
*   buffered write                                                      *
************************************************************************/
bool savefile::flushfilewrite(void)
{	if(!flushrlenibble())
      return false;
	return flushnibble();
}

/************************************************************************
* sread                                                                 *
* - This is the external call for reading from a file. Its a similar    *
*   format to ReadFile, and uses the rle compression routines           *
************************************************************************/
bool savefile::sread(LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead)
{  byte n;
	dword num;
   (*lpNumberOfBytesRead)=0;
   for(num=0;num<nNumberOfBytesToRead;num++)
   {  if(!getrlenibble(&n))
         return false;
   	((byte *)lpBuffer)[num]=(byte)(n<<4);
      if(!getrlenibble(&n))
         return false;
      ((byte *)lpBuffer)[num]+=n;
      (*lpNumberOfBytesRead)++;
   }
   return true;
}

/************************************************************************
* swrite                                                                *
* - This is the external call for writing to a file. Its a similar      *
*   format to WriteFile, and uses the rle compression routines          *
************************************************************************/
bool savefile::swrite(LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite)
{  dword num;
	for(num=0;num<nNumberOfBytesToWrite;num++)
   {	if(!putrlenibble((byte)(((byte *)lpBuffer)[num]>>4)))
         return false;
   	if(!putrlenibble((byte)(((byte *)lpBuffer)[num]&0x0f)))
         return false;
   }
   return true;
}


