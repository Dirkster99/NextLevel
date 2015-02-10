
#include "cdep.h"
#include "custr.h"
#include "cerr.h"

//Standard constructor for initialization without parameters
//Member variables can be set later since the are all public
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CDEP::CDEP():eacAccess(eAT_UNKNOWN) //,edbObjType(eOBJUNKNOWN)
{
}

CDEP::CDEP(const CDEP & depIn) //Copy Constructor
     :CDP(depIn)              //First of all: Call Copy CTor in BaseClasse
{
  this->eacAccess       = depIn.eacAccess;

  //This only used to resolve external DB dependencies
  this->sHTLMLink2ExtDB = depIn.sHTLMLink2ExtDB;
  this->sNewOwner       = depIn.sNewOwner;
}

void CDEP::CopyThis(const CDEP & cdepIn)
{
  CDP::CopyThis(cdepIn);
  eacAccess       = cdepIn.eacAccess;
  sHTLMLink2ExtDB = cdepIn.sHTLMLink2ExtDB;
  sNewOwner       = cdepIn.sNewOwner;
}

//Constructor to initialize everything at construction time
//========================================================>
CDEP::CDEP(const CUSTR & sServ,const CUSTR & sDB,const CUSTR & sOwn,const CUSTR & sName
          ,const eREFOBJ & eObjType,const UINT eacAccessIn,const int iFlags
          ,const CUSTR sHTLMLink2ExtDBIn, const CUSTR sNewOwnerIn)
     :CDP(sServ, sDB,sOwn,sName,eObjType, iFlags) //Copy constructor for base clase does this
{
  eacAccess       = eacAccessIn;
  sHTLMLink2ExtDB = sHTLMLink2ExtDBIn;
  sNewOwner       = sNewOwnerIn;
}

void CDEP::SetName(const UCHAR *pcServ, const UCHAR *pcDB, const UCHAR *pcOwn, const UCHAR *pcName
                  ,const eREFOBJ & eObjTypeIn)
{
  this->sServer  = pcServ;
  this->sDB      = pcDB;
  this->sOwner   = pcOwn;
  this->sObjName = pcName;
  this->eObjType = eObjTypeIn;
}

CDEP & CDEP::operator=(const CDEP & depObj) //Assignment operator
{
  CopyThis(depObj);

  return *this;
}

CDEP & CDEP::operator=(const CDEP *pDep) //Assignment operator
{
	if(pDep != NULL) *this = *pDep;

	return *this;
}

