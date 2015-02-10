
#include "ccg.h"

#include "cproccol.h"
#include "prettyprint.h"

CCG::CCG():pcgRoot(NULL)
{
}

//Construct a callgraph for a procedure called prcName
//using a collection of procedures and their direct callgraphs
//===========================================================>
CCG::CCG(CPRCCOL *prcColl
         ,CPROC *pPRC
         ,const THIS_DB & thisDB //Name of Database Server,Database we are running in
         ,const CUSTR   & sOutPath )
       :pcgRoot(NULL)           //PROC's with tree of direct calls
{
  if(pPRC==NULL)return;

  this->sServer   = thisDB.sServer;
  this->sDB       = pPRC->GetDB();    // Copy name of proc
  this->sOwner    = pPRC->GetOwner();
  this->sProcName = pPRC->GetName();

  this->sThisServer = thisDB.sServer;       //
  this->sThisDB     = thisDB.sDB;          //Database we are running in
  this->sThisOwner  = thisDB.sDBOwner;    //

  CG *pcgTmp;                   //Complete Callgraph for prcName Database Object
  CP_ROOT cpRoot;              //Call path for current path of evaluation
  int idx, iChildCnt=0;
  const UCHAR *pcRoot=NULL;
  CUSTR sNxtServ, sNxtDB, sNxtOwner;
  //Shortcut...
  CUSTR sServIn(pPRC->GetServer()),sDBIn(pPRC->GetDB())
        ,sOwnerIn(pPRC->GetOwner()), sNameIn(pPRC->GetName());

  //Render with a callgraph only local Server/DB Objects
  if(s::scmp_ci2(thisDB.sDB,thisDB.sServer, sDBIn,sServIn)==0)
  {
    //Simple Business: No Direct Calls - No CallGraph and, hence, NO Continue
    if(prcColl->GetProc(sServIn,sDBIn,sOwnerIn,sNameIn).m_dpDBObjDeps.CountAccTypes(eAT_EXEC) <= 0)
      return;

    //Create Root node of CG Graph
    pcgRoot = CreateCGNode(prcColl->GetProc(sServIn,sDBIn,sOwnerIn,sNameIn), prcColl
                          ,thisDB.sServer,thisDB.sDB,sOutPath);
  }
  else
    return;

	if(pcgRoot->CGSz <= 0) return;

	//Set Recursion flag, if required, so we do not recurse more than twice
	ResolveRecursion(pcgRoot, &cpRoot);

	InsertCG(pcgRoot, 0, &cpRoot); //Add first Call Level to Call-Path (Kind of Stack)

	while(cpRoot.pLast != NULL) //Loop as long as the stack has something to look at
	{
		//Set cpRoot.pLast->idx and doRoot to next set of child nodes, if available
		if((pcRoot = FindNextDirectCalls(&cpRoot, prcColl, &iChildCnt,
		                                 sNxtServ,sNxtDB, sNxtOwner)) == NULL)
		{
			//We need to remove the last level in call_path (there are no more children)
			DestroyLastCP(&cpRoot);
			if(cpRoot.pLast != NULL)  //Increase index for new current call_path
			{
				cpRoot.pLast->idx += 1;
			}
		}
		else //Proceed one level deeper into call_path
		{
      if(s::scmp_ci2(sDBIn,sServIn,  sNxtDB,sNxtServ)==0)
      {
			  pcgTmp = cpRoot.pLast->pcgNode;
			  idx    = cpRoot.pLast->idx;

        //Add Direct Calls into First CG Level
        AllocChildren(pcgTmp->ppCG[idx], iChildCnt);
        ExecDeps2CG(pcgTmp->ppCG[idx],&prcColl->GetProc(sNxtServ,sNxtDB
                    ,sNxtOwner,pcRoot).m_dpDBObjDeps,prcColl
                    ,thisDB.sServer,thisDB.sDB,sOutPath);

			  ResolveRecursion(pcgTmp->ppCG[idx], &cpRoot);
			  InsertCG(pcgTmp->ppCG[idx], 0, &cpRoot);  //Insert new last item in call_path
      }
		}
	}

	DestroyLastCP(&cpRoot); //Remove last item from call_path
}

