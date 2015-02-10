
#include "cserver.h"  //Database Server structures class
#include "cerr.h"    //Standard Error Class from XML parser sub-project
#include "prettyprint.h"  //Database Server structures class

// Initialize work sets hash function
#define INIT_HSZ 256
#define INIT_HSZ_MASK (INIT_HSZ-1)
#define H_SEED 161

// Constructor
CSERVER::CSERVER():iDbrRootSz(0),pphDBRRt(NULL)    //Hash table code
{
	int i;

	if((pphDBRRt = new H_DBR *[INIT_HSZ])==NULL)
		return;

	iDbrRootSz = INIT_HSZ;
	for(i=0;i<INIT_HSZ;i++)
	{
		pphDBRRt[i] = NULL;
	}
}

// Destructor
CSERVER::~CSERVER()
{
  hFreeColl();    //Get rid of hash table of table/view collection
}

//
//Hashing function is optimized for hashtable-size (INIT_HSZ) with a power of 2,
//readjust that size to suit your needs (in power of 2 steps)!
//
unsigned int CSERVER::hGetKey(const UCHAR *word) const
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
void CSERVER::hFreeColl()
{
	int i;
  H_DBR *phTmp, *phTmp1;

	if(pphDBRRt == NULL) return;

	for(i=0;i<INIT_HSZ; i++)
	{ 
		phTmp = pphDBRRt[i];
		while(phTmp != NULL)
		{
			phTmp1 = phTmp->pNext;
			delete phTmp;          //Destroy node data via destrutor

			phTmp = phTmp1;
		}
	}

	delete [] pphDBRRt; //Free root structure
	pphDBRRt=NULL;
}

bool CSERVER::hAddEntry(const UCHAR *pcServName, const UCHAR *pcDB, const CDB *pDB)
{
//Only checked things are warrented for <<<<<<<<<<<<<<<<
  if(pphDBRRt==NULL || pDB == NULL || pcServName == NULL || pcDB == NULL)
    return false;

  if(s::slen(pcServName)<= 0 || s::slen(pcDB)<= 0)
    return false;
//Only checked things are warrented for <<<<<<<<<<<<<<<<

  int hval = this->hGetKey(pcDB);

	//Hash bucket is empty so we insert right here
	if(pphDBRRt[hval] == NULL)
	{
		pphDBRRt[hval] = new H_DBR(pcServName, pcDB, pDB);

		return true;
	}

	H_DBR **ppFind; //Find point of insertion/change...
	
	ppFind = &pphDBRRt[hval];

	while(*ppFind != NULL)           //...in list of nodes
	{
    if(s::scmp_ci2((*ppFind)->sDB,(*ppFind)->sServName,  pcDB,pcServName) == 0)
			break;
		else
			ppFind = &((*ppFind)->pNext);
	}

	if((*ppFind) == NULL)               //Insert completely new entry at end of list
	{
		*ppFind = new H_DBR(pcServName, pcDB, pDB);

		return true;
	}

	//Add additional for item inserted data for this object (here not needed, yet)
  //So, we return false, because the DB object with this Server,DB-Name is already there
  return false;
}

#define _E_SRC "CSERVER::hGetEntry"
CDB & CSERVER::hGetEntry(const UCHAR *pcServer, const UCHAR *pcDB)
{
  H_DBR *phTmp = NULL;
  bool bFound=false;
  try
	{
    if(pphDBRRt != NULL)
    {
      int hval = this->hGetKey(pcDB);

      for(phTmp = this->pphDBRRt[hval]; phTmp!= NULL; phTmp=phTmp->pNext)
      if(phTmp->pDBR != NULL)
      {
      if(s::scmp_ci2(pcDB,pcServer
                    ,phTmp->pDBR->GetDBName(),phTmp->pDBR->GetServerName()) == 0)
      bFound=true;
      }
    }
  }
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: %s\n", _E_SRC);
	}
	catch(...) //Progress Error and throw to caller
	{
    printf("Unknown ERROR:\nCodeRef: %s\n", _E_SRC);
  }

  if(phTmp == NULL || bFound != true)
  {
	  CUSTR s("Can not find database: ");
	  s += pcServer;	s+= "."; s+= pcDB;
	  throw CERR(s, 0, _E_SRC);
  }
  return *phTmp->pDBR;
}
#undef _E_SRC

