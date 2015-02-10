
#include "ctabcol.h"
#include "custr.h"
#include "defs.h"
#include "cerr.h"

#include "prettyprint.h"
#include "parse.h"

#include "copts.h"

// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->

// Initialize work sets for hash function
#define INIT_HSZ 1024
#define INIT_HSZ_MASK (INIT_HSZ-1)
#define H_SEED 111

CTABCOL::~CTABCOL()
{
	//DEBUG();    Debug collection before destruction takes place
	FreeHTTab();    //Get rid of hash table of table/view collection
}

CTABCOL::CTABCOL():iTabRootSz(0),pphTabRt(NULL)    //Hash table code
{
	int i;

	if((pphTabRt = new H_TAB *[INIT_HSZ])==NULL)
		return;

	iTabRootSz = INIT_HSZ;
	for(i=0;i<INIT_HSZ;i++)
	{
		pphTabRt[i] = NULL;
	}
}

void CTABCOL::CopyThis(const CTABCOL & tbIn)
{
  this->FreeHTTab();                           //Reset Hash table collection settings

  if(tbIn.pphTabRt == NULL || tbIn.iTabRootSz <= 0) return; //Nothing to copy here :(

	if((pphTabRt = new H_TAB *[tbIn.iTabRootSz])==NULL) //Construct new table and copy each node
		return;

	this->iTabRootSz = tbIn.iTabRootSz;
	for(int i=0;i<this->iTabRootSz;i++)
	{
		pphTabRt[i] = NULL;

    H_TAB **pphInsHere = &pphTabRt[i], *hpTab = tbIn.pphTabRt[i];

    while(hpTab != NULL )
    {
      if(hpTab->ptbTable != NULL)
      {
        *pphInsHere = new H_TAB(hpTab->ptbTable); //Copy this table related info
        pphInsHere = &((*pphInsHere)->pNext);       //Look at end in destination
      }

      hpTab=hpTab->pNext; //Look at next node in source collection
    }
	}
}

// Copy Constructors and Equal Operators
CTABCOL::CTABCOL(const CTABCOL & tbIn):iTabRootSz(0),pphTabRt(NULL)
{CopyThis(tbIn);}

CTABCOL & CTABCOL::operator=(const CTABCOL & tbIn){CopyThis(tbIn); return *this;}
CTABCOL & CTABCOL::operator=(const CTABCOL *pDB)
{
  if(pDB != NULL) *this = *pDB;

	return *this;
}

//Hashing function is optimized for hashtable-size (INIT_HSZ)
//with a power of 2,
//re-adjust that size to suit your needs (in power of 2 steps)!
//============================================================>
unsigned int CTABCOL::hGetKey(const UCHAR *word) const
{
	char    c;
	unsigned int h;

	//Use each letter of the string to hash into a certain bucket
	for( h=H_SEED ; ( c=s::cToUpper(*word))!='\0' ; word++ )
	{
		if((c & 2) == 0)
			h ^= ( (h << 5) + c );
		else
			h ^= ( c + (h >> 2) );
	}

	return(h & INIT_HSZ_MASK);   //Cut-down to size of actually available buckets
}
#undef H_SEED
#undef INIT_HSZ_MASK

// Free information and nullify pointer
void CTABCOL::FreeHTTab()
{
	int i;
  H_TAB *phTmp, *phTmp1;

	if(pphTabRt == NULL) return;

	for(i=0;i<INIT_HSZ; i++)
	{ 
		phTmp = pphTabRt[i];
		while(phTmp != NULL)
		{
			phTmp1 = phTmp->pNext;
			delete phTmp;          //Destroy node data via destrutor

			phTmp = phTmp1;
		}
	}

	delete [] pphTabRt; //Free root structure
	pphTabRt=NULL;
}

//
// :'"#$;'"#$;:"#$;:'#$;:'"$;:'"#;:'"#$%:'"#$%;:'"#$%;:'"#$
// ;:'"#$%:'"#$%;:'"#$;'"#$;:"#$;:'#$;:'"$;:'"#;:'"#;:'"#;:
// :'"#$%;:'"#$;'"#$;:"#$;:'#$;:'"$;:'"#;:'"#$%:'"#$%:'"#$%
// 

