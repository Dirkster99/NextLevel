
#ifndef CTAB_COL_328593764857345
#define CTAB_COL_328593764857345

#include "custr.h"
#include "cpitem.h"
#include "cscol.h"
#include "cerr.h"
#include "defs.h"

#include "cprop.h"
#include "ctag.h"
#include "cpatrn.h"
#include "cdep.h"
#include "ctab.h"
#include "copts.h"

#include "DBDefs.h"

class CPRCCOL;

class CTABCOL
{
public:
  void CopyThis(const CTABCOL & tbIn);  //Helper for Copy Constructor/=operator
   CTABCOL();
  ~CTABCOL();

  // Copy Constructors and Equal Operators
  CTABCOL(const CTABCOL & tbIn);

  CTABCOL & operator=(const CTABCOL & tbIn);
  CTABCOL & operator=(const CTABCOL *pTAB);

  //Print information on SQL Tables
	//===============================>
	int HTML_PrettyPrintTabs(CTABCOL & tbcTabs,CPRCCOL & prcPrcs,
	                         const THIS_DB & thisDB, const CUSTR & sOutputPath //Path to HTML output
                          );

	//Print Information about database objects being accessed by this table/view
	//In other words, we show a table showing the View's select statements that
	//implement a read-only access to another table or view
	//--------------------------------------------------------------------------->
	void HTML_PrintViewAccess(FILE *fpOut,const UINT eaccMask,CTAB & tbTab,
	                          const THIS_DB thisDB, CTABCOL & tbcTabs
                                 ,CPRCCOL & prcPrcs,const CUSTR sOutputPath);

  // Add/change table in table collection depending on datamodifying access by procedure
	// ---------------------------------------------------------------------------------->
	bool AddTableDepend(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcTabName
	                   ,const eREFOBJ etbObjType, CDEP *pdpDBObjDep);

  //Views can have a table like column definition and sql source
	//This function parses the sql source to determine dependencies of this view
	//=========================================================================>
	void ParseTabFile(const CUSTR & sFPath, const CUSTR & sFName
                   ,const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const CUSTR & sName
                   ,COpts *pPrOpts);

  //Add table by CTAG name
	//The type of CTAG is expected to be "tab" or "view"
	//This tag must have a name property signifiying the name of the table (e.g. name="course")
	//If the tag has Children of the type col, this will be accepted as column descriptors
	//========================================================================================>
	void AddTableView(CTAG & ctgTable, const UCHAR *pcServer, const UCHAR *pcDB,
	                  const UCHAR *pcOwner, const eREFOBJ etbObjType);

  //Resolve unknown table objects (couldn't decide for view or table up to here)
	//Zzc======================================================================-zZ
	void ResolveUnknownTables(const UCHAR *pcServer, const UCHAR *pcDB)
	{
		ResolveUnknownTables(pphTabRt, pcServer, pcDB);
	}

  //Find the main entry for a Procedure/Trigger called pcKey
  //=======================================================>
  bool FindTABNode(const UCHAR *pcServer,const UCHAR *pcDB
                  ,const UCHAR *pcOwner,const UCHAR *pcName) const
  {
    if(FindTabNode(pphTabRt, pcServer,pcDB,pcOwner,pcName)!=NULL) return true;

    return false;
  }

  const CTAB *FindTab(const UCHAR *pcServer,const UCHAR *pcDB
                     ,const UCHAR *pcOwner,const UCHAR *pcName) const
  {
    H_TAB *pRet;

    if((pRet=FindTabNode(pphTabRt, pcServer,pcDB,pcOwner,pcName))==NULL) return NULL;

    return pRet->ptbTable;
  }

  //Find Table entry through human readable qualifiers: Name and database
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	CTAB & GetTable(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName) const;

  // *=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=>
	// Add information about trigger and events into collection of tables      <
	// *=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=>
  void AddTrigger2Tab(const UCHAR *pcServer, const UCHAR *pcDB,const UCHAR *pcOwner
                    ,const UCHAR *pcTabName, const UCHAR *pcTrigName
                    ,EVENT_CL evOnEvents);

