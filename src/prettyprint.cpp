
#include "prettyprint.h"
#include "cerr.h"
#include "copts.h"
#include "parse.h"

#include <time.h>

//link to nextlevel online pages in resource directory
const char HELP_IDX_FNAME[] = "res/help/index.html";

//Name of file to reference for external calls
const char EXTERN_CALLS_FNAME[] = "__ExternCalls.html";

//Database objects that are unreferenced within the database
const char UNREFERENCED_OBJECTS[] = "__unrefed_dbobj.html";

//Convert number to string and return formated version of table id
//This is done so that certain statements with certain access types
//(select, delete, update, insert) can link from the table overview
//page directly into the source code page...
//=================================================================>
void FormatTableKey(hlTYPE hlTyp, const int iKey, UCHAR *pcKey, const int iKeySz)
{
	int iSz;
	CUSTR s;

	s += iKey;

	if((s.slen()+6) >= iKeySz)
	{
		CUSTR sErr("ERROR: PrettyPrint::FormatTableKey String is to small: ");
		sErr += iKeySz;
		sErr += ", but required is at least: ";
		sErr += (s.slen()+6);

		throw CERR(sErr);
	}

	for(iSz=0;iSz<s.slen();iSz++)
	{
		pcKey[iSz] = s[iSz];
	}
	pcKey[iSz] = 0;

	pcKey[iSz++] = '_';
	switch(hlTyp)
	{
		case hlTableFrom: pcKey[iSz++] = 'S';
			break;
		case hlTableDel: pcKey[iSz++] = 'D';
			break;
		case hlTruncTable: pcKey[iSz++] = 'T';
		                   pcKey[iSz++] = 'T';
			break;
		case hlTableUpd: pcKey[iSz++] = 'U';
			break;
		case hlTableUpdStats:
			pcKey[iSz++] = 'U';
			pcKey[iSz++] = 'S';
			pcKey[iSz++] = 'T';
		break;
		case hlTableIns:
			pcKey[iSz++] = 'I';
		break;
		default:
		break;
	}
	pcKey[iSz++] = '\0';
}

//Here we count all dependencies on all table/view objects of that DB, so we can
//display a nice spreadsheet like format to show what depends on what in what DB
static int iCountDBDeps(CDEPS *pdpExt
                      ,const UCHAR *pcServer,const UCHAR *pcDB
                      ,CTABCOL & tbcTabs
                      ,eREFOBJ eRefObjType)  //count only certain references
{
	const UCHAR *pc;
	UINT eatAccess;
	eREFOBJ eObjType;
	CUSTR sServer,sDB, sUser;
	int iRet=0;

	try
	{
		pc = pdpExt->GetFirst(eObjType,sServer,sDB, sUser,eatAccess);
		while(pc != NULL)       //pc is the table/view name for which we list all deps
		{
			if(eRefObjType == eObjType && s::scmp_ci2(sDB,sServer,  pcDB,pcServer) == 0)
			{
				iRet+=tbcTabs.GetTable(sServer,pcDB,sUser,pc).m_dpDBObjDeps.CountObjTypes(eREFUNKNOWN, true);
			}
			pc = pdpExt->GetNext(eObjType,sServer,sDB, sUser,eatAccess);
		}
	}
	catch(CERR e)
	{
		printf("ERROR: PrettyPrint::iCountDBDeps %s\n", (const char *)e.getMsg());
	}
	catch(...)
	{
		printf("ERROR: PrettyPrint::iCountDBDeps An error has occured!\n");
	}

	return iRet;
}

#define TD_HEAD "align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\""
#define TD_HEAD1 "align=\"left\" valign=\"top\" bgcolor=\"#DDDDDD\""
#define TD_HD "id=\"BordHead\""