#define _E_SRC "CSERVER::hFind"
bool CSERVER::hFind(const UCHAR *pcServer, const UCHAR *pcDB) const
{
  if(pphDBRRt == NULL) return false;

  int i = this->hGetKey(pcDB);
  H_DBR *phTmp = NULL;

  for(phTmp = this->pphDBRRt[i]; phTmp!= NULL; phTmp=phTmp->pNext)
	{
    if(phTmp->pDBR != NULL)
    if(s::scmp_ci2(pcDB,pcServer
                  ,phTmp->pDBR->GetDBName(),phTmp->pDBR->GetServerName()) == 0)
      return true;
  }
  return false;
}
#undef _E_SRC

#define _E_SRC "CSERVER::hFind"
CPITEM *CSERVER::hGetUniqueDBs() const
{
  bool bFound=false;
  if(pphDBRRt == NULL) return false;

  CPITEM *pRet;
  if((pRet = new CPITEM())==NULL) return NULL;

  for(int i=0;i<INIT_HSZ; i++) //Run through each node in each hash bucket
	for(H_DBR *phTmp = this->pphDBRRt[i];phTmp != NULL; phTmp = phTmp->pNext)
	{
    if(phTmp->pDBR == NULL) continue;

    //Copy this name of server, database into collection
    pRet->AddString(phTmp->sServName,phTmp->sDB, phTmp->pDBR->GetOwnerName(),phTmp->sServName
                   ,phTmp->pDBR->GetOutPutDir(),0,0);
    bFound=true;
  }

  if(bFound==false)
  {
    delete pRet;
    return NULL;
  }
  else return pRet;
}
#undef _E_SRC
//=========================== END OF HASHING COLLECTION FUNCTIONs -------------->
//=========================== END OF HASHING COLLECTION FUNCTIONs -------------->
//=========================== END OF HASHING COLLECTION FUNCTIONs -------------->

//Define state nodes
//There may be more state nodes for a given state if a tag
//can occur in several places...
#define STN_DB_PTRS  2
#define STN_DB       3

//Define states these are closely bound to a tag
//that has been recognized, while STN_ states are
//bound to a recognized tag on a well defined path
//of the XML document
//))))))))))))))))))))))))))))))))))))))))))))))))
#define ST_DB_PTRS  2
#define ST_DB       3

// XML GRAMMAR Definitions used to seperate useful tags from boggos information
// Each node in the next array represents a state or xml tag of a certain name
#define stdDEFSSZ 2
const TRANSDEF stdDEFS[stdDEFSSZ] =
{
	 {"db_ptrs", ST_ROOT}
	,{"DB", ST_DB}
};

// ============================================================>
// Each node in the next array represents an interesting
// state transition from one XML tag to another
// All XML tags are parsed for syntax but only _*interesting*_
// tags on _*interesting*_ state transition paths will be shown
// to the caller of the parser...
// ============================================================>
// This parser can not process open_close_tags at root level
// just use <xyz> ... </xyz> and put your info where the ... is
#define trTransSZ 1
const STRANS trTrans[trTransSZ] =
{
	{{ST_ROOT, STN_ROOT},   {ST_DB, STN_DB}}
};

//Initiate Main Loop for XML parser
//--------------------------------->
void CSERVER::InitIndexStack(CSTAK & stkTags)
{
	//initialize network of main state transitions
	stkTags.InitMajorEngine(trTrans, trTransSZ, stdDEFS, stdDEFSSZ);
}

#define DATA_FOUND 0x01 //Tell when data is available in dictionary
#define OK         0x02 //Everythings fine ...just continue processing

