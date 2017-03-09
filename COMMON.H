//					common.h
//
// common definitions

#ifndef common_h
#define common_h

// basic type definitions.
typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned long dword;
typedef char * string; 

// basic class for lptr's - pointers comprised of a segment and offset.
// the intention is that addresses are well defined within Borg. So comparison
// operators exist in a well defined way. Addresses are not converted to 32 bit
// equivalents or whatever for this.
class lptr                //Pointer Struct 32-bit.
{ public:
	 word segm;               //segment
	 dword offs;              //offset

  public:
	 lptr(){};
	 lptr(word seg,dword off);
	 ~lptr(){};
	 void assign(word seg,dword off);
    bool between(const lptr& lwb,const lptr& upb);
	 bool operator==(const lptr& loc2);
	 bool operator<=(const lptr& loc2);
	 bool operator>=(const lptr& loc2);
	 bool operator<(const lptr& loc2);
	 bool operator>(const lptr& loc2);
	 bool operator!=(const lptr& loc2);
	 lptr operator+(dword offs2);
	 lptr& operator++(int x);
	 lptr& operator+=(dword offs2);
	 lptr operator-(dword offs2);
	 dword operator-(lptr& loc2);
};

// predefined null pointer.
extern const lptr nlptr;

// basic support functions
void cleanstring(char *str);
void CenterWindow(HWND hdwnd);
void demangle(char **nme);
void getfiletoload(char *fname);
void getfiletosave(char *fname);

#endif
