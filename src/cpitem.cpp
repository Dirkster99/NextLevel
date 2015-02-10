
#include "cpitem.h"
#include "cerr.h"

CPITEM::CPITEM()
{
	ppPatrItems = NULL;
	iKeySz = iKeyActSz = 0; //Reset to empty buffer
	epType = patrUnknown;
}

CPITEM::~CPITEM()
{
	Reset();
}

//Reset PATR structure to empty mem usage and rollback state to UKNOWN
//------------------------------------------------------------------->
void CPITEM::Reset()
{
	if(ppPatrItems == NULL) return;

	if(ppPatrItems != NULL)
	{
//    for(int i=0;i<iKeyActSz;i++)
//		{
//			//-if(ppPatrItems[i] != NULL)
//			//-	delete ppPatrItems[i];
//		}

		delete [] ppPatrItems;
		ppPatrItems = NULL;
		iKeySz = iKeyActSz = 0; //Reset to empty buffer
	}
	
	epType = patrUnknown;
}

//Create a node item and return with data
//
//from a string item and the file offset where we found that item
//
CPITEM::PATR_ITEM *
CPITEM::paCreatePATRNode(const CUSTR & sServ, const CUSTR & sDB
                        ,const CUSTR & sOwner,const CUSTR & sName
                        ,const CUSTR & sRawRef, long lFOffset, int iFlagsIn)
{
	PATR_ITEM *pRet;

	if((pRet=new PATR_ITEM)==NULL) return NULL;

  pRet->sServer = sServ;
	pRet->sDB     = sDB;
	pRet->sOwner  = sOwner;
	pRet->sName   = sName;
	pRet->sRawRef = sRawRef;

	pRet->lFOffset = lFOffset;
  pRet->iFlags = iFlagsIn;

	return pRet;
}

//Add another string into the collection of strings
//
//That is, we add a string item and the file offset where we found that item
//
#define INIT_PATR_SZ 10
void CPITEM::AddString(const CUSTR & sServ,const CUSTR & sDB, const CUSTR & sOwner,
                       const CUSTR & sName,const CUSTR & sRawRef,const long lFOffset, int iFlags)
{
	if(ppPatrItems == NULL) //Init on first call
	{
		if((ppPatrItems = new PATR_ITEM * [INIT_PATR_SZ])!=NULL)
		{
			iKeySz    = INIT_PATR_SZ;
			iKeyActSz = 1;
			ppPatrItems[0] = paCreatePATRNode(sServ,sDB,sOwner,sName,sRawRef,lFOffset,iFlags);
		}
		else
		{
			iKeySz    = 0;
			iKeyActSz = 0;
		}
	
		return;
	}

	//Need to allocate new array of pointers and grow from there...
	if(iKeyActSz >= iKeySz)    //Grow array of strings
	{
		PATR_ITEM **ppPatrTmp;
	
		if((ppPatrTmp = new PATR_ITEM * [INIT_PATR_SZ + iKeySz])==NULL)
			return; //Can't grow further :(

		for(int i=0;i<iKeySz;i++){ ppPatrTmp[i] = ppPatrItems[i]; }

		delete [] ppPatrItems;                //Get rid of old array of nodes

		ppPatrItems = ppPatrTmp;
		iKeySz      = INIT_PATR_SZ + iKeySz;
	}

	if(iKeyActSz < iKeySz) //Grow array of patterns
		ppPatrItems[iKeyActSz++] = paCreatePATRNode(sServ,sDB,sOwner,sName,sRawRef,lFOffset,iFlags);
}
#undef INIT_PATR_SZ

const UCHAR *CPITEM::GetStrITEM(int idx,CUSTR & sServ,CUSTR & sDB,CUSTR & sOwner,CUSTR & sDBObj,int & iFlags) const
{
	if(ppPatrItems != NULL) //Init on first call
	{
		if(idx < iKeyActSz)
		{
      sServ  = ppPatrItems[idx]->sServer;
			sDB    = ppPatrItems[idx]->sDB;
			sOwner = ppPatrItems[idx]->sOwner;
			sDBObj = ppPatrItems[idx]->sName;
      iFlags = ppPatrItems[idx]->iFlags;
			return ppPatrItems[idx]->sRawRef;
		}
	}

	CUSTR s("Illegal index Requested: "); s+= idx;
	      s+= " Index Size: ";            s+= iKeyActSz;
	throw CERR(s, ERR_INTERNAL, "CPITEM::ITEM");
}

