
// Manage database objects for collection of procedural code
// such as procedures & triggers
//
#ifndef CPROCCOL_328593764857345
#define CPROCCOL_328593764857345

#include "defs.h"
#include "DBDefs.h"
#include "custr.h"

#include "cscol.h"
#include "cpatrn.h"
#include "cpitem.h"

#include "ctabcol.h"
#include "cproc.h"

#include <stdio.h>

class CCG;

//
// This class organizes SQL triggers and stored procedures in a collection
// so that we can access and search for certain procs with certain properties
class CPRCCOL
{
public:
	 CPRCCOL();
	~CPRCCOL();

  CPRCCOL(const CPRCCOL & dbIn);
	CPRCCOL & operator=(const CPRCCOL *pDB);
	CPRCCOL & operator=(const CPRCCOL & dbObj);

	//
	// Add new proc node into proc collection
	// This will use the += CPROC operator, if entry was already there
	// *********************************************************
	void AddProc(CPROC *prcObject);

  //Find the main entry for a Procedure/Trigger called pcKey
  //=======================================================>
  bool FindPRCNode(const UCHAR *pcServer, const UCHAR *pcDB,
                   const UCHAR *pcOwner,const UCHAR *pcName)
  {
    if(FindPRCNode(pphPrcRt, pcServer,pcDB,pcOwner,pcName)!=NULL) return true;

    return false;
  }

	const CPROC *FindProc(const UCHAR *pcServer, const UCHAR *pcDB,
	                      const UCHAR *pcOwner,const UCHAR *pcName)
	{
		H_PRC *pRet;

		if((pRet=FindPRCNode(pphPrcRt, pcServer,pcDB,pcOwner,pcName))==NULL) return NULL;

		return pRet->pPRC;
	}

	//Resolve unknown table objects (couldn't decide for view or table up to here)
	//Z==========================================================================Z
	//Z                                                                          Z
	void ResolveUnknownTables(CTABCOL & tbcTabs, const UCHAR *pcServer,const UCHAR *pcDB)
	{
		ResolveUnknownTables(pphPrcRt, tbcTabs, pcServer,pcDB);
	}

  //Name of database this is stored in
  void ResolveUnknownProcs(const UCHAR *pcServer,const UCHAR *pcDB)
  {
    ResolveUnknownProcs(pphPrcRt, pcServer, pcDB);
  }

	//Resolve flag for external and internal DataBase Calls
	//Here we seperate references to external procedures
	//from invalid references (references to undefined internal objects)
	void ResolveExtIntDBCalls(const UCHAR *pcServer,const UCHAR *pcDB);

	//
	//Browse the collection of procs and add each trigger/event defs to Tables
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	void Trigger2Tables(CTABCOL & tbTables, const UCHAR *pcServer, const UCHAR *pcDB);

	//Create a procedure object for every call to an undefined procedure,
	//the only reason why we do this is to create a unique id, which in
	//turn can be used for linkage
	//=================================>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	void CreateUnknownProcEntries();

  //Get a proc and return it by reference
  //===========================================>
  CPROC & GetProc(const UCHAR *pcServer,const UCHAR *pcDB
                 ,const UCHAR *pcOwner,const UCHAR *pcName) const;

  //
	CDEPS *GetUnknownTabEntries() const;

  //DEBUG functions are useful for finding internal program errors only
	void DEBUG(FILE *fp=stdout);

  CDEPS *FindUnknownObjects(int iFlagCollect, const CUSTR & sServ, const CUSTR & sDB, CPITEM *pitDBs=NULL);

  //Resolve inbound/outbound dependencies across Database limits
  UINT ResolveForeignDBDeps(const CUSTR & sAbsOutputPath, const THIS_DB & thisDB, CDEPS *pdpDB);
  UINT SetForeignDBDeps(const THIS_DB & thisDB, CDEPS *pdpDB, CTABCOL & tbcTabs);

  int CountDeps(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcUser,const UCHAR *pc);
  CDEPS *GetDeps(const UCHAR *pcServer, const UCHAR *pcDB,const UCHAR *pcUser,const UCHAR *pcObj);