void CDEP::Debug(FILE *fp) const //Debugging function for error tracing only
{
  CDP::Debug(fp); //Debug contents of base class

  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF ACCESS_TYPE DEBUGGING
  fprintf(fp, "ACCESS_TYPE(%i): ", this->eacAccess);

	if FLAG(eAT_UNKNOWN,this->eacAccess) fprintf(fp, "UKNOWN ");

  if FLAG(eAT_DELETE ,this->eacAccess) fprintf(fp, "DELETE ");

  if FLAG(eAT_TRUNC_TAB ,this->eacAccess) fprintf(fp, "TRUNCATE TABLE ");

  if FLAG(eAT_INSERT ,this->eacAccess) fprintf(fp, "INSERT ");

  if FLAG(eAT_UPDATE ,this->eacAccess) fprintf(fp, "UPDATE ");

  if FLAG(eAT_UPDATEStats,this->eacAccess) fprintf(fp, "UPDATEStats ");

  if FLAG(eAT_SELECT,this->eacAccess) fprintf(fp, "SELECT ");

  if FLAG(eAT_EXEC,this->eacAccess) fprintf(fp, "EXECUTE ");

  if FLAG(eAT_ACC_EXT,this->eacAccess) fprintf(fp, "ACC_EXTERN ");

  if FLAG(eAT_ACC_UNDEF,this->eacAccess) fprintf(fp, "ACC_UNDEF ");

  if FLAG(eAT_SELECT_BY_VIEW,this->eacAccess) fprintf(fp, "eAT_SELECT_BY_VIEW ");
 
  if(this->eacAccess == 0) fprintf(fp, "UNKNOWN ACCESS TYPE (%i)",this->eacAccess);
  //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< END OF ACCESS_TYPE DEBUGGING

  fprintf(fp, "  sNewOwn: %s\n", (const char *)this->sNewOwner);
  fprintf(fp, "sExtDB: %s\n", (const char *)this->sHTLMLink2ExtDB);
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

CDEPS::CDEPS():pDBRoot(NULL),iEntrySz(0),
               pItDB(NULL), pItOwner(NULL), pItDBObj(NULL)
{}

CDEPS::~CDEPS()
{
	DestroyList(&(this->pDBRoot), &(this->iEntrySz));
}


CDEPS::CDEPS(const CDEPS & dpsIn):pDBRoot(NULL),iEntrySz(0), //Copy CTor
                                  pItDB(NULL), pItOwner(NULL), pItDBObj(NULL)
{
	CopyDeps(&(this->pDBRoot), &(this->iEntrySz), dpsIn.pDBRoot);
}

//Copy CDEP Entries from one list <dpsSrc> into other <dpsDst>
//-------====================================------------------>
void CDEPS::CopyDeps(DBND **ppDBDstRt, int *piDstEntrySz, const DBND *pDBSrcRtIn)
{
	this->pItDB = NULL, this->pItOwner = NULL, this->pItDBObj = NULL; //Reset Iterator
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	CDEP *pNewDep;
                                //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)    //We browse the source collection and
	{                           //insert each node in the destination collection
		pOWRoot = pDBSrcRtIn->pOwner;
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			while(pOBJRoot != NULL)
			{
				if((pNewDep = new CDEP(*pOBJRoot->pdpDep))!=NULL)
				{
					InsertDepDB(&pNewDep, ppDBDstRt, piDstEntrySz);
					delete pNewDep;          //Ptr should be null on return but delete in any case
				}
				pOBJRoot = pOBJRoot->pNext;
			}
			pOWRoot = pOWRoot->pNext;
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}
}

//
//Destroy list of references to dependency objects, which
//are sorted by database, owner, objectName
//-----------------================================>>>>>>
void CDEPS::DestroyList(DBND **ppDBRootIn, int *piEntrySz)
{
	this->pItDB = NULL, this->pItOwner = NULL, this->pItDBObj = NULL; //Reset Iterator

	DBND *pDBRootSrc, *pDBTmp;
	OWND *pOWRoot, *pOWTmp;
	OBJND *pOBJRoot, *pOBJTmp;
	
	pDBRootSrc = *ppDBRootIn;
	while(pDBRootSrc != NULL)
	{
		pOWRoot = pDBRootSrc->pOwner;
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			while(pOBJRoot != NULL)
			{
				pOBJTmp  = pOBJRoot;
				pOBJRoot = pOBJRoot->pNext;
				delete pOBJTmp;
			}

			pOWTmp  = pOWRoot;
			pOWRoot = pOWRoot->pNext;
			delete pOWTmp;
		}
	
		pDBTmp     = pDBRootSrc;
		pDBRootSrc = pDBRootSrc->pNext;
		delete pDBTmp;
	}
	
	*ppDBRootIn  = NULL;      //Reset Caller's root structure
	*piEntrySz   = 0;
}

CDEPS & CDEPS::operator=(const CDEPS & dpsObj)  //Assignment operator
{
	DestroyList(&(this->pDBRoot), &(this->iEntrySz));  //Destroy Me and Copy from You
	CopyDeps(&(this->pDBRoot), &(this->iEntrySz), dpsObj.pDBRoot);
	
	return *this;
}

CDEPS & CDEPS::operator=(const CDEPS *pDeps)  //Assignment operator
{
	DestroyList(&(this->pDBRoot), &(this->iEntrySz));  //Destroy Me and Copy from You

	if(pDeps != NULL) *this = *pDeps;

	return *this;
}

//Copy additional entries from other collection
void CDEPS::operator+=(const CDEPS & dpsIn)
{
	this->pItDB = NULL, this->pItOwner = NULL, this->pItDBObj = NULL; //Reset Iterator
	CopyDeps(&(this->pDBRoot), &(this->iEntrySz), dpsIn.pDBRoot);
}

//Copy additional entries from other collection pointed to by ptr
void CDEPS::operator+=(const CDEPS *pdpsIn)
{
	this->pItDB = NULL, this->pItOwner = NULL, this->pItDBObj = NULL; //Reset Iterator

	if(pdpsIn != NULL)
		CopyDeps(&(this->pDBRoot), &(this->iEntrySz), pdpsIn->pDBRoot);
}

//Copy additional entries from other collection
//(same as above but using CopyCTor)
CDEPS CDEPS::operator+(const CDEPS & dpsIn) const
{
	CDEPS ret(*this);

	ret += dpsIn;

	return ret;
}

