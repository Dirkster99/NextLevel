
#ifndef PITEM_328593764857345
#define PITEM_328593764857345

#include "defs.h"
#include "custr.h"
#include "cerr.h"

#include <stdio.h>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Define Classes of objects that our patr parsing engine can find
//
typedef enum {
	 patrUnknown     = 1 //Unknown or no statement found
	,patrExec        = 2 //EXEC <Procedure_Name>
	,patrDELETE      = 3 //DELETE [FROM] TABLE
	,patrUPDATE      = 4 //UPDATE TABLE
	,patrUPDATEStats = 5 //Any keyword that may need highlighting
	,patrINSERT      = 6 //INSERT [INTO] TABLE
	,patrDECLARE     = 7 //DECALARE CURSOR
	
	,patrDECL_CU     = 8    //Declare cursor command
	,patrUSAG_CU     = 9   //Open commad for cursor
	,patrDEAL_CU     = 10 //deallocating cursor statement

	,patrFROM        = 11 //FROM Clause (used in select, update, delete statements)
	,patrTRUNC_TAB   = 12 //TRUNCATE TABLE STATEMENT
	,patrFOR_UPDATE  = 13 //Parse Diff for update cursore from update <table>
} patrTYPE;

// This class supports the parsing engine by returning recognized
// string tokens in an array for further parsing
// The returned items are pre-classified by patrTYPE.epType
//
//A CPITEM is then similar to a parse tree in the way that we
//collect recognized keywords and classify the results
//as class1, class2 ... or unknown
class CPITEM
{
public:
	CPITEM();
	~CPITEM();

	CPITEM(CPITEM & ptrIn)
	{
		throw CERR("Copy constructor for CPITEM NOT implemented, yet!!!\n");
	}

	CPITEM & operator=(const CPITEM *pPtr)
	{
		throw CERR("Equal operator for CPITEM NOT implemented, yet!!!\n");
	}

	CPITEM & operator=(const CPITEM & itmObj)
	{
		throw CERR("Equal operator for CPITEM NOT implemented, yet!!!\n");
	}

	void DebugPITEM(FILE *fp) const;
	void Reset();

	void AddString(const CUSTR & sServ,const CUSTR & sDB, const CUSTR & sOwner, const CUSTR & sName,
	               const CUSTR & sRawRef, const long lFOffset, int iFlags);

	const UCHAR *GetStrITEM(int idx) const;
	const UCHAR *GetStrITEM(int idx,CUSTR & sServ,CUSTR & sDB,CUSTR & sOwner,CUSTR & sDBObj,int & iFlags) const;

	long GetOffset(int idx) const;

	int GetSz() const { return iKeyActSz; }

  bool FindDBItem(const CUSTR & sServer, const CUSTR & sDB);

  void SortByServDB(); //Sort entries by name of server/database

private:
	typedef struct patr_item
	{
		CUSTR sServer, sDB, sOwner, sName //Parsed referenced to database object
         ,sRawRef;                   //actual (raw) string parsed out of input stream
		long lFOffset;                  //This is the file offset where we found the above string

    //These are additional generic flags that can be set by the caller
    //The class itself does not care for the information stored in here
    int iFlags;

    void CopyThis(const patr_item *pt)
    {
      this->iFlags   = 0;
      this->lFOffset = 0;
      this->sServer = this->sDB = this->sOwner = this->sName  = this->sRawRef = "";

      if(pt != NULL)
	    if(pt != this)
	    {
        this->iFlags   = pt->iFlags;
        this->lFOffset = pt->lFOffset;
        this->sServer  = pt->sServer;
        this->sDB      = pt->sDB;
        this->sOwner   = pt->sOwner;
        this->sName    = pt->sName;
        this->sRawRef  = pt->sRawRef;
	    }
    }
	} PATR_ITEM;

	PATR_ITEM **ppPatrItems; //Collection of recognized items as seen on input stream
	int iKeySz;            //Total size of string array pointers
	int iKeyActSz;         //Actually used size of items in array

	PATR_ITEM *paCreatePATRNode(const CUSTR & sServ,const CUSTR & sDB, const CUSTR & sOwner,
                              const CUSTR & sName,const CUSTR & sRawRef, long lFOffset, int iFlags);

public:
	patrTYPE epType;      //Type of recognized pattern
};

#endif