void CTABCOL::DEBUG(FILE *fp, const CUSTR sTabOwner,const CUSTR sTabName) const
{
  int i,iCnt, iMax=0, iCntZero=0;
  H_TAB *phlTmp;

  if(sTabOwner.slen()<=0 && sTabName.slen()<=0)
  {
    fprintf(fp, "TAB COLLECTION - DEBUG HASH:\n");

    for(i=0;i<INIT_HSZ; i++)
    { 
      //Count nodes per hash table
      for(phlTmp = pphTabRt[i], iCnt=0;phlTmp != NULL; iCnt++, phlTmp = phlTmp->pNext)
      ;

      if(iMax < iCnt) iMax = iCnt;

      if(iCnt == 0) iCntZero++;

      fprintf(fp, "%i,", iCnt);

      if((i&31)==31) fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
    fprintf(fp, "Max Slot: %i Zero Count: %i\n", iMax, iCntZero);
  }

  //Debug each node in each bucket...
  for(i=0;i<INIT_HSZ; i++)
  { 
    for(phlTmp = pphTabRt[i], iCnt=0;phlTmp != NULL; iCnt++, phlTmp = phlTmp->pNext)
    {
      if(sTabOwner.slen()<=0 && sTabName.slen()<=0)
        phlTmp->ptbTable->Debug(fp);
      else
      {
        if(phlTmp->ptbTable->GetName().scmp_ci(sTabName)==0 &&
           phlTmp->ptbTable->GetOwner().scmp_ci(sTabOwner)==0)
          phlTmp->ptbTable->Debug(fp);
      }
    }
  }
}

bool CTABCOL::bMatchNode(const UCHAR *pcServer,const UCHAR *pcDB
                        ,const UCHAR *pcOwner,const UCHAR *pcName, H_TAB *phtbNode) const
{
  if(pcServer == NULL || pcDB  == NULL || pcOwner == NULL || pcName == NULL || phtbNode == NULL)
  {
    if(pcServer == NULL && pcDB == NULL && pcOwner == NULL && pcName == NULL && phtbNode == NULL)
      return true;

    return false;
  }

	if(phtbNode->sServer.scmp_ci(pcServer) == 0 &&
     phtbNode->sDB.scmp_ci(pcDB)         == 0 &&  //return the match, if any
		 phtbNode->sOwner.scmp_ci(pcOwner)   == 0 &&
		 phtbNode->sName.scmp_ci(pcName)     == 0)     return true;

	return false;
}

//Find a TAB_TREE node with pcKey as name, in other words:
//Find the main entry for a Table/View called pcKey
//-------------------------================*****00000000000>
CTABCOL::H_TAB *CTABCOL::FindTabNode(H_TAB **pphRoot
                                    ,const UCHAR *pcServer,const UCHAR *pcDB
                                    ,const UCHAR *pcOwner,const UCHAR *pcName) const
{
	//hash name and attempt to find target
	H_TAB *phlTmp = pphRoot[this->hGetKey(pcName)];

	for( ; phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
    if(bMatchNode(pcServer,pcDB,pcOwner,pcName, phlTmp)==true) return phlTmp;
	}

	return NULL;
}

//Resolve unknown table objects (couldn't decide for view or table up to here)
//
//We browse all dependencies within the tabcol and attempt to decide for TABLE
//or VIEW (by looking up objects in tabcol)
//Zzc======================================================================-zZ
void CTABCOL::ResolveUnknownTables(H_TAB **pphTabIn, const UCHAR *pcServer, const UCHAR *pcDB)
{
	if(pphTabIn == NULL) return;
	
	const UCHAR *pc;
	UINT eatAccess;
	eREFOBJ eObjType;
	CUSTR sServer,sDB, sUser;

	//Count nodes per hash table
	for(int i=0;i<INIT_HSZ; i++)
	{
		H_TAB *phlTmp;
		for(phlTmp = pphTabIn[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			sServer  ="", sDB  ="", sUser="";
			pc = phlTmp->ptbTable->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
		
			while(pc != NULL)
			{
        if(s::scmp_ci2(sDB,sServer, pcDB,pcServer) == 0) //Leave external objects as they are
				{                                               //Resolve unknown tables (view or table, don't know)
					if(eObjType == eREFTABVIEW)
					{
						H_TAB *ptbTmpTab;
						if((ptbTmpTab = FindTabNode(pphTabIn, sServer, sDB, sUser, pc)) != NULL)
						{
              phlTmp->ptbTable->m_dpDBObjDeps.SetObjType(phlTmp->ptbTable->GetObjType());
						}
					}

          //(Note: 1st alternativ may be obsolete <- see scmp_ci2 further up)
          if(s::scmp_ci2(sDB,sServer, pcDB,pcServer)!= 0) //Access to an external db object
						phlTmp->ptbTable->m_dpDBObjDeps.SetAccType(eAT_ACC_EXT);
					else
					{                              //Thats a reference to an internal db object
						phlTmp->ptbTable->m_dpDBObjDeps.UnSetAccType(eAT_ACC_EXT);
						phlTmp->ptbTable->m_dpDBObjDeps.UnSetAccType(eAT_ACC_UNDEF);
	
	                                        //Have we got a definition for this???
						H_TAB *pFindTab;
						if((pFindTab = FindTabNode(pphTabIn, sServer, sDB, sUser, pc)) == NULL)
							phlTmp->ptbTable->m_dpDBObjDeps.SetAccType(eAT_ACC_UNDEF);
						else
						{
							if(pFindTab->ptbTable->GetObjType() == eREFUNKNOWN)
								phlTmp->ptbTable->m_dpDBObjDeps.SetAccType(eAT_ACC_UNDEF);
						}
					}
				}   //END of xternal objects

				pc = phlTmp->ptbTable->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB, sUser,eatAccess);
			}
		}
	}
}
#undef INIT_HSZ

// Insert table/view db object dependency into hash table collection
// If necessary, db object is created from given name/db,type
// Dependencies are added, if supplied
// Reference to hash node is always returned
//----------------------------------------------------------------->
CTABCOL::H_TAB *CTABCOL::AddTableDep(const UCHAR *pcServer,const UCHAR *pcDB
                                    ,const UCHAR *pcOwner,const UCHAR *pcTabName
                                    ,const eREFOBJ etbObjType,CDEP *pdpDBObjDep)
{
	int hval = this->hGetKey(pcTabName);

	if(pphTabRt[hval] == NULL)
	{
		if((pphTabRt[hval] = new H_TAB(pcServer, pcDB, pcOwner, pcTabName, etbObjType))!=NULL)
			pphTabRt[hval]->ptbTable->m_dpDBObjDeps.AddDep(pdpDBObjDep);

		return pphTabRt[hval];
	}

	H_TAB **ppFind; //Find point of insertion/change...
	
	ppFind = &pphTabRt[hval];

	while(*ppFind != NULL)           //...in list of nodes
	{
    if(bMatchNode(pcServer,pcDB,pcOwner,pcTabName,(*ppFind))==true)
			break;
		else
			ppFind = &((*ppFind)->pNext);
	}

	if(*ppFind == NULL)               //Insert completely new entry at end of list
		*ppFind = new H_TAB(pcServer, pcDB, pcOwner, pcTabName, etbObjType);

	if(*ppFind != NULL)              //Add dependency entry
	{
		if((*ppFind)->ptbTable->GetObjType() == eREFUNKNOWN)
			(*ppFind)->ptbTable->SetObjType(etbObjType);

		if((*ppFind)->ptbTable->GetOwner().slen() <= 0 && s::slen(pcOwner) > 0)
			(*ppFind)->ptbTable->SetOwner(pcOwner);

		if(pdpDBObjDep != NULL)
			(*ppFind)->ptbTable->m_dpDBObjDeps.AddDep(pdpDBObjDep);
	}
	else //We should never get here unless somethings wrong...
	{
		CUSTR s("Object "); s += pcTabName, s += "not found and or not inserted";
		throw CERR(s, ERR_INTERNAL, "CTABCOL::AddTableDep");
	}

	return *ppFind;
}

// <^> PUBLIC FUNCTIONS <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^>
//  <^> PUBLIC FUNCTIONS <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^>
//   <^> PUBLIC FUNCTIONS <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^>

#define _SRC "CTABCOL::AddTableDepend"
bool CTABCOL::AddTableDepend(const UCHAR *pcServer,const UCHAR *pcDB
                            ,const UCHAR *pcOwner,const UCHAR *pcTabName
                            ,const eREFOBJ etbObjType, CDEP *pdpDBObjDep)
{
	if(s::slen(pcTabName) <= 0)
		throw CERR("Required \"name\" value for table/view not found!", ERR_SYNTAX, _SRC);

	if(s::slen(pcDB) <= 0)
	{
		CUSTR sErr("DB Objb: \'");
    sErr += pcTabName, sErr += "\' without DB name: \'", sErr += pcDB, sErr += "\' NOT SUPPORTED!";
		throw CERR(sErr, 8, _SRC);
	}

	if(AddTableDep(pcServer, pcDB, pcOwner, pcTabName, etbObjType, pdpDBObjDep) != NULL)
		return true;

  return false;	
}
#undef _SRC

//Views can have a table like column definition and sql source
//This function parses the sql source to determine dependencies of this view
//=========================================================================>
void CTABCOL::ParseTabFile(const CUSTR & sFPath, const CUSTR & sFName
                          ,const UCHAR *pcServer,const UCHAR *pcDB
                          ,const UCHAR *pcOwner,const CUSTR & sName
                            ,COpts *pPrOpts)
{
	if(sName.slen() <= 0)
		throw CERR("Required \"name\" value for table/view not found!", ERR_SYNTAX, "CTABCOL::ParseTabFile");

	if(s::slen(pcDB) <= 0 || s::slen(pcServer) <= 0)
	{
		CUSTR sErr("DB Objb: \'");
    sErr += sName + "\' without DB/Server name: \'", sErr += pcDB, sErr += "\' NOT SUPPORTED!";
		throw CERR(sErr, 8, "CTABCOL::ParseTabFile");
	}

	H_TAB *phtTabl;

	if((phtTabl = AddTableDep(pcServer,pcDB,pcOwner,sName, eREFVIEW, NULL))==NULL)
	{
		CUSTR s("ERROR: CTABCOL::ParseTabFile Can not add into table collection: ");
		s += sName;
		throw CERR(s);
	}

	ParseViewFile(sFPath, sFName, *phtTabl->ptbTable, pPrOpts, *this, pcServer, pcDB);
}

//Add table by CTAG name
//The type of CTAG is expected to be "tab" or "view"
//This tag must have a name property signifiying the name of the table (e.g. name="course")
//If the tag has Children of the type col, this will be accepted as column descriptors
//========================================================================================>
void CTABCOL::AddTableView(CTAG & ctgTable, const UCHAR *pcServer, const UCHAR *pcDB,
                           const UCHAR *pcOwner, const eREFOBJ etbObjType)
{
	CUSTR sName, sVal;
	int i;

	sName = ctgTable.Props.sGetVal("name");

  CDP::Handle_Requi_Params(pcServer,pcDB,pcOwner,sName,"CTABCOL::AddTableView");
	
	H_TAB *pTbTable;

	if((pTbTable = AddTableDep(pcServer,pcDB,pcOwner,sName, etbObjType,NULL))==NULL)
	{
		CUSTR s("Unable to add into table collection: ");
		s += ctgTable.Props.sGetVal("name");
		throw CERR(s,ERR_INTERNAL, "CTABCOL::AddTable");
	}

	for(i=0;i<ctgTable.Props.Size();i++) //additional props except for name & src string
	{
		ctgTable.Props.GetProp(i, sName, sVal);

		if(sName != "name" && sName != "src")
			pTbTable->ptbTable->m_prpTableProps.AddProp(sName, sVal);
	}

	for(i=0;i<ctgTable.ChildTag_Sz();i++) //Add columns from XML child tags of root
	{
		if(ctgTable[i].getTagName() == "col")
		{
			pTbTable->ptbTable->AddColumn(ctgTable[i].Props);
		}

		if(ctgTable[i].getTagName() == "tabsize")
		{
			CUSTR sSz, sPart;
			int iSz;

			sSz   = ctgTable[i].Props.sGetVal("index");
			sPart = ctgTable[i].Props.sGetVal("indexpart");

			if(sSz.slen()>0 && sPart.slen()>0)
			if((iSz = Convert2KB(sSz, sPart))>=0)
				pTbTable->ptbTable->iIndexSz = iSz;

			sSz   = ctgTable[i].Props.sGetVal("data");
			sPart = ctgTable[i].Props.sGetVal("datapart");

			if(sSz.slen()>0 && sPart.slen()>0)
			if((iSz = Convert2KB(sSz, sPart))>=0)
				pTbTable->ptbTable->iDataSz = iSz;
		}
	}
}

//Find Table entry through human readable qualifiers: Name and database
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CTAB & CTABCOL::GetTable(const UCHAR *pcServer,const UCHAR *pcDB
                        ,const UCHAR *pcOwner,const UCHAR *pcName) const
{
	H_TAB *pTbTable;
	
	if((pTbTable = FindTabNode(pphTabRt, pcServer,pcDB,pcOwner,pcName))==NULL)
	{
		CUSTR s("Can not find table: ");
		s+= pcServer, s+= ".";
		s+= pcDB,     s+= ".";
		s += pcOwner, s+= ".";
		s += pcName;
		throw CERR(s, 0, "CTABCOL::GetTable");
	}

	return *pTbTable->ptbTable;
}

// *=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=>
// Add information about trigger and events into collection of tables      <
// *=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=*=+*+=+*=>
void CTABCOL::AddTrigger2Tab(const UCHAR *pcServer, const UCHAR *pcDB,const UCHAR *pcOwner
                            ,const UCHAR *pcTabName, const UCHAR *pcTrigName,EVENT_CL evOnEvents)
{
	H_TAB *thisNode;

  CDP::Handle_Requi_Params(pcServer,pcDB,pcOwner,pcTabName,"CTABCOL::AddTrigger2Tab");

	//Create table in collextion, if necessary, and add trigger and fire-constrain
	if((thisNode = AddTableDep(pcServer, pcDB, pcOwner,pcTabName, eREFTABLE, NULL))!=NULL)
	{
		if(evOnEvents == 0 || pcTrigName == NULL) return;

		thisNode->ptbTable->AddTrigEvents(pcTrigName, evOnEvents);
	}

	return;
}

//Add a classification for this table
void CTABCOL::AddTableClass(const CUSTR & sServer, const CUSTR & sDB
                           ,const CUSTR & sOwner,const CUSTR & sName, const CUSTR & sClassify)
{
  H_TAB *pNode = FindTabNode(this->pphTabRt,sServer,sDB,sOwner,sName);

  if(pNode != NULL)
    pNode = AddTableDep(sServer,sDB,sOwner,sName,eREFTABVIEW,NULL);

  if(pNode == NULL) return; //Exit quitly (this should never ever ever happen though)

  if(pNode->ptbTable != NULL)
    pNode->ptbTable->m_csClasses.AddString(sClassify);
}

//==========================================================================>
//Collect references to unknown tables/vies database objects
//Parameter:
//iFlagCollect = 0 collect all unknown table/view objects (internal/external)
//iFlagCollect = 1 collect only internal unknown table/view objects
//                 sDB == ptbTable->sDB
//
//iFlagCollect = 2 collect only external unknown table/view objects
//                 sDB != ptbTable->sDB
//               |
//               +-> if pitDBs is set, we return only matched entries in pitDBs
//
// Return: Collection of references (eOBJUNKNOWNTABLE, sName, sDB pairs)
//         to above objects
//!!! Caller must free returned collection of database object references !!!
//==========================================================================>
#define _SRC "CTABCOL::FindUnknownObjects"
CDEPS *CTABCOL::FindUnknownObjects(int iFlagCollect, const CUSTR sServer,const CUSTR sDB, CPITEM *pitDBs)
{
	if(pphTabRt == NULL) return NULL;

	CDEPS *pdpRet = NULL;

	if((pdpRet = new CDEPS())== NULL) return NULL;

	//Count nodes per hash table
	for(int i=0;i<iTabRootSz; i++)
	{
		for(H_TAB *phlTmp = pphTabRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
      CTAB *pTab = phlTmp->ptbTable;
      if(pTab == NULL) continue;

			if(pTab->GetObjType() == eREFUNKNOWN)
			{
				switch(iFlagCollect)
				{
					case 0:           //Add all references of any kind (internal/external)
						pdpRet->AddDep(pTab->GetServer(),pTab->GetDB(),pTab->GetOwner(),pTab->GetName()
                          ,eREFTABVIEW,eAT_UNKNOWN,0);
						break;
					case 1:                                 //Add only internal references
            if(s::scmp_ci2(sDB,sServer,  pTab->GetDB(),pTab->GetServer())==0)
						pdpRet->AddDep(pTab->GetServer(),pTab->GetDB(),pTab->GetOwner(),pTab->GetName()
                          ,eREFTABVIEW,eAT_UNKNOWN,0);
						break;
					case 2:                                 //Add only external references
            if(s::scmp_ci2(sDB,sServer,  pTab->GetDB(),pTab->GetServer()) != 0)
            {
              if(pitDBs == NULL)
                pdpRet->AddDep(pTab->GetServer(),pTab->GetDB(),pTab->GetOwner(),pTab->GetName()
                              ,eREFTABVIEW,eAT_UNKNOWN,0);
              else
              { //Add dependency only, if it points into one of these DBs (otherwise skip)
                if(pitDBs->FindDBItem(pTab->GetServer(), pTab->GetDB()) == true)
						      pdpRet->AddDep(pTab->GetServer(),pTab->GetDB(),pTab->GetOwner(),pTab->GetName()
                                ,eREFTABVIEW,eAT_UNKNOWN,0);
              }
            }
						break;

					default:
						throw CERR("Unknown Parameter", ERR_INTERNAL, _SRC);
				}
			}
		}
	}

	return pdpRet;
}
#undef _SRC

//Resolve inbound/outbound dependencies across Database limits
UINT CTABCOL::ResolveForeignDBDeps(const CUSTR sAbsOutputPath, const THIS_DB thisDB, CDEPS *pdpDB)
{
  if(pdpDB == NULL) return 0;

  const UCHAR *pc;
  UINT eatAccess, iRet=0;
  eREFOBJ eObjType, eTabType;
  CUSTR sServer,sDB,sUser,sNewOwner;

  pc = pdpDB->GetFirst(eObjType,sServer,sDB, sUser,eatAccess);

  while(pc != NULL)
  {
    sNewOwner = "";

    //Attempt to look up this database object, if it is a table/view or tabview
    if(eObjType == eREFTABVIEW || eObjType == eREFTABLE || eObjType == eREFVIEW)
    if(s::scmp_ci2(thisDB.sDB,thisDB.sServer, sDB, sServer) == 0)
    {
      const H_TAB *pTab=NULL;

      if((pTab = FindTabNode(pphTabRt, sServer, sDB, sUser, pc))== NULL)
      {
        //Second chance: attempt match under alternative user
        if(FindUniqueOwnerOfKnownTab(sServer, sDB, pc, sNewOwner, eTabType)==true)
        {
          pTab = FindTabNode(pphTabRt, sServer, sDB, sNewOwner, pc);
          //printf("Re-aligning %s.%s.%s.%s to %s.%s.%s.%s\n"
          //  ,(const char *)sServer,(const char *)sDB,(const char *)sUser, pc
          //  ,(const char *)sServer,(const char *)sDB,(const char *)sNewOwner,pc);
        }
      }

      if(pTab != NULL)
      if(pTab->ptbTable != NULL)
      if(pTab->ptbTable->GetObjType() == eREFTABLE || pTab->ptbTable->GetObjType() == eREFVIEW)
      {
        iRet++;
        CUSTR sLink(sAbsOutputPath);
        sLink += pTab->ptbTable->GetTargFileName();

        pdpDB->SetHTML_LINK(sLink);

        if(sNewOwner.slen() > 0)
          pdpDB->SetNewOwner(sNewOwner); //Link is valid for this alternative owner

        pdpDB->SetObjType(pTab->ptbTable->GetObjType());
        //printf("Resolving ref for: %s.%s.%s\n                  -> %s\n"
        //  ,(const char *)sDB,(const char *)sUser,pc,(const char *)sLink); //Debugging loop...
      }
    }

    pc = pdpDB->GetNext(eObjType,sServer,sDB, sUser,eatAccess);
  }

  return iRet;
}

//Resolve inbound/outbound dependencies across Database limits (Step 2)
//This time we go the other way around, we browse through the unknowns in tabcol
//and attempt to find a resolved link in the CDEPS collection
//==============================================================================>
UINT CTABCOL::SetForeignDBDeps(const THIS_DB thisDB, CDEPS *pdpDB)
{
  UINT iRet=0;
	if(pphTabRt == NULL || pdpDB == NULL) return 0;

	for(int i=0;i<iTabRootSz; i++)
	for(H_TAB *phlTmp = pphTabRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
    if(phlTmp->ptbTable==NULL) continue;
    CTAB *pTab = phlTmp->ptbTable;
	  const UCHAR *pc;
	  UINT eatAccess, iRet=0;
	  eREFOBJ eObjType;
	  CUSTR sServer, sDB, sUser, sNewUser;

	  pc = pTab->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);

	  while(pc != NULL)
	  {
      //Attempt to look up this database object, if it is an unknowntable
      if(eObjType == eREFTABVIEW || eObjType == eREFVIEW)
      if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,  sDB,sServer)!=0)
      {
        CUSTR sLinkPath;
        eREFOBJ eType;
        UINT eAccType;

        if(pdpDB->FindDepByName(sServer,sDB,sUser,pc,sLinkPath, eType, eAccType,sNewUser)==true)
        {
          if(sNewUser.slen() <= 0)
          {
            pTab->m_dpDBObjDeps.SetHTML_LINK(sLinkPath);
            pTab->m_dpDBObjDeps.SetObjType(eType);
            iRet++;
          }
          else //TODO: Re-algning references if user changes from one DB to another
          {    //      via select statement in a view
          
          
          }
        }
      }
		  pc = pTab->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eatAccess);
	  }
	}

  return iRet;
}