//Privat Sub-functions to support sorted insert in internal structur
//We sort entries sorted by DB_Name.Owner_Name.Object_Name
//(This is how T-SQL Engines identify objects uniquely, and so do we)
//>>>>>>>>>>>>>==========================--------------------------<
void CDEPS::InsertDepDBObj(CDEP **pdpDep, OBJND **ppDBObjRt, int *piEntrySz)
{
	if(*ppDBObjRt == NULL) //Insert 1st node ever on this level
	{
		*ppDBObjRt = new OBJND((*pdpDep)->sObjName, *pdpDep);
		*pdpDep=NULL;
		*piEntrySz += 1;
		return;
	}

	if((*ppDBObjRt)->sObjName.scmp_ci((*pdpDep)->sObjName) > 0) //This node goes before 1st node
	{
		*ppDBObjRt = new OBJND((*pdpDep)->sObjName, *pdpDep, *ppDBObjRt);
		*pdpDep=NULL;
		*piEntrySz += 1;
		return;
	}

	while(*ppDBObjRt != NULL)
	{
		int iCmp = (*ppDBObjRt)->sObjName.scmp_ci((*pdpDep)->sObjName);

		if(iCmp == 0)                    //This Node's already there..
		{                               //Collect additional flags
			(*ppDBObjRt)->pdpDep->eacAccess |= (*pdpDep)->eacAccess;
			delete *pdpDep;
			*pdpDep = NULL;
			return;
		}
		else
		{
			if(iCmp < 0) //New node should be inserted right here
			{
				if((*ppDBObjRt)->pNext != NULL)
				{
					if((*ppDBObjRt)->pNext->sObjName.scmp_ci((*pdpDep)->sObjName) > 0)
					{
						(*ppDBObjRt)->pNext = new OBJND((*pdpDep)->sObjName, *pdpDep, (*ppDBObjRt)->pNext);
						*pdpDep=NULL;
						*piEntrySz += 1;
						return;
					}
				}
				else
				{
					(*ppDBObjRt)->pNext = new OBJND((*pdpDep)->sObjName, *pdpDep, (*ppDBObjRt)->pNext);
					*pdpDep=NULL;
					*piEntrySz += 1;
				
					return;
				}
			}
		}

		ppDBObjRt = &((*ppDBObjRt)->pNext);
	}
	//Insert new node at end of list (pNext is NULL and we are still here)
	*ppDBObjRt = new OBJND((*pdpDep)->sObjName, *pdpDep);
	*pdpDep=NULL;
	*piEntrySz += 1;
}

void CDEPS::InsertDepOwner(CDEP **pdpDep, OWND **ppOwnerRt, int *piEntrySz)
{
	if(*ppOwnerRt == NULL) //Insert 1st node ever on this level
	{
		if((*ppOwnerRt = new OWND((*pdpDep)->sOwner))!=NULL)
			InsertDepDBObj(pdpDep, &((*ppOwnerRt)->pDBObj), piEntrySz);

		return;
	}

	if((*ppOwnerRt)->sOwner.scmp_ci((*pdpDep)->sOwner) > 0) //This node goes before 1st node
	{
		*ppOwnerRt = new OWND((*pdpDep)->sOwner, *ppOwnerRt);
		InsertDepDBObj(pdpDep, &((*ppOwnerRt)->pDBObj), piEntrySz);
		return;
	}

	while(*ppOwnerRt != NULL)
	{
		int iCmp = (*ppOwnerRt)->sOwner.scmp_ci((*pdpDep)->sOwner);

		if(iCmp == 0) //This Node's already there..
		{
			if((*ppOwnerRt)->pDBObj != NULL)
				InsertDepDBObj(pdpDep, &((*ppOwnerRt)->pDBObj), piEntrySz);

			return;
		}
		else
		{
			if(iCmp < 0) //New node should be inserted right here
			{
				if((*ppOwnerRt)->pNext != NULL)
				{
					if((*ppOwnerRt)->pNext->sOwner.scmp_ci((*pdpDep)->sOwner) > 0)
					{
						(*ppOwnerRt)->pNext = new OWND((*pdpDep)->sOwner, (*ppOwnerRt)->pNext);
						InsertDepDBObj(pdpDep, &(((*ppOwnerRt)->pNext)->pDBObj), piEntrySz);
						return;
					}
				}
				else
				{
					if(((*ppOwnerRt)->pNext = new OWND((*pdpDep)->sOwner, (*ppOwnerRt)->pNext))!=NULL)
						InsertDepDBObj(pdpDep, &(((*ppOwnerRt)->pNext)->pDBObj), piEntrySz);
					
					return;
				}
			}
		}

		ppOwnerRt = &((*ppOwnerRt)->pNext);
	}
	
	if((*ppOwnerRt = new OWND((*pdpDep)->sOwner))!=NULL) //Insert new node at end of list (pNext is NULL)
		InsertDepDBObj(pdpDep, &((*ppOwnerRt)->pDBObj), piEntrySz);
}

