
#include "ccg.h"
#include "cproccol.h"
#include "copts.h"
#include "prettyprint.h"

// Name of file to reference for external db objects
extern const char EXTERN_CALLS_FNAME[];

// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->
// **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**->
// --*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_--*_->

// Initialize work sets hash function
#define INIT_HSZ 1024
#define INIT_HSZ_MASK (INIT_HSZ-1)
#define H_SEED 111

CPRCCOL::~CPRCCOL()
{
	//DEBUG();    //Debug collection at end of lifetime
	FreeHTTab();    //Get rid of hash table of table/view collection
}

CPRCCOL::CPRCCOL():iPrcRootSz(0),pphPrcRt(NULL)    //Hash table code
{
	int i;

	if((pphPrcRt = new H_PRC *[INIT_HSZ])==NULL)
		return;

	iPrcRootSz = INIT_HSZ;
	for(i=0;i<INIT_HSZ;i++)
	{
		pphPrcRt[i] = NULL;
	}
}

void CPRCCOL::CopyThis(const CPRCCOL & prcIn)
{
  this->FreeHTTab();                           //Reset Hash table collection settings

  if(prcIn.pphPrcRt == NULL || prcIn.iPrcRootSz <= 0) return; //Nothing to copy here :(

	if((pphPrcRt = new H_PRC *[prcIn.iPrcRootSz])==NULL) //Construct new table and copy each node
		return;

	this->iPrcRootSz = prcIn.iPrcRootSz;
	for(int i=0;i<this->iPrcRootSz;i++)
	{
		pphPrcRt[i] = NULL;

    H_PRC **pphInsHere = &pphPrcRt[i], *hpPRC = prcIn.pphPrcRt[i];

    while(hpPRC != NULL )
    {
      if(hpPRC->pPRC != NULL)
      {
        *pphInsHere = new H_PRC(hpPRC->pPRC); //Copy this table related info
        pphInsHere = &((*pphInsHere)->pNext);       //Look at end in destination
      }

      hpPRC=hpPRC->pNext; //Look at next node in source collection
    }
	}
}

// Copy Constructors and Equal Operators
CPRCCOL::CPRCCOL(const CPRCCOL & prcIn):iPrcRootSz(0),pphPrcRt(NULL)
{CopyThis(prcIn);}

CPRCCOL & CPRCCOL::operator=(const CPRCCOL & prcIn){CopyThis(prcIn); return *this;}
CPRCCOL & CPRCCOL::operator=(const CPRCCOL *pPRC)
{
  if(pPRC != NULL) *this = *pPRC;

	return *this;
}

//
//Hashing function is optimized for hashtable-size (INIT_HSZ) with a power of 2,
//readjust that size to suit your needs (in power of 2 steps)!
//
unsigned int CPRCCOL::hGetKey(const UCHAR *word) const
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
void CPRCCOL::FreeHTTab()
{
	int i;
  H_PRC *phTmp, *phTmp1;

	if(pphPrcRt == NULL) return;

	for(i=0;i<INIT_HSZ; i++)
	{ 
		phTmp = pphPrcRt[i];
		while(phTmp != NULL)
		{
			phTmp1 = phTmp->pNext;
			delete phTmp;          //Destroy node data via destrutor

			phTmp = phTmp1;
		}
	}

	delete [] pphPrcRt; //Free root structure
	pphPrcRt=NULL;
}