//This function prints all unknown dependencies for all tabs in CDEPS
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static void HTML_ExternalTabs(FILE *fpOut,
                              CDEPS *pdpExt, //Contains DEPS on table/views only
                              CTABCOL & tbcTabs,
                              CPRCCOL & prcPrcs,
                              const THIS_DB thisDB,
                              const CUSTR sOutputPath
                             )
{
	const UINT eaccMask = eAT_DELETE|eAT_TRUNC_TAB|eAT_INSERT|eAT_UPDATE \
	                    | eAT_UPDATEStats | eAT_SELECT  | eAT_SELECT_BY_VIEW;
	                                                            //| eAT_ACC_EXT;
	if(pdpExt == NULL) return;

	try
	{
		const UCHAR *pc;
		UINT eatAccess, eacExt;
		eREFOBJ eObjType;
		CUSTR sServer,sDB,sUser;

		//1st: find all (external) databases being accessed to print sorted order...
		pc = pdpExt->GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
    CDEPS cdpServDB;

		while(pc != NULL)
		{
			//count only those statements that refere to an unknown table/view
			if(eObjType == eREFTABVIEW)
      {
        cdpServDB.AddDep(sServer,sDB,"xxx","xxx",eREFTABVIEW,0,0);
      }

			pc = pdpExt->GetNext(eObjType,sServer,sDB,sUser,eatAccess);
		}

		//if(cdpServDB.Sz() <= 0) return; //Return empty handed, if this it

		fprintf(fpOut, "<table id=\"Bord\">\n");
		fprintf(fpOut, "<thead><tr><td %s>Foreign Database</td>\n", TD_HD);
		fprintf(fpOut, "<td %s>Table/View</td>\n", TD_HD);
		fprintf(fpOut, "<td %s>Procedure/Trigger/View</td>\n", TD_HD);
		fprintf(fpOut, "<td %s>Access</td></tr></thead>\n", TD_HD);

    eREFOBJ eExtOBJType;
    CUSTR sExtServ, sExtDB, sExtOwn;
		const UCHAR *pcExtName = cdpServDB.GetFirst(eExtOBJType,sExtServ,sExtDB,sExtOwn,eacExt);
		int iCntDB=0, iCntDBDeps, iCntObjDeps;

		while(pcExtName != NULL)
		{
			iCntDBDeps = iCountDBDeps(pdpExt, sExtServ, sExtDB, tbcTabs, eREFTABVIEW);

			if(iCntDB == 0)
				fprintf(fpOut, " <tr><td %s id=\"Bord\" rowspan=\"%i\"><h1>%s.%s</h1></td>\n"
               ,TD_HEAD, iCntDBDeps, (const char *)sExtServ, (const char *)sExtDB);
			else
				fprintf(fpOut, " <tr><td %s id=\"Bord\" rowspan=\"%i\"><h1>%s.%s</h1></td>\n"
               ,TD_HEAD1, iCntDBDeps, (const char *)sExtServ, (const char *)sExtDB);

			//Print rows of access statements for this database - objects, access stat
			pc = pdpExt->GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
			bool bFlag = false;
			while(pc != NULL)   //pc is the table/view name for which we list all deps
			{
        if(s::scmp_ci2(sDB,sServer,sExtDB,sExtServ)==0 && eObjType == eREFTABVIEW)
				{
					CUSTR sName(pc);
					if(thisDB.sDBOwner.scmp_ci(sUser)!=0) sName = sUser + "." + pc;

					bFlag = true;
					iCntObjDeps = tbcTabs.GetTable(sExtServ,sExtDB,sUser,pc).m_dpDBObjDeps.CountObjTypes(eREFUNKNOWN, true);
					fprintf(fpOut, "<td id=\"Bord\" rowspan=\"%i\">%s</td>\n",
					        iCntObjDeps, (const UCHAR *)sName);

					//Print Table Access Statements
					tbcTabs.HTML_PrintTableModifs(thisDB, fpOut, eaccMask, prcPrcs
					                             ,tbcTabs.GetTable(sExtServ,sExtDB,sUser,pc),sOutputPath
                                       , "id=\"Bord\"","id=\"BordHead\"","id=\"Bord\"",false);
				}
				//Do we get another line???
				if((pc = pdpExt->GetNext(eObjType,sServer,sDB,sUser,eatAccess)) != NULL)
        if(s::scmp_ci2(sDB,sServer, sExtDB,sExtServ)==0 &&
           bFlag == true && eObjType == eREFTABVIEW)
					fprintf(fpOut, "<tr>\n");
			}
			
			iCntDB ^= 1;
      pcExtName = cdpServDB.GetNext(eExtOBJType,sExtServ,sExtDB,sExtOwn,eacExt);
		}
		fprintf(fpOut, "</table>\n"); // End of outer database table
	}
	catch(CERR e)
	{
		CUSTR s(e.getsErrSrc());
		printf("ERROR: %s in PrettyPrint::HTML_ExternalTabs %s\n", (const char *)s, (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: PrettyPrint::HTML_ExternalTabs An error has occured!\n");
	}
}

//Here we count all dependencies on all table/view objects of that DB, so we can
//display a nice spreadsheet like format to show what depends on what in what DB
static int iCountDBDeps(CDEPS *pdpExt, const UCHAR *pcServer, const UCHAR *pcDB,
                        CPRCCOL & prcPrcs,
                        eREFOBJ eRefObjType)  //count only certain references
{
	const UCHAR *pc;
	UINT eatAccess;
	eREFOBJ eObjType;
	CUSTR sServer,sDB,sUser;
	int iRet=0;

	try
	{
		pc = pdpExt->GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
		while(pc != NULL)       //pc is the table/view name for which we list all deps
		{
			if(eRefObjType == eObjType && s::scmp_ci2(sDB,sServer ,pcDB,pcServer) == 0)
			{
				iRet += prcPrcs.CountDeps(sServer,sDB,sUser,pc);
			}
			pc = pdpExt->GetNext(eObjType,sServer,sDB,sUser,eatAccess);
		}
	}
	catch(CERR e)
	{
		printf("ERROR: PrettyPrint::iCountDBDeps %s\n", (const char *)e.getMsg());
	}
	catch(...)
	{
		printf("ERROR: PrettyPrint::iCountDBDeps An error has occured!\n");
	}

	return iRet;
}

//This function finds/prints all callers for each proc in psbTree
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static void HTML_ExternalProcs(FILE *fpOut, CDEPS *pdpExt, CPRCCOL & prcPrcs
                              ,const THIS_DB thisDB)
{
	if(pdpExt == NULL) return;

	try
	{
		const UCHAR *pc, *pc1;
		UINT eatAccess, eatAccess1;
		eREFOBJ eObjType, eObjType1;
		CUSTR sServer, sDB, sUser, sServer1, sDB1, sUser1;

		//1st: find all (external) databases being accessed to print sorted order...
		pc = pdpExt->GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
    CDEPS dpsExtDB;

		while(pc != NULL)
		{                                       //count only statement-refs to procs
			if(eObjType == eREFPROC)
        dpsExtDB.AddDep(sServer,sDB,"xxx","xxx",eREFUNKNOWN,0,0);

			pc = pdpExt->GetNext(eObjType,sServer,sDB,sUser,eatAccess);
		}

		//if(cslExtDB.Sz() <= 0) return; //Return empty handed, if this it

		fprintf(fpOut, "<table id=\"Bord\">\n");
		fprintf(fpOut, "<thead><tr><td id=\"BordHead\">Foreign Database</td>\n");
		fprintf(fpOut, "<td id=\"BordHead\">Foreign Procedure</td>\n");
		fprintf(fpOut, "<td id=\"BordHead\">Calling Procedure/Trigger</td>\n");
		fprintf(fpOut, "<td id=\"BordHead\">Access</td></tr></thead>\n");

    UINT eatExt;
    eREFOBJ eExtObjType;
    CUSTR sExtServ,sExtDB,sExtOwn;
		const UCHAR *pcExtName = dpsExtDB.GetFirst(eExtObjType,sExtServ,sExtDB,sExtOwn,eatExt);
		int iCntDB=0, iCntDBDeps, iCntObjDeps;

		while(pcExtName != NULL)
		{
			iCntDBDeps = iCountDBDeps(pdpExt, sExtServ,sExtDB, prcPrcs, eREFPROC);

			if(iCntDB == 0)
				fprintf(fpOut, " <tr><td %s rowspan=\"%i\"><h1>%s.%s</h1></td>\n"
               ,TD_HEAD, iCntDBDeps, (const char *)sExtServ, (const char *)sExtDB);
			else
				fprintf(fpOut, " <tr><td %s rowspan=\"%i\"><h1>%s.%s</h1></td>\n"
               ,TD_HEAD1, iCntDBDeps, (const char *)sExtServ, (const char *)sExtDB);

			//Print rows of access statements for this database - objects, access stat
			//OuterLoop's over unknown procs/Innerloop's over callers of unknown procs
			pc = pdpExt->GetFirst(eObjType,sServer,sDB,sUser,eatAccess);
			bool bFlag = false;
			CDEPS *pdpExec = NULL;
			while(pc != NULL)   //pc is the table/view name for which we list all deps
			{
				if(s::scmp_ci2(sDB,sServer,sExtDB,sExtServ) == 0 && eObjType == eREFPROC)
				{
					CUSTR sName(pc);
					if(thisDB.sDBOwner.scmp_ci(sUser)!=0) sName = sUser + "." + pc;

					bFlag = true;
					iCntObjDeps = prcPrcs.CountDeps(sExtServ,sExtDB,sUser,pc);
					fprintf(fpOut, "<td id=\"Bord\" rowspan=\"%i\">%s</td>\n", iCntObjDeps, pc);

					if((pdpExec = prcPrcs.GetDeps(sExtServ,sExtDB,sUser,pc))!=NULL)
					{
						pc1 = pdpExec->GetFirst(eObjType1,sServer1,sDB1,sUser1,eatAccess1);

						while(pc1 != NULL)   //pc is the table/view name for which we list all deps
						{
							CUSTR s = prcPrcs.GetProc(sServer1,sDB1,sUser1,pc1).GetTargFileName();
			
							fprintf(fpOut, "<td id=\"Bord\"><a href=\"%s\"><SPAN CLASS=\"%s\">%s</SPAN></a></td>\n",
							               (const char *)s, CSS_PNM, (const char *)pc1);

							fprintf(fpOut, "<td id=\"Bord\">EXEC</td></tr>\n");

							pc1 = pdpExec->GetNext(eObjType1,sServer1,sDB1,sUser1,eatAccess1);

							if(pc1 != NULL) fprintf(fpOut, "<tr>\n");
						}
						delete pdpExec;
						pdpExec = NULL;
					}
				}
				//Do we get another line???
				if((pc = pdpExt->GetNext(eObjType,sServer,sDB,sUser,eatAccess))!=NULL)
				if(s::scmp_ci2(sDB,sServer,sExtDB,sExtServ) == 0 && bFlag == true && eObjType == eREFPROC)
					fprintf(fpOut, "<tr>\n");
			}

			iCntDB ^= 1;
      pcExtName = dpsExtDB.GetNext(eExtObjType,sExtServ,sExtDB,sExtOwn,eatExt);
    }
		fprintf(fpOut, "</table>\n"); // End of outer database table
	}
	catch(CERR e)
	{
		printf("ERROR: PrettyPrint::HTML_ExternalProcs %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: PrettyPrint::HTML_ExternalProcs An error has occured!\n");
	}
}
#undef TD_HD

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Link to procedures, tables, and views that are unreferenced in this DB
//Print Unreferenced Database Objects HTML page on output stream
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define _SRC "PrettyPrint::HTML_PrintUnrefDBObj"
bool HTML_PrintUnrefDBObj(CUSTR & sOutPutDir,CPRCCOL & prcPrcs,CTABCOL & tbcTabs
                         ,const THIS_DB thisDB)
{
	bool bRet = false;
	CDEPS *pdpPrcs = NULL, *pdpTabs = NULL, *pdpViws = NULL;

	try
	{ //Collsof procs without caller in the DB, and tabs without reference in DB
		CDEPS *pdpPrcs = prcPrcs.GetProcWithZeroCallers();
		CDEPS *pdpTabs = tbcTabs.GetUnRefedTabView(eREFTABLE);
		CDEPS *pdpViws = tbcTabs.GetUnRefedTabView(eREFVIEW);

		//We will output data, if and when we get past this point...
		if(pdpPrcs == NULL && pdpTabs == NULL && pdpViws == NULL)
			return bRet;

		CUSTR sFile = s::CreateFileName(sOutPutDir, UNREFERENCED_OBJECTS, NULL);
		FILE *fpOut=NULL;

		//Write HTML Ouput File for external database calls
		if((fpOut = fopen(sFile, "wb"))!=NULL)
		{
			bRet = true;
			CUSTR s = thisDB.sServer + "/" + thisDB.sDB;
			fprintf(fpOut, "%s\n<html>\n\t<head>\n\t\t<title>%s (NextLevel Index)</title>\n", DOCTYPE_TAG,(const char *)s);
			fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/style.css\">");
			fprintf(fpOut, "<link rel=\"shortcut icon\" href=\"res/cogweel_16.jpg\">");
			fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/office_xp.css\" title=\"Office XP\" />\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenu.js\"></script>\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"menuconf.js\"></script>\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenubar.js\"></script>\n");

			fprintf(fpOut, "<META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\">\n");
			fprintf(fpOut, "<META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\">\n");

      fprintf(fpOut, "</head>\n<body onload=\"initjsDOMenu(\'%s\')\">\n", (const char *)s);
			
			fprintf(fpOut, "<table id=\"NoBord\" width=\"100%%\">\n");
			fprintf(fpOut, "<tr>\n<td><h1>Unreferenced Objects</h1></td>\n");
			fprintf(fpOut, "<td Align=\"right\" valign=\"top\">\n");
			fprintf(fpOut, "<div id=\"staticMenu\"></div></td></tr></table>");

			fprintf(fpOut, "<p><a href=\"res/help/feats.html#Unrefed\"><img border=\"0\" src=\"res/question.png\"></a>\n");
			fprintf(fpOut, "Objects on this page are unreferenced in this database.\n");
			fprintf(fpOut, " This means, NextLevel was presented with a definition\n");
			fprintf(fpOut, " for a database object, but found no evidence for its usage.</p>\n");

      fprintf(fpOut, "<table id=\"Bord\">\n");
			fprintf(fpOut, "<thead><tr>");
			fprintf(fpOut, "<TD id=\"BordHead\">Table</TD>\n");
			fprintf(fpOut, "<TD id=\"BordHead\">Size</TD>\n");
			fprintf(fpOut, "<TD id=\"BordHead\">Procedure</TD>\n");
			fprintf(fpOut, "<TD id=\"BordHead\">View</TD>\n");

			//Name of db object
			const UCHAR *pcPrc=NULL, *pcTab=NULL, *pcViw=NULL;
			eREFOBJ eObjTyp;
			CUSTR sPrcServ,sPrcDB, sPrcUser
           ,sTabServ,sTabDB,sTabUser
           ,sViwServ,sViwDB,sViwUser,sName;
			UINT eacAcc;

			if(pdpTabs!=NULL)pcTab=pdpTabs->GetFirst(eObjTyp,sTabServ,sTabDB,sTabUser,eacAcc);
			if(pdpPrcs!=NULL)pcPrc=pdpPrcs->GetFirst(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc);
			if(pdpViws!=NULL)pcViw=pdpViws->GetFirst(eObjTyp,sViwServ,sViwDB,sViwUser,eacAcc);

			fprintf(fpOut, "</tr></thead>\n");

			while(pcPrc != NULL || pcTab != NULL || pcViw != NULL)
			{
				fprintf(fpOut, "<tr>\n");
				if(pcTab != NULL)
				{
					int iDataSz, iIndexSz;
					CUSTR s = tbcTabs.GetTable(sTabServ,sTabDB,sTabUser,pcTab).GetTargFileName();

					iIndexSz = tbcTabs.GetTable(sTabServ,sTabDB,sTabUser,pcTab).iIndexSz;
					iDataSz  = tbcTabs.GetTable(sTabServ,sTabDB,sTabUser,pcTab).iDataSz;

					if(thisDB.sDBOwner.scmp_ci(sTabUser)==0) sName = pcTab;
						else sName = sTabUser + "." + pcTab;

					fprintf(fpOut, "<td id=\"Bord\"><a href=\"%s\">%s</a></td>\n",
					               (const char *)s, (const char *)sName);

					if(iIndexSz != -1 && iDataSz != -1)
					{
						CUSTR s = ConvertKB2UsefulSz(iIndexSz + iDataSz);
						fprintf(fpOut, " <td id=\"Bord\">%s</td>\n", (const char *)s);
					}
					else
					  fprintf(fpOut, "<td id=\"Bord\"></td>\n");
				}
				else fprintf(fpOut, "<td></td><td></td>\n");

				if(pcPrc != NULL)
				{
					CUSTR s = prcPrcs.GetProc(sPrcServ,sPrcDB,sPrcUser,pcPrc).GetTargFileName();
		
					if(thisDB.sDBOwner.scmp_ci(sPrcUser)==0) sName = pcPrc;
						else sName = sPrcUser + "." + pcPrc;

					fprintf(fpOut, "<td id=\"Bord\"><a href=\"%s\">%s</a></td>\n",
					               (const char *)s, (const char *)sName);
				}
				else fprintf(fpOut, "<td></td>\n");


				if(pcViw != NULL)
				{
					CUSTR s = tbcTabs.GetTable(sViwServ,sViwDB,sViwUser,pcViw).GetTargFileName();
		
					if(thisDB.sDBOwner.scmp_ci(sViwUser)==0) sName = pcViw;
						else sName = sViwUser + "." + pcViw;

					fprintf(fpOut, "<td id=\"Bord\"><a href=\"%s\">%s</a></td>\n",
					               (const char *)s, (const char *)sName);
				}
				else fprintf(fpOut, "<td></td>\n");

				fprintf(fpOut, "</tr>\n");

				if(pdpTabs!=NULL) pcTab = pdpTabs->GetNext(eObjTyp,sTabServ,sTabDB,sTabUser,eacAcc);
				if(pdpPrcs!=NULL) pcPrc = pdpPrcs->GetNext(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc);
				if(pdpViws!=NULL) pcViw = pdpViws->GetNext(eObjTyp,sViwServ,sViwDB,sViwUser,eacAcc);
			}

			fprintf(fpOut, "</table>\n</body></html>");

			FCLOSE(fpOut);
		}
		else
		{
			printf("ERROR: %s Can not write to file %s!\n", _SRC,(const char *)sFile);

			return false;
		}
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: %s\n", _SRC);
		return false;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR ooccured\n", 0, _SRC);
	}

	delete pdpPrcs;
	delete pdpTabs;
	delete pdpViws;

	return bRet;
}
#undef _SRC