  //==========================================================================>
  // Description     : Collect references to unknown tables/vies database objects
  // Details: See class definition
	//==========================================================================>
  CDEPS *FindUnknownObjects(int iFlagCollect,const CUSTR sServer,const CUSTR sDB,CPITEM *pitDBs=NULL);

  //Resolve inbound/outbound dependencies across Database limits
  UINT ResolveForeignDBDeps(const CUSTR sAbsOutputPath, const THIS_DB thisDB, CDEPS *pdpDB);
  UINT SetForeignDBDeps(const THIS_DB thisDB, CDEPS *pdpDB);

  void HTML_PrintTableModifs(const THIS_DB thisDB,FILE *fpOut
	                          ,const UINT eaccMask,CPRCCOL & prcPrcs,CTAB & tbTab
                            ,const CUSTR sOutputPath
                            ,const char *pcTBL_ID="\0",const char *pcTDHead_ID="\0",const char *pcTD_ID="\0"
                            ,bool bTabHeader=true);

  void CreateUnknownTabEntries(CPRCCOL & prcColl);

  //DEBUG functions are useful for finding internal program errors only
  void DEBUG(FILE *fp=stdout, const CUSTR sTabOwner="",const CUSTR sTabName="") const;

  //Get name of html file to link to
  CUSTR GetTargFilename(const UCHAR *pcServer,const UCHAR *pcDB
                        ,const UCHAR *pcUser,const UCHAR *pcName) const;

  //Get the names of all procedures in this database
  //Get eTVIEW, eTTable, eTUNKNOWN or all table refs
  //
  //iGet Interpretation:
  //1 > Get all objects in sorted order, no matter what
  //2 > Get all objects from Database shown in sDB
  //3 > Get all objects from Database shown in sDB and ObjType in eOBJ_TypeIn
  //4 > Get all objects of the ObjType in eOBJ_TypeIn (regardless of DB)
  //
  //!!! Caller must free returned object !!!
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  CDEPS *ListObj(const eREFOBJ eOBJ_TypeIn,const CUSTR sServer,const CUSTR sDB, int iGet);

  //Function to count table or view or all objects in collection
  //Print eTTABLE, TVIEW,  eTUNKNOWN or all tabobjs
  //===========================================================00000>
  int iCntTabObjects(const eREFOBJ etbTab, bool bAll);

  //Find all table/view objects that are unreferenced in this DB
  //===========================================================00000>
  CDEPS *GetUnRefedTabView(const eREFOBJ etbTab,bool bAll=false);

	//Get name of trigger for a certain event on a certain table
	//==========================================================>
	CUSTR FindTrigger(int dmModifier             //Data modification
                   ,const UCHAR *pcServer
                   ,const UCHAR *pcDB
                   ,const UCHAR *pcOwner
                   ,const UCHAR *pcTabName) const; //Data Container

	//Get the name of a table and its ID Key so that we know linking to this is safe
	//
	bool GetTableMatch(const UCHAR *pcServer
                    ,const UCHAR *pcDB    //Ref to data container
	                  ,const UCHAR *pcOwner
	                  ,const UCHAR *pcName
	                  ,int *pIDKey, eREFOBJ *ptbcTBLClass=NULL) const;

	//Find out whether any table has size information or not
	//===========================================================00000>
	bool bTabsSizeAvail() const;

  //Find the name of the owner of a database object and return it
  //True is returned if there was no other owner, false however,
  //if there are multible objects with same name but different owner
  bool FindUniqueOwnerOfKnownTab(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcTabName
                                ,CUSTR & sRetOwner, eREFOBJ & eTABType);

  void RemoveAssumedTableDeps(const UCHAR *pcServ,const UCHAR *pcDB
                             ,const UCHAR *pcUser,const UCHAR *pcName
                             ,const UCHAR *pcDepServ,const UCHAR *pcDepDB
                             ,const UCHAR *pcDepOwner, const UCHAR *pcDepName);