void CPRCCOL::DEBUG(FILE *fp)
{
	int i,iCnt, iMax=0, iCntZero=0;
  H_PRC *phlTmp;

	fprintf(fp, "PROC COLLECTION - DEBUG HASH:\n");

	for(i=0;i<INIT_HSZ; i++)
	{ 
		//Count nodes per hash table
		for(phlTmp = pphPrcRt[i], iCnt=0;phlTmp != NULL;
        iCnt++, phlTmp = phlTmp->pNext)
		;

		if(iMax < iCnt) iMax = iCnt;

		if(iCnt == 0) iCntZero++;

		fprintf(fp, "%i,", iCnt);

		if((i&31)==31) fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	fprintf(fp, "Max Slot: %i Zero Count: %i\n", iMax, iCntZero);

	//Count nodes per hash table
	for(i=0;i<INIT_HSZ; i++)
	{ 
		for(phlTmp = pphPrcRt[i], iCnt=0;phlTmp != NULL; iCnt++, phlTmp = phlTmp->pNext)
      phlTmp->pPRC->DebugPROC(fp);
	}
}

// *~-~ *~-~ *~-~ *~-~*~-~ *~-~*~-~ *~-~*~-~ *~-~*~-~ *~-~*~-~ *~-~
// ~-~ ~-~ ~-~  ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~ ~-~
//  ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- ~- 
// ~ ~  ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

//Print Database Object Information in HTML
//
//Print: Direct Callers, Calls and CallGraph
//
#define COL_ALIGN  "align=\"right\" valign=\"top\""
#define COL_ALIGN1 "align=\"left\" valign=\"top\""

void HTML_AccessModifCells(FILE *fpOut, const UINT eAccType, const int iKeyID)
{
	UCHAR pcKey[NUM_SZ];         //String Representation of Key to id table+Access

	if FLAG(eAccType, eAT_SELECT)
	{
		//Get table key and format anker for modifier (update,delete,insert)
		FormatTableKey(hlTableFrom, iKeyID, &pcKey[0], NUM_SZ);
		if(pcKey[0] != '\0')
			fprintf(fpOut, "<td %s><a href=\"#%s\">S</a></td>\n", COL_ALIGN1, pcKey);
		else
			fprintf(fpOut, "<td %s>S</td>\n", COL_ALIGN1);
	}
	else
		fprintf(fpOut, "<td></td>\n");

	if FLAG(eAccType, eAT_UPDATE)
	{
		FormatTableKey(hlTableUpd, iKeyID, &pcKey[0], NUM_SZ);
		if(pcKey[0] != '\0')
			fprintf(fpOut, "<td %s><a href=\"#%s\">U</a></td>", COL_ALIGN1, pcKey);
		else
			fprintf(fpOut, "<td %s>U</td>\n", COL_ALIGN1);
	}
	else
		fprintf(fpOut, "<td></td>\n");

	if FLAG(eAccType, eAT_INSERT)
	{
		FormatTableKey(hlTableIns, iKeyID, &pcKey[0], NUM_SZ);
		if(pcKey[0] != '\0')
			fprintf(fpOut, "<td %s><a href=\"#%s\">I</a></td>", COL_ALIGN1, pcKey);
		else
			fprintf(fpOut, "<td %s>I</td>\n", COL_ALIGN1);
	}
	else
		fprintf(fpOut, "<td></td>\n");

	if FLAG(eAccType, eAT_DELETE)
	{
		FormatTableKey(hlTableDel, iKeyID, &pcKey[0], NUM_SZ);
		if(pcKey[0] != '\0')
			fprintf(fpOut, "<td %s><a href=\"#%s\">D</a></td>", COL_ALIGN1, pcKey);
		else
			fprintf(fpOut, "<td %s>D</td>\n", COL_ALIGN1);
	}
	else
		fprintf(fpOut, "<td></td>\n");

	if FLAG(eAccType, eAT_TRUNC_TAB)
	{
		FormatTableKey(hlTruncTable, iKeyID, &pcKey[0], NUM_SZ);
		if(pcKey[0] != '\0')
			fprintf(fpOut, "<td %s><a href=\"#%s\">TT</a></td>", COL_ALIGN1, pcKey);
		else
			fprintf(fpOut, "<td %s>TT</td>\n", COL_ALIGN1);
	}
	else
		fprintf(fpOut, "<td></td>\n");

	if FLAG(eAccType, eAT_UPDATEStats) //More or less optional update statistics...
	{
		FormatTableKey(hlTableUpdStats, iKeyID, &pcKey[0], NUM_SZ);
		if(pcKey[0] != '\0')
			fprintf(fpOut, "<td %s><a href=\"#%s\">US</a></td>", COL_ALIGN1, pcKey);
		else
			fprintf(fpOut, "<td %s>US</td>\n", COL_ALIGN1);
	}
}

#define HTML_PROC_INTRO "<SPAN CLASS=\""CSS_PNM"\">"
#define HTML_PROC_OUTRO "</SPAN>"

//
//Print Callers/Calls FOR A given pRocEduRe (the laTTer with compLeTe callgraph)
//----------------------------------------------------------------------------->
void CPRCCOL::HTML_PrintProc(const THIS_DB & thisDB
                            ,const CUSTR   & sSrcPath
                            ,CPROC *prc                  //rendering this procedural object
                            ,const CTABCOL & tbcTabs    //Collection of tables
                            ,const CPRCCOL & prcPrcs   //Collection of procedures
                            ,const char *pcInputPath  //Relative link path from HTML2SQL
                            ,CCG *pCallGraph
                            ,FILE *fpOut
                            ,const CUSTR   & sOutputPath
                            )
{
	int iCntCallers = 0;
	CUSTR s;
	FILE *fpSQLSrc=NULL;

	if(prc == NULL) return;

	try
	{ //Print intro table...
		fprintf(fpOut, "<table id=\"NoBord\" width=\"100%%\"><tr><td %s>\n", COL_ALIGN);
    CDP::HTML_IconClassLink(fpOut,prc->GetObjType());
		fprintf(fpOut, "</td><td %s>\n", COL_ALIGN1);
	
		if(pcInputPath == NULL) //Can we link back to original source file???
			fprintf(fpOut, "%s", (const char *)prc->GetName());
		else
		{
			CUSTR sFile, sSQLSrc;
	
			sFile   = s::sGetFileName(prc->sSrcFile);
			sFile   = s::CreateFileName(pcInputPath, sFile, NULL);
			sSQLSrc = s::CreateFileName(sSrcPath, prc->sSrcFile, NULL);
	
			if((fpSQLSrc=fopen(sSQLSrc, "rb"))!=NULL)
				fprintf(fpOut, "<a href=\"%s\">%s%s%s</a>", (const char *)sFile,
				        HTML_PROC_INTRO, (const char *)prc->GetName(), HTML_PROC_OUTRO);
			else
				fprintf(fpOut, "%s", (const char *)prc->GetName());
		}

    //Collect calling procedures on the fly ...get all callers for this proc
		CDEPS *pdpsCallers = FindCalls(&iCntCallers,prc);
	
	  //Print summary information in header of page
		fprintf(fpOut, "&nbsp;(%i Callers,", iCntCallers);
		fprintf(fpOut, "&nbsp;%i Calls,", prc->m_dpDBObjDeps.CountAccTypes(eAT_EXEC));
		fprintf(fpOut, "&nbsp;%i Tables Accessed)&nbsp;", prc->m_dpDBObjDeps.CountAccTypes(eAT_DEP_ON_TABVIEW));
		fprintf(fpOut, "</td>");
	
    fprintf(fpOut, "<td Align=\"right\" valign=\"top\"><div id=\"staticMenu\"></div></td>\n");
		fprintf(fpOut, "</tr>");
	
		if(prc->GetObjType() == eREFTRIG) //Is this a trigger object?
		{
			bool bPrint=false;
			s = tbcTabs.GetTargFilename(prc->GetServer(),prc->GetDB(),prc->GetOwner(),prc->sOnTable);
	
			fprintf(fpOut, "<tr><td %s>ON ",COL_ALIGN);
      CDP::HTML_IconClassLink(fpOut,eREFTABLE);
			fprintf(fpOut, "</td><td><a href=\"%s\"><SPAN CLASS=\"%s\">%s</SPAN></a> (",
				              (const char *)s, CSS_TABLE, (const char *)prc->sOnTable);
	
			if FLAG(prc->evOnEvents, evUPDATE)      //Pretty-Print Events for this Trigger
      {
        fprintf(fpOut, "UPDATE"), bPrint=true;
      }
	
			if FLAG(prc->evOnEvents, evINSERT)
			{
				if(bPrint== true) fprintf(fpOut, ",&nbsp;");
	
				fprintf(fpOut, "INSERT"); bPrint=true;
			}
	
			if FLAG(prc->evOnEvents, evDELETE)
			{
				if(bPrint== true) fprintf(fpOut, ",&nbsp;");
	
				fprintf(fpOut, "DELETE");
			}

			fprintf(fpOut, ")</td><td>\n");
		}

    if(prc->m_csClasses.Sz() > 0) //Print classification, if available
    {
	    fprintf(fpOut, "<tr colspan=\"3\"><td>");

      const UCHAR *pcClassName = prc->m_csClasses.GetFirst();

	    while(pcClassName != NULL) //Print property names for each type in header of table
	    {
        if(thisDB.PrcClassDefs.Get_i(pcClassName) == true)
        {
          CPROP tPROP;
          thisDB.PrcClassDefs.Get(pcClassName,tPROP);
          fprintf(fpOut, "<img src=\"%s\" title=\"%s\">\n",
                  (const char*)tPROP.sGetVal("icon"), (const char*)tPROP.sGetVal("text"));
        }
		    pcClassName = prc->m_csClasses.GetNext();
	    }
    
	    fprintf(fpOut, "</td></tr>\n");
    }
    
    fprintf(fpOut, "</table>\n"); //End of one line Header Table

		fprintf(fpOut, "\n<table id=\"NoBord\" cellPadding=\"3\" cellSpacing=\"0\" width=\"100%%\">\n");
		fprintf(fpOut, "<tr><td align=\"left\" valign=\"top\">\n");

    int iCallsCnt  = prc->m_dpDBObjDeps.CountAccTypes(eAT_EXEC);
	
		//Count access statements that are implemented by this proc
		int iAccessCnt  = prc->m_dpDBObjDeps.CountObjTypes(eREFVIEW);
		    iAccessCnt += prc->m_dpDBObjDeps.CountObjTypes(eREFTABLE);
		    iAccessCnt += prc->m_dpDBObjDeps.CountObjTypes(eREFTABVIEW);
	
		//Print intro to JavaScript TreeView
		if(iCallsCnt > 0 && pCallGraph != NULL)
			pCallGraph->PrintJSCallGraph_HTML(fpOut);

		eREFOBJ eObjType=eREFUNKNOWN;
		CUSTR sServer,sDB, sUser, sDBObj;
		UINT eacAccess=eAT_UNKNOWN;
		
		if(pdpsCallers != NULL)
			sDBObj = pdpsCallers->GetFirst(eObjType,sServer,sDB, sUser,eacAccess);

		//Display List of Callers and Calls for this DB-Objekt
		if(sDBObj.slen() > 0 || iCallsCnt > 0 || iAccessCnt > 0)
		{
			fprintf(fpOut, "<br>\n<table id=\"Bord\">\n");
			fprintf(fpOut, "<thead><tr>\n");
	
			//print header information
			if(sDBObj.slen() > 0)
				fprintf(fpOut, "<TD id=\"BordHead\">Callers</TD>\n");
			
			if(iCallsCnt > 0)
				fprintf(fpOut, "<TD id=\"BordHead\">Calls</TD>\n");
	
			if(iAccessCnt > 0)
				fprintf(fpOut, "<TD id=\"BordHead\">Tables/Views</TD>\n");
			
			fprintf(fpOut, "</tr></thead><tr>\n");
	
			if(sDBObj.slen() > 0)
			{               //Print list of caller's, if first caller is available
				CPROC *pPRC; //We have collected calling procs on the fly...(further above)

				fprintf(fpOut, "<td id=\"Bord\">\n<table id=\"NoBord\">\n");
				while(sDBObj.slen() > 0)
				{
					fprintf(fpOut, "<tr>\n");
					CUSTR s("UNKNOWN");
          //1> Print only links to local callers
          if(s::scmp_ci2(thisDB.sDB, thisDB.sServer, sDB,sServer)==0)
          {
					  if((pPRC = this->FindPROCNode(this->pphPrcRt, sServer,sDB,sUser,sDBObj)) != NULL)
						  s = pPRC->GetTargFileName();

            fprintf(fpOut, "<td id=\"NoBord\"><a href=\"%s\"><SPAN CLASS=\"%s\">%s</SPAN></a></td>",
                    (const char *)s,CSS_PNM,(const char *)sDBObj);
          }
          else //2> Print only external callers
          {
            s  = s::sGetPath(pdpsCallers->GetHTML_LINK());   //Get Path to DB External Object
            s  = s::GetRelPath(sOutputPath, s);
            s += s::sGetFileName(pdpsCallers->GetHTML_LINK());

            fprintf(fpOut, "<td id=\"NoBord\"><a href=\"%s\"><SPAN CLASS=\"%s\">%s.%s.%s</SPAN></a></td>",
                    (const char *)s,CSS_PNM,(const char *)sDB,(const char *)sUser,(const char *)sDBObj);
            
          }
					fprintf(fpOut, "</tr>\n");
	
					sDBObj = pdpsCallers->GetNext(eObjType,sServer,sDB, sUser,eacAccess);
				}
				fprintf(fpOut, "</table>\n");
			}
			
			if(iCallsCnt > 0) //print callgraph as java-script tree
			{
				fprintf(fpOut,"<td id=\"Bord\">");
				fprintf(fpOut,"<script type=\"text/javascript\" language=\"JavaScript\">\n");
				fprintf(fpOut,"<!--//\n");
				fprintf(fpOut,"  new tree (TREE_ITEMS, TREE_TPL);\n");
				fprintf(fpOut,"//-->\n");
				fprintf(fpOut,"</script></td>\n");
			}

			if(iAccessCnt > 0)
			{
				fprintf(fpOut, "<td id=\"Bord\">\n<table id=\"NoBord\" cellpadding=\"3\" cellSpacing=\"0\">\n");
				const UCHAR *pcName;
				eREFOBJ eObjType;
				UINT eacAccess;

				//Print overview information on tables/views being accessed from this code
				for(pcName = prc->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eacAccess);
				    pcName != NULL;
				    pcName = prc->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eacAccess))
				{
					if(eObjType == eREFVIEW || eObjType == eREFTABLE || eObjType == eREFTABVIEW)
					{
						//Link2Definition File
						CUSTR s = tbcTabs.GetTargFilename(sServer,sDB,sUser,pcName);

            //Attempt to resolve external links to other databases (indexing more than one DB)
            eREFOBJ tbcTBLClass=eREFUNKNOWN;    //return view/table or unknown
            eREFOBJ ObjType=eREFPROC;
            if(prc->GetObjType() == eREFTRIG) ObjType = eREFTRIG;

            s = sResolveExtDBLink(prcPrcs, tbcTabs,thisDB,prc->GetOwner(),prc->GetName(),ObjType
                                 ,sOutputPath,sServer,sDB,sUser,pcName,s,tbcTBLClass);

            if(tbcTBLClass != eREFUNKNOWN) eObjType = tbcTBLClass;

            fprintf(fpOut, "<tr>\n");
						fprintf(fpOut, "<td valign=\"top\" align=\"left\"><a href=\"%s\">",(const char *)s);
						CUSTR sPrint = FormatUsefulDBObjRef(sServer,sDB,sUser,pcName, thisDB);
            CDP::HTML_Color_Obj(fpOut, eObjType, sPrint);
						fprintf(fpOut, "</a></td>");
	
						HTML_AccessModifCells(fpOut,eacAccess,tbcTabs.GetTable(sServer,sDB,sUser,pcName).GetID());
						fprintf(fpOut, "</tr>\n");
					}
				}
				fprintf(fpOut, "</table></td>\n");
			}
			fprintf(fpOut, "</tr></table>\n"); //End of outter visible table
		}

    fprintf(fpOut, "</td><td Align=\"right\" valign=\"top\">\n");
		if(prc->m_prpProps.Size() > 0)         //Unwind XML properties, if available
		{
			fprintf(fpOut, "<small>");
			CUSTR sName, sVal;
			for(int i=0;i<prc->m_prpProps.Size();i++)
			{
				prc->m_prpProps.GetProp(i, sName, sVal);
				if((i+1)<prc->m_prpProps.Size())
					fprintf(fpOut,"%s=\"%s\"<br>",(const UCHAR *)sName,(const UCHAR *)sVal);
				else
					fprintf(fpOut,"%s=\"%s\"",(const UCHAR *)sName,(const UCHAR *)sVal);
			}
			fprintf(fpOut, "</small>\n");
		}
		fprintf(fpOut, "<td>\n");

		fprintf(fpOut, "</tr></table>\n"); //Last row of intro table
	
		//Output Sql with highlighting and linkage information
    eREFOBJ depPrcType = prc->GetObjType();
    CUSTR sAbsOutPath;
    sAbsOutPath = s::GetAbsPath(s::sGetCWD(), sOutputPath);

    HTML_HighLightSQL(prcPrcs,tbcTabs,prc->m_dpDBObjDeps,prc->hlPatrn
                    ,prc->GetOwner()          //Name of owner for this object
                    ,prc->GetName()          //Name of this object
                    ,depPrcType
                    ,fpSQLSrc,fpOut,thisDB,sAbsOutPath);

    delete pdpsCallers;

		fprintf(fpOut,"\n");
		FCLOSE(fpSQLSrc);
	}
	catch(CERR e)
	{
		printf("ERROR: CPROCCOL::HTML_PrintProc %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: CPROCCOL::HTML_PrintProc An error has occured!\n");
	}
}

