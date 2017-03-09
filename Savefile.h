//			savefile.h
//

#ifndef savefile_h
#define savefile_h

#include "list.h"
#include "common.h"

#define RBUFF_MAXLEN 4096
#define rle_code 0x0f

class savefile
{  private:
      HANDLE sfile;
      unsigned long rbufflen;
      unsigned long rbuffptr;
      byte rbuff[RBUFF_MAXLEN+1];
      bool rbhigh;
      bool rlemode;
      bool rlestart;
      byte rlecount;
      byte rlebyte;

	private:
   	bool getnibble(byte *n);
	   bool getrlenibble(byte *n);
      bool putnibble(byte n);
      bool flushnibble(void);
      bool putrlenibble(byte n);
      bool flushrlenibble(void);

	public:
 		savefile();
		~savefile();
   	bool sopen(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,
			DWORD dwCreationDistribution,DWORD dwFlagsAndAttributes);
   	void sclose(void);
   	bool sread(LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead);
   	bool swrite(LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite);
      bool flushfilewrite(void);
};

#endif