#define COL_ALIGN "align=\"right\" valign=\"top\""
#define COL_ALIGN1 "align=\"left\" valign=\"top\""

//Print Information about database objects accessing this table/view
void CTABCOL::HTML_PrintTableModifs(const THIS_DB thisDB, FILE *fpOut,
                                    const UINT eaccMask
                                   ,CPRCCOL & prcPrcs
                                   ,CTAB & tbTab, const CUSTR sOutputPath
                                   ,const char *pcTBL_ID,const char *pcTDHead_ID,const char *pcTD_ID
                                   ,bool bTabHeader)
{
	UCHAR pcKey[NUM_SZ];         //String Representation of Key to id table+Access

	if(bTabHeader == true)
	{
		fprintf(fpOut, "\n<table %s>\n", pcTBL_ID);
		fprintf(fpOut, "<thead><tr>\n");
		fprintf(fpOut, "<TD %s %s>Database Object</TD>\n", pcTDHead_ID, COL_ALIGN1);
		fprintf(fpOut, "<TD %s %s>Access</TD>\n", pcTDHead_ID, COL_ALIGN1);
		fprintf(fpOut, "</tr></thead>\n");
	}

	eREFOBJ eObjType;
	UINT eacAccess;
	CUSTR sServer,sDB, sOwner, sName;
	const UCHAR *pcName;
  int iCnt;

	try
	{
	//Cycle through dependencies and print their access type (if they are in Mask)
	for(pcName = tbTab.m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sOwner, eacAccess),iCnt=0;
      pcName != NULL;
	    pcName = tbTab.m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sOwner,eacAccess),iCnt++)
	{
		if NFLAG(eacAccess,eaccMask) continue; //Filter relevant entries

		bool bPrint=false;

		CUSTR s;
		if(eObjType == eREFVIEW)
    {
      if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,   sDB,sServer)==0)
			  s = this->GetTargFilename(sServer,sDB,sOwner,pcName);
      else
      {
        s  = s::sGetPath(tbTab.m_dpDBObjDeps.GetHTML_LINK());   //Get Path to DB External Object
        s  = s::GetRelPath(sOutputPath, s);
        s += s::sGetFileName(tbTab.m_dpDBObjDeps.GetHTML_LINK());
      }
    }
		else
		if(eObjType == eREFPROC || eObjType == eREFTRIG)
    {
      if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,  sDB,sServer)==0)
  			s = prcPrcs.GetProc(sServer,sDB,sOwner,pcName).GetTargFileName();
      else
      {
        s  = s::sGetPath(tbTab.m_dpDBObjDeps.GetHTML_LINK());   //Get Path to DB External Object
        s  = s::GetRelPath(sOutputPath, s);
        s += s::sGetFileName(tbTab.m_dpDBObjDeps.GetHTML_LINK());
      }
    }

		if(bTabHeader == false)
		{
			if(iCnt > 0) fprintf(fpOut,"<tr>");
		}
		else
			fprintf(fpOut,"<tr>");

		//Link to file database object accessing this table
		fprintf(fpOut,"<td %s %s><a href=\"%s\">",pcTD_ID, COL_ALIGN1,(const char *)s);
		CUSTR sPrint = FormatUsefulDBObjRef(sServer,sDB,sOwner,pcName, thisDB);
    CDP::HTML_Color_Obj(fpOut, eObjType, sPrint);
		fprintf(fpOut,"</a></td>");

		if FLAG(eAT_SELECT, eacAccess) //Implementing SELECT on this view/table ???
		{
			bPrint=true;

			fprintf(fpOut, "<td %s %s>", pcTD_ID, COL_ALIGN1); //Print Data Modifier for UPDATES

			//Get table key and format anker for modifier (update,delete,insert)
			FormatTableKey(hlTableFrom, tbTab.GetID(), &pcKey[0], NUM_SZ);
			if(pcKey[0] != '\0')
				fprintf(fpOut, "<a href=\"%s#%s\">SELECT</a>", (const char *)s,pcKey );
			else
				fprintf(fpOut, "SELECT"); //Print Data Modifier for SELECT
		}

		if FLAG(eAT_UPDATEStats, eacAccess) //Implementing an Update statistics ?
		{
			if(bPrint == false)
			{
				fprintf(fpOut, "<td %s %s>", pcTD_ID, COL_ALIGN1); //Print Data Modifier for UPDATES
				bPrint=true;
			}
			else
				fprintf(fpOut, ", ");

			//Get table key and format anker for modifier (update,delete,insert)
			FormatTableKey(hlTableUpdStats, tbTab.GetID(), &pcKey[0], NUM_SZ);
			if(pcKey[0] != '\0')
				fprintf(fpOut, "<a href=\"%s#%s\">UPDATE STATISTICS</a>", (const char *)s, pcKey);
			else
				fprintf(fpOut, "UPDATE STATISTICS"); //Print Data Modifier for INSERTION
		}

		if FLAG(eAT_UPDATE, eacAccess) //Implementing UPDATE on this tabele/view?
		{
			if(bPrint == false)
			{
				fprintf(fpOut, "<td %s %s>", pcTD_ID, COL_ALIGN1); //Print Data Modifier for UPDATES
				bPrint=true;
			}
			else
				fprintf(fpOut, ", ");

			//Get table key and format anker for modifier (update,delete,insert)
			FormatTableKey(hlTableUpd, tbTab.GetID(), &pcKey[0], NUM_SZ);
			if(pcKey[0] != '\0')
				fprintf(fpOut, "<a href=\"%s#%s\">UPDATE</a>", (const char *)s, pcKey);
			else
				fprintf(fpOut, "UPDATE"); //Print Data Modifier for INSERTION
		}

		if FLAG(eAT_INSERT, eacAccess) //Implementing INSERT on this tabele/view?
		{
			if(bPrint == false)
			{
				fprintf(fpOut, "<td %s %s>", pcTD_ID, COL_ALIGN1); //Print Data Modifier for UPDATES
				bPrint=true;
			}
			else
				fprintf(fpOut, ", ");

			//Get table key and format anker for modifier (update,delete,insert)
			FormatTableKey(hlTableIns, tbTab.GetID(), &pcKey[0], NUM_SZ);
			if(pcKey[0] != '\0')
				fprintf(fpOut, "<a href=\"%s#%s\">INSERT</a>", (const char *)s, pcKey );
			else
				fprintf(fpOut, "INSERT");
		}

		if FLAG(eAT_DELETE, eacAccess) //Implementing DELETE on this tabele/view?
		{
			if(bPrint == false)
			{
				fprintf(fpOut, "<td %s %s>", pcTD_ID, COL_ALIGN1); //Print Data Modifier for UPDATES
				bPrint=true;
			}
			else
				fprintf(fpOut, ", ");

			FormatTableKey(hlTableDel, tbTab.GetID(), &pcKey[0], NUM_SZ);
			if(pcKey[0] != '\0')
				fprintf(fpOut, "<a href=\"%s#%s\">DELETE</a>", (const char *)s, pcKey);
			else
				fprintf(fpOut, "DELETE");
		}

		if FLAG(eAT_TRUNC_TAB, eacAccess) //Implementing DELETE on this tabele/view?
		{
			if(bPrint == false)
			{
				fprintf(fpOut, "<td %s %s>", pcTD_ID, COL_ALIGN1); //Print Data Modifier for UPDATES
				bPrint=true;
			}
			else
				fprintf(fpOut, ", ");

			FormatTableKey(hlTruncTable, tbTab.GetID(), &pcKey[0], NUM_SZ);
			if(pcKey[0] != '\0')
				fprintf(fpOut, "<a href=\"%s#%s\">TRUNCATE TABLE</a>", (const char *)s, pcKey);
			else
				fprintf(fpOut, "TRUNCATE TABLE");
		}

		if(bPrint == true) fprintf(fpOut, "</td>");
		
		fprintf(fpOut, "</tr>\n");
	}
	if(bTabHeader == true) fprintf(fpOut, "</table>\n");

	}
	catch(CERR e)
	{
		CUSTR s(e.getsErrSrc());
		printf("ERROR: %s in CTABCOL::::HTML_PrintTableModifs %s\n", (const char *)s, (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: CTABCOL::HTML_PrintTableModifs An error has occured!\n");
	}
}

CUSTR CTABCOL::GetTargFilename(const UCHAR *pcServer,const UCHAR *pcDB
                              ,const UCHAR *pcOwner,const UCHAR *pcName) const
{
	H_TAB *pTbTable;
	
	if((pTbTable = FindTabNode(pphTabRt, pcServer,pcDB,pcOwner,pcName))==NULL)
	{
		CUSTR s("ERROR: CTABCOL::GetTargFilename Can not find table: ");
		s += pcName;
		s += " Server: ";
		s += pcServer;
		s += " Database: ";
		s += pcDB;
		throw CERR(s);
	}

	return pTbTable->ptbTable->GetTargFileName();
}

//Create a table object for every access to an undefined table,
//the only reason why we do this is to create a unique id, which in
//turn can be used for linkage
//=================================>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CTABCOL::CreateUnknownTabEntries(CPRCCOL & prcColl)
{
	if(pphTabRt == NULL) return;

	try
	{
		CDEPS *pCol = new CDEPS();
	
		//Run through each node in each hash bucket
		for(int i=0;i<iTabRootSz; i++)
		{
			H_TAB *phlTmp;
			//Cycle through tab coll and add info for selects on external/undef objects
			//These must be views because no others can implement selects in tab coll
			const UINT eAccMask = eAT_SELECT | eAT_ACC_EXT | eAT_ACC_UNDEF;
	
			for(phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
			{
				phlTmp->ptbTable->m_dpDBObjDeps.GetDependencyFilter2(eREFVIEW,eAccMask,pCol);
			}
		}
	
		eREFOBJ eObjType = eREFUNKNOWN; //Create an object entry for each
		CUSTR sServer,sDB, sUser;      //unresolved dependency to a tab/view obj
		UINT eacAccess = eAT_UNKNOWN;
		const UCHAR *pc;              //1st Cycle through TABS-COLL
	
		for(pc = pCol->GetFirst(eObjType, sServer,sDB,sUser, eacAccess); pc != NULL;
	      pc = pCol->GetNext(eObjType, sServer,sDB,sUser, eacAccess))
		{
			AddTableDep(sServer,sDB,sUser,pc, eREFUNKNOWN,NULL);
		}
	
		delete pCol;

		if((pCol = prcColl.GetUnknownTabEntries())!=NULL) //Cycle through PROCS-COLL
		{
			for(pc = pCol->GetFirst(eObjType, sServer,sDB,sUser, eacAccess); pc != NULL;
		      pc = pCol->GetNext(eObjType, sServer,sDB,sUser, eacAccess))
			{
				if(eObjType == eREFTABVIEW)
					AddTableDep(sServer,sDB,sUser,pc, eREFUNKNOWN,NULL);
			}
			delete pCol;
		}
	}
	catch(CERR e)
	{
		printf("ERROR: CPRCCOL::CreateUnknownProcEntries %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: CPRCCOL::CreateUnknownProcEntries An error has occured!\n");
	}
}

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
CDEPS *CTABCOL::ListObj(const eREFOBJ eOBJ_TypeIn,const CUSTR sServerIn,const CUSTR sDBIn, int iGet)
{
	if(pphTabRt == NULL) return NULL;

	CDEPS *pdpRet;
	if((pdpRet = new CDEPS())==NULL) return NULL;

	//Run through each node in each hash bucket
	for(int i=0;i<iTabRootSz; i++)
	{
		for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			CTAB *pTab = phlTmp->ptbTable; //just a lazy shorthand

      //Add dependency to result collection
      if( iGet == 1
        || (iGet == 2 && s::scmp_ci2(pTab->GetDB(),pTab->GetServer()  ,sDBIn,sServerIn)== 0)
        || (iGet == 3 && s::scmp_ci2(pTab->GetDB(),pTab->GetServer()  ,sDBIn,sServerIn)== 0
            && eOBJ_TypeIn == pTab->GetObjType())
        || (iGet == 4 && eOBJ_TypeIn == pTab->GetObjType())
        )
      {
        pdpRet->AddDep(pTab->GetServer(),pTab->GetDB(),pTab->GetOwner(),pTab->GetName()
                      ,pTab->GetObjType(),eAT_UNKNOWN,0);
			}
		}
	}

	if(pdpRet->iCntDeps() <= 0)
	{
		delete pdpRet;
		pdpRet = NULL;
	}

	return pdpRet;
}
//Function to count table or view or all objects in collection
//Print eTTABLE, TVIEW,  eTUNKNOWN or all tabobjs
//===========================================================00000>
int CTABCOL::iCntTabObjects(const eREFOBJ etbTab, bool bAll)
{
	if(pphTabRt == NULL) return 0;

	int iRet=0;

	//Run through each node in each hash bucket
	for(int i=0;i<iTabRootSz; i++)
	{
		for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			//Add object to be counted
			if(phlTmp->ptbTable->GetObjType() == etbTab || bAll == true)
				iRet++;
		}
	}

	return iRet;
}