void CCG::FreeCGBranch(CG *pcgCalls)
{
	int i;

	if(pcgCalls == NULL) return;

	for(i=0;i<pcgCalls->CGSz;i++)
	{
		if(pcgCalls->ppCG[i] != NULL)
		{
			FreeCGBranch(pcgCalls->ppCG[i]);
		}
	}
	delete [] pcgCalls->ppCG;
	delete pcgCalls;
}

//
//Free a complete CallGraph from memory
//=======================================>
void CCG::FreeCG(CG **ppcgCalls)
{
	CG *pcgCalls = *ppcgCalls;

	if(pcgCalls != NULL)
	{
		FreeCGBranch(pcgCalls);

		*ppcgCalls = NULL;  //and Auto-Reset Root Ptr
	}
}

//
CCG::~CCG()
{
	FreeCG(&pcgRoot);
}

//Allocate a certain number of callgraph nodes in an array
//========================================================>
void CCG::AllocChildren(CG *pcgThis, int iChildCnt)
{
	int i;

	if((pcgThis->ppCG = new CG *[iChildCnt])==NULL) return;

	pcgThis->CGSz  = iChildCnt;

	for(i=0;i<iChildCnt;i++)
	{
		pcgThis->ppCG[i] = NULL;
	}
}

//
//Transfer all exec entries from one dep collecton into
//a sorted list of CG entries (de-queue the collection)
//
//prcDBProcs required for lookup of unique proc-IDs for linkage later on
//=====================================================================>
void CCG::ExecDeps2CG(CG *pCG, CDEPS *pdpsProc, CPRCCOL *prcDBProcs
                     ,const UCHAR *pcThisServ,const UCHAR *pcThisDB, const CUSTR & sOutPath)
{
  if(prcDBProcs == NULL || pCG == NULL || pdpsProc == NULL) return;

  eREFOBJ eObjType; //Type of dependency modelled through object/access type
  UINT eacAccess;
  const UINT iExtMask = eAT_ACC_EXT | eAT_ACC_UNDEF;

  CUSTR sFileName, sDepServ,sDepDB,sDepOwner;
  const UCHAR *pcPrcName;

	for(pcPrcName = pdpsProc->GetFirst(eObjType, sDepServ,sDepDB,sDepOwner, eacAccess);
	    pcPrcName != NULL;
	    pcPrcName = pdpsProc->GetNext(eObjType, sDepServ,sDepDB,sDepOwner, eacAccess))
	{
		//Only procedures can be execed
		if(eObjType == eREFPROC  && FLAG(eacAccess, eAT_EXEC))
		{
			int i, ID;
			for(i=0;i<pCG->CGSz && pCG->ppCG[i] != NULL;i++) //Find empty slot
				;

			if(i<pCG->CGSz)
			{
        if(s::scmp_ci2(pcThisDB,pcThisServ, sDepDB,sDepServ)==0)
        {
				  ID        = prcDBProcs->GetProcID(sDepServ,sDepDB,sDepOwner,pcPrcName);
				  sFileName = prcDBProcs->GetProc(sDepServ,sDepDB,sDepOwner,pcPrcName).GetTargFileName();

				  if((pCG->ppCG[i] = AllocCG(sDepServ,sDepDB,sDepOwner,pcPrcName, ID,sFileName))!=NULL)
				  {
					  //Check calltree item for external call
					  if(pdpsProc->FindDep(sDepServ,sDepDB,sDepOwner,pcPrcName,eREFPROC,iExtMask)==true)
						  SETFLAG(pCG->ppCG[i]->iFlags, F_EXTERN_CALL);
					  else
						  UNSETFLAG(pCG->ppCG[i]->iFlags, F_EXTERN_CALL);
				  }
        }
        else
        {
          if(pdpsProc->GetHTML_LINK().slen()>0) //Thats a resolved call to an external DB ...???
          {
            sFileName  = s::sGetPath(pdpsProc->GetHTML_LINK());   //Get Path to DB External Object
            sFileName  = s::GetRelPath(sOutPath, sFileName);
            sFileName += s::sGetFileName(pdpsProc->GetHTML_LINK());

				    if((pCG->ppCG[i] = AllocCG(sDepServ,sDepDB,sDepOwner,pcPrcName, -1, sFileName))!=NULL)
						  SETFLAG(pCG->ppCG[i]->iFlags, (F_EXTERN_CALL | F_EXTRES_CALL));
          }
          else //It is an UNRESOLVED call to an external DataBase_Object!!!
          {
				    if((pCG->ppCG[i] = AllocCG(sDepServ,sDepDB,sDepOwner,pcPrcName, -1, sFileName))!=NULL)
						  SETFLAG(pCG->ppCG[i]->iFlags, F_EXTERN_CALL);
          }
        }
			}
		}
	}
}