#undef COL_ALIGN
#undef HTML_PROC_INTRO
#undef HTML_PROC_OUTRO

bool CPRCCOL::bMatchNode(const UCHAR *pcServer,const UCHAR *pcDB
                        ,const UCHAR *pcOwner,const UCHAR *pcName, H_PRC *phtbNode) const
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

//
//Find a PRC_TREE node with pcKey as name, in other words:
//Find the main entry for a Procedure/Trigger called pcKey
//-------------------------================*****00000000000>
CPRCCOL::H_PRC *CPRCCOL::FindPRCNode(H_PRC **pphRoot,const UCHAR *pcServer
                                    ,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName) const
{
	//hash name and attempt to find target
  for(H_PRC *phlTmp = pphRoot[this->hGetKey(pcName)]; phlTmp != NULL;
      phlTmp = phlTmp->pNext)
  {
    if(this->bMatchNode(pcServer,pcDB,pcOwner,pcName,phlTmp) == true)
      return phlTmp;
  }

  return NULL;
}

//When a references from a procedure to a table changes, because we can now
//tell this reference goes to another object (same name but belonging to another owner)
//We need to reset and delete a few things here and there, this function does the work
//------------------------------------------------------------------------------------
void CPRCCOL::ResetTableOwnerRef(CPROC *pPRC, const UCHAR *pcServer,const UCHAR *pcDB
                                ,const UCHAR *pcUser, const UCHAR *pcName
		                            ,UINT eatAccess, CTABCOL & tbcTabs
                                ,const CUSTR sUniqueOwnerIn, const eREFOBJ eTableTypeIn)
{
  eREFOBJ eTableType;
  CUSTR sUniqueOwner;

  if(sUniqueOwnerIn.slen() <= 0)
    tbcTabs.FindUniqueOwnerOfKnownTab(pcServer,pcDB,pcName, sUniqueOwner, eTableType);
  else
  {
    sUniqueOwner = sUniqueOwnerIn;
    eTableType   = eTableTypeIn;
  }

  if(sUniqueOwner.slen() > 0)
  {
    eREFOBJ eTAB_DEP_TYPE = eTableType;

    //Remove assumed dependency and insert corrected version (Reset name of owner and type of dependence)
    pPRC->m_dpDBObjDeps.RemoveAssumedDeps(pcServer,pcDB,pcUser, pcName);
    pPRC->m_dpDBObjDeps.AddDep(pcServer,pcDB, sUniqueOwner, pcName, eTAB_DEP_TYPE, eatAccess, 0);

    //Add this newly configured dependence to collection of table objects
    CDEP depNewDep(pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner(),pPRC->GetName(),eREFPROC,eatAccess, 0);
    tbcTabs.AddTableDepend(pcServer,pcDB,sUniqueOwner,pcName, eTableType, &depNewDep);

    //type of highlighting needs to be changed as well
    pPRC->hlPatrn.ResetRef2AmbigousOwner(pcServer,pcDB,pcUser,pcName, sUniqueOwner,mFLAG_ASSUMED_OWNER);

    tbcTabs.RemoveAssumedTableDeps(pcServer,pcDB,pcUser,pcName
                                  ,pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner(), pPRC->GetName());

//    printf("Assumed dependence found in %s.%s: pointing at:",
//      (const char *)pPRC->GetOwner(),(const char *)pPRC->GetName());
//    printf("%s.%s -> Corrected to: %s\n",
//      (const char *)pcUser,(const char *)pcName, (const char *)sUniqueOwner);
  }
}