int CSERVER::ManageXMLState(int iThisSt, CTAG *prtNext)
{
	switch(prtNext->getTagTyp())
	{
	case TGP_CLO_OPEN:
		switch(iThisSt)
		{
			case STN_ROOT:  //XML tag pointing to database XML index file found
        if(prtNext->getTagName() == "DB") return DATA_FOUND;
      break;

      default:
        break;
		}
	break;
/*                 ==> These may be needed in future versions
	case TGP_CLOSE:
		switch(iThisSt)
		{
			case STN_TABLE:        //Table definition tag found
				return DATA_FOUND;
		}
	break;

	case TGP_OPEN:
		if(iThisSt == STN_ROOT && prtNext->getTagName() == "parse_files")
			return DATA_FOUND;
	break;
*/
	default:          //Nothing todo on default (keeps away compiler warnings)
		break;
	}
		
	return OK;
}

#undef ST_DB

#undef trTransSZ
#undef stdDEFSSZ

CTAG *CSERVER::SearchXMLFile(CPARS & xpPState, CSTAK & stkTags, bool & bIntro)
{
	CTAG *prtNext=NULL;

	while(xpPState.eof() == false)
	{
		prtNext = xpPState.getNextRawTag(); //Get next XML Tag

		if(prtNext == NULL) continue;

		if(bIntro == true) //Skip the prelude unseen
		{
			if(prtNext->getTagTyp() == TGP_PI || prtNext->getTagTyp() == TGP_COMMENT)
			{
				FREE_OBJ(prtNext)
				continue;
			}
			else bIntro = false;
		}

		if(prtNext->getTagTyp() != TGP_CLOSE)  //Put this tag on the stack
		{
			//Keep track of OPEN/CLOSE tags only here
			//Note that other tags can be read below by caller or via minor state engine
			stkTags.Push(*prtNext);
		}
		else                      //Current Tag is a closing tag
		{                         //So, we attempt to find the corresponding open tag
			CTAG *ptgRet=NULL;

			while (stkTags.empty() == false)
			{
				CTAG tg = stkTags.Top();

				if(tg.getTagTyp() == TGP_OPEN) //We Pop for the next open tag
				{
					if(stkTags.Top().getTagName() == prtNext->getTagName())
					{
						ptgRet = stkTags.Pop(); //Tag name matched, pop it and exit

						if(ptgRet != NULL)
						{
							FREE_OBJ(prtNext)
							return ptgRet;
						}

						break; //Tag wasn't one of the state engines (minor or major)
					}
					else
						xpPState.throwErr(CUSTR("Error: Unbalanced tag at: ") + prtNext->getTagName(), ERR_SYNTAX);
				}
				ptgRet = stkTags.Pop();
				delete ptgRet;
				ptgRet=NULL;
			}
		}

		int iRet = ManageXMLState(stkTags.getStateID(), prtNext);

		if(iRet == DATA_FOUND)
		{
			return prtNext;
		}

		FREE_OBJ(prtNext)
		return NULL;
	} //////////// Read Character by character from file (End Of While)

	if(stkTags.getUsed() == false)
	{
		//printf("No Data found!\n");
	}
	else
	{
		if(stkTags.empty() == false) //Stack was used but is not empty
		{
			FREE_OBJ(prtNext) //destroy raw tag element
			xpPState.throwErr("SYNTAX ERROR: Unbalanced tag - Missing closing tag!\n");
		}
	}

	FREE_OBJ(prtNext) //destroy raw tag element
	return NULL;
}
#undef DATA_FOUND //Tell when data is available ...
#undef OK         //Everythings fine ...just continue processing