void CDEPS::InsertDepDB(CDEP **pdpDep, DBND **ppDBRoot, int *piEntrySz)
{
	if(*pdpDep == NULL) return;

	if(*ppDBRoot == NULL) //Insert 1st node ever on this level
	{
		if((*ppDBRoot = new DBND((*pdpDep)->sServer,(*pdpDep)->sDB))!=NULL)
		InsertDepOwner(pdpDep, &((*ppDBRoot)->pOwner), piEntrySz);
		return;
	}

  //This node goes before 1st node
	if(s::scmp_ci2((*ppDBRoot)->sDB,(*ppDBRoot)->sServer,(*pdpDep)->sDB,(*pdpDep)->sServer) > 0)
	{
		*ppDBRoot = new DBND((*pdpDep)->sServer, (*pdpDep)->sDB, *ppDBRoot);
		InsertDepOwner(pdpDep, &((*ppDBRoot)->pOwner), piEntrySz);
		return;
	}

	while(*ppDBRoot != NULL)
	{
		int iCmp = s::scmp_ci2((*ppDBRoot)->sDB,(*ppDBRoot)->sServer,(*pdpDep)->sDB,(*pdpDep)->sServer);

		if(iCmp == 0) //This Node's already there..
		{
			if((*ppDBRoot)->pOwner != NULL)
				InsertDepOwner(pdpDep, &((*ppDBRoot)->pOwner), piEntrySz);

			return;
		}
		else
		{
			if(iCmp < 0) //New node should be inserted right here
			{
				if((*ppDBRoot)->pNext != NULL)
				{
          if(s::scmp_ci2((*ppDBRoot)->pNext->sDB,(*ppDBRoot)->pNext->sServer,(*pdpDep)->sDB,(*pdpDep)->sServer)>0)
					{
						(*ppDBRoot)->pNext = new DBND((*pdpDep)->sServer, (*pdpDep)->sDB, (*ppDBRoot)->pNext);
						InsertDepOwner(pdpDep, &(((*ppDBRoot)->pNext)->pOwner), piEntrySz);
						return;
					}
				}
				else
				{
					(*ppDBRoot)->pNext = new DBND((*pdpDep)->sServer, (*pdpDep)->sDB, (*ppDBRoot)->pNext);
					InsertDepOwner(pdpDep, &(((*ppDBRoot)->pNext)->pOwner), piEntrySz);

					return;
				}
			}
		}

		ppDBRoot = &((*ppDBRoot)->pNext);
	}
	
	if((*ppDBRoot = new DBND((*pdpDep)->sServer, (*pdpDep)->sDB))!=NULL) //Insert new node at end of list (pNext is NULL)
		InsertDepOwner(pdpDep, &((*ppDBRoot)->pOwner), piEntrySz);
}

//Add another dependence into the collection
void CDEPS::AddDep(const CUSTR & sServ,const CUSTR & sDB, const CUSTR & sOwner, const CUSTR & sObjName
                  ,const eREFOBJ & eType,const UINT eacAccess
                  ,const int iFlags, const CUSTR sExtDBHTMLLink, const CUSTR sNewOwner)
{
  CDP::Handle_Requi_Params(sServ, sDB,sOwner,sObjName, "CDEPS::AddDep");

	this->pItDB = NULL, this->pItOwner = NULL, this->pItDBObj = NULL; //Reset Iterator
	CDEP *pNewDep;

	if((pNewDep = new CDEP(sServ,sDB,sOwner,sObjName,eType,eacAccess,iFlags,sExtDBHTMLLink,sNewOwner))!=NULL)
	{
		InsertDepDB(&pNewDep, &(this->pDBRoot), &(this->iEntrySz));
		delete pNewDep;           //Ptr may be null on return but delete in any case
	}
}