//Create a Callgraph node with a name pcKey and its ChildNodes
//////////////////////////////////////////////////////////////
CCG::CG *CCG::CreateCGNode(CPROC & rcProced, CPRCCOL *prcCol
                          ,const UCHAR *pcThisServ,const UCHAR *pcThisDB, const CUSTR & sOutPath)
{
	int ID;
	CUSTR sFileName;
	CG *pcgRet=NULL;

	ID        = rcProced.getID();
	sFileName = rcProced.GetTargFileName();

  if((pcgRet = AllocCG(rcProced.GetServer(),rcProced.GetDB()
                      ,rcProced.GetOwner(),rcProced.GetName(),ID,sFileName))==NULL)
		return NULL;

	//Count child nodes & allocate array for childnodes of that size
	AllocChildren(pcgRet, rcProced.m_dpDBObjDeps.CountAccTypes(eAT_EXEC));

	//Add collection items into ChildNodes of CG
	ExecDeps2CG(pcgRet, &rcProced.m_dpDBObjDeps,prcCol,pcThisServ,pcThisDB,sOutPath);

	return pcgRet;
}

//Set Recursion flag, if required, so we do not recurse more than twice
//in the callgraph (we flag first recursion and stop recursing there)
//====================================================================>
void CCG::ResolveRecursion(CG *pcgNode, CP_ROOT *pcpPath)
{
	int i;

	if(pcgNode == NULL) return;
	if(pcgNode->ppCG == NULL || pcgNode->CGSz == 0) return;

	//This is called for the first direct calls in CG
	if(pcpPath->pFirst == NULL) //So, we compare all root calls with pcKey in Root
	{                           //and Flag a Re-occurence and return
		for(i=0;i<pcgNode->CGSz;i++)
		{
			if(pcgNode->ppCG[i] != NULL)
			if(pcgNode->ppCG[i]->sServer.scmp_ci(pcgNode->sServer) ==0 &&
         pcgNode->ppCG[i]->sDB.scmp_ci(pcgNode->sDB)         ==0 &&
			   pcgNode->ppCG[i]->sOwner.scmp_ci(pcgNode->sOwner)   ==0 &&
			   pcgNode->ppCG[i]->sName.scmp_ci(pcgNode->sName)     ==0)
			{
				SETFLAG(pcgNode->ppCG[i]->iFlags, F_RECURSION);
			}
		}
		return;
	}

  //Normal Case: Compare current CallPath against current children
  //             A match is a recursion in the path -> Flag It and Continue
  for(i=0;i<pcgNode->CGSz;i++)
  {
    //InDirect Trivial Recursion or Direct Call Recursion...
    if(pcgNode->ppCG[i] != NULL)
    if(FindCG_ByName(pcgNode->ppCG[i]->sServer,pcgNode->ppCG[i]->sDB
                    ,pcgNode->ppCG[i]->sOwner,pcgNode->ppCG[i]->sName,pcpPath) == true ||

    ( pcgNode->ppCG[i]->sServer.scmp_ci(pcgNode->sServer) ==0 &&
      pcgNode->ppCG[i]->sDB.scmp_ci(pcgNode->sDB)         ==0 &&
      pcgNode->ppCG[i]->sOwner.scmp_ci(pcgNode->sOwner)   ==0 &&
      pcgNode->ppCG[i]->sName.scmp_ci(pcgNode->sName)     ==0 ))
    {
      SETFLAG(pcgNode->ppCG[i]->iFlags, F_RECURSION);
    }
  }
}