//==========================================================================================>
// Import XML index pointing to multible XML index files
// Each describing DDL information for one relational database
// Return Value: Negative -> Indicates Error/Warning
//               Zero     -> Indicates Success
//==========================================================================================>
#define _E_SRC "CSERVER::ProcessXMLSrvFile"
int CSERVER::ProcessXMLSrvFile(COpts & prgOptions, const CUSTR & sXMLSRVInput)
{
	CPARS xpPState;   // input file stream
	CSTAK stkTags;     //Stack and TAG object used for XML parsing
	CTAG *pTAGS;
	bool  bIntro=true; //We parse XML from the very start
  int iCntDB=1;
  int iTrigs=0, iProcs=0;

  if(sXMLSRVInput.slen() <=0 )
	  throw CERR("ERROR: No index file name set!\n",0, _E_SRC);

  try
	{
		xpPState.open(sXMLSRVInput); //Open XML source file

		this->InitIndexStack(stkTags); //Init stack obj with XML definitions

		while(xpPState.eof() == false)
		{
			if((pTAGS = this->SearchXMLFile(xpPState, stkTags, bIntro)) != NULL)
      {
        if(pTAGS->getTagName() == "DB")
        {
          if(pTAGS->Props.sGetVal("locate").slen() <= 0 //Check required properties
          || pTAGS->Props.sGetVal("outdir").slen() <= 0)
          {
						xpPState. throwErr("Required property locate or outdir missing in <DB> XML-Tag", ERR_SYNTAX);
						return -3;                          //(This is never reached)
          }

          //Process XML TAG
          printf("\n>>> %02i Processing DB Pointer into XML DB\n",iCntDB++);
          printf("        Item: %s\n",(const char *)pTAGS->Props.sGetVal("locate"));
          printf("    Location: %s\n",(const char *)pTAGS->Props.sGetVal("outdir"));

          int iErr = ParseInput(prgOptions, pTAGS->Props.sGetVal("outdir"), pTAGS->Props.sGetVal("locate")
                               ,iTrigs, iProcs);

          if(iErr < 0)
          {
	          printf("WARNING ParseInput returned Error Code %i in CSERVER::ProcessXMLSrvFile.\n", iErr);
	          return iErr;
          }
        }
      }

      FREE_OBJ(pTAGS); //Free tag data used in last loop...
		}                 //while loop from main-parser

    printf("\n>>> Total Files Parsed Triggers: %i Procedures %i <<<\n",iTrigs, iProcs);
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: %s\n", _E_SRC);
		return -2;
	}
	catch(...) //Progress Error and throw to caller
	{ xpPState.throwErr(_E_SRC,0);  }

  return iCntDB;
}
#undef _E_SRC

//===========================================================================>
//Find all external objects that depend on a given object and return their
//access codes
//===========================================================================>
UINT CSERVER::FindAllOutboundDeps(CDEPS & dpRet,const CUSTR & sServer,const CUSTR & sDB
                                               ,const CUSTR & sOwner, const CUSTR & sName) const
{
  UINT iRet = 0;

  for(int i=0;i<INIT_HSZ; i++) //Run through each node in each hash bucket
	for(H_DBR *phTmp = this->pphDBRRt[i];phTmp != NULL; phTmp = phTmp->pNext)
	{
    if(phTmp->pDBR == NULL) continue;

    if(s::scmp_ci2(sDB,sServer,  phTmp->pDBR->GetDBName(),phTmp->pDBR->GetServerName()) != 0)
      iRet += phTmp->pDBR->FindAllOutboundDeps(dpRet,sServer,sDB,sOwner,sName);
  }
  return iRet;
}

// this function will return the database matching the name sought
//================================================================>
void CSERVER::AddOutboundDeps(CDEPS & dpDepIn
                             ,const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner
                             ,const UCHAR *pcName, const eREFOBJ eObjType)
{
  if(pphDBRRt != NULL)
  {
    int hval = this->hGetKey(pcDB);

    for(H_DBR *phTmp = this->pphDBRRt[hval]; phTmp!= NULL; phTmp=phTmp->pNext)
    if(phTmp->pDBR != NULL)
    {
      if(phTmp->pDBR->GetServerName().scmp_ci(pcServer) == 0 &&
         phTmp->pDBR->GetDBName().scmp_ci(pcDB) == 0)
      {
        phTmp->pDBR->AddOutboundDeps(dpDepIn,pcServer,pcDB,pcOwner,pcName, eObjType);
      }
    }
  }
}