  //Remove Unknown Tables which hold no dependency
  //==============================================>
  void RemoveAssumedTables();

  //Find all external objects that depend on a given object and return their access codes
  UINT FindAllOutboundDeps(CDEPS & dpRet,const CUSTR sServer,const CUSTR sDB,const CUSTR sOwner, const CUSTR sName
                          ,const CUSTR sAbsOutPath);

  // Add the given dependencies to the dependencies addressed by: pcDB.pcOwner.pcName
  //==================================================================================>
  void AddOutboundDeps(CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner
                      ,const UCHAR *pcName);

  //Add a classification for this table
  void AddTableClass(const CUSTR & sServer, const CUSTR & sDB
                    ,const CUSTR & sOwner,const CUSTR & sName, const CUSTR & sClassify);

private:
  //======================================================================>
	//A collection of tables and views lives in a hash table
  //We hash the dbobject name and keep objects in different db's with same
  //name in chained list, this way we keep track of same objects names but
  //different db (db1..x1 and db2..x1 in same bucket list but diff. list entry)
	typedef struct htab
	{
		//Name, database, and owner of database object in the collection
		CUSTR sServer, sDB, sOwner, sName; //Name is used as hash-key
                                      //sServer,sDB, sOwner is secondary key in linked list
		CTAB *ptbTable;

		struct htab *pNext; //Link to next item

		htab():ptbTable(NULL),pNext(NULL) {}
		htab(CTAB *ptbThisTable):ptbTable(NULL),pNext(NULL)
		{
      if(ptbThisTable == NULL) //Check requirements here...
        return;

      if(ptbThisTable->GetServer().slen()<=0 || ptbThisTable->GetDB().slen()<=0
      || ptbThisTable->GetName().slen()<=0 || ptbThisTable->GetOwner().slen()<=0)
        return;

      ptbTable = new CTAB(*ptbThisTable);

      sServer = ptbThisTable->GetServer();
			sDB     = ptbThisTable->GetDB();
			sOwner  = ptbThisTable->GetOwner();
			sName   = ptbThisTable->GetName();  //Copy hash keys
		}
		htab(const UCHAR *pcServer, const UCHAR *pcDB, const UCHAR *pcOwner, const UCHAR *pcTabName,
		     const eREFOBJ etbObjType):ptbTable(NULL),pNext(NULL)
		{
			ptbTable = new CTAB(pcServer,pcDB,pcOwner,pcTabName,NULL,NULL,NULL,etbObjType);

      sServer = pcServer;
			sDB     = pcDB;
			sOwner  = pcOwner;
			sName   = pcTabName; //Copy hash keys
		}
	
		~htab()
		{
			delete ptbTable;
			//pNext(NULL)
		}
	} H_TAB;

	int iTabRootSz;       //Size of root array
	H_TAB **pphTabRt;  //Root of table collection

  //Private Private Private Hashing functions
	unsigned int hGetKey(const UCHAR *word) const;
	void FreeHTTab();

  bool bMatchNode(const UCHAR *pcServer,const UCHAR *pcDB
                 ,const UCHAR *pcOwner,const UCHAR *pcName, H_TAB *phtbNode) const;
	//
	//Find a TAB_TREE node with pcKey as name, in other words:
	//Find the main entry for a Table/View called pcKey
	//-------------------------================*****00000000000>
	H_TAB *FindTabNode(H_TAB **pphRt,const UCHAR *pcServer,const UCHAR *pcDB,
	                   const UCHAR *pcOwner,const UCHAR *pcName) const;

	//Resolve unknown table objects (couldn't decide for view or table up to here)
	//Zzc======================================================================-zZ
	void ResolveUnknownTables(H_TAB **pphTabIn, const UCHAR *pcServ, const UCHAR *pcDB);

  H_TAB *AddTableDep(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcTabName
                    ,const eREFOBJ etbObjType,CDEP *pdpDBObjDep);

	//Find a reference to a able object by its name
	H_TAB *FindTableByName(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwn,const UCHAR *pc) const;
};

#endif
