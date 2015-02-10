
#ifndef CDEP_328593764857345
#define CDEP_328593764857345

#include "custr.h"
#include "defs.h"
#include "cdp.h"

//Types of access from one DBObj to another
#define eAT_UNKNOWN     0x0 //Initializer value that should never stay...
#define eAT_DELETE      0x1  //
#define eAT_TRUNC_TAB   0x2  //Values are used in a bit mask to point
#define eAT_INSERT      0x4   //at one procedure doing more than one class
#define eAT_UPDATE      0x8    //of  operation e.g. proc2: delete, insert on table x
#define eAT_UPDATEStats 0x10    //update statistics command implemented
#define eAT_SELECT      0x20  //Table names that appear in a FROM clause are handled
#define eAT_EXEC        0x40 //Proc/trig executes this proc/trig directly
#define eAT_ACC_EXT     0x80 //This FLAG is set, if we Access
                             //an external database proc/table/view object

#define eAT_ACC_UNDEF   0x100 //This FLAG is set, if we Access
                             //an undefined database proc/table/view object
                             //(thats an internal db object without definition)

//renamed: eAT_EXEC -> eAT_ACC_EXT

//This View implements a SELECT on this table/view, not to be confused with eAT_SELECT,
//which means: This proc implements a select on this view, we have 2 different deps:
//                            |
//PROC ->      SELECT -> View/Table ->            SELECT ->  View/Table
//PROC -> e_AT_SELECT -> View/Table -> eAT_SELECT_BY_VIEW -> View/Table
//                            |
#define eAT_SELECT_BY_VIEW 0x200

//!!! NOTE: DEB_ACCESS_TYPE MACRO should be in sync with the above definitions!!!

//This is a simple short hand definition if we are after any
//type of access on another table/view
#define eAT_DEP_ON_TABVIEW (eAT_DELETE|eAT_TRUNC_TAB|eAT_INSERT|eAT_UPDATE \
                            | eAT_UPDATEStats \
                            | eAT_SELECT | eAT_ACC_EXT | eAT_ACC_UNDEF)

#define eAT_EXEC_BY 0x400 //Procedure is called by this procedure

//This class models the dependency of one database object X to another db-obj Y
//X is known because CDEP (or CDEPS below) is part of X
//Y is known from the member variables of CDEP (name, type etc...)
//
//This is a simple container class, so that callers may access member data freely
//Usually, a CDEP object is created in a high level function, for example, the
//SQL parser stores objects referneces in a proc/tab object in a prccol/tabcol
//in a DB object, in a ...Server object ... in a ...
//===============================================================================
class CDEP : public CDP
{
public:
  //Class Constructors
	//==================>
	CDEP();

  CDEP(const CUSTR & sServ,const CUSTR & sDB,const CUSTR & sOwn,const CUSTR & sName
      ,const eREFOBJ & eObjType,const UINT eacAccessIn,const int iFlags=0
      ,const CUSTR sHTLMLink2ExtDBIn="",const CUSTR sNewOwnerIn="");

  void CopyThis(const CDEP & cdepIn);
	CDEP(const CDEP & depIn);                 //Copy CTor

  CDEP & operator=(const CDEP & depObj);   //Assignment operator
	CDEP & operator=(const CDEP *pDep);     //Assignment operator

	void Debug(FILE *fp=stdout) const; //Debugging function

  void SetName(const UCHAR *pcServ, const UCHAR *pcDB, const UCHAR *pcOwn, const UCHAR *pcName
              ,const eREFOBJ & eObjType);

//PUBLIC PROPERTIES:
	UINT eacAccess;        //Bitmap of access types that implements the dependence
                        //For table/view: select, update..., proc: exec...

  //here we set the hyperlink to an external DB Object, if the CSERVER object
  //was able to look up the database and its object definiton
  CUSTR sHTLMLink2ExtDB;
  CUSTR sNewOwner;       //HTML Link is valid under this alternativ owner
};