//===========================================================================>
// Multible Databases can depend on each other through objects (tables, views
// procedures) being used across database limits, here we attempt to connect
// these dependencies so that HTML linking can take place
//===========================================================================>
#define _E_SRC "CSERVER::ResolveDBDeps"
void CSERVER::ResolveDBDeps()
{
  try
	{
    CPITEM *pitDBs=NULL;
    CDEPS *pDeps=NULL;

    //No DB to ref from??? this would indicate an error, but continue anyway
    if((pitDBs = hGetUniqueDBs())==NULL) return;

    for(int i=0;i<INIT_HSZ; i++) //Run through each node in each hash bucket
		for(H_DBR *phTmp = this->pphDBRRt[i];phTmp != NULL; phTmp = phTmp->pNext)
		{
      if(phTmp->pDBR == NULL) continue; //No Deps refering to an external DB found...

      CDEPS *pTmpDeps=NULL;
      pTmpDeps = phTmp->pDBR->FindExtDBObjects(pitDBs);

      if(pTmpDeps != NULL) //Collect these dependencies in result set
      {
        if(pDeps == NULL)
          pDeps = pTmpDeps;
        else
        {
          *pDeps += *pTmpDeps;
          delete pTmpDeps;
          pTmpDeps = NULL;
        }
      }
    }

    //1> We found outbound dependencies belonging to one of the DBs parsed/stored
    //printf("External References found:\n"); pDeps->Debug();

    UINT iResolved=0; //Call each DB-proc/tab collection and return an HTML_LINK
    if(pDeps != NULL)
    for(int i=0;i<INIT_HSZ; i++)
		for(H_DBR *phTmp = this->pphDBRRt[i];phTmp != NULL; phTmp = phTmp->pNext)
		{
      if(phTmp->pDBR == NULL) continue;
    
      iResolved += phTmp->pDBR->ResolveForeignDBDeps(pDeps);
    }

    //2>Each object referenct should now have HTML linkage information pointing to that object
    //printf("\n=============================================\n");
    //printf("External References found:\n"); pDeps->Debug();

    if(iResolved > 0) //Call each DB-proc/tab collection and set HTML_LINK for external reference
    for(int i=0;i<INIT_HSZ; i++) //Run through each node in each hash bucket
		for(H_DBR *phTmp = this->pphDBRRt[i];phTmp != NULL; phTmp = phTmp->pNext)
		{
      if(phTmp->pDBR == NULL) continue;
    
      phTmp->pDBR->SetForeignDBDeps(pDeps);
    }

    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //This part of the function searches and resolves inbound dependencies >>>>>>>>>>>>>>>>>>>>>>
	  const UCHAR *pc;
	  UINT eatAccess, iRet=0;
	  eREFOBJ eObjType;
	  CUSTR sServer,sDB, sOwner, sNewOwner;

	  pc = pDeps->GetFirst(eObjType, sServer,sDB,sOwner ,eatAccess);

	  while(pc != NULL)
	  {
      CDEPS dpRet; //This is defined here, so that we re-construct CDEPS in each loop
      sNewOwner = pDeps->GetNewOwner();

      if(sNewOwner.slen()>0) //Chnage to alternative owner, if owner changes from
        sOwner = sNewOwner; //source to destination between deps (across two different DBs, phew)

      iRet = FindAllOutboundDeps(dpRet, sServer,sDB,sOwner,pc);

      if(iRet > 0)
      {
        AddOutboundDeps(dpRet,sServer,sDB,sOwner,pc, eObjType);
        //printf("-------------------------------------------------------\n");
        //printf("Dependency count for %s.%s.%s  is %i\n",(const char *)sDB,(const char *)sOwner,pc, iRet);
        //dpRet.Debug();
      }

      pc = pDeps->GetNext(eObjType, sServer,sDB,sOwner, eatAccess);
	  }

    delete pDeps;   //Free list of outbound dependencies that cross database limits
    delete pitDBs; //Free list of available databases stored on this server object
  }
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: %s\n", _E_SRC);
		return;
	}
	catch(...) //Progress Error and throw to caller
	{
    printf("Unknown ERROR:\nCodeRef: %s\n", _E_SRC);
  }

  return;
}
#undef _E_SRC