#define TABCEL1 "<td align=\"right\" valign=\"top\">"

//Print External Objects HTML page on output stream
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HTML_Externals(CUSTR sOutPutDir,CPRCCOL & prcPrcs,CTABCOL & tbcTabs,const THIS_DB thisDB)
{
	try
	{ //Collect external/internal references to unknowns
		CDEPS *pdpInt, *pdpExt, *pdpIntPrc, *pdpExtPrc;

		//Add unknown(internal) and external tabs and procs to collections
		pdpInt = tbcTabs.FindUnknownObjects(1, thisDB.sServer, thisDB.sDB);
		pdpExt = tbcTabs.FindUnknownObjects(2, thisDB.sServer, thisDB.sDB);

		pdpIntPrc = prcPrcs.FindUnknownObjects(1, thisDB.sServer, thisDB.sDB);
		pdpExtPrc = prcPrcs.FindUnknownObjects(2, thisDB.sServer, thisDB.sDB);

		if(pdpInt != NULL)
		{
			*pdpInt += pdpIntPrc;
			delete pdpIntPrc;
			pdpIntPrc = NULL;
		}
		else
			pdpInt = pdpIntPrc;

		if(pdpExt != NULL)
		{
			*pdpExt += pdpExtPrc;
			delete pdpExtPrc;
			pdpExtPrc = NULL;
		}
		else
			pdpInt = pdpIntPrc;

		//Not creating this page or any children means not linking to this page
		if(pdpInt == NULL && pdpExt == NULL)
			return false;

		CUSTR sFile = s::CreateFileName(sOutPutDir, EXTERN_CALLS_FNAME, NULL);
		FILE *fpOut=NULL;

		//Write HTML Ouput File for external database calls
		if((fpOut = fopen(sFile, "wb"))!=NULL)
		{
			CUSTR s = thisDB.sServer + "/" + thisDB.sDB;

      fprintf(fpOut, "%s\n<html>\n\t<head>\n\n", DOCTYPE_TAG);
			fprintf(fpOut, "<title>External/Undefined Objects in %s</title>\n", (const UCHAR *)s);
			fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/style.css\">");
			fprintf(fpOut, "<link rel=\"shortcut icon\" href=\"res/radar16.jpg\">\n");

			fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/office_xp.css\" title=\"Office XP\" />\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenu.js\"></script>\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"menuconf.js\"></script>\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenubar.js\"></script>\n");

			fprintf(fpOut, "<META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\">\n");
			fprintf(fpOut, "<META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\">\n");

      fprintf(fpOut, "</head>\n<body onload=\"initjsDOMenu(\'%s\')\">\n", (const char *)s);

			fprintf(fpOut, "<table id=\"NoBord\" cellPadding=\"3\" cellSpacing=\"0\" width=\"100%%\">\n");
			fprintf(fpOut, "<tr>\n<td><h1>Access to External and Unknown Database Objects</h1></td>\n");
			fprintf(fpOut, "%s<div id=\"staticMenu\"></div></td>\n", TABCEL1);
			fprintf(fpOut, "</tr>\n</table>");
			
			//Get callers for each external proc and print both in a table
			if(pdpExt != NULL)
			{
				fprintf(fpOut, "<p><a href=\"res/help/conns.html\"><IMG border=\"0\" src =\"res/question.png\"></a>\n");
				fprintf(fpOut, "The following tables show all databases being accessed\n");
				fprintf(fpOut, "from within <b>%s</b>. This includes the table/view being accessed,\n",(const UCHAR *)s);
				fprintf(fpOut, "the implementing database object, and the kind of access implemented\n");
				fprintf(fpOut, "(next table shows the same information for stored procedures).</p>\n");

				HTML_ExternalTabs(fpOut, pdpExt, tbcTabs, prcPrcs, thisDB,sOutPutDir);
				fprintf(fpOut, "<br>\n");
				HTML_ExternalProcs(fpOut, pdpExt, prcPrcs, thisDB);
			}

			if(pdpInt != NULL)
			{
				fprintf(fpOut, "<p>The following tables show all undefined objects being accessed\n");
				fprintf(fpOut, "from within <b>%s</b>. A database object is <b>undefined</b>, if an\n",(const UCHAR *)s);
				fprintf(fpOut, "access statement was found, but no definition for the object being accessed.\n");
				fprintf(fpOut, "This includes the table/view being accessed,\n");
				fprintf(fpOut, "the implementing database object, and the kind of access implemented\n");
				fprintf(fpOut, "(next table shows the same information for stored procedures).</p>\n");

				HTML_ExternalTabs(fpOut, pdpInt, tbcTabs, prcPrcs, thisDB,sOutPutDir);
				fprintf(fpOut, "<br>\n");
				HTML_ExternalProcs(fpOut, pdpInt, prcPrcs, thisDB);
			}

			fprintf(fpOut, "</body></html>");

			FCLOSE(fpOut);
		}
		else
		{
			printf("ERROR: PrettyPrint::HTML_Externals Can not create file %s!\n",
             (const char *)sFile);

			return false;
		}

		delete pdpExt;
		delete pdpInt;
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: PrettyPrint.cpp::HTML_Externals\n");
		return false;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR ooccured in PrettyPrint.cpp::HTML_Externals\n");
		return false;
	}

	return true;
}
#undef TABCEL1

#define HTML_TRIG_INTRO "<SPAN CLASS=\""CSS_PNM"\"><small>"
#define HTML_TRIG_OUTRO "</small></SPAN>"