void CDEPS::AddDep(const CDEP & dpDBObj)
{
  AddDep(dpDBObj.sServer,dpDBObj.sDB,dpDBObj.sOwner,dpDBObj.sObjName
        ,dpDBObj.eObjType, dpDBObj.eacAccess,dpDBObj.iFlags
        ,dpDBObj.sHTLMLink2ExtDB,dpDBObj.sNewOwner);
}

void CDEPS::AddDep(const CDEP *pdpDBObj)
{
  if(pdpDBObj != NULL)
  {
    AddDep(pdpDBObj->sServer,pdpDBObj->sDB,pdpDBObj->sOwner,pdpDBObj->sObjName
          ,pdpDBObj->eObjType, pdpDBObj->eacAccess,pdpDBObj->iFlags
          ,pdpDBObj->sHTLMLink2ExtDB,pdpDBObj->sNewOwner);
  }
}


void CDEPS::Debug(FILE *fp) const //Debugging function for error tracing only
{
	DBND *pDBRNode = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	int i=0;
	
	while(pDBRNode != NULL)
	{
    fprintf(fp,"Nodes for:%s/%s\n", (const UCHAR *)pDBRNode->sServer,(const UCHAR *)pDBRNode->sDB);
		pOWRoot = pDBRNode->pOwner;
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			for( ; pOBJRoot != NULL; i++, pOBJRoot = pOBJRoot->pNext)
			{
				//fprintf(fp,"Key %4i %s.%s.%s -> ", i+1, (const UCHAR *)pDBRNode->sDB
				//                                      , (const UCHAR *)pOWRoot->sOwner
				//													               , (const UCHAR *)pOBJRoot->sObjName);
				if(pOBJRoot->pdpDep != NULL)
					pOBJRoot->pdpDep->Debug(fp);
				else
					fprintf(fp, "\n");
			}
			pOWRoot = pOWRoot->pNext;
		}
		pDBRNode = pDBRNode->pNext;
	}

	if(i != this->iEntrySz)
		fprintf(fp, "!!! ERROR: List IS CORRUPT !!! Refs Found: %i != Refs Stored: %i\n\n" ,i, this->iEntrySz);
	else
		fprintf(fp, "Refs Found: %i == Refs Stored: %i\n\n" ,i, this->iEntrySz);
}

//Count dependencies for this particular database
//Thats the number of unique db objects in this database we depend on
//(There may be more unique access statements, such as select, update etc)
//----------------------------------------------------------------------->
int CDEPS::CountDBDeps(const CUSTR & sServ,const CUSTR & sDB) const
{
	int iCnt=0;

	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node under the sDB node
    if(s::scmp_ci2(pDBSrcRtIn->sDB,pDBSrcRtIn->sServer,  sDB,sServ) == 0)
		{
			pOWRoot = pDBSrcRtIn->pOwner;
			while(pOWRoot != NULL)
			{
				pOBJRoot = pOWRoot->pDBObj;
				while(pOBJRoot != NULL)
				{
					iCnt++;
					pOBJRoot = pOBJRoot->pNext;
				}
				pOWRoot = pOWRoot->pNext;
			}

			return iCnt; //All objects in that DB browsed, so return result
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}

	return iCnt;
}

//Count Dependency references of the same type
//<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}
int CDEPS::CountObjTypes(const eREFOBJ eObjTypeIn, bool bCountAll) const
{
	int iCnt=0;

	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node that matches the condition
		pOWRoot = pDBSrcRtIn->pOwner;			
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			while(pOBJRoot != NULL)
			{
				if(bCountAll == true || pOBJRoot->pdpDep->eObjType == eObjTypeIn) iCnt++;

				pOBJRoot = pOBJRoot->pNext;
			}
			pOWRoot = pOWRoot->pNext;
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}

	return iCnt;
}

//Count Dependency references of the same access type
//<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}
int CDEPS::CountAccTypes(const UINT eAccTypeIn, bool bCountAll) const
{
	int iCnt=0;

	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node that matches the condition
		pOWRoot = pDBSrcRtIn->pOwner;			
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			while(pOBJRoot != NULL)
			{
				if(bCountAll == true || FLAG(pOBJRoot->pdpDep->eacAccess, eAccTypeIn)) iCnt++;

				pOBJRoot = pOBJRoot->pNext;
			}
			pOWRoot = pOWRoot->pNext;
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}

	return iCnt;
}