  //Get the names of all procedures in this database
	//Print eTRIG, ePROC, eUNKNOWN or all code refs
	//
	//iGet Interpretation:
	//1 > Get all objects in sorted order, no matter what
	//2 > Get all objects from Database shown in sDB
	//3 > Get all objects from Database shown in sDB and ObjType in eOBJ_TypeIn
	//4 > Get all objects of the ObjType in eOBJ_TypeIn (regardless of DB)
	//
	//!!! Caller must free returned object !!!
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	CDEPS *ListObj(const eREFOBJ & eOBJ_TypeIn,const CUSTR & sServerIn,const CUSTR & sDB, const int & iGet);

  //Function to count table or view or all objects in collection
	//Print eUNKNOWN, eTRIG, ePROC or all objects
	//===========================================================00000>
	int iCntPrcObjects(const eREFOBJ eprcType, bool bAll);

  //Get all procedures that have no caller from within the database
	//!!! Caller must free returned object !!!
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	CDEPS *GetProcWithZeroCallers();

  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	// Find all callers for the procedure named pcName & count 'am
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	int CountFindCalls(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcUser,const UCHAR *pcObj);

  //Get the ID for a procedure or -1, if not available
	int GetProcID(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwn,const UCHAR *pcName) const
	{
		H_PRC *pRet;

		if((pRet=FindPRCNode(pphPrcRt,pcServer,pcDB,pcOwn,pcName))==NULL)
			return -1;

		return pRet->pPRC->getID();
	}

  // <<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<
	// Print contents of proc collection: Each trigger/proc goes into one file
	// <-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>->
	void ppPrintCGs(const CTABCOL & tbcTabs, const CUSTR & sSrcPath, const CPRCCOL & prcPrcs
                 ,const char *pcPath, const THIS_DB & thisDB
                 ,const CUSTR & sOutPath                        //Output directory
                 ,int *piCntProcs=NULL, int *piCntTrigs=NULL
                 );

  //Find all callers for the procedure named pcName & return an iterator collection
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  CDEPS *FindCalls(int *piCnt,const UCHAR *pcServer,const UCHAR *pcDB
                  ,const UCHAR *pcOwn,const UCHAR *pcName);

  CDEPS *FindCalls(int *piCnt, const CPROC *prc) //Handy_Dandy_ShortCut
	{
		if(prc != NULL)
			return FindCalls(piCnt,prc->GetServer(),prc->GetDB(),prc->GetOwner(),prc->GetName());
		else
			return NULL;
	}

  //Find all external objects that depend on a given object and return their access codes
  UINT FindAllOutboundDeps(CDEPS & dpRet,const CUSTR & sServer,const CUSTR & sDB
                          ,const CUSTR & sOwner, const CUSTR & sName
                          ,const CUSTR & sAbsOutPath);

  void AddOutboundDeps(const CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB
                      ,const UCHAR *pcOwner,const UCHAR *pcName);

  //Add a classification for this proc
  void AddProcClass(const CUSTR & sServer, const CUSTR & sDB
                   ,const CUSTR & sOwner,const CUSTR & sName, const CUSTR & sClassify);

private:
  void CopyThis(const CPRCCOL & prcIn);  //Helper for Copy Constructor/=operator