#define _SRC "CTABCOL::GetUnRefedTabView"
//Find all table/view objects that are unreferenced in this DB
//===========================================================00000>
CDEPS *CTABCOL::GetUnRefedTabView(const eREFOBJ etbTab, bool bAll)
{
	if(pphTabRt == NULL) return NULL;

	const UINT AccMask = eAT_DELETE|eAT_TRUNC_TAB|eAT_INSERT|eAT_UPDATE \
	                   | eAT_UPDATEStats|eAT_SELECT;

	CDEPS *pdpRet;
	if((pdpRet = new CDEPS())==NULL) return NULL;

	for(int i=0;i<iTabRootSz; i++)     //Run through each node in each hash bucket
	{
		for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
      if(phlTmp->ptbTable->GetObjType() == etbTab || bAll == true)
      if(phlTmp->ptbTable->m_dpDBObjDeps.CountAccTypes(AccMask) <= 0 )
        pdpRet->AddDep(phlTmp->sServer,phlTmp->sDB
                      ,phlTmp->sOwner,phlTmp->sName,eREFPROC,eAT_UNKNOWN,0);
		}
	}

	if(pdpRet->iCntDeps() <= 0)
	{
		delete pdpRet;
		pdpRet = NULL;
	}

	return pdpRet;
}
#undef _SRC