const UCHAR *CPITEM::GetStrITEM(int idx) const
{
	if(ppPatrItems != NULL) //Init on first call
	{
		if(idx < iKeyActSz)
			return ppPatrItems[idx]->sRawRef;
	}

	CUSTR s("Illegal index Requested: "); s+= idx;
	      s+= " Index Size: ";            s+= iKeyActSz;
	throw CERR(s, ERR_INTERNAL, "CPITEM::ITEM");
}

long CPITEM::GetOffset(int idx) const
{
	if(ppPatrItems != NULL) //Init on first call
	{
		if(idx < iKeyActSz)
			return ppPatrItems[idx]->lFOffset;
	}

	CUSTR s("Illegal index Requested: "); s+= idx;
	      s+= " Index Size: ";            s+= iKeyActSz;
	throw CERR(s, ERR_INTERNAL, "CPITEM::GetOffset");
}

void CPITEM::DebugPITEM(FILE *fp) const
{
	int i;

	if(ppPatrItems == NULL)
	{
		fprintf(fp, "No parse data available (PATR PTR == NULL)\n");
		return;
	}

	switch(epType)
	{
		case patrUnknown:
			fprintf(fp, "Unknown statement recognized (PATR TYPE: %i)!\n", epType);
			break;
		case patrExec:
			fprintf(fp, "Parsing EXEC statement (PATR TYPE: %i):\n", epType);
			break;
		case patrDELETE:
			fprintf(fp, "Parsing DELETE [FROM] TABLE statement (PATR TYPE: %i):\n", epType);
			break;
		default:
			fprintf(fp, "Unknown statement with unrecognized PATR TYPE (PATR TYPE: %i)!\n", epType);
	}

	if(iKeyActSz <= 0)
	{
		fprintf(fp, "No keywords stored!\n");
		return;
	}

	for(i=0;i<iKeyActSz; i++)
	{
    fprintf(fp, "iFlags: %i Keyword: %s\n",ppPatrItems[i]->iFlags,(const UCHAR*)ppPatrItems[i]->sRawRef);
	}
}

//==================================================================>
//Determine whether a reference to this Database is available or not
//==================================================================>
bool CPITEM::FindDBItem(const CUSTR & sServer, const CUSTR & sDB)
{
 	if(ppPatrItems == NULL) return false;

  for(int idx=0;idx<iKeyActSz;idx++)
  {
    //Return match if a Ref 2 sDB is here
    if(s::scmp_ci2(ppPatrItems[idx]->sDB,ppPatrItems[idx]->sServer,  sDB,sServer)==0)
      return true;
  }

  return false;
}

//==================================================================>
//Sort entries by name of server/database
//This is a simple bubble sort so use it only for small sets
//==================================================================>
void CPITEM::SortByServDB()
{
 	if(ppPatrItems == NULL) return;

  bool bContinue = true;

  if(this->iKeyActSz > 1)
  while(bContinue == true)
  {
    bContinue = false;
    for(int idx=0;idx<iKeyActSz-1;idx++)
    {
      //Sort Criteria is the name of Server/Database combination
      if(s::scmp_ci2(ppPatrItems[idx]->sDB,ppPatrItems[idx]->sServer
                    ,ppPatrItems[idx+1]->sDB,ppPatrItems[idx+1]->sServer) > 0)
      {
        bContinue = true;

        PATR_ITEM ptTmp;
        ptTmp.CopyThis(ppPatrItems[idx]);               //Exchange 2 spots and keep going
        ppPatrItems[idx]->CopyThis(ppPatrItems[idx+1]);
        ppPatrItems[idx+1]->CopyThis(&ptTmp);
      }
    }
  }
}