//======================================================================>
//Find the next direct call candidate in current set of child nodes in CG
//based on the last cg node and index in the call_path structure
//======================================================================>
const UCHAR *CCG::FindNextDirectCalls(CP_ROOT *pcpRoot, CPRCCOL *pdoTree, int *piChildCnt,
                                      CUSTR & sServOut,CUSTR & sDBOut, CUSTR & sOwnerOut)
{
	CG *pcgThis;
	int idx, i;

	*piChildCnt = 0;

	if(pcpRoot == NULL || pdoTree == NULL) return NULL;
	if(pcpRoot->pLast == NULL) return NULL;

	pcgThis = pcpRoot->pLast->pcgNode;
	idx     = pcpRoot->pLast->idx;

	for(i=idx;i<pcgThis->CGSz;i++)
	{
		if(pcgThis->ppCG[i] != NULL)
		if NFLAG(pcgThis->ppCG[i]->iFlags, F_RECURSION)

    if(pdoTree->FindProc(pcgThis->ppCG[i]->sServer,pcgThis->ppCG[i]->sDB
                        ,pcgThis->ppCG[i]->sOwner,pcgThis->ppCG[i]->sName) != NULL)
		{
      if((*piChildCnt = pdoTree->GetProc(pcgThis->ppCG[i]->sServer,
                                         pcgThis->ppCG[i]->sDB,
                                         pcgThis->ppCG[i]->sOwner,
                                         pcgThis->ppCG[i]->sName
                                         ).m_dpDBObjDeps.CountAccTypes(eAT_EXEC))>0)
			{
        pcpRoot->pLast->idx = i;
        sServOut    = pcgThis->ppCG[i]->sServer; //Return with these values to caller
        sDBOut      = pcgThis->ppCG[i]->sDB;
        sOwnerOut   = pcgThis->ppCG[i]->sOwner;

        return pcgThis->ppCG[i]->sName;
			}
		}
	}

	return NULL;
}

// ==============================================================
// ================== Private Private Stuff =====================
// ==============================================================

// Name of file to reference for external DB objects
extern const char EXTERN_CALLS_FNAME[];

CCG::CALL_PATH *CCG::CreateCPNode(CG *pcgNode, int idxIn)
{
	CALL_PATH *pRet;

	if((pRet = new CALL_PATH())!= NULL)
	{
		pRet->idx     = idxIn;
		pRet->pcgNode = pcgNode;
		pRet->pNext   = NULL;
	}

	return pRet;
}

bool CCG::FindCG_ByName(const UCHAR *pcServer,const UCHAR *pcDB
                       ,const UCHAR *pcOwn,const UCHAR *pcName, CP_ROOT *plRoot)
{
	CALL_PATH *pTmp;

	pTmp = plRoot->pFirst;

	while(pTmp != NULL)
	{
		if(pTmp->pcgNode != NULL)
		if(pTmp->pcgNode->sServer.scmp_ci(pcServer) ==0 &&
       pTmp->pcgNode->sDB.scmp_ci(pcDB)         ==0 &&
		   pTmp->pcgNode->sOwner.scmp_ci(pcOwn)     ==0 &&
		   pTmp->pcgNode->sName.scmp_ci(pcName)     ==0 )
    {
			return true;
		}

		pTmp = pTmp->pNext;
	}

	return false;
}