// **** **** **** **** **** **** **** **** **** **** **** **** **** ****
// *** *** *** **** *** *** *** *** **** *** *** *** *** **** *** *** 
// *** *** *** **** *** *** *** *** **** *** *** *** *** **** *** *** 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

#define _SRC "CTABCOL::HTML_PrintTabs"
//Print information on SQL Tables in HTML output format
//=====================================================>
int CTABCOL::HTML_PrettyPrintTabs(CTABCOL & tbcTabs
                                 ,CPRCCOL & prcPrcs
                                 ,const THIS_DB & thisDB
                                 ,const CUSTR   & sOutputPath     //Path to HTML output
                                 )
{
	if(pphTabRt == NULL) return 0;

	int iCntFiles=0;

	for(int i=0;i<iTabRootSz; i++) //Run through each node in each hash bucket
	{
		for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			CTAB *pTab = phlTmp->ptbTable;

			//Do not render depndencies for external/unknown tables
			//We render them on one seperate page for all external objects
			if(pTab->GetObjType() == eREFUNKNOWN) continue;
      if(s::scmp_ci2(pTab->GetDB(), pTab->GetServer(), thisDB.sDB, thisDB.sServer)!=0) continue;

			FILE *fpOut=NULL;                 //Print Table Definition on HTML file
			CUSTR sFile;
			const UINT eaccMask = eAT_SELECT | eAT_UPDATE | eAT_INSERT | eAT_DELETE \
			                    |eAT_UPDATEStats| eAT_TRUNC_TAB; // | eAT_SELECT_BY_VIEW;

			int iCntAccess, iCntAccByView;
	
			CUSTR sSQLSrc;
			FILE *fpSQLSrc;
	
			sFile = s::CreateFileName(sOutputPath, pTab->GetTargFileName(), NULL);
			if((fpOut = fopen(sFile, "wb"))!=NULL)
			{
				iCntFiles+=1;
				if(pTab->GetObjType() == eREFVIEW) //Attach view/table header
				{
					fprintf(fpOut, "%s\n<html>\n\t<head>\n\t\t<title>View: %s.%s.%s.%s</title>\n\n",DOCTYPE_TAG
                         ,(const UCHAR *)pTab->GetServer(),(const UCHAR *)pTab->GetDB()
                         ,(const UCHAR *)pTab->GetOwner(),(const UCHAR *)pTab->GetName());
					fprintf(fpOut,"<link rel=\"shortcut icon\" href=\"res/view.jpg\">\n");
				}
				else
				{
					fprintf(fpOut, "%s\n<html>\n\t<head>\n\t\t<title>Table: %s.%s.%s.%s</title>\n\n",DOCTYPE_TAG
                         ,(const UCHAR *)pTab->GetServer(),(const UCHAR *)pTab->GetDB()
                         ,(const UCHAR *)pTab->GetOwner(),(const UCHAR *)pTab->GetName());
					fprintf(fpOut,"<link rel=\"shortcut icon\" href=\"res/table.jpg\">\n");
				}
	
				fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/style.css\">\n");

				//Collapse Drop Down info structure, if input file is available
        if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,  pTab->GetDB(),pTab->GetServer())==0)
				if(IncludeDocuFile(NULL, thisDB.sHTMLDocuPath, pTab->GetOwner(), pTab->GetName()) == true)
				{
					fprintf(fpOut, "<script type=\"text/javascript\" language=\"JavaScript\" src=\"res/dropdown.js\"></script><script type=\"text/javascript\">\n");
					fprintf(fpOut, "<!-- Begin\n");
					fprintf(fpOut, "window.onload=function() {if(document.getElementById && document.createElement) {initCollapse();}}\n");
					fprintf(fpOut, "//-->\n");
					fprintf(fpOut, "</script>\n");
				}

				CUSTR s = thisDB.sServer + "/" + thisDB.sDB;
				fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/office_xp.css\" title=\"Office XP\" />\n");
				fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenu.js\"></script>\n");
				fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenubar.js\"></script>\n");
				fprintf(fpOut, "<script type=\"text/javascript\" src=\"menuconf.js\"></script>\n");

        fprintf(fpOut, "<META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\">\n");
			  fprintf(fpOut, "<META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\">\n");

        fprintf(fpOut, "</head>\n<body onload=\"initjsDOMenu(\'%s\')\">\n", (const char *)s);

				CUSTR sUpdTrigSrc, sDelTrigSrc, sInsTrigSrc;
	
        if(pTab->DelTrig() != NULL)
          sDelTrigSrc = prcPrcs.GetProc(pTab->GetServer(),pTab->GetDB()
                                        ,pTab->GetOwner(),pTab->DelTrig()).GetTargFileName();
	
        if(pTab->InsTrig() != NULL)
          sInsTrigSrc = prcPrcs.GetProc(pTab->GetServer(),pTab->GetDB()
                                       ,pTab->GetOwner(),pTab->InsTrig()).GetTargFileName();
	
        if(pTab->UpdTrig() != NULL)
          sUpdTrigSrc = prcPrcs.GetProc(pTab->GetServer(),pTab->GetDB()
                                       ,pTab->GetOwner(),pTab->UpdTrig()).GetTargFileName();

				//Call tab pretty Print function to print itself in HTML
				pTab->HTML_PrintTable(fpOut, sOutputPath, sUpdTrigSrc, sDelTrigSrc, sInsTrigSrc,thisDB);
	
				//Call prettyprint function to attempt include from added documentation
        if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,  pTab->GetDB(),pTab->GetServer())==0)
				IncludeDocuFile(fpOut, thisDB.sHTMLDocuPath, pTab->GetOwner(), pTab->GetName());
	
				switch(pTab->GetObjType())
				{
				case eREFVIEW:                                  //>>> START OF VIEW HEADER
					iCntAccess   =pTab->m_dpDBObjDeps.CountAccTypes(eaccMask);
					iCntAccByView=pTab->m_dpDBObjDeps.CountAccTypes(eAT_SELECT_BY_VIEW);
	
					if(iCntAccess > 0 || iCntAccByView > 0)
					{
						fprintf(fpOut, "<table id=\"NoBord\" cellspacing=\"4\"><tr>\n");
		
						if(iCntAccess > 0)
							fprintf(fpOut, "<td valign=\"top\" align=\"left\">Objects Accessing View</td>\n");
		
						if(iCntAccByView > 0)
							fprintf(fpOut, "<td valign=\"top\" align=\"left\">Objects Accessed by View</td>\n");
		
						fprintf(fpOut, "</tr><tr>\n");	     //>>>>> END OF TABLE HEADER <<<
						
						//Display table trigger def and access statements refering to table
						if(iCntAccess > 0)
						{
							fprintf(fpOut, "<td valign=\"top\" align=\"left\">\n");
							HTML_PrintTableModifs(thisDB, fpOut, eaccMask, prcPrcs, *pTab, sOutputPath
                                   ,"id=\"Bord\"","id=\"BordHead\"","id=\"Bord\"");
							fprintf(fpOut, "\n</td>\n");
						}
		
						//Other Tables/Views being accessed by View
						if(iCntAccByView > 0)
						{                                                    //1st Main Cell
							fprintf(fpOut, "\n<td valign=\"top\" align=\"left\">\n");
							HTML_PrintViewAccess(fpOut,eAT_SELECT_BY_VIEW,*pTab,thisDB,tbcTabs,prcPrcs,sOutputPath);
							fprintf(fpOut, "\n\n</td>\n");
						}
	
						fprintf(fpOut, "\n\n</tr></table>\n");
					}
	
					//Second row of headers =======================================================>
					fprintf(fpOut, "<table id=\"NoBord\" cellspacing=\"4\">\n");
					fprintf(fpOut, "<tr><td valign=\"top\" align=\"left\">Table/View Definition</td>\n");
					fprintf(fpOut, "<td valign=\"top\" align=\"left\">Source code</td></tr>\n");
					fprintf(fpOut, "<tr><td valign=\"top\" align=\"left\">\n");	//>>>>> END 2nd OF HEADER
	
					pTab->HTML_PrintTableDefinition(fpOut);
	
					//Print SQL Source Code
					fprintf(fpOut, "\n</td><td valign=\"top\" align=\"left\" bgcolor=\"#DDDDDD\">\n");
	
					//TODO: Source file name & path should be split here
					sSQLSrc = pTab->sSource;

          if(sSQLSrc.slen() <= 0)
            throw CERR("File name for SQL-source code is not available!",0,_SRC);

					if((fpSQLSrc=fopen(sSQLSrc, "rb"))!=NULL)
					{
            eREFOBJ depTabType = pTab->GetObjType();
            CUSTR sAbsOutPath;
            sAbsOutPath = s::GetAbsPath(s::sGetCWD(), sOutputPath);

						HTML_HighLightSQL(prcPrcs,tbcTabs,pTab->m_dpDBObjDeps,pTab->hlPatrn,
						                  pTab->GetOwner(),pTab->GetName(),depTabType,
                              fpSQLSrc, fpOut,thisDB,sAbsOutPath);
	
						FCLOSE(fpSQLSrc)
					}
					else
						fprintf(fpOut, "FILE: %s not found.", (const char *)sSQLSrc);
	
					fprintf(fpOut, "\n\n</td></tr></table>\n");
				break;
	
				case eREFTABLE:
				case eREFTABVIEW:
				case eREFUNKNOWN:
					//>>> START OF HEADER
					iCntAccess = pTab->m_dpDBObjDeps.CountAccTypes(eaccMask);
					fprintf(fpOut, "<table id=\"NoBord\" cellspacing=\"4\"><tr>\n");
	
					if(pTab->iDataSz >=0 && pTab->iIndexSz >= 0)
					{
						//Print empty columns if access statements will be printed for dbobj
						if(iCntAccess > 0) fprintf(fpOut, "<td></td>");

						fprintf(fpOut, "<td valign=\"top\" align=\"left\">\n");	//>>>>> END OF HEADER <<<
						CUSTR str = ConvertKB2UsefulSz(pTab->iDataSz);
						fprintf(fpOut, "<small>Data %s\n", (const char *)str);

						str = ConvertKB2UsefulSz(pTab->iIndexSz);
						fprintf(fpOut, "+ index %s\n", (const char *)str);

						str = ConvertKB2UsefulSz(pTab->iIndexSz + pTab->iDataSz);
						fprintf(fpOut, "= %s</small>\n", (const char *)str);

						fprintf(fpOut, "\n\n</td></tr><tr>\n");
					}

					if(iCntAccess > 0)
						fprintf(fpOut, "<td valign=\"top\" align=\"left\">Objects Accessing Table</td>\n");

					fprintf(fpOut, "<td valign=\"top\" align=\"left\">Table Definition</td></tr><tr>\n");

					if(iCntAccess > 0) //Display access statements refering to this table
					{
						fprintf(fpOut, "<td valign=\"top\" align=\"left\">\n");	//>>>>> END OF HEADER <<<
						HTML_PrintTableModifs(thisDB, fpOut,eaccMask,prcPrcs,*pTab,sOutputPath
                                 ,"id=\"Bord\"","id=\"BordHead\"","id=\"Bord\"");
						fprintf(fpOut, "\n</td>\n");                           //1st Main Cell
					}

					fprintf(fpOut, "\n<td valign=\"top\" align=\"left\">\n"); //1st Main Cell
					pTab->HTML_PrintTableDefinition(fpOut);
					fprintf(fpOut, "\n\n</td>\n");

					fprintf(fpOut, "</tr></table>\n");
				break;
				default:
					fprintf(fpOut, "ERROR: Unknown database object %s! Discontinue rendering...\n",
												 (const UCHAR *)pTab->GetName());
				}
				fprintf(fpOut, "</body></html>\n");
				FCLOSE(fpOut);
			}
			else
				printf("CTABCOL1::HTML_PrintTabs WARNING: Unable to write file %s\n", (const char *)sFile);
		}
	}

	return iCntFiles;
}
#undef _SRC