//Resolve unknown table objects (couldn't decide for view or table up to here)
//Z==========================================================================Z
//Z                                                                          Z
void CPRCCOL::ResolveUnknownTables(H_PRC **pphPrcIn           //Collection of Proc Objects in DB
                                  ,CTABCOL & tbcTabs   //Collection of Tab Objects in DB
                                  //Name of database this is stored in
                                  ,const UCHAR *pcServer, const UCHAR *pcDB
                                  )
{
	if(pphPrcIn == NULL) return;
	
	try
	{	
		const UCHAR *pc;
		UINT eatAccess;
		eREFOBJ eObjType;
		CUSTR sServer, sDB, sUser;

		//Run through each node in each hash bucket
		for(int i=0;i<INIT_HSZ; i++)
		{
			H_PRC *phlTmp;
			for(phlTmp = pphPrcIn[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
			{
				sDB  ="", sUser="";
        CPROC *pPRC = phlTmp->pPRC;   //This is just a handy short-hand
        if(pPRC==NULL) continue;

				pc=pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);

				while(pc != NULL)
				{
					if(eObjType == eREFTABVIEW)
					{
            //Attempt to find this object in the list of table objects, if this fails and iFlags indicates an assumed user,
            //then we try again, this time without explicit user and we succeed only if ONE object with required name was found
            //(Ref stays unresolved, if there are two objects with same name but different owner AND our reference does not explicitly indicate the owner)
            const CTAB *ptbNode = tbcTabs.FindTab(sServer,sDB,sUser,pc);

						//Resolve unknown tables, if more info is available ...
						if(ptbNode != NULL)
						{
							switch(ptbNode->GetObjType())
							{
							case eREFTABLE:
								pPRC->m_dpDBObjDeps.SetObjType(eREFTABLE);
								break;
							case eREFVIEW:
								pPRC->m_dpDBObjDeps.SetObjType(eREFVIEW);
								break;
							case eREFUNKNOWN:  //If object is DB internal and owner is asumed, we try resolution_again
                if(s::scmp_ci2(sDB,sServer,pcDB,pcServer)== 0)
                if FLAG(pPRC->m_dpDBObjDeps.iGetFlags(), mFLAG_ASSUMED_OWNER)
                {
                  CUSTR sTmpServ(pcServer),sTmpDB(pcDB), sTmpName(pc);
                  //De-reference strings because memory will be freed
                  ResetTableOwnerRef(pPRC, sTmpServ, sTmpDB, sUser, sTmpName, eatAccess, tbcTabs);
                  
                  //Erase assumed flag, because we got this right for once and all (otherwise we loop 'til XMAS)
                  pPRC->m_dpDBObjDeps.iSetFlags(pPRC->m_dpDBObjDeps.iGetFlags() & (!mFLAG_ASSUMED_OWNER));

                  //Above function will change list_of_deps -> So, we start searching at the start
                  pc=pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
                  continue;
                }
              break;
              default:
								break;
							}
	
              if(s::scmp_ci2(sDB,sServer,pcDB,pcServer)!= 0)  //Thats a call to an external db object
								 pPRC->m_dpDBObjDeps.SetAccType(eAT_ACC_EXT);
							else
							{                          //Thats a call to an internal db object
								phlTmp->pPRC->m_dpDBObjDeps.UnSetAccType(eAT_ACC_EXT);
							}
						}
						else //Got no definition for this???
						{  //If the owner is assumed we try resolution with another owner,
              //otherwise, if it is external, its undefined, unknown, you nameit
              if FLAG(pPRC->m_dpDBObjDeps.iGetFlags(), mFLAG_ASSUMED_OWNER)
              {
                //De-reference strings becaue memory will be freed
                CUSTR sTmpServ(pcServer),sTmpDB(pcDB), sTmpName(pc);
                ResetTableOwnerRef(pPRC, sTmpServ, sTmpDB, sUser, sTmpName, eatAccess, tbcTabs);

                //Erase assumed flag, because we got this right for once and all (otherwise we loop 'til XMAS)
                pPRC->m_dpDBObjDeps.iSetFlags(pPRC->m_dpDBObjDeps.iGetFlags() & (!mFLAG_ASSUMED_OWNER));

                //Above function will change list_of_deps -> So, we start searching at the start
                pc=pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
                continue;
              }

              if(s::scmp_ci2(sDB,sServer,pcDB,pcServer)!= 0)
								pPRC->m_dpDBObjDeps.SetAccType(eAT_ACC_UNDEF);
						}
					}
					else  //eObjType != eOBJUNKNOWNTABLE
					{    //End of table - have a dependency on proc/trig object
              //if it is external, its undefined, unknown, you nameit
						if(s::scmp_ci2(sDB,sServer,pcDB,pcServer)!= 0)
						if(FindPRCNode(pphPrcIn, sServer,sDB, sUser, pc) == NULL)
							pPRC->m_dpDBObjDeps.SetAccType(eAT_ACC_UNDEF);
					}

					sServer="", sDB  ="", sUser="";
					pc=pPRC->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eatAccess);
				}
			}		  
    }
	}
	catch(CERR e)
	{
		printf("ERROR: CPRCCOL::ResolveUnknownTables %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: CPRCCOL::ResolveUnknownTables An error has occured!\n");
	}
}

//Z==========================================================================Z
//Resolve unknown procedure objects (exec <owner><proc> statements)
//
//The parsing algorithm may create some objects and references to them under
//a wrongly assumed user, here we correct this mistake and go for the correct
//user, if a better reference is available
//Z==========================================================================Z
void CPRCCOL::ResolveUnknownProcs(H_PRC **pphPrcIn        //Collection of Proc Objects in DB
                                  ,const UCHAR *pcServer
                                  ,const UCHAR *pcDB     //Name of database this is stored in
                                  )
{
	if(pphPrcIn == NULL) return;
	
	try
	{	
		const UCHAR *pc;
		UINT eatAccess;
		eREFOBJ eObjType;
		CUSTR sServer,sDB, sUser;

		//Run through each node in each hash bucket
		for(int i=0;i<INIT_HSZ; i++)
		{
			H_PRC *phlTmp;
			for(phlTmp = pphPrcIn[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
			{
				sDB  ="", sUser="";
        CPROC *pPRC = phlTmp->pPRC;   //This is just a handy short-hand
        if(pPRC==NULL) continue;

				pc=pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);

				while(pc != NULL)
				{
					if(eObjType == eREFPROC)
					{
            //Attempt to find this object in the list of procs, if this fails and iFlags
            //indicates an assumed user, then we try again, this time without explicit user
            //and we succeed only if ONE object with required name was found
            //(Ref stays unresolved, if there are two objects with same name but different owner
            // AND our reference does not explicitly indicate the owner)
            const CPROC *ptbNode = this->FindProc(sServer,sDB,sUser,pc);

						//Resolve unknown tables, if more info is available ...
						if(ptbNode == NULL)
						{
              if(s::scmp_ci2(sDB,sServer, pcDB,pcServer)== 0)
              if FLAG(pPRC->m_dpDBObjDeps.iGetFlags(), mFLAG_ASSUMED_OWNER)
              {
                //Erase assumed flag, because we got this right for once and all (otherwise we'll loop 'til XMAS)
                pPRC->m_dpDBObjDeps.iSetFlags(pPRC->m_dpDBObjDeps.iGetFlags() & (!mFLAG_ASSUMED_OWNER));
                CUSTR sUniqueOwner;

                if(this->FindUniqueOwnerOfKnownProc(sServer,pcDB,pc, sUniqueOwner,eREFPROC)==true)
                {
                  CUSTR sName(pc);
                  //Remove assumed dependency and insert corrected version (Reset name of owner and type of dependence)
                  pPRC->m_dpDBObjDeps.RemoveAssumedDeps(sServer,sDB,sUser,sName);
                  pPRC->m_dpDBObjDeps.AddDep(sServer,sDB, sUniqueOwner, sName, eREFPROC, eatAccess, 0);

                  //type of highlighting needs to be changed as well
                  pPRC->hlPatrn.ResetRef2AmbigousOwner(sServer,sDB,sUser,sName, sUniqueOwner,mFLAG_ASSUMED_OWNER);

                  //printf("Assumed dependence found in %s.%s: pointing at:",
                  //  (const char *)pPRC->GetOwner(),(const char *)pPRC->GetName());
                  //printf("%s.%s\n",(const char *)sUser,(const char *)sName);

                  //pPRC->DebugPROC(stdout);
                  //Above function will change list_of_deps -> So, we start searching at the start
                  pc=pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
                  continue;
                }
              }
            }
          }

          sServer="",sDB  ="", sUser="";
					pc=pPRC->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eatAccess);
        } //End of while loop for this list of dependencies
			}	 //End of hash linked-list iterator
    }   //End of hash bucket iterator
	}
	catch(CERR e)
	{
		printf("ERROR: CPRCCOL::ResolveUnknownProcs %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: CPRCCOL::ResolveUnknownProcs An error has occured!\n");
	}
}

bool CPRCCOL::FindUniqueOwnerOfKnownProc(const UCHAR *pcServer,const UCHAR *pcDB, const UCHAR *pcDBObjName
                                        ,CUSTR & sRetOwner, const eREFOBJ eProcType)
{
  sRetOwner="";

	int hval = this->hGetKey(pcDBObjName);

	if(this->pphPrcRt[hval] == NULL) return false; //Nothing found here

	H_PRC *pFind; //Find point of occurance...
	
	pFind = this->pphPrcRt[hval];

	while(pFind != NULL)           //...in list of nodes
	{
		if(pFind->sServer.scmp_ci(pcServer)  == 0 &&
       pFind->sDB.scmp_ci(pcDB)          == 0 &&
       pFind->sName.scmp_ci(pcDBObjName) == 0 &&
       pFind->pPRC->GetObjType() == eProcType)
			break;
		else
			pFind = pFind->pNext;
	}

  if(pFind == NULL) return false; //Nothing found here
  sRetOwner = pFind->sOwner;
  pFind = pFind->pNext;

	while(pFind != NULL)           //...search remainder of list
	{                             //to make sure there is no ambigouity left
		if(pFind->sServer.scmp_ci(pcServer)  == 0 &&
       pFind->sDB.scmp_ci(pcDB)          == 0 &&
       pFind->sName.scmp_ci(pcDBObjName) == 0 &&
       pFind->pPRC->GetObjType()         == eProcType)
			break;
		else
			pFind = pFind->pNext;
	}

  if(pFind != NULL) return false; //Reference to owner is not unique
  else
    return true;                //There was no other reference
}