//==========================================================================================>
//Parse XML index file and create a database object from information found in SQL Source Code
//Return Value: Negative -> Indicates Error/Warning
//              Zero     -> Indicates Success
//==========================================================================================>
int CSERVER::ParseInput(COpts & prgOptions, const CUSTR & sOutputPath, const CUSTR & sIdxInput,
                        int & iTrigsOut, int & iProcsOut)
{
  int iErr, iTrigs, iProcs;

  try
  { CDB *pDBTmp;
    if((pDBTmp = new(CDB))== NULL) return -3; //Ran out of memory or other resources?
    
    pDBTmp->OutPutDir(sOutputPath);
    pDBTmp->IdxFile(sIdxInput);

    //Parse XML index and all SQL Source files and generate callgraph and other stuff
    iTrigs = iProcs=0;
    iErr = pDBTmp->ParseFiles(&prgOptions, iProcs, iTrigs);
    if(iErr < 0)
    {
	    printf("WARNING CSERVER::ParseFiles returns %i\n", iErr);
	    if(iErr < 0) return iErr;
    }

    iTrigsOut += iTrigs;
    iProcsOut += iProcs;
    printf("<<< Database: %s.%s Parsing: %03i files (%03i Procedures, %03i Triggers) completed.\n",
      (const char *)pDBTmp->GetServerName(),(const char *)pDBTmp->GetDBName(), iErr, iProcs, iTrigs);

    //Add new DB object into collection (will fail, if Server/DB Combo is already there)
    this->hAddEntry(pDBTmp->GetServerName(),pDBTmp->GetDBName(), pDBTmp);

    //DEBUGEBUGDBUGDEUGDEBGDEBUDEBUGEBUGDBUGDEUGDEBGDEBUDEBUGEBUGDBUGDEUGDEBGDEBU
    //Two powerfull debugging functions that can be called without HTML Output
    //pDBTmp->DEBUG_Tables();
    //pDBTmp->DEBUG_Procs();
    //DEBUGEBUGDBUGDEUGDEBGDEBUDEBUGEBUGDBUGDEUGDEBGDEBUDEBUGEBUGDBUGDEUGDEBGDEBU
  }
  catch(CERR e)
  {
    printf("ERROR: %s\n", (const char *)e.getMsg());
    return -2;
  }
  catch(...)
  {
    printf("An unknown error has occured in CSERVER::ParseInput!\n");
  }

  return 0;
}
 
//==========================================================================================>
//Write HTML Output files into each directory indicated by -DirOut and/or <parse_files ...>
//Return Value: Negative -> Indicates Error/Warning
//              Zero     -> Indicates Success
//==========================================================================================>
int CSERVER::WriteAllDBOutput(COpts & prgOptions)
{
  try //Print resulting information from parse run into HTML file
  {
    int iErr, iTrigs, iProcs, iTabs, iCntDB=1;
    int iTrigsAll=0, iProcsAll=0,iTabsAll=0;

    //No DB to ref from??? this would indicate an error, but continue anyway
    CPITEM *pitDBs = hGetUniqueDBs();

    printf("\n");

    for(int i=0;i<iDbrRootSz; i++)
    for(H_DBR *phlTmp = pphDBRRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
    {
      if(phlTmp->pDBR->GetOutPutDir().slen() > 0)
      {
        printf("\n%03i Writing HTML files for database %s/%s ...\n",iCntDB++
          ,(const char *)phlTmp->pDBR->GetServerName(), (const char *)phlTmp->pDBR->GetDBName());

        iTrigs=iProcs=iTabs=0;
        iErr = phlTmp->pDBR->PrintHTML(&prgOptions, iProcs, iTrigs, iTabs);
        if(iErr < 0)
        {
          printf("WARNING CSERVER::PrintHTML return %i\n", iErr);
          if(iErr < 0) return iErr;
        }

        THIS_DB thisDB;
        phlTmp->pDBR->SetDBProps(thisDB);

        JS_PrintMenuConf(thisDB, phlTmp->pDBR->GetOutPutDir(), pitDBs);
        iErr++;

        iTrigsAll+=iTrigs, iProcsAll+=iProcs,iTabsAll+=iTabs;
        printf("    Writing: %03i files (%03i Procedures, %03i Triggers, %03i Tables) completed.\n",
                iErr, iProcs, iTrigs, iTabs);
      }
    }

    delete pitDBs;
    printf("\n>>> Total Files Written Triggers: %i Procedures: %i Tables: %i <<<\n",iTrigsAll, iProcsAll,iTabsAll);
  }
  catch(CERR e)
  {
    printf("ERROR: %s\n", (const char *)e.getMsg());
    return -2;
  }
  catch(...)
  {
    printf("An error has occured!\n");
  }
  return 0;
}

#undef INIT_HSZ