#define COL_ALIGN "align=\"right\" valign=\"top\""
#define COL_ALIGN1 "align=\"left\" valign=\"top\""

//Print Information about database objects being accessed by this table/view
//In other words, we show a table showing the View's select statements that
//implement a read-only access to another table or view
//---------------------------------------------------------------------------->
void CTABCOL::HTML_PrintViewAccess(FILE *fpOut,const UINT eaccMask,CTAB & tbTab
                                  ,const THIS_DB thisDB, CTABCOL & tbcTabs
                                  ,CPRCCOL & prcPrcs,const CUSTR sOutputPath)
{
	try
	{
		if(tbTab.m_dpDBObjDeps.CountAccTypes(eaccMask) > 0) //Are there any deps?
		{
			UCHAR pcKey[NUM_SZ];     //String Representation of Key to id table+Access
		
			fprintf(fpOut, "\n<table id=\"Bord\">\n");
			fprintf(fpOut, "<thead><tr>\n");
			fprintf(fpOut, "<TD id=\"BordHead\" %s>Table/View</TD>\n", COL_ALIGN1);
			fprintf(fpOut, "<TD id=\"BordHead\" %s>Access</TD>\n", COL_ALIGN1);
			fprintf(fpOut, "</tr></thead>\n");
	
			eREFOBJ eObjType;
			UINT eacAccess;
			CUSTR sObjServ,sObjDB, sOwner, sName;
	
			//Cycle through dependencies and print their access type (if they are in Mask)
			for(sName = tbTab.m_dpDBObjDeps.GetFirst(eObjType, sObjServ,sObjDB,sOwner, eacAccess);
			    sName.slen() > 0;
			    sName = tbTab.m_dpDBObjDeps.GetNext(eObjType, sObjServ,sObjDB,sOwner, eacAccess))
			{
				if NFLAG(eacAccess,eaccMask) continue; //Filter relevant entries
	
        CUSTR s;
        //BUGFIX DB 2006-03-21 (Find Link to file dependency)
        if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,   sObjDB,sObjServ)==0)
				  s = this->GetTargFilename(sObjServ,sObjDB,sOwner,sName);
        else
        {
          s  = s::sGetPath(tbTab.m_dpDBObjDeps.GetHTML_LINK());   //Get Path to DB External Object
          s  = s::GetRelPath(sOutputPath, s);
          s += s::sGetFileName(tbTab.m_dpDBObjDeps.GetHTML_LINK());
        }

        //Attempt to resolve external links to other databases (indexing more than one DB)
        eREFOBJ tbcTBLClass=eREFUNKNOWN; //return view/table or unknown

        s = sResolveExtDBLink(prcPrcs, tbcTabs,thisDB,tbTab.GetOwner(),tbTab.GetName(),eREFVIEW
                    ,sOutputPath,sObjServ,sObjDB,sOwner,sName,s,tbcTBLClass);

        if(tbcTBLClass != eREFUNKNOWN) eObjType = tbcTBLClass;

				//Link to file database object accessing this table
				fprintf(fpOut,"<tr><td id=\"Bord\" %s><a href=\"%s\">", COL_ALIGN1, (const char *)s);
				CUSTR sPrint = FormatUsefulDBObjRef(sObjServ,sObjDB, sOwner, sName, thisDB);

				if FLAG(eacAccess, (eAT_ACC_EXT|eAT_ACC_UNDEF))  //Buck or feat not sure
					eObjType = eREFTABVIEW;                       //But undefined is UNDEF

        CDP::HTML_Color_Obj(fpOut, eObjType, sPrint);
				fprintf(fpOut,"</a></td>");
	
				if FLAG(eAT_SELECT_BY_VIEW, eacAccess) //Implementing SELECT on this view/table ???
				{
					fprintf(fpOut, "<td id=\"Bord\" %s>", COL_ALIGN1); //Print Data Modifier for UPDATES

          if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,   sObjDB,sObjServ)==0)
          {
					  //Get table key and format anker for modifier (update,delete,insert)
					  FormatTableKey(hlTableFrom, tbTab.GetID(), &pcKey[0], NUM_SZ);
  	
					  if(pcKey[0] != '\0')
					  {
						  if(this->GetTable(sObjServ,sObjDB,sOwner,sName).GetObjType() == eREFVIEW)
							  fprintf(fpOut, "<a href=\"%s#%s\">SELECT</a>", (const char *)s,pcKey );
						  else
							  fprintf(fpOut, "<a href=\"%s\">SELECT</a>", (const char *)s); //Link 2 Table
					  }
					  else
						  fprintf(fpOut, "SELECT"); //Print Data Modifier for SELECT
          }
					else
            fprintf(fpOut, "<a href=\"%s\">SELECT</a>", (const char *)s); //Link 2 External DB Table
				}
				fprintf(fpOut, "</td></tr>\n");
			}
			fprintf(fpOut, "</table>\n");	
		}
	}
	catch(CERR e)
	{
		printf("ERROR: CTab::HTML_PrintViewAccess %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: CTab::HTML_PrintViewAccess An error has occured!\n");
	}
}