#undef INIT_HSZ

// <^> PUBLIC FUNCTIONS <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^>
//  <^> PUBLIC FUNCTIONS <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^>
//   <^> PUBLIC FUNCTIONS <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^> <^>

//
// Add new proc node into proc collection
// This will use the += CPROC operator, if entry was already there
// ****************************************************************
#define _SRC "CPRCOL::AddProc"
void CPRCCOL::AddProc(CPROC *prcObject)
{
	if(prcObject == NULL) return;

  CDP::Handle_Requi_Params(prcObject->GetServer(),prcObject->GetDB()
                          ,prcObject->GetOwner(),prcObject->GetName(), "CPRCOL::AddProc");

	int hval = this->hGetKey(prcObject->GetName());

	//Hash bucket is empty so we insert right here
	if(pphPrcRt[hval] == NULL)
	{
    pphPrcRt[hval]=new H_PRC(prcObject->GetServer(),prcObject->GetDB()
                            ,prcObject->GetOwner(),prcObject->GetName(),prcObject);
		return;
	}

	H_PRC **ppFind; //Find point of insertion/change...
	
	ppFind = &pphPrcRt[hval];

	while(*ppFind != NULL)           //...in list of nodes
	{
    if((*ppFind)->sServer.scmp_ci(prcObject->GetServer()) == 0 && 
       (*ppFind)->sDB.scmp_ci(prcObject->GetDB())         == 0 &&
       (*ppFind)->sOwner.scmp_ci(prcObject->GetOwner())   == 0 &&
       (*ppFind)->sName.scmp_ci(prcObject->GetName())     == 0 )
			break;
		else
			ppFind = &((*ppFind)->pNext);
	}

	if(*ppFind == NULL)               //Insert completely new entry at end of list
	{
    *ppFind = new H_PRC(prcObject->GetServer(), prcObject->GetDB()
                       ,prcObject->GetOwner(),prcObject->GetName(),prcObject);

		return;
	}

	*((*ppFind)->pPRC) += *prcObject; //Add additional data for this object

	return;
}
#undef _SRC

//Resolve flag for external and internal DataBase Calls
//Here we seperate references to external procedures
//from invalid references (references to undefined internal objects)
void CPRCCOL::ResolveExtIntDBCalls(const UCHAR *pcServer,const UCHAR *pcDB)
{
	if( pphPrcRt == NULL) return;

	//Run through each node in each hash bucket
	for(int i=0;i<iPrcRootSz; i++)
	{
		H_PRC *phlTmp;
		eREFOBJ eObjType; //Cycle through deps and get Direct Calls for this proc
		UINT eacAccess;
		const UCHAR *pcCall;
		CUSTR sServer,sDB, sUser;
		CPROC *pPrc;

		for(phlTmp = pphPrcRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			if((pPrc=phlTmp->pPRC) != NULL)
			if(pPrc->m_dpDBObjDeps.CountAccTypes(eAT_EXEC) > 0)
			{
				eObjType   = eREFUNKNOWN;  //Cycle through deps and get
				eacAccess  = eAT_UNKNOWN; //Direct Calls for this proc
				sDB = sUser="";

				for(pcCall= pPrc->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eacAccess);
				    pcCall != NULL;
				    pcCall= pPrc->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eacAccess))
				{
          if(s::scmp_ci2(sServer,sDB, pcServer,pcDB) != 0) //A call to an external db object
						pPrc->m_dpDBObjDeps.SetAccType(eAT_ACC_EXT);
					else
					{                              //Thats a call to an internal db object
						pPrc->m_dpDBObjDeps.UnSetAccType(eAT_ACC_EXT);

                                          //Have we got a definition for this???
						if(FindPRCNode(pphPrcRt, sServer, sDB, sUser, pcCall) == NULL)
							pPrc->m_dpDBObjDeps.SetAccType(eAT_ACC_UNDEF);
					}
				}
			}
		}
	}
}

//
//Browse the collection of procs and add each trigger/event defs to Tables
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CPRCCOL::Trigger2Tables(CTABCOL & tbTables, const UCHAR *pcServer, const UCHAR *pcDB)
{
	if( pphPrcRt == NULL) return;

	//Run through each node in each hash bucket
	for(int i=0;i<iPrcRootSz; i++)
	{
		H_PRC *phlTmp;   //Cycle proc collection and add trigger info to table coll
		CPROC *pPrc;

		for(phlTmp = pphPrcRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			if((pPrc = phlTmp->pPRC) != NULL)            //Got a trigger definition???
        if(pPrc->GetObjType() == eREFTRIG &&
           s::scmp_ci2(pPrc->GetDB(), pPrc->GetServer(),pcDB,pcServer)==0)
			{
				//Add this trigger and events into the tree of table objects
				tbTables.AddTrigger2Tab(pPrc->GetServer(), pPrc->GetDB(), pPrc->GetOwner(),
				                        pPrc->sOnTable, pPrc->GetName(), (EVENT_CL)pPrc->evOnEvents);
			}
		}
	}
}