void CCG::DisplayList(CP_ROOT *plRoot)
{
	CALL_PATH *pTmp;

	pTmp = plRoot->pFirst;

	while(pTmp != NULL)
	{
		if(pTmp->pcgNode != NULL)
		{
			if(pTmp->pNext != NULL)
				printf("%s ", (const char *)pTmp->pcgNode->sName);
			else
				printf("%s\n", (const char *)pTmp->pcgNode->sName);
		}

		pTmp = pTmp->pNext;
	}
}

void CCG::DebugList(CP_ROOT *plRoot)
{
	CALL_PATH *pTmp;

	pTmp = plRoot->pFirst;

	while(pTmp != NULL)
	{
		if(pTmp->pcgNode != NULL)
			printf("%s idx: %i\n", (const char *)pTmp->pcgNode->sName, pTmp->idx);
		else
			printf("NO CG idx: %i\n", pTmp->idx);

		pTmp = pTmp->pNext;
	}
}

void CCG::InsertCG(CG *pcgNode, int idxIn, CP_ROOT *pcpRoot)
{
	if(pcpRoot->pFirst == pcpRoot->pLast && pcpRoot->pLast == NULL)
		pcpRoot->pFirst = pcpRoot->pLast = CreateCPNode(pcgNode, idxIn);
	else
	{
		pcpRoot->pLast->pNext = CreateCPNode(pcgNode, idxIn);
		pcpRoot->pLast        = pcpRoot->pLast->pNext;
	}
}

void CCG::DestroyList(CP_ROOT *plRoot)
{
	CALL_PATH *plNode, *plTmp;

	plNode = plRoot->pFirst;

	while(plNode != NULL)
	{
		plTmp = plNode -> pNext;

		delete plNode;

		plNode = plTmp;
	}

	plRoot->pFirst = plRoot->pLast = NULL;
}

int CCG::CPLen(CP_ROOT *plRoot)
{
	int iRet=0;

	CALL_PATH *pTmp;

	pTmp = plRoot->pFirst;

	while(pTmp != NULL)
	{
		iRet++;
		pTmp = pTmp->pNext;
	}

	return iRet;
}

void CCG::DestroyLastCP(CP_ROOT *plRoot)
{
	CALL_PATH *pcpNode;

	if(plRoot->pFirst == NULL || plRoot->pLast == NULL) return;

	if(plRoot->pFirst == plRoot->pLast)      //List holds only one item
	{                                        //Destroy that item and return
		delete plRoot->pFirst;
		plRoot->pFirst = plRoot->pLast = NULL;
		return;
	}

	pcpNode = plRoot->pFirst;

	//Iterate until second last item is found -> Free it and patch list
	while(pcpNode != NULL && pcpNode->pNext != plRoot->pLast)
		pcpNode = pcpNode->pNext;

	if(pcpNode != NULL)
	if(pcpNode->pNext == plRoot->pLast)  //Free Last node and reset to 'second last'
	{
		delete plRoot->pLast;
		plRoot->pLast  = pcpNode; //Mark new end of list
		pcpNode->pNext = NULL;
	}
}

//Create a CG Node and return it
//
CCG::CG *CCG::AllocCG(const UCHAR *pcServ,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName
                     ,int ID, CUSTR & sFileName)
{
	CG *pcgRet=NULL;

	if((pcgRet = new CG())==NULL) return NULL;

  pcgRet->sServer   = pcServ;
	pcgRet->sDB       = pcDB;
	pcgRet->sOwner    = pcOwner;
	pcgRet->sName     = pcName;
	pcgRet->iProcID   = ID;
	pcgRet->sFileName = sFileName;
	pcgRet->CGSz      = 0;
	pcgRet->iFlags    = 0;
	pcgRet->ppCG      = NULL;

	return pcgRet;
}