#undef COL_ALIGN
#undef COL_ALIGN1

//Find a reference to a able object by its name
CTABCOL::H_TAB *CTABCOL::FindTableByName(const UCHAR *pcServer
                                        ,const UCHAR *pcDB  //Data Container
			 	                                ,const UCHAR *pcOwner
				                                ,const UCHAR *pcName) const
{
  CDP::Handle_Requi_Params(pcServer,pcDB, pcOwner, pcName,"CTABCOL::FindTableByName");

	if(pphTabRt == NULL) return NULL;

	//Run through each node in each hash bucket
	for(int i=0;i<iTabRootSz; i++)
	{
		for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
      if(this->bMatchNode(pcServer,pcDB,pcOwner,pcName,phlTmp) == true)
				return phlTmp;
		}
	}
		
	return NULL;
}

//Get name of trigger for a certain event on a certain table
//==========================================================>
CUSTR CTABCOL::FindTrigger(int dmModifier        //Data modification
                          ,const UCHAR *pcServer
                          ,const UCHAR *pcDB    //Data Container
                          ,const UCHAR *pcOwner
                          ,const UCHAR *pcName) const
{
	H_TAB *ptbTable;
	CTAB    *ptbObj;

	if((ptbTable = FindTableByName(pcServer,pcDB,pcOwner,pcName))!=NULL)
	{
		if((ptbObj = ptbTable->ptbTable) == NULL) return "";

		switch(dmModifier)   //Return name of a trigger for a certain event
		{                   //if available
		case dmDELETE:
			if(ptbObj->DelTrig() != NULL)
				return CUSTR(ptbObj->DelTrig());
			break;
		case dmINSERT:
			if(ptbObj->InsTrig() != NULL)
				return CUSTR(ptbObj->InsTrig());
			break;
		case dmUPDATE:
			if(ptbObj->UpdTrig() != NULL)
				return CUSTR(ptbObj->UpdTrig());
		}
	}

	return "";
}