//Create a procedure object for every call to an undefined procedure,
//the only reason why we do this is to create a unique id, which in
//turn can be used for linkage
//=================================>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CPRCCOL::CreateUnknownProcEntries()
{
	if( pphPrcRt == NULL) return;

	try
	{
		CDEPS *pCol = new CDEPS();
	
		//Run through each node in each hash bucket
		for(int i=0;i<iPrcRootSz; i++)
		{
			H_PRC *phlTmp;
			//Cycle through proc coll and add info for calls to external/undef objects
			const UINT eAccMask = eAT_ACC_EXT | eAT_ACC_UNDEF;
	
			for(phlTmp = pphPrcRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
			{
				phlTmp->pPRC->m_dpDBObjDeps.GetDependencyFilter2(eREFPROC,eAccMask,pCol);
				phlTmp->pPRC->m_dpDBObjDeps.GetDependencyFilter2(eREFTRIG,eAccMask,pCol);
			}
		}

		eREFOBJ eObjType = eREFUNKNOWN;
		CUSTR sServer,sDB,sUser;
		UINT eacAccess = eAT_UNKNOWN;
		const UCHAR *pc;
	
    for(pc = pCol->GetFirst(eObjType, sServer,sDB,sUser, eacAccess); pc != NULL;
        pc = pCol->GetNext(eObjType, sServer,sDB,sUser, eacAccess))
		{
			CPROC *prcObj=NULL;
	
			if((prcObj = new CPROC(sServer,sDB,sUser,pc, eREFUNKNOWN))!= NULL)
			{
				AddProc(prcObj);
				delete prcObj;
			}
		}
	
		delete pCol;
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

// Get a proc and return it by reference
// ===========================================>
CPROC & CPRCCOL::GetProc(const UCHAR *pcServer,const UCHAR *pcDB
                        ,const UCHAR *pcOwner,const UCHAR *pcName) const
{
	H_PRC *prc;

	if((prc = FindPRCNode(this->pphPrcRt, pcServer,pcDB,pcOwner,pcName))==NULL)
	{
		CUSTR s("ERROR: Can not find proc: ");
		s += pcName;
		s += " Database: ";
		s += pcDB;

		throw CERR(s,ERR_INTERNAL, "CPRCCOL::GetProc");
	}

	return *prc->pPRC;
}

CDEPS *CPRCCOL::GetUnknownTabEntries() const
{
	if( pphPrcRt == NULL) return NULL;

	try
	{
		CDEPS *pCol = new CDEPS();
	
		//Run through each node in each hash bucket
		for(int i=0;i<iPrcRootSz; i++)
		{
			H_PRC *phlTmp;
			//Cycle through proc coll and add info for calls to external/undef objects
			const UINT eAccMask = eAT_DEP_ON_TABVIEW;
	
			for(phlTmp = pphPrcRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
			{
				phlTmp->pPRC->m_dpDBObjDeps.GetDependencyFilter2(eREFPROC,eAccMask,pCol);
				phlTmp->pPRC->m_dpDBObjDeps.GetDependencyFilter2(eREFTRIG,eAccMask,pCol);
			}
		}
		return pCol;
	}
	catch(CERR e)
	{
		printf("ERROR: CPRCCOL::GetUnknownTabEntries %s\n", (const char *)e.getMsg());
		return NULL;
	}
	catch(...)
	{
		printf("ERROR: CPRCCOL::GetUnknownTabEntries An error has occured!\n");
	}

	return NULL;
}

#undef COL_ALIGN
#undef HTML_PROC_INTRO
#undef HTML_PROC_OUTRO

//==========================================================================>
//Collect references to unknown procedure database objects
//Parameter:
//iFlagCollect = 0 collect all unknown procedures (internal and external)
//iFlagCollect = 1 collect only internal unknown proc objects
//                 sDB == ptbTable->sDB
//
//iFlagCollect = 2 collect only external unknown proc objects
//                 sDB != ptbTable->sDB
//               |
//               +-> if pitDBs is set, we return only matched entries in pitDBs
//
// Return: Collection of references (eOBJPROC, sName, sDB pairs)
//         to above objects
//!!! Caller must free returned collection of database object references !!!
//==========================================================================>
#define _SRC "CPRCCOL::FindUnknownObjects"
CDEPS *CPRCCOL::FindUnknownObjects(int iFlagCollect, const CUSTR & sServ,const CUSTR & sDB
                                  ,CPITEM *pitDBs)
{
	if(pphPrcRt == NULL) return NULL;

	CDEPS *pdpRet = NULL;

	if((pdpRet = new CDEPS())== NULL) return NULL;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	{
		H_PRC *phlTmp;
    CPROC *pPRC;
		for(phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
      pPRC = phlTmp->pPRC;
			if(phlTmp->pPRC->GetObjType() == eREFUNKNOWN)
			{
				switch(iFlagCollect)
				{
					case 0:           //Add all references of any kind (internal/external)
            pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner()
                          ,pPRC->GetName(),eREFPROC,eAT_UNKNOWN,0);
						break;
					case 1:                                 //Add only internal references
            if(s::scmp_ci2(sDB,sServ, pPRC->GetDB(),pPRC->GetServer()) == 0)
              pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner()
                            ,pPRC->GetName(),eREFPROC,eAT_UNKNOWN,0);
						break;
					case 2:                                 //Add only external references
            if(s::scmp_ci2(sDB,sServ, pPRC->GetDB(),pPRC->GetServer()) != 0)
            {
              if(pitDBs == NULL)
                pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner()
                          ,pPRC->GetName(),eREFPROC,eAT_UNKNOWN,0);
              else
              {
                //Add dependency only, if it points into one of these DBs (otherwise skip)
                if(pitDBs->FindDBItem(pPRC->GetServer(),pPRC->GetDB()) == true)
                  pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner()
                          ,pPRC->GetName(),eREFPROC,eAT_UNKNOWN,0);
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
UINT CPRCCOL::ResolveForeignDBDeps(const CUSTR & sAbsOutputPath, const THIS_DB & thisDB, CDEPS *pdpDB)
{
  if(pdpDB == NULL) return 0;

	const UCHAR *pc;
	UINT eatAccess, iRet=0;
	eREFOBJ eObjType;
	CUSTR sServer, sDB, sUser, sNewOwner;

	pc = pdpDB->GetFirst(eObjType,sServer,sDB, sUser,eatAccess);

	while(pc != NULL)
	{
    sNewOwner = "";

    //Attempt to look up this database object, if it is a dependency on a procedure
    if(eObjType == eREFPROC)
    if(s::scmp_ci2(thisDB.sDB,thisDB.sServer,sDB,sServer) == 0)
    {
      const H_PRC *pPrc=NULL;

      if((pPrc = this->FindPRCNode(pphPrcRt, sServer,sDB,sUser,pc))== NULL)
      {
        //Second chance: attempt match under alternative user
        if(this->FindUniqueOwnerOfKnownProc(sServer, sDB, pc, sNewOwner, eREFPROC)==true)
        {
          pPrc = this->FindPRCNode(pphPrcRt, sServer,sDB,sNewOwner,pc);
          //printf("Re-aligning %s.%s.%s.%s to %s.%s.%s.%s\n"
          //  ,(const char *)sServer,(const char *)sDB,(const char *)sUser, pc
          //  ,(const char *)sServer,(const char *)sDB,(const char *)sNewOwner,pc);
        }
      }

      if(pPrc != NULL)
      if(pPrc->pPRC != NULL)
      if(pPrc->pPRC->GetObjType() == eREFPROC) //Triggers are not supported here.
      {
        iRet++;
        CUSTR sLink(sAbsOutputPath);
        sLink += pPrc->pPRC->GetTargFileName();

        if(sNewOwner.slen() > 0)
          pdpDB->SetNewOwner(sNewOwner); //Link is valid for this alternative owner

        pdpDB->SetHTML_LINK(sLink);
        pdpDB->SetObjType(eREFPROC);
        //printf("Resolving ref for: %s.%s.%s -> %s\n",(const char *)sDB //Debugging loop...
        //      ,(const char *)sUser,pc,(const char *)sLink);
      }
    }
		pc = pdpDB->GetNext(eObjType,sServer,sDB, sUser,eatAccess);
	}

  return iRet;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//Assign a new owner for this dependency,
//if the initially assumed user appears to be wrong
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CPRCCOL::ReAssignTableOwnerRef(CPROC *pPrc, CTABCOL & tbcTabs, CDEP & dpTmpCpy
                              ,const UINT eatAccess,   const CUSTR & sNewUser
                              ,const CUSTR & sLinkPath,const eREFOBJ & eRefType)
{
  pPrc->m_dpDBObjDeps.RemoveAssumedDeps(dpTmpCpy.sServer,dpTmpCpy.sDB,dpTmpCpy.sOwner,dpTmpCpy.sObjName);

  tbcTabs.RemoveAssumedTableDeps(dpTmpCpy.sServer,dpTmpCpy.sDB,dpTmpCpy.sOwner,dpTmpCpy.sObjName,
                                 pPrc->GetServer(),pPrc->GetDB(),pPrc->GetOwner(),pPrc->GetName());
  
  tbcTabs.RemoveAssumedTables(); //First remove dependency, and then remove tables without deps

  //type of highlighting needs to be changed as well
  pPrc->hlPatrn.ResetRef2AmbigousOwner(dpTmpCpy.sServer,dpTmpCpy.sDB,dpTmpCpy.sOwner,dpTmpCpy.sObjName
                                       ,sNewUser,mFLAG_ASSUMED_OWNER);

  dpTmpCpy.sOwner          = sNewUser; //Reset owner,Link and type info from info collection
  dpTmpCpy.sHTLMLink2ExtDB = sLinkPath;
  dpTmpCpy.eObjType        = eRefType;

  //Erase assumed flag, because we got this right for once and all (otherwise we loop 'til XMAS)
  dpTmpCpy.iFlags    = dpTmpCpy.iFlags & (!mFLAG_ASSUMED_OWNER);
  pPrc->m_dpDBObjDeps.AddDep(dpTmpCpy);

  CDEP depNewDep(pPrc->GetServer(),pPrc->GetDB(),pPrc->GetOwner(),pPrc->GetName()
                ,pPrc->GetObjType(),eatAccess, 0);
  tbcTabs.AddTableDepend(dpTmpCpy.sServer,dpTmpCpy.sDB,sNewUser,dpTmpCpy.sObjName, eREFUNKNOWN, &depNewDep);
}

//Resolve inbound/outbound dependencies across Database limits
//This time we go the other way around, we browse through the unknowns in proccol
//and attempt to find a resolved link in the CDEPS collection
//==============================================================================>
UINT CPRCCOL::SetForeignDBDeps(const THIS_DB & thisDB, CDEPS *pdpDB, CTABCOL & tbcTabs)
{
  UINT iRet=0;
	if(pphPrcRt == NULL || pdpDB == NULL) return 0;

	for(int i=0;i<this->iPrcRootSz; i++)
	for(H_PRC *phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
    CPROC *pPrc;
    if((pPrc=phlTmp->pPRC) == NULL) continue;

	  const UCHAR *pc;
	  UINT eatAccess, iRet=0;
	  eREFOBJ eObjType;
	  CUSTR sServer,sDB, sUser;

	  pc = pPrc->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);

	  while(pc != NULL)
	  { //Attempt to look up this database object, if it is a table/view/unknowntable/proc
      if(eObjType == eREFTABVIEW || eObjType == eREFTABLE || eObjType == eREFVIEW || eObjType == eREFPROC)
      if(s::scmp_ci2(thisDB.sDB,thisDB.sServer, sDB,sServer) != 0)
      {
        CUSTR sLinkPath,sNewUser;
        eREFOBJ eRefType;
        UINT eAccType;

        if(pdpDB->FindDepByName(sServer,sDB,sUser,pc, sLinkPath,eRefType,eAccType,sNewUser)==true)
        {
          if(sNewUser.slen() <= 0)
          {
            pPrc->m_dpDBObjDeps.SetHTML_LINK(sLinkPath);
            pPrc->m_dpDBObjDeps.SetObjType(eRefType);
            iRet++;
            //printf("Resetting ref for: %s.%s.%s",(const char *)pPrc->GetDB() //Debugging loop...
            //, (const char *)pPrc->GetOwner(),pPrc->GetName());
            //printf("\n to: %s.%s.%s -> %s\n",(const char *)sDB //Debugging loop...
            //, (const char *)sUser,pc,(const char *)sLinkPath);
          }
          else
          {
            if(eObjType == eREFTABVIEW || eObjType == eREFTABLE || eObjType == eREFVIEW)
            if FLAG(pPrc->m_dpDBObjDeps.iGetFlags(), mFLAG_ASSUMED_OWNER)
            {
              iRet++;
              CUSTR sTmpName(pc);  //Copy current dep, reset owner,delete and insert dep reference
              CDEP dpTmpCpy = pPrc->m_dpDBObjDeps.GetDependency();

              ReAssignTableOwnerRef(pPrc, tbcTabs, dpTmpCpy, eatAccess,sNewUser,sLinkPath,eRefType);

              //Above function will reset curretn cursor position -> So, we re-start looping
	            pc = pPrc->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
              continue;
            }
          }
        }
      }
		  pc = pPrc->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eatAccess);
	  }
	}

  return iRet;
}

#define _SRC "CPRCCOL::CountDeps"
int CPRCCOL::CountDeps(const UCHAR *pcServer,const UCHAR *pcDB,
                       const UCHAR *pcUser,const UCHAR *pcObjName)
{
  CDP::Handle_Requi_Params(pcServer,pcDB,pcUser,pcObjName, _SRC);
	
	if(pphPrcRt == NULL) return 0;

	int iRet=0;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	{
		H_PRC *phlTmp;
		for(phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			eREFOBJ eObjType = eREFUNKNOWN;
			CUSTR sServer, sDB, sUser;
			UINT eacAccess = eAT_UNKNOWN;
			const UCHAR *pc;
			CPROC *pPRC = phlTmp->pPRC; //Just a lazy short_cut

			for(pc = pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eacAccess);
          pc != NULL;
		      pc = pPRC->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eacAccess))
			{
        if(sServer.scmp_ci(pcServer) ==0 &&
           sDB.scmp_ci(pcDB)         ==0 &&
           sUser.scmp_ci(pcUser)     ==0 &&
           s::scmp_ci(pc, pcObjName) ==0)
					iRet++;
			}
		}
	}

	return iRet;
}
#undef _SRC

#define _SRC "CPRCCOL::GetDeps"
//Search for Objects that have a reference to the given object
//Input:  Object Reference: DB, Owner, Name of DB Object
//Output: Collection of DB-Objects that refere to input object
//We need this mainly because there is no dependency like "Called By",
//there is only like "Calling", so we resolve this and similar matters
//with this function
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CDEPS *CPRCCOL::GetDeps(const UCHAR *pcServer, const UCHAR *pcDB
                       ,const UCHAR *pcUser,const UCHAR *pcObj)
{
  CDP::Handle_Requi_Params(pcServer,pcDB,pcUser,pcObj, _SRC);
	
	if(pphPrcRt == NULL) return 0;

	CDEPS *pdpRet;

	if((pdpRet = new CDEPS())==NULL) return NULL;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	{
		H_PRC *phlTmp;
		for(phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			eREFOBJ eObjType = eREFUNKNOWN;
			CUSTR sServer,sDB,sUser;
			UINT eacAccess = eAT_UNKNOWN;
			const UCHAR *pc;
			CPROC *pPRC = phlTmp->pPRC; //Just a lazy short_cut

			for(pc = pPRC->m_dpDBObjDeps.GetFirst(eObjType, sServer,sDB,sUser,eacAccess);
          pc != NULL;
		      pc = pPRC->m_dpDBObjDeps.GetNext(eObjType, sServer,sDB,sUser,eacAccess))
			{
        if(sServer.scmp_ci(pcServer) ==0 &&
           sDB.scmp_ci(pcDB)         ==0 &&
           sUser.scmp_ci(pcUser)     ==0 &&
           s::scmp_ci(pc, pcObj)     ==0)
          //Add dependency to result collection
          pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(),pPRC->GetOwner(),pPRC->GetName(),
                          pPRC->GetObjType(), eacAccess,0);
			}
		}
	}

	return pdpRet;
}
#undef _SRC

// **** **** **** **** **** **** **** **** **** **** **** **** **** ****
// *** *** *** **** *** *** *** *** **** *** *** *** *** **** *** *** 
//*** *** *** **** *** *** *** *** **** *** *** *** *** **** *** *** 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
// Get the names of all procedures in this database
// Print eTRIG, ePROC, eUNKNOWN or all code refs
//
// iGet Interpretation:
// 1 > Get all objects in sorted order, no matter what
// 2 > Get all objects from Database shown in sDB
// 3 > Get all objects from Database shown in sDB and ObjType in eOBJ_TypeIn
// 4 > Get all objects of the ObjType in eOBJ_TypeIn (regardless of DB)
//
//!!! Caller must free returned object !!!
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CDEPS *CPRCCOL::ListObj(const eREFOBJ & eOBJ_TypeIn,const CUSTR & sServerIn
                       ,const CUSTR & sDBIn, const int & iGet)
{
	if(pphPrcRt == NULL) return 0;

	CDEPS *pdpRet;
	if((pdpRet = new CDEPS())==NULL) return NULL;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	{
		H_PRC *phlTmp;
		for(phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			CPROC *pPRC = phlTmp->pPRC;

      //Add dependency to result collection
      if( iGet == 1
      || (iGet == 2 && s::scmp_ci2(pPRC->GetDB(),pPRC->GetServer(),sDBIn,sServerIn)==0 )
      || (iGet == 3 && s::scmp_ci2(pPRC->GetDB(),pPRC->GetServer(),sDBIn,sServerIn)==0
          && eOBJ_TypeIn == pPRC->GetObjType())
      || (iGet == 4 && eOBJ_TypeIn == pPRC->GetObjType())
        )
			{
				pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(), pPRC->GetOwner(),pPRC->GetName()
                      ,pPRC->GetObjType(), eAT_UNKNOWN,0);
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
//Print eUNKNOWN, eTRIG, ePROC or all objects
//===========================================================00000>
int CPRCCOL::iCntPrcObjects(const eREFOBJ eprcType, bool bAll)
{
	if(pphPrcRt == NULL) return 0;

	int iRet=0;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	for(H_PRC *phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
		//Add thus object to be counted
		if( eprcType == phlTmp->pPRC->GetObjType() || bAll == true)
			iRet++;
	}

	return iRet;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Find all callers for the procedure named pcName & count 'am
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define _SRC "CPRCCOL::CountFindCalls"
int CPRCCOL::CountFindCalls(const UCHAR *pcServer,const UCHAR *pcDB
                           ,const UCHAR *pcUser,const UCHAR *pcObj)
{
  CDP::Handle_Requi_Params(pcServer,pcDB,pcUser,pcObj, _SRC);
	
	if(pphPrcRt == NULL) return 0;

	int iCnt=0;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	for(H_PRC *phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
		if(phlTmp->pPRC->m_dpDBObjDeps.FindDep(pcServer,pcDB,pcUser,pcObj,eREFPROC,eAT_EXEC) == true)
			iCnt++;
	}

	return iCnt;
}
#undef _SRC

//Get all procedures that have no caller from within the database
//!!! Caller must free returned object !!!
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define _SRC "CPRCCOL::GetProcWithZeroCallers"
CDEPS *CPRCCOL::GetProcWithZeroCallers()
{
	if(pphPrcRt == NULL) return NULL;

	CDEPS *pdpRet;
	if((pdpRet = new CDEPS())==NULL) return NULL;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	for(H_PRC *phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
		if(phlTmp->pPRC->GetObjType() == eREFPROC)
		if(CountFindCalls(phlTmp->sServer,phlTmp->sDB,phlTmp->sOwner,phlTmp->sName)<=0)
			pdpRet->AddDep(phlTmp->sServer,phlTmp->sDB,phlTmp->sOwner,phlTmp->sName,eREFPROC,eAT_UNKNOWN,0);
	}

	if(pdpRet->iCntDeps() <= 0)
	{
		delete pdpRet;
		pdpRet = NULL;
	}

	return pdpRet;
}
#undef _SRC

// <<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<->><<
// Print contents of proc collection: Each trigger/proc goes into one file
// <-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>-><-<>->
void CPRCCOL::ppPrintCGs(const CTABCOL & tbcTabs
                        ,const CUSTR & sSrcPath, const CPRCCOL & prcPrcs   //Path to SQL + ProcObj
                        ,const char *pcPath                 //Relative link path from HTML2SQL
                        ,const THIS_DB & ThisDB
                        ,const CUSTR   & sOutPath           //Output directory
                        ,int *piCntProcs, int *piCntTrigs  //just statistics
                        )
{
	CCG *pCallGraph=NULL;

	if(pphPrcRt == NULL) return;

	//Count nodes per hash table
	for(int i=0;i<iPrcRootSz; i++)
	for(H_PRC *phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
	{
		CPROC *pPRC = phlTmp->pPRC;

		//Do not render external procs and their deps here
		//We render them on a seperate page for all external objects
		if(phlTmp->pPRC->GetObjType() != eREFUNKNOWN)
		{
			//Get CallGraph for this object
			pCallGraph = new CCG(this,pPRC, ThisDB,sOutPath);
			if(pCallGraph ==  NULL) return;
	
			FILE *fpOut=NULL;
			CUSTR sFile;
	
			sFile = s::CreateFileName(sOutPath, pPRC->GetTargFileName(),NULL);
	
			if((fpOut = fopen(sFile, "wb"))!=NULL)
			{
				switch(phlTmp->pPRC->GetObjType())
				{
				case eREFTRIG:
					fprintf(fpOut, "%s\n<html>\n\t<head>\n\t\t<title>Trig: %s.%s.%s</title>\n",DOCTYPE_TAG
                         , (const UCHAR *)phlTmp->pPRC->GetDB()
                         ,(const UCHAR *)phlTmp->pPRC->GetOwner(),(const UCHAR *)phlTmp->pPRC->GetName());
	
	    		if(piCntTrigs != NULL) *piCntTrigs += 1;
					fprintf(fpOut, "\t<link rel=\"shortcut icon\" href=\"res/trigger.jpg\">\n");
					break;
				
				case eREFPROC:
					fprintf(fpOut, "%s\n<html>\n\t<head>\n\t\t<title>Proc: %s.%s.%s</title>\n",DOCTYPE_TAG
                         , (const UCHAR *)phlTmp->pPRC->GetDB()
                         ,(const UCHAR *)phlTmp->pPRC->GetOwner(),(const UCHAR *)phlTmp->pPRC->GetName());
	
	    		if(piCntProcs != NULL) *piCntProcs += 1;
					fprintf(fpOut, "\t<link rel=\"shortcut icon\" href=\"res/proc.jpg\">\n");
					break;
				default:
					break;
				}
	
				fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/style.css\">\n");
				CUSTR s = ThisDB.sServer + "/" + ThisDB.sDB;
				fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/office_xp.css\" title=\"Office XP\" />\n");
        fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenu.js\"></script>\n");
				fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenubar.js\"></script>\n");
				fprintf(fpOut, "<script type=\"text/javascript\" src=\"menuconf.js\"></script>\n");

			  fprintf(fpOut, "<META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\">\n");
			  fprintf(fpOut, "<META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\">\n");

        fprintf(fpOut, "</head>\n<body onload=\"initjsDOMenu(\'%s\')\">\n", (const char *)s);
	
        HTML_PrintProc(ThisDB,sSrcPath,phlTmp->pPRC, tbcTabs
                      ,prcPrcs,pcPath,pCallGraph,fpOut,sOutPath);
				
				fprintf(fpOut, "</body></html>");
				
				FCLOSE(fpOut);
			}
			else
				printf("CPROCCOL::ppPrintCGs WARNING: Unable to write output file %s\n",
				                                                   (const char *)sFile);
			delete pCallGraph;
		}
	}
}

//Find all callers for the procedure named pcName & return an iterator collection
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CDEPS *CPRCCOL::FindCalls(int *piCnt,const UCHAR *pcServer, const UCHAR *pcDB
                         ,const UCHAR *pcOwn,const UCHAR *pcName)
{
  CDP::Handle_Requi_Params(pcServer,pcDB,pcOwn,pcName, "CPRCCOL::FindCalls");
	
	if(piCnt!=NULL)*piCnt=0;
	if(pphPrcRt == NULL) return 0;

	CDEPS *pdpRet;
	if((pdpRet = new CDEPS())==NULL) return NULL;

	for(int i=0;i<iPrcRootSz; i++) //Find Calls from inside of the database on the fly...
	{
		for(H_PRC *phlTmp = pphPrcRt[i];phlTmp != NULL; phlTmp = phlTmp->pNext)
		{
			CPROC *pPRC = phlTmp->pPRC;

			if(phlTmp->pPRC->m_dpDBObjDeps.FindDep(pcServer,pcDB,pcOwn,pcName, eREFPROC, eAT_EXEC)==true)
			{
				if(piCnt!=NULL) *piCnt += 1;

        pdpRet->AddDep(pPRC->GetServer(),pPRC->GetDB(), pPRC->GetOwner(),pPRC->GetName()
                      ,pPRC->GetObjType(),eAT_UNKNOWN,0);
			}
		}
	}

  //Resolve eAT_EXEC_BY, if available
  H_PRC *phlTmp = FindPRCNode(this->pphPrcRt,pcServer,pcDB,pcOwn,pcName);

  if(phlTmp != NULL)
  if(phlTmp->pPRC != NULL)
  {
    const UCHAR *pc;
	  UINT eatAccess;
	  eREFOBJ eObjType;
	  CUSTR sServer,sDB,sUser;

    pc = phlTmp->pPRC->m_dpDBObjDeps.GetFirst(eObjType,sServer,sDB,sUser,eatAccess);

	  while(pc != NULL)
	  {
      if FLAG (eatAccess, eAT_EXEC_BY)
				pdpRet->CopyIteratorDep(phlTmp->pPRC->m_dpDBObjDeps);

      pc = phlTmp->pPRC->m_dpDBObjDeps.GetNext(eObjType,sServer,sDB,sUser,eatAccess);
	  }
  }

  if(pdpRet->iCntDeps() <= 0)
	{
		delete pdpRet;
		pdpRet = NULL;
	}

	return pdpRet;
}

//Find all external objects that depend on a given object and return their access codes
UINT CPRCCOL::FindAllOutboundDeps(CDEPS & dpRet,const CUSTR & sServer,const CUSTR & sDB
                                               ,const CUSTR & sOwner, const CUSTR & sName
                                               ,const CUSTR & sAbsOutPath)
{
  if(pphPrcRt == NULL) return 0;
  UINT iRet=0;
  CUSTR sLinkPath,sDummy;
  eREFOBJ eObjType;
  UINT eAccType;
  CPROC *pPrc;

  for(int i=0;i<this->iPrcRootSz; i++)
	for(H_PRC *phlTmp = pphPrcRt[i]; phlTmp != NULL; phlTmp = phlTmp->pNext)
  {
    if((pPrc = phlTmp->pPRC) == NULL) continue;

    //If this proc object holds a ref to the searched object, we add the proc object into the collection
    if(pPrc->m_dpDBObjDeps.FindDepByName(sServer,sDB,sOwner,sName, sLinkPath,eObjType,eAccType,sDummy)==true)
    {
      iRet++;

      if FLAG(eAccType, eAT_EXEC)
      {
        UNSETFLAG(eAccType, eAT_EXEC);
          SETFLAG(eAccType, eAT_EXEC_BY);
      }

      dpRet.AddDep(pPrc->GetServer(),pPrc->GetDB(),pPrc->GetOwner(),pPrc->GetName()
                  ,pPrc->GetObjType(), eAccType,//eAT_EXEC_BY,
                   0,sAbsOutPath + pPrc->GetTargFileName());
    }
  }  

  return iRet;
}

// Add the given dependencies to the dependencies addressed by: pcDB.pcOwner.pcName
//==================================================================================>
void CPRCCOL::AddOutboundDeps(const CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB
                             ,const UCHAR *pcOwner,const UCHAR *pcName)
{
  if(pphPrcRt == NULL) return;

  H_PRC *phlTmp = this->FindPRCNode(pphPrcRt,pcServer,pcDB, pcOwner,pcName);

  if(phlTmp != NULL)
  if(phlTmp->pPRC != NULL)
    phlTmp->pPRC->m_dpDBObjDeps += dpDepIn;
}

//Add a classification for this proc
void CPRCCOL::AddProcClass(const CUSTR & sServer, const CUSTR & sDB
                          ,const CUSTR & sOwner,const CUSTR & sName, const CUSTR & sClassify)
{
  H_PRC *pNode = this->FindPRCNode(this->pphPrcRt,sServer,sDB,sOwner,sName);

  if(pNode != NULL)
  {
    CPROC tProc(sServer,sDB,sOwner,sName, eREFUNKNOWN);
    this->AddProc(&tProc);
    pNode = FindPRCNode(this->pphPrcRt,sServer,sDB,sOwner,sName);
  }

  if(pNode == NULL) return; //Exit quitly (this should never ever ever happen though)

  if(pNode->pPRC != NULL)
    pNode->pPRC->m_csClasses.AddString(sClassify);
}