//Collection of dependencies (CDEP objects)
//
//This is a list of references to dependency objects, which
//are sorted by database, owner, objectName
//-----------------================================>>>>>>>>
class CDEPS
{
public:
	 CDEPS();
	~CDEPS();

	CDEPS(const CDEPS & dpsIn); //Copy CTor

	CDEPS & operator=(const CDEPS & dpsObj);  //Assignment operator
	CDEPS & operator=(const CDEPS *pDeps);   //Assignment operator

	void operator+=(const CDEPS & dpsIn);
	//Copy additional entries from other collection pointed to by ptr
	void operator+=(const CDEPS *pdpsIn);

	//Copy additional entries from other collection
	//(same as above but using CopyCTor)
	CDEPS operator+(const CDEPS & dpsIn) const;

	//Add another dependence into the collection
  void AddDep(const CUSTR & sServ,const CUSTR & sDB, const CUSTR & sOwner, const CUSTR & sObjName
            ,const eREFOBJ & eType,const UINT eacAccess
            ,const int iFlags, const CUSTR sExtDBHTMLLink="", const CUSTR sNewOwner="");

	void AddDep(const CDEP & dpDBObj);
	void AddDep(const CDEP *pdpDBObj);

  void Debug(FILE *fp=stdout) const; //Debugging function for error tracing only


	//<==========================================================================>
	//Fourth Version of dependency collection functions are Server,DB,Owner,Objname
	//compliant, this means two entries are only equal if, all entries listed
	//are equal, otherwise we are looking at two different things
	//<==========================================================================>
	//Add all dependencies of a certain type to collection of strings
	int GetDependencyFilter2(const eREFOBJ & eObjType,const UINT eAccType, CDEPS *pRet);
	
	//Find a type of dependency and report a match/mismatch
	//{([?!.,;:][!.,;:?][.,;:?!])([,;:?!.][;:?!.,][:?!.,;])}
	int FindDep(const UCHAR *pcServ, const UCHAR *pcDB, const UCHAR *pcOwner, const UCHAR *pcDBObj,
                   const eREFOBJ eObjTypeIn, const UINT eAccType, UINT iFindMode=0) const;

  //{([,;:?!.][;:?!.,][:?!.,;])([?!.,;:][!.,;:?][.,;:?!])}
  //Find a dependency and report its type and sLink_Path
  bool FindDepByName(const UCHAR *pcServ,const UCHAR *pcDB, const UCHAR *pcOwner, const UCHAR *pcDBObj,
                     CUSTR & sLinkPath, eREFOBJ & eObjType, UINT & eAccType, CUSTR & sNewOwner);

  //Count Dependency references of the same access type
	//<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}
	int CountAccTypes(const UINT eAccTypeIn, bool bCountAll=false) const;

	//Count Dependency and DBObjTyp references of the same type
	//<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}
	int CountAccObjTypes(const eREFOBJ eObjTypeIn, const UINT eAccTypeIn,bool bCountAll=false) const;

	//Count Dependency references of the same type
	//<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}
	int CountObjTypes(const eREFOBJ eObjTypeIn, bool bCountAll=false) const;

	//Count dependencies for this particular database
	//Thats the number of unique db objects in this database we depend on
	//(There may be more unique access statements, such as select, update etc)
	//----------------------------------------------------------------------->
	int CountDBDeps(const CUSTR & sServ,const CUSTR & sDB) const;

  //>>>>>>>>>>>>>>>>>>> ITERATOR FUNCTIONS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  CDEP GetDependency();

  //Functions that let us browse the collection and access each object's data
	bool SetObjType(const eREFOBJ eObjTypeIn);

  //Returns 0, if iterator is invalid, otherwise contents of additional iFlags
  int iGetFlags();
  void iSetFlags(const int iFlagsIn);

	//Set type of access flag
	void SetAccType(const UINT iMask)
	{
		if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
			SETFLAG(this->pItDBObj->pdpDep->eacAccess, iMask);
	}

	//Unset type of access flag
	void UnSetAccType(const UINT iMask)
	{
		if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
			UNSETFLAG(this->pItDBObj->pdpDep->eacAccess, iMask);
	}