//Count Dependency and DBObjTyp references of the same type
//<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}<?>(!)-+-[P]{T}
int CDEPS::CountAccObjTypes(const eREFOBJ eObjTypeIn, const UINT eAccTypeIn,
                             bool bCountAll) const
{
	int iCnt=0;

	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node that matches the condition
		pOWRoot = pDBSrcRtIn->pOwner;			
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			while(pOBJRoot != NULL)
			{
				if(bCountAll == true || 
					 (pOBJRoot->pdpDep->eObjType == eObjTypeIn &&
            FLAG(pOBJRoot->pdpDep->eacAccess, eAccTypeIn))
					)
				iCnt++;

				pOBJRoot = pOBJRoot->pNext;
			}
			pOWRoot = pOWRoot->pNext;
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}

	return iCnt;
}

//{([,;:?!.][;:?!.,][:?!.,;])([?!.,;:][!.,;:?][.,;:?!])}
//Find a type of dependency and report a match/mismatch
//{([?!.,;:][!.,;:?][.,;:?!])([,;:?!.][;:?!.,][:?!.,;])}
int CDEPS::FindDep(const UCHAR *pcServ, const UCHAR *pcDB, const UCHAR *pcOwner, const UCHAR *pcDBObj,
                   const eREFOBJ eObjTypeIn, const UINT eAccType, UINT iFindMode) const
{
	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node that matches the condition
    if(s::scmp_ci2(pDBSrcRtIn->sDB,pDBSrcRtIn->sServer,  pcDB,pcServ) == 0)
		{
			pOWRoot = pDBSrcRtIn->pOwner;			
			while(pOWRoot != NULL)
			{
				if(pOWRoot->sOwner.scmp_ci(pcOwner) == 0)
				{
					pOBJRoot = pOWRoot->pDBObj;
					while(pOBJRoot != NULL)
					{
            switch(iFindMode)
            {
            case 0:
						  if(pOBJRoot->pdpDep->eObjType == eObjTypeIn  && //Match Object Type
						    s::scmp_ci(pOBJRoot->sObjName ,pcDBObj)==0 && //DB.Owner.ObjName path
							  FLAG(pOBJRoot->pdpDep->eacAccess, eAccType))  //and (phew) required access type
						    return true;
            break;

            case 1:        //Match DB.Owner.ObjName path only
              if(s::scmp_ci(pOBJRoot->sObjName ,pcDBObj)==0)
                return true;
            break;
            }

						pOBJRoot = pOBJRoot->pNext;
					}
				}
				pOWRoot = pOWRoot->pNext;
			}
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}
	return false;
}

//{([,;:?!.][;:?!.,][:?!.,;])([?!.,;:][!.,;:?][.,;:?!])}
//Find a dependency and report its type and sLink_Path
//{([?!.,;:][!.,;:?][.,;:?!])([,;:?!.][;:?!.,][:?!.,;])}
bool CDEPS::FindDepByName(const UCHAR *pcServ,const UCHAR *pcDB, const UCHAR *pcOwner, const UCHAR *pcDBObj,
                   CUSTR & sLinkPath, eREFOBJ & eObjType, UINT & eAccType, CUSTR & sNewOwner)
{
	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node that matches the condition
    if(s::scmp_ci2(pDBSrcRtIn->sDB,pDBSrcRtIn->sServer,  pcDB,pcServ) == 0)
		{
			pOWRoot = pDBSrcRtIn->pOwner;			
			while(pOWRoot != NULL)
			{
				if(pOWRoot->sOwner.scmp_ci(pcOwner) == 0)
				{
					pOBJRoot = pOWRoot->pDBObj;
					while(pOBJRoot != NULL)
					{ //If we have a match -> Set return values and return
            if(s::scmp_ci(pOBJRoot->sObjName ,pcDBObj)==0)
            {
              sLinkPath = pOBJRoot->pdpDep->sHTLMLink2ExtDB;
              sNewOwner = pOBJRoot->pdpDep->sNewOwner;
						  eObjType  = pOBJRoot->pdpDep->eObjType;
							eAccType  = pOBJRoot->pdpDep->eacAccess;
              
              return true;
            }

						pOBJRoot = pOBJRoot->pNext;
					}
				}
				pOWRoot = pOWRoot->pNext;
			}
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}
	return false;
}