//=================================================================================>
//Print a Pretty Tree with all Bells'n Whistles'n all and interactivity and all that
//Here we use C/C++ to print javascript code that will print DHTML code (phew)
//=================================================================================>
void CCG::PrintJSCallGraph_HTMLBody(CG *pcgCalls, FILE *fpOut, int iCntLevel,
                                    const CUSTR & sCallerName, const THIS_DB & thisDB)
{
	if(pcgCalls == NULL) return;

  int i;
  CUSTR sTmpRef;

  sTmpRef = FormatUsefulDBObjRef(pcgCalls->sServer,pcgCalls->sDB
                                ,pcgCalls->sOwner,pcgCalls->sName,thisDB);

  if FLAG(pcgCalls->iFlags, F_EXTERN_CALL)  //Have we got linkage info 2 source?
  {
    if (FLAG(pcgCalls->iFlags ,F_EXTRES_CALL)  //This is an external call a with resolved link
     && pcgCalls->sFileName.slen()>0)
    { //Print name of procedure first and link+arrow link below
      //we should escape '/' in Javascript for better HTML coding
      fprintf(fpOut, "[\'<code>%s<\\/code>\',",(const char *)sTmpRef);

      fprintf(fpOut, "'%s\'",(const char *)pcgCalls->sFileName);
    }
    else
    { //Print name of procedure first and link+arrow link below
		  fprintf(fpOut, "[\'<code><SPAN class=\\\"%s\\\">", CSS_UNKNOWN_PROC);

		  //Print full qualifier for different DBAccess or just different owner access
			fprintf(fpOut, "%s",(const char *)sTmpRef);
  		
		  //we should escape '/' in Javascript for better HTML coding
		  fprintf(fpOut, "<SPAN><\\/code>\',");

		  fprintf(fpOut, "'%s\'",EXTERN_CALLS_FNAME);
    }
	}
	else
	{
		//Print name of procedure first and link+arrow link below
		fprintf(fpOut, "[\'<code>%s",(const char *)sTmpRef);
	
		if FLAG(pcgCalls->iFlags, F_RECURSION)
			fprintf(fpOut, " (Recursion)");

		//we should escape '/' in Javascript for better HTML coding
		fprintf(fpOut, "<\\/code>\',");

		if(iCntLevel != 0)
		{
			CUSTR sKey;          //Generate unique ID for linkage
			sKey += pcgCalls->iProcID;
			sKey += "x";

			//link with resolved ID to jump to point of occurrence in source text
			if(iCntLevel == 1)
				fprintf(fpOut, "'%s|#%s\'",(const char *)pcgCalls->sFileName, (const char *)sKey);
			else
				fprintf(fpOut, "'%s|%s#%s\'",
				        (const char *)pcgCalls->sFileName,
								(const char *)sCallerName, (const char *)sKey);
		}
	}

	if(pcgCalls->CGSz<=0)
	{
		fprintf(fpOut, "],\n");
	}
	else
	{
		fprintf(fpOut, ",\n");
		for(i=0;i<pcgCalls->CGSz;i++)
		{
			PrintJSCallGraph_HTMLBody(pcgCalls->ppCG[i], fpOut, iCntLevel+1, pcgCalls->sFileName,thisDB);
		}
		fprintf(fpOut, "],\n");
	}
}

//=======================================================>
//Print a Pretty Tree with all Bells'n Whistles'n all that
//=======================================================>
void CCG::PrintJSCallGraph_HTML(FILE *fpOut)
{
  THIS_DB thisDB;
  thisDB.sServer  = this->sThisServer;
  thisDB.sDB      = this->sThisDB;
  thisDB.sDBOwner = this->sThisOwner;

	fprintf(fpOut,"<script type=\"text/javascript\" language=\"JavaScript\" src=\"res/tree.js\"></script>\n");
	fprintf(fpOut,"<script type=\"text/javascript\" language=\"JavaScript\">\n");
	fprintf(fpOut,"var TREE_ITEMS = \n");
	fprintf(fpOut,"[\n");

  PrintJSCallGraph_HTMLBody(this->pcgRoot, fpOut, 0, "", thisDB);

  fprintf(fpOut,"];\n");
	fprintf(fpOut,"</script>\n");
	fprintf(fpOut,"<script type=\"text/javascript\" language=\"JavaScript\" src=\"res/tree_tpl.js\"></script>\n\n");
}