	//Fourth version of Iterator functions
	const UCHAR *GetFirst(eREFOBJ & eObjType,CUSTR & sServ,CUSTR & sDB,CUSTR & sUser,UINT & eacAccess);
	const UCHAR *GetNext(eREFOBJ & eObjType,CUSTR & sServ,CUSTR & sDB,CUSTR & sUser,UINT & eacAccess);

	//Debugging function for Iterator error tracing only
	void Debug_It(FILE *fp=stdout);

  //Copy contents of current iterator
  void CopyIteratorDep(const CDEPS & dpCopyIt);

  //Does the current iterator hold a valid entry?
  bool IteratorIsValid() const;

	//Count the total number of dependencies (minus access statements that is)
	int iCntDeps(){ return iEntrySz; };

  void SetHTML_LINK(const CUSTR & sLink);
  CUSTR GetHTML_LINK();

  void SetNewOwner(const CUSTR & sNewOwner);
  CUSTR GetNewOwner();

  //Sometimes failure is possible and part of the algorithm, and this is ok
  //as long as failures are corrected
  //Here, we wrongly assumed an owner of a database object, to correct this
  //we erase this reference here and insert the correct reference elsewhere
  void RemoveAssumedDeps(const UCHAR *pcServ,const UCHAR *pcDB,const UCHAR *pcOwner, const UCHAR *pcDBObj);

private:
	typedef struct objnd
	{
		CUSTR sObjName;

		struct objnd *pNext;
		CDEP *pdpDep;

		objnd(const CUSTR & sObjNameIn, CDEP *pdpDepIn, struct objnd *pNextIn=NULL): pNext(NULL), pdpDep(NULL)
		{
			pdpDep   = pdpDepIn;
			sObjName = sObjNameIn;
			pNext    = pNextIn;
		}                     //CTor to init required values
		~objnd()
		{
			delete pdpDep;   //Destroy container -> Destroy pLoad
		}
	} OBJND;

	typedef struct ownd
	{
		CUSTR sOwner;
		
		struct ownd  *pNext;
		struct objnd *pDBObj;

		ownd(const CUSTR & sOwnerIn, struct ownd *pNextIn=NULL): pNext(NULL), pDBObj(NULL)
		{
			sOwner = sOwnerIn;
			pNext  = pNextIn;
		}                     //CTor to init required values
	} OWND;

	typedef struct dbnd
	{
		CUSTR sServer, sDB;
		
		struct dbnd  *pNext;
		struct ownd *pOwner;

		dbnd(const CUSTR & sServIn, const CUSTR sDBIn, struct dbnd *pNextIn=NULL): pNext(NULL), pOwner(NULL)
		{
      sServer = sServIn;
			sDB     = sDBIn;
			pNext   = pNextIn;
		}                  //CTor to init required values
	} DBND;

	DBND *pDBRoot; //Pointer and size-count of internal collection
	int iEntrySz;

	DBND *pItDB;       //Iterator pointer to actual node of database objects
	OWND *pItOwner;   //Iterator pointer to actual node of owner objects
	OBJND *pItDBObj; //Iterator pointer to actual node of database object
	
	//Destroy internal data nodes on destruction
	void DestroyList(DBND **ppDBRoot, int *piEntrySz);

  //<==========================================================================>
	//Third Version of dependency collection functions are Objname, Owner, DB_Name
	//compliant, this means two entries are only equal if all three, object_name,
	//owner and database name are equal, otherwise we are having two distinct entries
	//<==========================================================================>
	void InsertDepDB(CDEP **pdpDep, DBND **ppDBRoot, int *piEntrySz);

	void InsertDepOwner(CDEP **pdpDep, OWND **ppDBRoot, int *piEntrySz);
	void InsertDepDBObj(CDEP **pdpDep, OBJND **ppDBObjRt, int *piEntrySz);

	//Copy CDEP Entries from one list <dpsSrc> into other <dpsDst>
	//-------====================================------------------>
	void CopyDeps(DBND **ppDBDstRt, int *piSz, const DBND *pDBSrcRt);
};
#endif

