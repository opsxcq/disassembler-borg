/************************************************************************
*				   list.cpp                                                 *
* basic list class, use with inheritance.                               *
* - maintains a list class of pointers.                                 *
* - can add/insert items                                                *
* - can use binary search                                               *
* - will resize array upwards in 4k chunks.                             *
* - downsizing has not been added but will be carried out when a        *
* disassembly is saved and reloaded as a database.                      *
* this class is central to many other classes.                          *
*                                                                       *
* Note: there is only one listiter per list. Some functions reset or    *
* change listiter, like find, and so any derived classes must be        *
* careful not to try and move through a list whilst inadvertently using *
* these functions. (It is also why the secondary program thread must be *
* often stopped during user searches, and dialogs since otherwise the   *
* databases would be constantly changing.                               *
*                                                                       *
* Rewritten for Version 2.22 this is now a template class, all          *
* functions are below and are essentially compiled for each class       *
* which uses the list.                                                  *
* - still work to be done clearing it up though                         *
* - delfunc and delfrom basically call delete item and so delfunc       *
*   should be overridden if this is not wanted                          *
************************************************************************/

#ifndef list_h
#define list_h

#include "common.h"

template <class listitem> class slist
{ private:
	 listitem *listptr;
	 dword listsize,maxsize,listiter;

  public:
	 slist(void);
	 ~slist(void);
	 virtual int compare(listitem a,listitem b);
    virtual void delfunc(listitem d);
	 void addto(listitem);
	 void delfrom(listitem);
	 listitem find(listitem);
    listitem findnext(listitem);
	 void resetiterator(void);
	 listitem nextiterator(void);
	 listitem lastiterator(void);
	 listitem processqueue(void);
    listitem peekfirst(void);
	 dword numlistitems(void);
};

/************************************************************************
* constructor function                                                  *
* - resets list, with small size list and no items.                     *
* - sets compare function and deletion function to NULL.                *
************************************************************************/
template <class listitem> slist<listitem>::slist(void)
{ listptr=new listitem[1024];
  listiter=0;
  listsize=0;
  maxsize=1024;
}

/************************************************************************
* destructor function                                                   *
* - calls the deletion function for each item in the list               *
************************************************************************/
template <class listitem> slist<listitem>::~slist(void)
{ dword i=0;
  while(i<listsize)
  { delfunc(listptr[i]);
	 i++;
  }
  delete listptr;
}

/************************************************************************
* sets the compare function                                             *
* - the compare function is used in ordering and searching the list     *
*   but it must be user defined, and so this routine should be called   *
*   as soon as possible to set up the compare function for each list    *
************************************************************************/
template <class listitem> int slist<listitem>::compare(listitem a,listitem b)
{ if(a<b) return -1;
  else if (a>b) return 1;
  return 0;
}


/************************************************************************
* the delete function                                                   *
* - the delete function is called when deleting items from a list and   *
*   can be overridden within some classes                               *
************************************************************************/
template <class listitem> void slist<listitem>::delfunc(listitem d)
{ delete d;
}

/************************************************************************
* add an item to the list                                               *
* - this takes care of list size, resizing the list if more space is    *
*   needed. It inserts a new item, using the lists compare function to  *
*   order the list                                                      *
************************************************************************/
template <class listitem> void slist<listitem>::addto(listitem ptr)
{ dword i;
  listitem *nlist;
  if(listsize==maxsize)
  { // resize list
	 maxsize+=1024;
	 nlist=new listitem[maxsize];
	 for(i=0;i<listsize;i++)
      nlist[i]=listptr[i];
	 delete listptr;
	 listptr=nlist;
  }
  if(find(ptr)==NULL)
  { // empty list
	 listptr[0]=ptr;
	 listsize=1;
	 return;
  }
  else
  { // use listiter set by find
	 // ensure can add to end
	 if(compare(listptr[listiter],ptr)==-1)
      listiter++;
	 // move array up
	 for(i=listsize;i>listiter;i--)
      listptr[i]=listptr[i-1];
	 listptr[listiter]=ptr;
	 listsize++;
  }
}

/************************************************************************
* delete an item in the list                                            *
* - this just deletes one item from the list, and closes the gap        *
*   afterwards. It does not perform downsizing of the list              *
************************************************************************/
template <class listitem> void slist<listitem>::delfrom(listitem ptr)
{ dword i;
  if(!listsize)
    return;
  if(find(ptr)==NULL)
  { // empty list
	 return;
  }
  if(!compare(listptr[listiter],ptr))  // ensure equal
  { delfunc(listptr[listiter]);
	 // move the rest down
	 for(i=listiter;i<listsize-1;i++)
      listptr[i]=listptr[i+1];
	 listsize--;
  }
}