//Add all dependencies of a certain type to collection of strings
int CDEPS::GetDependencyFilter2(const eREFOBJ & eObjType,const UINT eAccType, CDEPS *pRet)
{
	if(pRet == NULL) return 0;

	int iCnt=0;
	DBND *pDBSrcRtIn = this->pDBRoot;
	OWND *pOWRoot;
	OBJND *pOBJRoot;
	                               //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)     //We browse the source collection and
	{                            //note each node that matches the condition
		pOWRoot = pDBSrcRtIn->pOwner;			
		while(pOWRoot != NULL)
		{
			pOBJRoot = pOWRoot->pDBObj;
			while(pOBJRoot != NULL)
			{
				if(pOBJRoot->pdpDep->eObjType == eObjType &&
				   FLAG(pOBJRoot->pdpDep->eacAccess,eAccType))
				{
					iCnt++;
					pRet->AddDep(pOBJRoot->pdpDep);
				}
				pOBJRoot = pOBJRoot->pNext;
			}
			pOWRoot = pOWRoot->pNext;
		}
		pDBSrcRtIn = pDBSrcRtIn->pNext;
	}

	return iCnt;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>> START OF ITERATOR >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

CDEP CDEPS::GetDependency()
{
	//Work only with valid Iterator
	if(this->pItDB == NULL || this->pItOwner == NULL || this->pItDBObj == NULL)
	{
		CUSTR s("Can not find current dependency in CDEPS");
		throw CERR(s, 8, "CDEPS::GetDependency");
	}

	return * this->pItDBObj->pdpDep;
}

//Return additional flags, if available
int CDEPS::iGetFlags()
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		return this->pItDBObj->pdpDep->iFlags;
	}

	return 0;
}

//Set additional flags, if iterator is available
void CDEPS::iSetFlags(const int iFlagsIn)
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		this->pItDBObj->pdpDep->iFlags = iFlagsIn;
	}
}

bool CDEPS::SetObjType(const eREFOBJ eObjTypeIn)
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		this->pItDBObj->pdpDep->eObjType = eObjTypeIn;
		return true;
	}

	return false;
}

//Fourth version of Iterator functions
const UCHAR *CDEPS::GetFirst(eREFOBJ & eObjType,CUSTR & sServ,CUSTR & sDB,CUSTR & sOwn,
                             UINT & eacAccess)
{

	if((pItDB = pDBRoot)!=NULL)
	if((this->pItOwner = pItDB->pOwner)!= NULL)
		this->pItDBObj = this->pItOwner->pDBObj;

	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		eObjType    = this->pItDBObj->pdpDep->eObjType;
		eacAccess   = this->pItDBObj->pdpDep->eacAccess;
    sServ       = this->pItDBObj->pdpDep->sServer;
		sDB         = this->pItDBObj->pdpDep->sDB;
		sOwn        = this->pItDBObj->pdpDep->sOwner;

		return this->pItDBObj->pdpDep->sObjName;
	}

	return NULL;
}

const UCHAR *CDEPS::GetNext(eREFOBJ & eObjType,CUSTR & sServ,CUSTR & sDB,CUSTR & sOwn,
                             UINT & eacAccess)
{

	if(this->pItDB == NULL || this->pItOwner == NULL || this->pItDBObj == NULL)
	{
		this->pItDB    = NULL;
		this->pItOwner = NULL;
		this->pItDBObj = NULL;

		return NULL;
	}

	if((this->pItDBObj = this->pItDBObj->pNext)==NULL)    //browse one step further
	{                                                    //in internal structure
		if((this->pItOwner = this->pItOwner->pNext)!=NULL)
			this->pItDBObj = this->pItOwner->pDBObj;
	}

	if(this->pItOwner == NULL)
	{
		if((this->pItDB = this->pItDB->pNext)!=NULL)
			if((this->pItOwner = this->pItDB->pOwner)!=NULL)
				this->pItDBObj = this->pItOwner->pDBObj;
	}
	
	//Return next data item if available
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		eObjType    = this->pItDBObj->pdpDep->eObjType;
		eacAccess   = this->pItDBObj->pdpDep->eacAccess;
    sServ       = this->pItDBObj->pdpDep->sServer;
		sDB         = this->pItDBObj->pdpDep->sDB;
		sOwn        = this->pItDBObj->pdpDep->sOwner;

		return this->pItDBObj->pdpDep->sObjName;
	}

	return NULL;
}

void CDEPS::Debug_It(FILE *fp)
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		this->pItDBObj->pdpDep->Debug(fp);
	}
}

//Copy contents of current iterator
void CDEPS::CopyIteratorDep(const CDEPS & dpCopyIt)
{
  if(dpCopyIt.IteratorIsValid()==true)
    this->AddDep(dpCopyIt.pItDBObj->pdpDep);
}

  //Does the current iterator hold a valid entry?