// *=->*=->*=->*=->*=->*=->*=->*=->*=->*=->*=->*=->
//Print a table call with trigger information
// *=->*=->*=->*=->*=->*=->*=->*=->*=->*=->*=->*=->
void PrintTableCell(const THIS_DB thisDB
                   ,const UCHAR *pcServer, const UCHAR *pcDB
                   ,const UCHAR *pcOwner,  const UCHAR *pcTabName
                   ,CPRCCOL *prcProcs, CTABCOL *ptbTables, FILE *fpOut)
{
	try
	{
		CUSTR s = ptbTables->GetTargFilename(pcServer,pcDB,pcOwner,pcTabName), sName(pcTabName);

		if(thisDB.sDBOwner.scmp_ci(pcOwner) != 0)
			sName = pcOwner, sName += ".", sName += pcTabName;

		fprintf(fpOut, "<td id=\"Idx\"><a href=\"%s\">%s</a>"
		      ,(const char *)s,(const char *)sName);
	
		const UCHAR *pcDel, *pcUpd, *pcIns; //Get trigger names and print

		pcDel = ptbTables->GetTable(pcServer,pcDB,pcOwner,pcTabName).DelTrig();
		pcUpd = ptbTables->GetTable(pcServer,pcDB,pcOwner,pcTabName).UpdTrig();
		pcIns = ptbTables->GetTable(pcServer,pcDB,pcOwner,pcTabName).InsTrig();
	
		if(pcUpd != NULL)
		{
			s = prcProcs->GetProc(pcServer,pcDB,pcOwner,pcUpd).GetTargFileName();
			if(pcDel != pcUpd && pcIns != pcUpd)
				fprintf(fpOut, "&nbsp;<a href=\"%s\">%sU%s</a>", (const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
			else
			if(pcDel == pcUpd && pcIns != pcUpd)
				fprintf(fpOut, "&nbsp;<a href=\"%s\">%sUD%s</a>", (const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
			else
			if(pcDel != pcUpd && pcIns == pcUpd)
				fprintf(fpOut, "&nbsp;<a href=\"%s\">%sUI%s</a>", (const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
			else
			if(pcDel == pcUpd && pcIns == pcUpd)
				fprintf(fpOut, "&nbsp;<a href=\"%s\">%sUID%s</a>", (const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
		}
		
		if(pcIns != NULL)
		if(pcIns != pcUpd)
		{
			s = prcProcs->GetProc(pcServer,pcDB,pcOwner,pcIns).GetTargFileName();
			if(pcIns != pcDel)
				fprintf(fpOut, "&nbsp;<a href=\"%s\">%sI%s</a>", (const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
			else
				fprintf(fpOut, "&nbsp;<a href=\"%s\">%sID%s</a>",(const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
	
			fprintf(fpOut, "\n");
		}
		
		if(pcDel != NULL)
		if(pcDel != pcUpd)
		if(pcDel != pcIns)
		{
			s = prcProcs->GetProc(pcServer,pcDB,pcOwner,pcDel).GetTargFileName();
			fprintf(fpOut, "&nbsp;<a href=\"%s\">%sD%s</a>",(const char *)s,HTML_TRIG_INTRO,HTML_TRIG_OUTRO);
		}
	
		fprintf(fpOut, "</td>");
	}
	catch(CERR e)
	{
		printf("ERROR: PrettyPrint::PrintTableCell %s\n", (const char *)e.getMsg());
		return;
	}
	catch(...)
	{
		printf("ERROR: PrettyPrint::PrintTableCell An error has occured!\n");
	}
}
#undef HTML_TRIG_INTRO
#undef HTML_TRIG_OUTRO

//
//Write Main index.html page into output stream
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WriteHTMLIndex(FILE *fpOut, CPRCCOL *prcPrcs,CTABCOL *ptbTabs,
                    const THIS_DB & thisDB)
{
	if(fpOut == NULL || prcPrcs == NULL || ptbTabs == NULL)
	{
		throw CERR("Parameter(s) NULL",ERR_INTERNAL,"PrettyPrint::WriteHTMLIndex");
	}

	//Get sorted names of procs, tables, and views for this database
	CDEPS *pdpsPrc = prcPrcs->ListObj(eREFPROC  ,thisDB.sServer,thisDB.sDB, 3);
	CDEPS *pdpsTab = ptbTabs->ListObj(eREFTABLE ,thisDB.sServer,thisDB.sDB, 3);
	CDEPS *pdpsViw = ptbTabs->ListObj(eREFVIEW  ,thisDB.sServer,thisDB.sDB, 3);

	bool bSTabSz   = ptbTabs->bTabsSizeAvail(); //Tables having a size property???

	//Source refs found??? If there is no valid input, there is no data to index..
	if(pdpsPrc != NULL || pdpsTab != NULL|| pdpsViw != NULL)
	{
		const UCHAR *pcPrc=NULL, *pcTab=NULL,*pcViw=NULL; //Names of objects

		eREFOBJ eObjTyp;
		CUSTR sPrcServ, sPrcDB, sPrcUser
         ,sTabServ, sTabDB, sTabUser
         ,sViwServ, sViwDB, sViwUser;
		UINT eacAcc;

		if(pdpsPrc != NULL)pcPrc=pdpsPrc->GetFirst(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc);
		if(pdpsTab != NULL)pcTab=pdpsTab->GetFirst(eObjTyp,sTabServ,sTabDB,sTabUser,eacAcc);
		if(pdpsViw != NULL)pcViw=pdpsViw->GetFirst(eObjTyp,sViwServ,sViwDB,sViwUser,eacAcc);

		//Print Header of HTML Index TABLE
		fprintf(fpOut, "\n<table id=\"Idx\">\n");
		fprintf(fpOut, "<thead><tr>\n");

		if(pcTab != NULL)
    {
			fprintf(fpOut, "<TD id=\"IdxHead\">Table</TD>\n");

			if(bSTabSz == true)
				fprintf(fpOut, "<TD id=\"IdxHead\">Size</TD>\n");
		}

		if(pcPrc != NULL)
			fprintf(fpOut, "<TD id=\"IdxHead\">Procedure</TD>\n");

		if(pcViw != NULL)
			fprintf(fpOut, "<TD id=\"IdxHead\">View</TD>\n");

		fprintf(fpOut, "</tr></thead>\n");

		CUSTR str;
		while(pcPrc != NULL || pcTab != NULL || pcViw != NULL)
		{
			fprintf(fpOut, "<tr>\n");

			if(pcTab != NULL)          //Print Table name if there is one in the queue
			{
				PrintTableCell(thisDB, sTabServ,sTabDB,sTabUser,pcTab, prcPrcs, ptbTabs, fpOut);

				int iIndexSz = ptbTabs->GetTable(sTabServ,sTabDB,sTabUser,pcTab).iIndexSz;
				int iDataSz  = ptbTabs->GetTable(sTabServ,sTabDB,sTabUser,pcTab).iDataSz;

				if(iIndexSz != -1 && iDataSz != -1) //Print table size
				{
					CUSTR s = ConvertKB2UsefulSz(iIndexSz + iDataSz);
					fprintf(fpOut, " <td id=\"Idx\">%s</td>\n", (const char *)s);
				}
				else
					if(bSTabSz == true) fprintf(fpOut, "<td></td>\n");
			}
			else
				if(pdpsTab != NULL) fprintf(fpOut, "<td></td><td></td>");

			if(pcPrc != NULL) //Print Procedure name if there is one in the queue
			{
				str = prcPrcs->GetProc(sPrcServ,sPrcDB,sPrcUser,pcPrc).GetTargFileName();

				CUSTR s(pcPrc);
				if(thisDB.sDBOwner.scmp_ci(sPrcUser)!=0) s = sPrcUser + "." + s;

				fprintf(fpOut, "<td id=\"Idx\"><a href=\"%s\">%s</a></td>",
				                          (const char*)str,(const char*)s);
			}
			else
				if(pdpsPrc != NULL) fprintf(fpOut, "<td></td>");

			if(pcViw != NULL)          //Print Table name if there is one in the queue
				PrintTableCell(thisDB,sViwServ,sViwDB,sViwUser,pcViw, prcPrcs,ptbTabs, fpOut);
			else
				if(pdpsViw != NULL) fprintf(fpOut, "<td></td>");

			fprintf(fpOut, "</tr>\n");

			if(pdpsPrc != NULL)pcPrc=pdpsPrc->GetNext(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc);
			if(pdpsTab != NULL)pcTab=pdpsTab->GetNext(eObjTyp,sTabServ,sTabDB,sTabUser,eacAcc);
			if(pdpsViw != NULL)pcViw=pdpsViw->GetNext(eObjTyp,sViwServ,sViwDB,sViwUser,eacAcc);
		}

		fprintf(fpOut, "</table>\n");
	}
	else
		fprintf(fpOut, "<p>No SQL source code found.</p>\n");

	fprintf(fpOut, "\n");

	delete pdpsPrc;
	delete pdpsTab;
	delete pdpsViw;
}

//Find trigger from a table in dependence of the event
//We need this function to convert highlighting types into tabcol types
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CUSTR FindTrigger(const CTABCOL & tbcTabs, const UCHAR *pcServ,const UCHAR *pcDB,
                  const UCHAR *pcOwn, const UCHAR *pcTabName, hlTYPE hlPatrnType)
{
	CUSTR sRet;

	switch(hlPatrnType)          //Find trigger name to match data modification
	{
	case hlTableDel:
		sRet = tbcTabs.FindTrigger(dmDELETE, pcServ,pcDB, pcOwn, pcTabName);
		break;
	case hlTableUpd:
		sRet = tbcTabs.FindTrigger(dmUPDATE, pcServ,pcDB, pcOwn, pcTabName);
		break;
	case hlTableIns:
		sRet = tbcTabs.FindTrigger(dmINSERT, pcServ,pcDB, pcOwn, pcTabName);
		break;
	default:
	break;
	}

	return sRet;
}

static void HTML_fputc(UCHAR c, FILE *fpOut)
{
	if(c >= '\t')
	switch(c)
	{
		case '<':
			fprintf(fpOut, "&lt;");
		break;
		
		case '>':
			fprintf(fpOut, "&gt;");
		break;
		
		case '\t':
			fprintf(fpOut, "  "); //print spaces instead of tabs
		break;
		
		case '&':
			fprintf(fpOut, "&amp;"); //print spaces instead of tabs
		break;

		default:
		fprintf(fpOut, "%c", c);
	}
}

//Attempt to resolve external links to other databases (indexing more than one DB)
CUSTR sResolveExtDBLink(const CPRCCOL & prcPrcs
                       ,const CTABCOL & tbcTabs
                       ,const THIS_DB & thisDB
                       ,const CUSTR   & sThisOwner          //Name of owner for this object
                       ,const CUSTR   & sThisObjName       //Name of this object
                       ,const eREFOBJ & ObjType
                       ,const CUSTR   & sAbsOutPath
                       ,const CUSTR   & sServer      //
                       ,const CUSTR   & sDB         //We look to resolve a dependency to this DB
                       ,const CUSTR   & sOwner
                       ,const CUSTR   & sDBObj
                       ,const CUSTR   & sInLink    //Return this, if link's not available
                       ,eREFOBJ       & eObjClass //return view/table/proc or unknown
                      )
{
  CUSTR sLinkPath, str;
  UINT AccType;

  //Change sInLink only, if reference to external server,db is to be resolved
  if(s::scmp_ci2(thisDB.sDB, thisDB.sServer,  sDB,sServer)!= 0)
  {
    switch(ObjType)     //Determine for who we are running (view or proc/trigger)
    {                  //then, lookup dependency path for myself (absolute path)
    case eREFTABLE:
    case eREFVIEW:
      tbcTabs.GetTable(thisDB.sServer,thisDB.sDB,sThisOwner,sThisObjName)
              .m_dpDBObjDeps.FindDepByName(sServer,sDB,sOwner,sDBObj,sLinkPath,eObjClass, AccType,str);
      break;
    case eREFPROC:
    case eREFTRIG:
      prcPrcs.GetProc(thisDB.sServer,thisDB.sDB,sThisOwner,sThisObjName)
              .m_dpDBObjDeps.FindDepByName(sServer,sDB,sOwner,sDBObj,sLinkPath,eObjClass, AccType,str);
      break;
    default:
      break;
    }

    if(sLinkPath.slen()>0)  //Got a path, so convert file link into relative link & Return
    {
      CUSTR sRelPath = s::GetRelPath(sAbsOutPath, s::sGetPath(sLinkPath));
      sRelPath += s::sGetFileName(sLinkPath);

      return sRelPath;
    }
    else
      eObjClass = eREFUNKNOWN;
  }

  return sInLink;
}

#define HL_UPD_DEL_INS_COMMAND 1
#define HL_TABLE_NAME          2
#define HL_PROCEDURE_NAME      3
#define HL_SQL_KEYWORD         4
#define HL_SQL_COMMENT         5
#define HL_SQL_STRING          6
#define HL_DECLARE_CU          7
#define HL_DECLARE_VR          8
#define HL_TABLE_FROM          9
#define HL_ANCER_MODIF         10 //Table to anker to inside of page

#define _SRC "prettyprint::HTML_HighLightSQL"
//Highlight SQL Source texts in HTML
void HTML_HighLightSQL(const CPRCCOL & prcPrcs, const CTABCOL & tbcTabs
                      ,const CDEPS  & dpDBObjDeps    //Database Proc/Tric Object Deps
                      ,CPATRN & hlPatrn       //Highlighting for recognized objects
                      ,const CUSTR   & sThisOwner          //Name of owner for this object
                      ,const CUSTR   & sThisObjName       //Name of this object
                      ,const eREFOBJ & ObjType
                      ,FILE *fpSQLSrc          //Input SQL source file
                      ,FILE *fpOut            //HTML output file
                      ,const THIS_DB & thisDB
                      ,const CUSTR   & sAbsOutPath)
{
	int iOffBegin, iOffEnd,	iKey=0, iHighLight,c,iCntChars=0,iFlags;
	hlTYPE hlPatrnType;
	eREFOBJ tbcTmpTBLClass;
  eREFOBJ  prcCodeClass;
	CUSTR s, sTmp, sName, sServ,sDB, sOwner, sDBObj;    //Parsed ref to db objects
	const UCHAR *pcTTName1=NULL;                       //Name of Trigger or Table
	bool bTTNameFound=false;
	UCHAR pcKey[NUM_SZ];         //String Representation of Key to identify a table

	if(fpSQLSrc == NULL || fpOut == NULL) return;

	fprintf(fpOut, "<PRE><CODE>\n"); //print lead in for HTML code highlighting

	try
	{
	hlPatrn.SetFirst();
	while(hlPatrn.GetNext(iOffBegin,iOffEnd,hlPatrnType,sServ,sDB,sOwner,sDBObj,sName,iFlags) == true)
	{
    iHighLight=-1, iCntChars=0;
		//Print character on stream 'till next highlighting mark, if any
		while(!feof(fpSQLSrc) && ftell(fpSQLSrc) < iOffBegin && (c=fgetc(fpSQLSrc))!= EOF)
		{
			HTML_fputc(c, fpOut);
			iCntChars++;
			if(iCntChars > 512) //Limit Maximum length of line to about:512-1024 chars
			{
				if(iCntChars < 1024)
				{
					if(c <= ' ' || s::schr((UCHAR *)";:,.<>/?!\\|'\"\'{}[]()", c) != NULL)
					{	fprintf(fpOut,"\n"); iCntChars=0;}
				}
				else
					{fprintf(fpOut,"\n"); iCntChars=0;}
			}
		}
		
		switch(hlPatrnType) //Highlight begining of something
		{
		case hlTableDel:      //Beginning update, insert, or delete command
		case hlTableUpd:      //We try to find a trigger and table to match this
		case hlTableIns:      //with a different color highlighting.
			iHighLight = HL_UPD_DEL_INS_COMMAND;
			if((bTTNameFound = tbcTabs.GetTableMatch(sServ,sDB,sOwner,sDBObj, &iKey))==true)
				FormatTableKey(hlPatrnType, iKey, &pcKey[0], NUM_SZ);

			//Attempt to color & link to trigger source, if statement fires a trigger
			//Establish Link2Trig if statement could cause an event to be triggered
			sTmp = FindTrigger(tbcTabs,sServ,sDB,sOwner,sDBObj,hlPatrnType);
			if(sTmp.slen() > 0)
			{
				s = prcPrcs.GetProc(sServ,sDB,sOwner,sTmp).GetTargFileName();
				if(pcKey[0] == '\0')
					fprintf(fpOut, "<a href=\"%s\">", (const char *)s );
				else
					fprintf(fpOut, "<a name=\"%s\" href=\"%s\">",pcKey,(const char *)s);

        CDP::HTML_Color_Obj(fpOut, eREFTRIG);  //Color SQL LINK
			}
			else                     //Standard keyword color for unmatched triggers
			{
				if(pcKey[0] != '\0') fprintf(fpOut, "<a name=\"%s\">",pcKey);

				fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_KWD);
			}
		break;

		case hlDECLARE:    //Beginning cursor declare command
		case hlDECLARE_TYP:    //Beginning cursor declare command
			iHighLight = HL_DECLARE_VR;
			fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_CURSOR);
		break;
                         //Found the name of a cursor declare command
		case hlDECL_CUName: //Install link to usage cursor, if it is used further down
			if(hlPatrn.GetFindNext(hlUsageCU, sName) == true)
			{
				iHighLight = HL_DECLARE_CU;
				fprintf(fpOut, "<a name=\"cu_%s\" href=\"#co_%s\"><SPAN CLASS=\"%s\">",
											 (const UCHAR *)sName, (const UCHAR *)sName, CSS_CURSOR);
			}
		break;

		case hlUsageCU: //Found the name of a cursor open, fetch, deallocate, or close command
			if(hlPatrn.GetFindPrev(hlDECL_CUName, sName) == true)
			{
				iHighLight = HL_DECLARE_CU;
				fprintf(fpOut, "<a name=\"co_%s\" href=\"#cu_%s\"><SPAN CLASS=\"%s\">",
											 (const UCHAR *)sName, (const UCHAR *)sName, CSS_CURSOR);
			}
		break;
		case hlTableFrom:              //Highlight table references for from clause
			iHighLight = HL_TABLE_FROM;
			//Build an anker so that others may be able to link to this statement
			if((bTTNameFound = tbcTabs.GetTableMatch(sServ,sDB,sOwner,sDBObj, &iKey))==true)
				FormatTableKey(hlTableFrom, iKey, &pcKey[0], NUM_SZ);

			//Get name of table and its ID Key so that we know linking to this is safe
			if(tbcTabs.GetTableMatch(sServ,sDB,sOwner,sDBObj,&iKey,&tbcTmpTBLClass)==true)
			{
				s = tbcTabs.GetTargFilename(sServ,sDB,sOwner,sDBObj);   //Link2Definition file
				if(pcKey[0] == '\0')
					fprintf(fpOut, "<a href=\"%s\">", (const char *)s );
				else
        {
          if(tbcTmpTBLClass == eREFUNKNOWN)
          { //Attempt to resolve external links to other databases (indexing more than one DB)
            s = sResolveExtDBLink(prcPrcs,tbcTabs,thisDB,sThisOwner,sThisObjName,ObjType
                                 ,sAbsOutPath,sServ,sDB,sOwner,sDBObj, s,tbcTmpTBLClass);

            fprintf(fpOut, "<a name=\"%s\" href=\"%s\">", pcKey, (const char *)s);
          }
          else
					  fprintf(fpOut, "<a name=\"%s\" href=\"%s\">", pcKey, (const char *)s);
        }
        CDP::HTML_Color_Obj(fpOut, tbcTmpTBLClass,NULL,eREFTABLE); //Do correct color highlightinh
			}
			else               //Color as unknown object (thats an exception, normally not used)
				CDP::HTML_Color_Obj(fpOut, eREFUNKNOWN,NULL, eREFTABLE);
		break;

		case hlTableName:
			iHighLight = HL_TABLE_NAME;
			//Get the name of a table and its ID Key so that we know linking to this is safe
			if((bTTNameFound = tbcTabs.GetTableMatch(sServ,sDB,sOwner,sDBObj,&iKey,&tbcTmpTBLClass))==true)
			{
				s = tbcTabs.GetTargFilename(sServ,sDB,sOwner,sDBObj);   //Link2Definition file
        s = sResolveExtDBLink(prcPrcs,tbcTabs,thisDB,sThisOwner,sThisObjName,ObjType
                             ,sAbsOutPath,sServ,sDB,sOwner,sDBObj, s,tbcTmpTBLClass);

        fprintf(fpOut, "<a href=\"%s\">", (const char *)s);
        CDP::HTML_Color_Obj(fpOut, tbcTmpTBLClass,NULL, eREFTABLE);
			}
			else
			{
				printf("CodeRef: %s\n", _SRC);
				printf("WARNING (Internal ERROR): Unknwown Table Obj: %s.%s.%s.%s RawTag: %s PatternType: %i\n",
				(const char*)sServ,(const char*)sDB, (const char*)sOwner, (const char*)sDBObj,
				(const char*)sName,hlPatrnType);

        CDP::HTML_Color_Obj(fpOut, eREFUNKNOWN,NULL, eREFTABLE); //Color as unknown object
			}
		break;

		case hlProcedure:
			iHighLight = HL_PROCEDURE_NAME;

      if(dpDBObjDeps.FindDep(sServ,sDB,sOwner,sDBObj,eREFPROC,eAT_ACC_UNDEF) == true
        || s::scmp_ci2(thisDB.sDB,thisDB.sServer,  sDB,sServ) != 0)
			{ 
           s = sResolveExtDBLink(prcPrcs,tbcTabs,thisDB,sThisOwner,sThisObjName,ObjType
                      ,sAbsOutPath,sServ,sDB,sOwner,sDBObj, EXTERN_CALLS_FNAME,prcCodeClass);

        //Link to externals page, if link to external proc remains undefined
				fprintf(fpOut, "<a href=\"%s\">", (const char *)s);
        CDP::HTML_Color_Obj(fpOut, prcCodeClass, NULL, eREFPROC); //Write SPAN TAG to file ...
			}
			else
			{
				CUSTR sKey;

				sKey += prcPrcs.GetProcID(sServ, sDB,sOwner,sDBObj); //Link to actual proc page
				sKey += "x";
				s     = prcPrcs.GetProc(sServ,sDB,sOwner,sDBObj).GetTargFileName();

				fprintf(fpOut, "<a name=\"%s\" href=\"%s\">", (const char *)sKey,(const char *)s);
        CDP::HTML_Color_Obj(fpOut, eREFPROC);
			}
			break;

		case hlTruncTable:     //Handle 'Truncate Table' like SQL Keywords
		case hlTableUpdStats: //Handle 'Update Statistics' likewise
			iHighLight = HL_ANCER_MODIF;
			if((bTTNameFound = tbcTabs.GetTableMatch(sServ,sDB,sOwner,sDBObj, &iKey))==true)
				FormatTableKey(hlPatrnType, iKey, &pcKey[0], NUM_SZ);

			if(pcKey[0] != '\0') fprintf(fpOut, "<a name=\"%s\">",pcKey);

			fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_KWD);
			break;

		case hlSQLKeyword:
			iHighLight = HL_SQL_KEYWORD;
			fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_KWD);
			break;

		case hlComment:
			iHighLight = HL_SQL_COMMENT;
			fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_CMT);
			break;

		case hlString:
			iHighLight = HL_SQL_STRING;
			fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_STR);
		break;

		default:
			printf("\n\n::HTML_HighlightSQL WARNING: Unhandled VALUE %i for hlPatrnType\n\n", hlPatrnType);
			printf("\n\n::HTML_HighlightSQL WARNING: Object %s.%s.%s\n\n",
        (const char *)thisDB.sDB,(const char *)sThisOwner,(const char *)sThisObjName);
		break;
		}

		while(!feof(fpSQLSrc) && ftell(fpSQLSrc) < iOffEnd && (c=fgetc(fpSQLSrc))!= EOF)
		{
			HTML_fputc(c, fpOut); //Print character on stream
			iCntChars++;
		}

		switch(iHighLight)
		{
		case HL_UPD_DEL_INS_COMMAND:  //END OF hlTableDel, hlTableUpd, hlTableIns
			fprintf(fpOut, "</SPAN>");

			if(bTTNameFound == true && pcKey[0] != '\0')
				fprintf(fpOut, "</a>");                //Close anker and/or hyperlink
		break;

		case HL_ANCER_MODIF:
			fprintf(fpOut, "</SPAN>");

			if(pcKey[0] != '\0') fprintf(fpOut, "</a>"); //Close anker
		break;

		case HL_TABLE_NAME:
			fprintf(fpOut, "</SPAN>");
			if(bTTNameFound == true)
			{
				fprintf(fpOut, "</a>"); //Close hyperlink to table being linked
			}
		break;

		case HL_TABLE_FROM:
			fprintf(fpOut, "</SPAN>");                                   //Close anker
			if(bTTNameFound == true) fprintf(fpOut, "</a>");
		break;

		case HL_PROCEDURE_NAME:
			fprintf(fpOut, "</SPAN>");
			fprintf(fpOut, "</a>");     //Close hyperlink to procedure being called
		break;

		case HL_DECLARE_CU:           //End of name of cursor declare command
			fprintf(fpOut, "</SPAN></a>");
		break;

		case HL_DECLARE_VR:
		case HL_SQL_STRING:           //End of sql string
		case HL_SQL_KEYWORD:         //End of sql keyword
		case HL_SQL_COMMENT:        //End of comment
			fprintf(fpOut, "</SPAN>");
		break;
		}
		
		if(iCntChars > 1024){ fprintf(fpOut,"\n"); iCntChars=0;} //Maximum length of line is limited to roughly 1024
		pcKey[0]  = '\0';
		pcTTName1 = NULL;
		bTTNameFound = false;
	}                               //End of while loop

	while(!feof(fpSQLSrc) && (c=fgetc(fpSQLSrc))!= EOF)
	{
		HTML_fputc(c, fpOut); //Print character on stream
		iCntChars++;
		if(iCntChars > 1024){ fprintf(fpOut,"\n"); iCntChars=0;} //Maximum length of line is limited to roughly 1024
	}
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: %s\n", _SRC);
		printf("DEBUG Info: %s.%s.%s RawTag: %s PatternType: %i\n", (const char*)sDB,
		                                            (const char*)sOwner,
		                                            (const char*)sDBObj,
		                                            (const char*)sName,hlPatrnType);
		return;
	}
	catch(...)
	{
		printf("UNKNOWN ERROR: Occurred in\n");
		printf("CodeRef: %s\n", _SRC);
		return;
	}

	fprintf(fpOut, "</CODE></PRE>\n");
}
#undef _SRC

#undef HL_UPD_DEL_INS_COMMAND
#undef HL_TABLE_NAME
#undef HL_PROCEDURE_NAME
#undef HL_SQL_KEYWORD
#undef HL_SQL_COMMENT
#undef HL_SQL_STRING
#undef HL_DECLARE_CU
#undef HL_DECLARE_VR
#undef HL_TABLE_FROM
#undef HL_ANCER_MODIF

static void GenWinHelpProcRefs(const THIS_DB thisDB,CPRCCOL & prcProcs,
                               eREFOBJ prcObjDef,
                               const bool bAll, const bool bExtern,
                               const int ImageNo, FILE *fpOut)
{
	CDEPS *pCol;
	CUSTR sPrcUser, sPrcServ,sPrcDB,sPrcName,sPrint;
	UINT eacAcc;
	eREFOBJ eObjTyp;

	int iGet=3; //Get all objects type prcObjDef in DB

	if(bAll == true) iGet = 1;      //Get all objects in DB

	//Create index to triggers and procedures
	if((pCol = prcProcs.ListObj(prcObjDef,thisDB.sServer,thisDB.sDB,iGet))!= NULL)
	{
		for(sPrcName = pCol->GetFirst(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc);
		    sPrcName.slen()>0;
				sPrcName = pCol->GetNext(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc)
		   )
		{
			sPrint = FormatUsefulDBObjRef(sPrcServ,sPrcDB, sPrcUser,sPrcName, thisDB);
			fprintf(fpOut, "\t<LI> <OBJECT type=\"text/sitemap\">\n");
			fprintf(fpOut, "\t\t<param name=\"Name\" value=\"%s\">\n",(const char *)sPrint);

			if(ImageNo > 0)
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"%i\">\n", ImageNo);

			CUSTR s = prcProcs.GetProc(sPrcServ,sPrcDB,sPrcUser,sPrcName).GetTargFileName();
			fprintf(fpOut, "\t\t<param name=\"Local\" value=\"%s\">\n",(char const *)s);
			fprintf(fpOut, "\t</OBJECT>\n");
		}
		delete pCol;
	}

	//Create index to unknown procedures/triggers used in the code
	if(bExtern == true)
	if((pCol = prcProcs.FindUnknownObjects(0, thisDB.sServer,thisDB.sDB))!= NULL)
	{
		for(sPrcName = pCol->GetFirst(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc);
		    sPrcName.slen()>0;
				sPrcName = pCol->GetNext(eObjTyp,sPrcServ,sPrcDB,sPrcUser,eacAcc)
		   )
		{
			sPrint = FormatUsefulDBObjRef(sPrcServ,sPrcDB, sPrcUser,sPrcName, thisDB);
			fprintf(fpOut, "\t<LI> <OBJECT type=\"text/sitemap\">\n");
			fprintf(fpOut, "\t\t<param name=\"Name\" value=\"%s\">\n",(const char *)sPrint);

			if(ImageNo > 0)
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"%i\">\n", ImageNo);

			fprintf(fpOut, "\t\t<param name=\"Local\" value=\"%s\">\n",EXTERN_CALLS_FNAME);
			fprintf(fpOut, "\t</OBJECT>\n");
		}
		delete pCol;
	}
}


static void GenWinHelpTableRefs(const THIS_DB thisDB,CTABCOL & tclTables,
                                eREFOBJ tbcObjDef, const bool bAll,
                                const int ImageNo, FILE *fpOut)
{
	CDEPS *pCol;
	CUSTR sTabUser, sTabDB,sTabServ,sTabName,sPrint;
	UINT eacAcc;
	eREFOBJ eObjTyp;

	int iGet=3; //Get all objects type prcObjDef in DB

	if(bAll == true) iGet = 1;      //Get all objects in DB

	if((pCol = tclTables.ListObj(tbcObjDef,thisDB.sServer,thisDB.sDB, iGet)) != NULL)
	{
		for(sTabName = pCol->GetFirst(eObjTyp,sTabServ,sTabDB,sTabUser,eacAcc);
		    sTabName.slen()>0;
				sTabName = pCol->GetNext(eObjTyp,sTabServ,sTabDB,sTabUser,eacAcc)
		   )
		{
			int iKey=0;
			eREFOBJ tbcTable;
			CUSTR s;
					                      //Create index to known and unknown tables/views
			if(tclTables.GetTableMatch(sTabServ,sTabDB,sTabUser,sTabName,&iKey,&tbcTable) == true)
				s = tclTables.GetTargFilename(sTabServ,sTabDB,sTabUser,sTabName); //Link2Def_file
			else
				s = "interror.html";

			fprintf(fpOut, "\t<LI> <OBJECT type=\"text/sitemap\">\n");
			fprintf(fpOut, "\t\t<param name=\"Name\" value=\"%s\">\n",(const char *)sTabName);

			if(ImageNo > 0)
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"%i\">\n", ImageNo);

			fprintf(fpOut, "\t\t<param name=\"Local\" value=\"%s\">\n",(const char *)s);
			fprintf(fpOut, "\t</OBJECT>\n");
		}
		delete pCol;
	}
}

// Generate WinHelp Header files
// Input Parameter: Programm options
// Output:          Header Files for Windows Help Workshop
//===============_______________________---------------------->>>
void WriteWinHelpHeaderFiles(const THIS_DB thisDB
                            ,CTABCOL & tbcTabs,CPRCCOL & prcPrcs
                            ,CPROP & prpDBProps
                            ,CUSTR sOutputPath  //Output path for HTML Files
                            )
{
	try
	{
		//Base Name of Help Project and Printable Name for Titles etc...
		CUSTR sIdxName, sPrintName;
		if(prpDBProps.sGetVal("SERVER").slen()>0 || prpDBProps.sGetVal("DB").slen()>0)
		{
			sIdxName   = prpDBProps.sGetVal("SERVER");
			sPrintName = sIdxName;
			
			if(prpDBProps.sGetVal("DB").slen()>0)
			{
				sIdxName += "_";
				sIdxName += prpDBProps.sGetVal("DB");

				sPrintName += "/";
				sPrintName += prpDBProps.sGetVal("DB");
				sPrintName += " ";
			}
		}
		else
		{
			sIdxName   = "DB_HELP";
			sPrintName = "Server/Database ";
		}

		//Print status info to stdout...
		printf("    Writing WinHelp Workshop Header Files: %s.hhk, %s.hhp\n",
											 (const char *)sIdxName, (const char *)sIdxName);

		CUSTR sFile;
		sFile = s::CreateFileName(sOutputPath, sIdxName, ".hhp");
		FILE *fpOut;

		if((fpOut = fopen(sFile, "wb"))!=NULL)
		{
			fprintf(fpOut, "[OPTIONS]\n");
			fprintf(fpOut, "Compatibility=1.1 or later\n");
			fprintf(fpOut, "Compiled file=%s.chm\n", (const char *)sIdxName);
			fprintf(fpOut, "Contents file=%s.hhc\n", (const char *)sIdxName);
			fprintf(fpOut, "Default Window=Main\n");
			fprintf(fpOut, "Default topic=index.html\n");
			fprintf(fpOut, "Display compile progress=Yes\n");
			fprintf(fpOut, "Enhanced decompilation=Yes\n");
			fprintf(fpOut, "Full-text search=Yes\n");
			fprintf(fpOut, "Index file=%s.hhk\n", (const char *)sIdxName);
			fprintf(fpOut, "Language=0x409 English (United States)\n");
			fprintf(fpOut, "Title=%s\n", (const char *)sPrintName);
			fprintf(fpOut, "\n");
			fprintf(fpOut, "[WINDOWS]\n");
			fprintf(fpOut, "Main=,\"%s.hhc\",\"%s.hhk\",\"index.html\",\"index.html\",,,,,0x63520,,0x307e,[97,77,833,757],0x1030000,,,,1,,0\n",
			               (const char *)sIdxName, (const char *)sIdxName);
			fprintf(fpOut, "\n");
			fprintf(fpOut, "[FILES]\n");
			fprintf(fpOut, "\n");
			fprintf(fpOut, "res\\Base.gif\n");
			fprintf(fpOut, "res\\EMPTY.GIF\n");
			fprintf(fpOut, "res\\radar.jpg\n");
			fprintf(fpOut, "res\\FOLDER.GIF\n");
			fprintf(fpOut, "res\\folderopen.gif\n");
			fprintf(fpOut, "res\\JOIN.GIF\n");
			fprintf(fpOut, "res\\joinbottom.gif\n");
			fprintf(fpOut, "res\\LINE.GIF\n");
			fprintf(fpOut, "res\\Link2Src.GIF\n");
			fprintf(fpOut, "res\\MINUS.GIF\n");
			fprintf(fpOut, "res\\minusbottom.gif\n");
			fprintf(fpOut, "res\\PAGE.GIF\n");
			fprintf(fpOut, "res\\PLUS.GIF\n");
			fprintf(fpOut, "res\\plusbottom.gif\n");
			fprintf(fpOut, "res\\style.css\n");
			fprintf(fpOut, "res\\TREE.JS\n");
			fprintf(fpOut, "res\\Tree_tpl.js\n");
			fprintf(fpOut, "res\\nlogo.jpg\n");
			fprintf(fpOut, "res\\proc.jpg\n");
			fprintf(fpOut, "res\\table.jpg\n");
			fprintf(fpOut, "res\\trigger.jpg\n");

			fprintf(fpOut, "res\\nl_logo.png\n");   //javascript Menu resources...
			fprintf(fpOut, "res\\question.png\n");
			fprintf(fpOut, "res\\radar.png\n");
			fprintf(fpOut, "res\\cogweel_16.png\n");
			fprintf(fpOut, "res\\office_xp_arrow.png\n");
			fprintf(fpOut, "res\\office_xp_arrow_o.png\n");
			fprintf(fpOut, "res\\office_xp_divider.png\n");
			fprintf(fpOut, "res\\office_xp_menu_left.png\n");

			//We need this, because its linked only through the javascript menu
			fprintf(fpOut, "%s\n", UNREFERENCED_OBJECTS);


			fprintf(fpOut, "\n");
			fprintf(fpOut, "[INFOTYPES]\n");

			FCLOSE(fpOut);
		}

		//Write index file for help index shown in left pane
		sFile = s::CreateFileName(sOutputPath, sIdxName, ".hhk");
		if((fpOut = fopen(sFile, "wb"))!=NULL)
		{
			fprintf(fpOut, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n");
			fprintf(fpOut, "%s\n<HTML>\nHEAD\n", DOCTYPE_TAG);
			fprintf(fpOut, "<meta name=\"GENERATOR\" content=\"%s %s (Build %i)\">\n",
											TOOL_NAME, SOFT_VERSION, TOOL_BUILD);

			fprintf(fpOut, "<!-- Sitemap 1.0 -->\n");
			fprintf(fpOut, "</HEAD><BODY>\n");
			fprintf(fpOut, "<UL>\n");

			//Generate References to Tables/Views/Unknown Tables/Views
			GenWinHelpTableRefs(thisDB,tbcTabs, eREFUNKNOWN, true, -1, fpOut);

			GenWinHelpProcRefs(thisDB,prcPrcs, eREFUNKNOWN, true, true, -1, fpOut);

			fprintf(fpOut, "</UL>\n");
			fprintf(fpOut, "</BODY></HTML>\n");

			FCLOSE(fpOut);
		}

		//Write contents file for help contents pane shown in left pane
		sFile = s::CreateFileName(sOutputPath, sIdxName, ".hhc");
		if((fpOut = fopen(sFile, "wb"))!=NULL)
		{
			fprintf(fpOut, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n");
			fprintf(fpOut, "<HTML><HEAD>\n");
			fprintf(fpOut, "<meta name=\"GENERATOR\" content=\"%s %s (Build %i)\">\n",
											TOOL_NAME, SOFT_VERSION, TOOL_BUILD);

			fprintf(fpOut, "<!-- Sitemap 1.0 -->\n");
			fprintf(fpOut, "</HEAD><BODY>\n");
			fprintf(fpOut, "<UL>\n");

				//Generate References to Procedures
				fprintf(fpOut, "<LI> <OBJECT type=\"text/sitemap\">\n");
				fprintf(fpOut, "<param name=\"Name\" value=\"Procedures\">\n");
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"5\">\n");
				fprintf(fpOut, "</OBJECT>\n");
				fprintf(fpOut, "<UL>\n");
				GenWinHelpProcRefs(thisDB,prcPrcs, eREFPROC, false, true, 39, fpOut);
				fprintf(fpOut, "</UL>\n");

				//Generate References to Triggers
				fprintf(fpOut, "<LI> <OBJECT type=\"text/sitemap\">\n");
				fprintf(fpOut, "<param name=\"Name\" value=\"Triggers\">\n");
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"5\">\n");
				fprintf(fpOut, "</OBJECT>\n");
				fprintf(fpOut, "<UL>\n");
				GenWinHelpProcRefs(thisDB,prcPrcs, eREFTRIG, false, false, 40, fpOut);
				fprintf(fpOut, "</UL>\n");

				//Generate References to Tables
				fprintf(fpOut, "<LI> <OBJECT type=\"text/sitemap\">\n");
				fprintf(fpOut, "<param name=\"Name\" value=\"Tables\">\n");
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"5\">\n");
				fprintf(fpOut, "</OBJECT>\n");
				fprintf(fpOut, "<UL>\n");
				GenWinHelpTableRefs(thisDB,tbcTabs, eREFTABLE, false, 11, fpOut);
				fprintf(fpOut, "</UL>\n");

				//Generate References to Tables
				fprintf(fpOut, "<LI> <OBJECT type=\"text/sitemap\">\n");
				fprintf(fpOut, "<param name=\"Name\" value=\"Views\">\n");
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"5\">\n");
				fprintf(fpOut, "</OBJECT>\n");
				fprintf(fpOut, "<UL>\n");
				GenWinHelpTableRefs(thisDB,tbcTabs, eREFVIEW, false, 11, fpOut);
				fprintf(fpOut, "</UL>\n");

				//Generate References to Tables
				fprintf(fpOut, "<LI> <OBJECT type=\"text/sitemap\">\n");
				fprintf(fpOut, "<param name=\"Name\" value=\"Unknown/External Tables\">\n");
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"5\">\n");
				fprintf(fpOut, "</OBJECT>\n");
				fprintf(fpOut, "<UL>\n");
				GenWinHelpTableRefs(thisDB,tbcTabs, eREFUNKNOWN, false, 37, fpOut);
				fprintf(fpOut, "</UL>\n");

				//Generate References to Tables
				fprintf(fpOut, "<LI> <OBJECT type=\"text/sitemap\">\n");
				fprintf(fpOut, "<param name=\"Name\" value=\"Unknown Procedures\">\n");
				fprintf(fpOut, "<param name=\"ImageNumber\" value=\"5\">\n");
				fprintf(fpOut, "</OBJECT>\n");
				fprintf(fpOut, "<UL>\n");
				GenWinHelpProcRefs(thisDB,prcPrcs, eREFUNKNOWN, false, false, 37, fpOut);
				fprintf(fpOut, "</UL>\n");

			fprintf(fpOut, "</UL>\n");
			fprintf(fpOut, "</BODY></HTML>\n");
			FCLOSE(fpOut);
		}	
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: PrettyPrint::WriteWinHelpHeaderFiles\n");
		return;
	}
	catch(...)
	{
		printf("UNKNOWN ERROR: Occurred in\n");
		printf("CodeRef: PrettyPrint::WriteWinHelpHeaderFiles\n");
		return;
	}
}