//Get the name of a table and its ID Key so that we know linking to this is safe
//
bool CTABCOL::GetTableMatch(const UCHAR *pcServer, const UCHAR *pcDB    //Ref to data container
                           ,const UCHAR *pcOwner
                           ,const UCHAR *pcName,int *pIDKey, eREFOBJ *ptbcTBLClass) const
{
	H_TAB *ptbTab;

	if((ptbTab = FindTableByName(pcServer,pcDB,pcOwner,pcName))!=NULL)
	if(ptbTab->ptbTable != NULL)
	{
		*pIDKey = ptbTab->ptbTable->GetID();    //get KEY INFO and return name

		if(ptbcTBLClass != NULL)              //Return object class if asked for
			*ptbcTBLClass = ptbTab->ptbTable->GetObjType();

		return true;
	}

	return false;
}

#define _SRC "CTABCOL::bTabsSizeAvail"
//Find out whether any table has size information or not
//===========================================================00000>
bool CTABCOL::bTabsSizeAvail() const
{
	if(pphTabRt == NULL) return false;

	for(int i=0;i<iTabRootSz; i++)     //Run through each node in each hash bucket
	{
		for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			if(phlTmp->ptbTable->iDataSz >= 0 || phlTmp->ptbTable->iIndexSz >= 0)
				return true;
		}
	}

	return false;
}
#undef _SRC

#define _SRC "CTABCOL::FindUniqueOwnerOfTab"
//Find the name of the owner of a database object and return it
//True is returned if there was no other owner, false however,
//if there are multible objects with same name but different owner
bool CTABCOL::FindUniqueOwnerOfKnownTab(const UCHAR *pcServer,const UCHAR *pcDB
                                       ,const UCHAR *pcTabName,CUSTR & sRetOwner, eREFOBJ & eTABType)
{
  sRetOwner="";
  eTABType=eREFUNKNOWN;

	int hval = this->hGetKey(pcTabName);

	if(pphTabRt[hval] == NULL) return false; //Nothing found here

	H_TAB *pFind; //Find point of occurance...
	
	pFind = pphTabRt[hval];

	while(pFind != NULL)           //...in list of nodes
	{
		if(pFind->sServer.scmp_ci(pcServer)== 0 &&
       pFind->sDB.scmp_ci(pcDB)        == 0 &&
       pFind->sName.scmp_ci(pcTabName) == 0 &&
       pFind->ptbTable->GetObjType()  != eREFUNKNOWN)
			break;
		else
			pFind = pFind->pNext;
	}

  if(pFind == NULL) return false; //Nothing found here
  sRetOwner = pFind->sOwner;
  eTABType  = pFind->ptbTable->GetObjType();
  pFind = pFind->pNext;

	while(pFind != NULL)           //...search remainder of list
	{                             //to make sure there is no ambigouity left
		if(pFind->sServer.scmp_ci(pcServer)== 0 &&
       pFind->sDB.scmp_ci(pcDB)        == 0 &&
       pFind->sName.scmp_ci(pcTabName) == 0 &&
       pFind->ptbTable->GetObjType()   != eREFUNKNOWN)
			break;
		else
			pFind = pFind->pNext;
	}

  if(pFind != NULL) return false; //Reference to owner is not unique
  else
    return true;                //There was no other reference
}
#undef _SRC

//Remove a table dependency, if it appears to be wrongly assumed
//through standard db owner
//===========================================================--->
void CTABCOL::RemoveAssumedTableDeps(const UCHAR *pcServ,const UCHAR *pcDB
                                    ,const UCHAR *pcUser,const UCHAR *pcName
                                    ,const UCHAR *pcDepServ,const UCHAR *pcDepDB
                                    ,const UCHAR *pcDepOwner,const UCHAR *pcDepName)
{
  H_TAB *pFindTab = FindTableByName(pcServ, pcDB, pcUser, pcName);

  if(pFindTab != NULL)              //Find Table object and tell dependency container to get rit of these deps
  if(pFindTab->ptbTable != NULL)
    pFindTab->ptbTable->m_dpDBObjDeps.RemoveAssumedDeps(pcDepServ, pcDepDB,pcDepOwner, pcDepName);
}

//Remove Unknown Tables that hold no dependency
//==============================================>
void CTABCOL::RemoveAssumedTables()
{
	for(int i=0;i<iTabRootSz; i++)
	{
    H_TAB *phlLastTmp,*phlTmp;
		for(phlTmp = pphTabRt[i], phlLastTmp = NULL; phlTmp != NULL; )
		{
			if(phlTmp->ptbTable->GetObjType() == eREFUNKNOWN)
      if(phlTmp->ptbTable->m_dpDBObjDeps.iCntDeps() == 0)
      {
        H_TAB *phlDelTmp = phlTmp;

        if(phlLastTmp != NULL)               //Isolate this link and delete entry
          phlTmp = phlLastTmp->pNext = phlTmp->pNext;
        else
          phlTmp = pphTabRt[i]       = phlTmp->pNext;

        delete phlDelTmp;
        continue;               //continue loop to force imidiate check of loop condition
      }
      phlLastTmp = phlTmp;
      phlTmp = phlTmp->pNext;
    }
	}
}

//Find all external objects that depend on a given object and return their access codes
UINT CTABCOL::FindAllOutboundDeps(CDEPS & dpRet,const CUSTR sServer,const CUSTR sDB
                                 ,const CUSTR sOwner, const CUSTR sName, const CUSTR sAbsOutPath)
{
  if(pphTabRt == NULL) return 0;
  UINT iRet=0;
  CUSTR sLinkPath, sDummy;
  eREFOBJ eObjType;
  UINT eAccType;
  CTAB *pTab;

  for(int i=0;i<iTabRootSz; i++)
	for(H_TAB *phlTmp = pphTabRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
  {
    if((pTab = phlTmp->ptbTable) == NULL) continue;

    //If this tab object holds a ref to the searched object, we add the tab object into the collection
    if(pTab->m_dpDBObjDeps.FindDepByName(sServer,sDB,sOwner,sName, sLinkPath,eObjType,eAccType,sDummy)==true)
    {
      iRet++;

      dpRet.AddDep(pTab->GetServer(),pTab->GetDB(),pTab->GetOwner(),pTab->GetName()
                  ,pTab->GetObjType(),eAccType,0,sAbsOutPath + pTab->GetTargFileName());
    }
  }  

  return iRet;
}

//==================================================================================>
// 1> FLAGS given here are added to table/view object
// 2> Dependencies in dpDepIn are inbound dependencies for these views
//    this means, we have to convert SELECT_BY_VIEW into SELECT FLAG
//    just so that SELECT ACCESS by views to other tables/views is shown correctly
// Add the given dependencies to the dependencies addressed by: pcDB.pcOwner.pcName
//==================================================================================>
void CTABCOL::AddOutboundDeps(CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner
                             ,const UCHAR *pcName)
{
  if(pphTabRt == NULL) return;

  if(dpDepIn.CountAccTypes(eAT_SELECT_BY_VIEW) > 0)
  {
    CUSTR sDBObj, sDummy, sDummy1, sDummy2;
    eREFOBJ eObjType;
    UINT eAccType;

    sDBObj = dpDepIn.GetFirst(eObjType, sDummy, sDummy1, sDummy2, eAccType);
    while(sDBObj.slen()>0)
    {
      if FLAG(eAT_SELECT_BY_VIEW, eAccType)
      {
        UNSETFLAG(eAccType,eAT_SELECT_BY_VIEW);
          SETFLAG(eAccType,eAT_SELECT        );

        dpDepIn.SetAccType(eAccType);  //Reset flags, so that select_by_view becomes select
      }
    
      sDBObj = dpDepIn.GetNext(eObjType, sDummy, sDummy1, sDummy2, eAccType);
    }
  }

  H_TAB *phlTmp = FindTabNode(pphTabRt,pcServer,pcDB,pcOwner,pcName);

  if(phlTmp != NULL)
  if(phlTmp->ptbTable != NULL)
  {
    phlTmp->ptbTable->m_dpDBObjDeps += dpDepIn;
  }
}