/************************************************************************
* find an item in a list                                                *
* - this is used to find items, it performs a binary search using the   *
*   lists compare function. It returns a pointer to the nearest item    *
*   and sets the list iterator to that item.                            *
* The nearest item is:                                                  *
* - NULL if the list is empty                                           *
* - the first item if ptr< first item                                   *
* - the last item if ptr> last item                                     *
* - the first item such that ptr=item                                   *
* - the nth item if nth item<ptr and n+1th item> ptr                    *
************************************************************************/
template <class listitem> listitem slist<listitem>::find(listitem ptr)
{ dword i,j,k;
  if(!listsize)
  { listiter=0;
    return NULL;
  }
  // i moves from the front of the array and j from the back
  // until they are equal which is the returned item
  i=0;
  j=listsize-1;
  while(i!=j)
  { // k=middle item for binary search
    k=(i+j)>>1;
	 switch(compare(listptr[k],ptr))
	 { case -1:      // listptr[k]->cmp < ptr->cmp
		  if(j-i==1)  // special case
		  { if(i==k)
			 { if(compare(listptr[j],ptr)!=1)
              i=j;
				else
              j=i;
			 }
			 else
            i=j;
		  }
		  else
          i=k;   // move lower bound up
		  break;
		case 0:       // listptr[k]->cmp==ptr->cmp
		  j=k;        // only gets the first one of this type
		  break;
		case 1:       // listptr[k]->cmp > ptr->cmp
		  if(j-i==1)  // special case
		  { if(i==k)
            j=k;
			 else
            j=i;
		  }
		  else
          j=k;   // move upper bound down
		  break;
	 }
  }
  listiter=i;
  return listptr[i];
}

/************************************************************************
* findnext - finds an item in a list                                    *
* - this is used to find items, it performs a binary search using the   *
*   lists compare function. It returns a pointer to the nearest item    *
*   after the request and sets the list iterator to that item. In this  *
*   way it differs slightly from find.                                  *
* The nearest item is:                                                  *
* - NULL if the list is empty                                           *
* - the first item if ptr< first item                                   *
* - NULL if ptr> last item                                              *
* - the first item such that ptr=item                                   *
* - the n+1th item if nth item<ptr and n+1th item> ptr                  *
************************************************************************/
template <class listitem> listitem slist<listitem>::findnext(listitem ptr)
{ dword i,j,k;
  if(!listsize)
  { listiter=0;
    return NULL;
  }
  // i moves from the front of the array and j from the back
  // until they are equal which is the returned item
  i=0;
  j=listsize-1;
  // check if its beyond the bounds....
  if(compare(listptr[j],ptr)==-1)
    return NULL;
  while(i!=j)
  { // k=middle item for binary search
    k=(i+j)>>1;
	 switch(compare(listptr[k],ptr))
	 { case -1:      // listptr[k]->cmp < ptr->cmp
		  if(j-i==1)  // special case
		  { if(i==k)
			   i=j;
			 else
            j=i;
		  }
		  else
          i=k;   // move lower bound up
		  break;
		case 0:       // listptr[k]->cmp==ptr->cmp
		  j=k;        // only gets the first one of this type
		  break;
		case 1:       // listptr[k]->cmp > ptr->cmp
		  if(j-i==1)  // special case
          j=i;
		  else
          j=k;   // move upper bound down
		  break;
	 }
  }
  listiter=i;
  return listptr[i];
}

/************************************************************************
* reset list iterator to start of list                                  *
************************************************************************/
template <class listitem> void slist<listitem>::resetiterator(void)
{ listiter=0;
}

/************************************************************************
* return next item in list using list iterator                          *
* - if listiter is beyond list then returns NULL                        *
* - otherwise returns listiter item, and then increases listiter        *
* NB after finding an item the next item returned by this function will *
*    be the same item                                                   *
************************************************************************/
template <class listitem> listitem slist<listitem>::nextiterator(void)
{ if(listiter>=listsize)
  { listiter=0;
	 return NULL;
  }
  return listptr[listiter++];
}

/************************************************************************
* return previous item in list using list iterator                      *
* - if listiter is at start of list then returns NULL                   *
* - otherwise decreases listiter and then returns the listitem          *
* NB after finding an item the next item returned by this function will *
*    be the previous item                                               *
************************************************************************/
template <class listitem> listitem slist<listitem>::lastiterator(void)
{ if(!listiter)
  { return NULL;
  }
  listiter--;
  return listptr[listiter];
}

/************************************************************************
* process queue function                                                *
* - this was written for the scheduler mainly. It is used to process    *
* the list as a queue. It returns the first item from the list and also *
* removes it from the list                                              *
************************************************************************/
template <class listitem> listitem slist<listitem>::processqueue(void)
{ dword i;
  listitem t;
  if(!listsize)
    return NULL;
  t=listptr[0];
  for(i=0;i<listsize-1;i++)
	 listptr[i]=listptr[i+1];
  listsize--;
  return t;
}

/************************************************************************
* peekfirst                                                             *
* - returns the first item from the list, without removal               *
************************************************************************/
template <class listitem> listitem slist<listitem>::peekfirst(void)
{ if(!listsize)
    return NULL;
  return listptr[0];
}

/************************************************************************
* numlistitems                                                          *
* - simply returns the number of items in the list                      *
************************************************************************/
template <class listitem> dword slist<listitem>::numlistitems(void)
{ return listsize;
}


#endif