//Include information taken from an external File
//
bool IncludeDocuFile(FILE *fpOut, const char *pcHTMLDocuPath
                    ,const char *pcOwner, const char *pcDBOBJName)
{
	FILE *fpIn;
	bool bRet = false;
	CUSTR sName(pcOwner);
	sName += "_";
	sName += pcDBOBJName;

	CUSTR sFileName = s::CreateFileName(pcHTMLDocuPath, sName, ".html");

	try
	{
		if((fpIn = fopen(sFileName, "rb"))!=NULL)
		{
			bRet = true;

			if(fpOut != NULL)
			{
				fprintf(fpOut, "<div align=\"left\"><dl id=\"content\"><dt>Documentation</dt>\n");
				fprintf(fpOut, "<dd  style=\"border: 1px solid rgb(51, 125, 51); color: rgb(0, 0, 0); background-color: rgb(252, 255, 248); padding: 0.4em 0.9em 0.9em;\">");
	
				//fprintf(fpOut, "<table cellspacing=\"3\"><tbody><tr valign=\"top\">\n");
				//fprintf(fpOut, "<td style=\"border: 1px solid rgb(51, 125, 51); color: rgb(0, 0, 0); background-color: rgb(248, 255, 248);\" width=\"100%%\">\n");
				//fprintf(fpOut, "<div style=\"padding: 0.4em 0.9em 0.9em;\">\n");
	
				char c = fgetc(fpIn);
		
				while(!feof(fpIn))
				{
					fputc(c, fpOut);
					c = fgetc(fpIn);
				}
		
				//fprintf(fpOut, "</div></td></tr></tbody></table>\n");
				fprintf(fpOut, "</dd></dl></div>\n");
			}
			fclose(fpIn);
		}
	}
	catch(...){}

	return bRet;
}