  //======================================================================>
	//A collection of tables and views lives in a hash table
  //We hash the dbobject name and keep objects in different db's with same
  //name in chained list, this way we keep track of same objects names but
  //different db (db1..x1 and db2..x1 in same bucket list but diff. list entry)
	typedef struct hprc
	{
		//Name, database, and owner of database object in the collection
		CUSTR sServer,sDB,sOwner,sName; //Name is used as hash-key
                                   //Server, DB, sOwner is secondary key in linked list
		CPROC *pPRC;

		struct hprc *pNext; //Link to next item

		hprc():pPRC(NULL),pNext(NULL) {}
    hprc(CPROC *pPRCIn):pPRC(NULL),pNext(NULL)
		{
      if(pPRCIn == NULL) //Check requirements here...
        return;

      if(pPRCIn->GetName().slen()<=0 || pPRCIn->GetOwner().slen()<=0
      || pPRCIn->GetDB().slen()<=0 || pPRCIn->GetServer().slen()<=0)
        return;

      pPRC = new CPROC(*pPRCIn);

      sServer = pPRCIn->GetServer();
      sDB     = pPRCIn->GetDB();
			sOwner  = pPRCIn->GetOwner();
			sName   = pPRCIn->GetName(); //Copy hash keys
		}

    hprc(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName,CPROC *pPRCIn)
		 :pPRC(NULL),pNext(NULL)
		{
			pPRC    = new CPROC(*pPRCIn);
      sServer = pcServer;
			sName   = pcName;
			sDB     = pcDB;
			sOwner  = pcOwner;
		}
		~hprc()
		{
			delete pPRC;
			//pNext(NULL)
		}
	} H_PRC;

	int iPrcRootSz;       //Size of root array
	H_PRC **pphPrcRt;  //Root of table collection

	//Private Private Private Hashing functions
	unsigned int hGetKey(const UCHAR *word) const;
	void FreeHTTab();
  bool bMatchNode(const UCHAR *pcServer,const UCHAR *pcDB
                  ,const UCHAR *pcOwner,const UCHAR *pcName, H_PRC *phtbNode) const;

	//
	//Find a PRC_TREE node with pcKey as name, in other words:
	//Find the main entry for a Procedure/Trigger called pcKey
	H_PRC *FindPRCNode(H_PRC **pphRoot, const UCHAR *pcServer, const UCHAR *pcDB,
	                                    const UCHAR *pcOwner, const UCHAR *pcName) const;

  CPROC *FindPROCNode(H_PRC **pphRoot, const UCHAR *pcServer, const UCHAR *pcDB
                     ,const UCHAR *pcOwner,const UCHAR *pcName) const
	{
		H_PRC *pRet = FindPRCNode(pphRoot, pcServer,pcDB,pcOwner,pcName);

		if(pRet == NULL) return NULL;
		else
			return pRet->pPRC;
	}

  //Resolve unknown table objects (couldn't decide for view or table up to here)
	//Z==========================================================================Z
	//Z                                                                          Z
	void ResolveUnknownTables(H_PRC **pphPrcIn,CTABCOL & tbcTabs,
	                          const UCHAR *pcServer,const UCHAR *pcDB);

  //Print Callers and Calls (the latter with complete callgraph)
  //----------------------------------------------------------->
  void HTML_PrintProc(const THIS_DB & thisDB, const CUSTR & sSrcPath, CPROC *prc,
                      const CTABCOL & tbcTabs, const CPRCCOL & prcPrcs,
                      const char *pcInputPath, CCG *pCallGraph,
                      FILE *fpOut,const CUSTR & sOutputPath);

  void ResetTableOwnerRef(CPROC *pPRC, const UCHAR *pcServer,const UCHAR *pcDB
                         ,const UCHAR *pcUser, const UCHAR *pcName
                         ,UINT eatAccess, CTABCOL & tbcTabs
                         ,const CUSTR sUniqueOwner="", const eREFOBJ eTableTypeIn=eREFUNKNOWN);

  //Collection of Proc Objects in DB
  void ResolveUnknownProcs(H_PRC **pphPrcIn,const UCHAR *pcServer,const UCHAR *pcDB);

  bool FindUniqueOwnerOfKnownProc(const UCHAR *pcServer,const UCHAR *pcDB, const UCHAR *pcDBObjName
                                 ,CUSTR & sUniqueOwner, const eREFOBJ eProcType);

  void ReAssignTableOwnerRef(CPROC *pPrc, CTABCOL & tbcTabs, CDEP & dpTmpCpy
                            ,const UINT eatAccess,   const CUSTR & sNewUser
                            ,const CUSTR & sLinkPath,const eREFOBJ & eRefType);
};

#endif