bool CDEPS::IteratorIsValid() const 
{
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
    return true;

  return false;
}

void CDEPS::SetHTML_LINK(const CUSTR & sLink)
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		this->pItDBObj->pdpDep->sHTLMLink2ExtDB = sLink;
	}
}

CUSTR CDEPS::GetHTML_LINK()
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		return this->pItDBObj->pdpDep->sHTLMLink2ExtDB;
	}
  return ("");
}

void CDEPS::SetNewOwner(const CUSTR & sNewOwner)
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		this->pItDBObj->pdpDep->sNewOwner = sNewOwner;
	}
}

CUSTR CDEPS::GetNewOwner()
{
	//Work only with valid Iterator
	if(this->pItDB != NULL && this->pItOwner != NULL && this->pItDBObj != NULL)
	{
		return this->pItDBObj->pdpDep->sNewOwner;
	}
  return ("");
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>> END OF ITERATOR >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//Sometimes failure is possible and part of the algorithm, and this is ok
//as long as failures are corrected
//Here, we wrongly assumed an owner of a database object, to correct this
//we erase this reference here and insert the correct reference elsewhere
void CDEPS::RemoveAssumedDeps(const UCHAR *pcServ,const UCHAR *pcDB,const UCHAR *pcOwner, const UCHAR *pcDBObj)
{
	this->pItDB = NULL, this->pItOwner = NULL, this->pItDBObj = NULL; //Reset Iterator

	DBND *pDBSrcRtIn = this->pDBRoot, *pLastDBSrcRt = NULL;
	OWND *pOWRoot, *pLastOWRoot = NULL;
	OBJND *pOBJRoot, *pLastOBJRoot = NULL;
	                                             //This is surprisingly simple:
	while(pDBSrcRtIn != NULL)                   //We browse the source collection and
	{                                          //note each node that matches the condition
	  if(s::scmp_ci2(pDBSrcRtIn->sDB,pDBSrcRtIn->sServer,  pcDB,pcServ) == 0)
		{
			pOWRoot = pDBSrcRtIn->pOwner;			
			while(pOWRoot != NULL)
			{
				if(pOWRoot->sOwner.scmp_ci(pcOwner) == 0)
				{
					pOBJRoot = pOWRoot->pDBObj;
					while(pOBJRoot != NULL)
					{ //Now free the node we are currently looking at and leave dependency graph consistently
						if(s::scmp_ci(pOBJRoot->sObjName ,pcDBObj)==0) //DB.Owner.ObjName path
            {
              OBJND *pFreeThisOBJRoot = pOBJRoot;
              iEntrySz--;
            
              if(pLastOBJRoot != NULL)
              {
                pLastOBJRoot->pNext = pFreeThisOBJRoot->pNext; //Close chain, seperate node
                delete pFreeThisOBJRoot;                      //and free memory
                return;
              }
              //Propagate into one level up (from ObjectName to Owner)...
              pOWRoot->pDBObj = pFreeThisOBJRoot->pNext; //Close chain, seperate node
              delete pFreeThisOBJRoot;                  //and free memory

              //Now if there is no other object under this owner, we can free the owner as well
              if(pOWRoot->pDBObj == NULL)
              {
                if(pLastOWRoot != NULL)
                {
                  pLastOWRoot->pNext = pOWRoot->pNext;
                  delete pOWRoot;
                  return;
                }
                //If there is no lastptr we need to propagate into db level and reset nextptr
                pDBSrcRtIn->pOwner = pOWRoot->pNext;
                delete pOWRoot;

                //Now, if there is no owner under this db entry, we might as well free the db entry
                if(pDBSrcRtIn->pOwner == NULL)
                {
                  if(pLastDBSrcRt != NULL)
                    pLastDBSrcRt->pNext = pDBSrcRtIn->pNext;
                  else
                    this->pDBRoot = pDBSrcRtIn->pNext;

                  delete pDBSrcRtIn;
                  return;
                }
                return;
              }
              return;
            }
		
						pLastOBJRoot = pOBJRoot;
						pOBJRoot     = pOBJRoot->pNext;
					}
				}
        pLastOWRoot = pOWRoot;
        pOWRoot     = pOWRoot->pNext;
			}
		}
		pLastDBSrcRt = pDBSrcRtIn;      //Remember Last Node visited before current
    pDBSrcRtIn = pDBSrcRtIn->pNext;
	}
}