//Format a reference to a database object such that it is most useful for output
//We show the name of the database server or database, if we access an external database
//We show the owner, if it is not the owner of the database
//------------------------------------------------------------------------------------->
CUSTR FormatUsefulDBObjRef(const UCHAR *pcServer,const UCHAR *pcDB
                          ,const UCHAR *pcOwner,const UCHAR *pcName, const THIS_DB thisDB)
{
	int iCmpSrv = thisDB.sServer.scmp_ci(pcServer),
	    iCmpDB  = thisDB.sDB.scmp_ci(pcDB),
	    iCmpOwn = thisDB.sDBOwner.scmp_ci(pcOwner);
	CUSTR sName;

	if(iCmpSrv != 0)
		sName  = pcServer, sName += ".", sName += pcDB, sName += ".",
    sName += pcOwner, sName += ".", sName += pcName;
	else
  {
	  if(iCmpDB != 0)
		  sName = pcDB, sName += ".", sName += pcOwner, sName += ".", sName += pcName;
	  else
	  {
		  if(iCmpOwn != 0) sName = pcOwner, sName += + ".", sName += pcName;
		  else
			  sName = pcName;
	  }
  }
	return sName;
}

//Write custom javascript menu-bar to navigate
//inside a database and across to other DBs
void JS_PrintMenuConf(const THIS_DB & thisDB, const CUSTR & sOutDir, CPITEM *pitServDBs)
{
  CUSTR sOutFile;   //Filename to write to

  sOutFile = s::GetAbsPath(s::sGetCWD(), sOutDir);
  sOutFile = s::CreateFileName(sOutFile, "menuconf.js", NULL);

  FILE *fpOut=NULL;

  if((fpOut=fopen(sOutFile,"wb"))!=NULL)
  {
    fprintf(fpOut,"function createjsDOMenu(sDBServerName)\n");
    fprintf(fpOut,"{\n");
    fprintf(fpOut,"\n");
    fprintf(fpOut,"  if(sDBServerName == null) sDBServerName=\"Index\";\n");
    fprintf(fpOut,"\n");
    fprintf(fpOut,"  staticMenu1_1 = new jsDOMenu(130, \"absolute\");\n");
    fprintf(fpOut,"  with (staticMenu1_1)\n");
    fprintf(fpOut,"  {\n");

    if(thisDB.sRootLinkURL.slen()>0) //Show back to root link if XML parameter was set
    {
      fprintf(fpOut,"    addMenuItem(new menuItem(\"Back to Top\", \"Back\", \"%s\"));\n", (const char*)thisDB.sRootLinkURL);
    }

    fprintf(fpOut,"    addMenuItem(new menuItem(\"Unreferenced\", \"LnF\", \"__unrefed_dbobj.html\"));\n");
    fprintf(fpOut,"    addMenuItem(new menuItem(\"Connections\", \"Con\", \"__ExternCalls.html\"));\n");
    fprintf(fpOut,"    addMenuItem(new menuItem(\"-\", \"\", \"\"));\n");
    fprintf(fpOut,"\n");
    fprintf(fpOut,"    addMenuItem(new menuItem(\"Help\", \"hlp\", \"res/help/index.html\"));\n");
    fprintf(fpOut,"    addMenuItem(new menuItem(\"About\", \"nl\", \"res/help/about.html\"));\n");
    fprintf(fpOut,"  }\n");
    fprintf(fpOut,"\n");

    if(thisDB.sRootLinkURL.slen()>0) //Show back to root link if XML parameter was set
    {
      fprintf(fpOut,"  staticMenu1_1.items.Back.showIcon(\"BackIcon\", \"BackIcon\");\n");
    }
    fprintf(fpOut,"  staticMenu1_1.items.LnF.showIcon(\"LostAndFound\", \"LostAndFound\"); //Icons are defined here\n");
    fprintf(fpOut,"  staticMenu1_1.items.Con.showIcon(\"Connections\", \"Connections\");\n");
    fprintf(fpOut,"  staticMenu1_1.items.nl.showIcon(\"nl\", \"nl\");\n");
    fprintf(fpOut,"  staticMenu1_1.items.hlp.showIcon(\"help\", \"help\");\n");
    fprintf(fpOut,"\n");

    int iMenuSz       = 250; //Size of main menu-bar is variable
    bool bUsingDBMenu = false;

    //Have we got multible DBs to link between them???
    if(pitServDBs != NULL)
    if(pitServDBs->GetSz()>1)
    {
      iMenuSz      = 350;
      bUsingDBMenu = true;

      //Find Length of of DB/Server Name to size sub-menu accordingly
      CUSTR sServ, sDB, sOwner, sDBObj;
      int iFlags;
      int iMaxDBServerLen = 0;
      for(int i=0;i<pitServDBs->GetSz();i++)
      {
        pitServDBs->GetStrITEM(i, sServ, sDB, sOwner, sDBObj, iFlags);
        if(s::scmp_ci2(thisDB.sDB, thisDB.sServer, sDB,sServ) != 0)
        {
          if((sServ.slen() + sDB.slen() + 1) > iMaxDBServerLen)
            iMaxDBServerLen = sServ.slen() + sDB.slen() + 1;
        }
      }
      if(iMaxDBServerLen < 10)
        iMaxDBServerLen = (iMaxDBServerLen*10)+5;
      else
        iMaxDBServerLen = iMaxDBServerLen*10;

      //Sort entries by name of server/database
      pitServDBs->SortByServDB();
      fprintf(fpOut,"  staticServerDB = new jsDOMenu(%i, \"absolute\");\n", iMaxDBServerLen);
      fprintf(fpOut,"  with (staticServerDB)\n");
      fprintf(fpOut,"  {\n");

      for(int i=0;i<pitServDBs->GetSz();i++)
      {
        CUSTR sExtDBOutDir = pitServDBs->GetStrITEM(i, sServ, sDB, sOwner, sDBObj, iFlags);
        if(s::scmp_ci2(thisDB.sDB, thisDB.sServer, sDB,sServ) != 0)
        {
          //Compute relative HTML Link from this DB to other database index file
          CUSTR sLinkFile = s::GetAbsPath(s::sGetCWD(), sExtDBOutDir);
          sLinkFile = s::CreateFileName(sLinkFile, "index.html", NULL);
          sLinkFile = s::GetRelPath(sOutFile,sLinkFile);

          fprintf(fpOut,"    addMenuItem(new menuItem(\"%s/%s\", \"%s_%s_idx\", \"%s\"));\n"
            ,(const char *)sServ.UpperCase(),(const char *)sDB.UpperCase()
            ,(const char *)sServ,(const char *)sDB
            ,(const char *)sLinkFile
            );
        }
      }

      fprintf(fpOut,"  }\n");
    }
    fprintf(fpOut,"\n");
    fprintf(fpOut,"  absoluteMenuBar = new jsDOMenuBar(\"static\", \"staticMenu\", false, \"\",%i);\n", iMenuSz);
    fprintf(fpOut,"  with (absoluteMenuBar)\n");
    fprintf(fpOut,"  {\n");

    fprintf(fpOut,"    addMenuBarItem(new menuBarItem(sDBServerName, null,\"idx\",true,\"index.html\"));\n");

    if(bUsingDBMenu == true)
      fprintf(fpOut,"    addMenuBarItem(new menuBarItem(\"Server/DB\", staticServerDB,\"s0\",true,\"index.html\"));\n");

    fprintf(fpOut,"    addMenuBarItem(new menuBarItem(\"Browse\", staticMenu1_1,\"s1\"));\n");
    fprintf(fpOut,"  }\n");
    fprintf(fpOut,"}\n");

    fclose(fpOut);
  }
}

