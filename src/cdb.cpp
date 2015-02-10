
#include "cdb.h"              //Database structures class

#include "custr.h"          //string class

#include "cpars.h"         //xml parser
#include "parse.h"        //sql parser
#include "prettyprint.h" //html generator

#include <stdio.h>
#include <time.h>

// Standard Constructor/Destructor
CDB::CDB(){}
CDB::~CDB(){}

void CDB::CopyThis(const CDB & dbIn)
{
  prpDBProps  = dbIn.prpDBProps;
  tbcTabs     = dbIn.tbcTabs;  //Collection of Table/View Objects
  prcPrcs     = dbIn.prcPrcs; //Collection of procedure/trigger objects

  TabClassDefs  = dbIn.TabClassDefs;
  PrcClassDefs  = dbIn.PrcClassDefs;

  //URL and content for href to link to root page of NextLevel pages
  //in the XML import this is the <backlink .../> tag inside <htmlroot>
  sRootLinkURL  = dbIn.sRootLinkURL;
  sRootLink     = dbIn.sRootLink;
  sOwner	      = dbIn.sOwner;
  sDB	          = dbIn.sDB;
  sServer	      = dbIn.sServer;
  sOutPutDir    = dbIn.sOutPutDir;      //Directory for output files
  sHTMLDocuPath = dbIn.sHTMLDocuPath;
  sInpFile      = dbIn.sInpFile;       //Input Path for SQL Files
  sIdxFile      = dbIn.sIdxFile;      //Path and Name of input index.xml (and sql input files)   
}

// Copy Constructors and Equal Operators
CDB::CDB(const CDB & dbIn){CopyThis(dbIn);}

CDB & CDB::operator=(const CDB & dbIn){CopyThis(dbIn); return *this;}
CDB & CDB::operator=(const CDB *pDB)
{
  if(pDB != NULL) *this = *pDB;

	return *this;
}

//Define state nodes
//There may be more state nodes for a given state if a tag
//can occur in several places...
#define STN_CLASS_DEF 2
#define STN_PROC      3
#define STN_TABLE     4
#define STN_TABCOL    5

#define STN_VIEW      6
#define STN_VIEWCOL   7
#define STN_HTMLROOT  8
#define STN_PROGOPTS  9 //Program options imported via HTML

//Define states these are closely bound to a tag
//that has been recognized, while STN_ states are
//bound to a recognized tag on a well defined path
//of the XML document
//))))))))))))))))))))))))))))))))))))))))))))))))
#define ST_CLASS_DEF 2
#define ST_PROC      3
#define ST_TABLE     4
#define ST_TABCOL    5

#define ST_VIEW     6
#define ST_VIEWCOL  7
#define ST_HTMLROOT 8
#define ST_PROGOPTS 9

// XML GRAMMAR Definitions used to seperate useful tags from boggos information
// Each node in the next array represents a state or xml tag of a certain name
#define stdDEFSSZ 9
const TRANSDEF stdDEFS[stdDEFSSZ] =
{
	 {"parse_files", ST_ROOT}
	,{"CLASS_DEF", ST_CLASS_DEF}
	,{"procs", ST_PROC}
	,{"tab", ST_TABLE}
	,{"col", ST_TABCOL}

	,{"view", ST_VIEW}
	,{"col", ST_VIEWCOL}
	,{"htmlroot", ST_HTMLROOT}
	,{"program_opts", ST_PROGOPTS}
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
#define trTransSZ 6
const STRANS trTrans[trTransSZ] =
{
	 {{ST_ROOT, STN_ROOT},   {ST_CLASS_DEF, STN_CLASS_DEF}}
	,{{ST_ROOT, STN_ROOT},   {ST_PROC, STN_PROC}}

	,{{ST_ROOT, STN_ROOT},   {ST_TABLE, STN_TABLE}}       //XML TABLE definition tag
	,{{ST_ROOT, STN_ROOT},   {ST_VIEW,  STN_VIEW}}        //XML TABLE definition tag

	,{{ST_ROOT, STN_ROOT},   {ST_HTMLROOT, STN_HTMLROOT}} //HTMLROOT TAG
	,{{ST_ROOT, STN_ROOT},   {ST_PROGOPTS, STN_PROGOPTS}} //PROG_OPTS ROOT TAG
};

//The next states describe XML TAGS which will be loaded with child tags
#define trTransSZ_TAB 1
const STRANS trTrans_TAB[trTransSZ_TAB] =
{
	{{ST_TABLE, STN_TABLE}, {ST_TABCOL, STN_TABCOL}} //TABLE COLUMN TAG
};

#define trTransSZ_VIEW 1
const STRANS trTrans_VIEW[trTransSZ_VIEW] =
{
	{{ST_VIEW, STN_VIEW}, {ST_VIEWCOL, STN_VIEWCOL}} //VIEW COLUMN TAG
};

//Initiate Main Loop for XML parser
//--------------------------------->
void CDB::InitIndexStack(CSTAK & stkTags)
{
	//initialize network of main state transitions
	stkTags.InitMajorEngine(trTrans, trTransSZ, stdDEFS, stdDEFSSZ);

	//define interesting tags and their interesting children
	stkTags.InitMinorEngine(trTrans_TAB, trTransSZ_TAB);

	//define interesting tags and their interesting children
	stkTags.InitMinorEngine(trTrans_VIEW, trTransSZ_VIEW);
}

#define DATA_FOUND 0x01 //Tell when data is available in dictionary
#define OK         0x02 //Everythings fine ...just continue processing

int CDB::ManageXMLState(int iThisSt, CTAG *prtNext)
{
	switch(prtNext->getTagTyp())
	{
	case TGP_CLO_OPEN:
		switch(iThisSt)
		{
			case STN_ROOT:                                  //XML tag pointing to class
        if(prtNext->getTagName() == "CLASS_DEF" ||   //definition/classification
           prtNext->getTagName() == "CLASS")        //of database objects
          return DATA_FOUND;
      break;

      case STN_HTMLROOT:       //HTMLROOT XML tag found
			case STN_PROGOPTS:       //HTMLROOT XML tag found
			case STN_PROC:          //procedure XML tag found
				return DATA_FOUND;
		}
	break;

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

	default:          //Nothing todo on default (keeps away compiler warnings)
		break;
	}
		
	return OK;
}

#undef ST_CLASS_DEF
#undef ST_PROC

#undef trTransSZ
#undef stdDEFSSZ

CTAG *CDB::SearchXMLFile(CPARS & xpPState, CSTAK & stkTags, bool & bIntro)
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

int CDB::ParseFiles(COpts *pPrOpts, int & iCntProcs, int & iCntTrigs)
{
	if(pPrOpts==NULL) return -1;

	CPARS xpPState;   // input file stream
	CSTAK stkTags;     //Stack and TAG object used for XML parsing
	CTAG *pTAGS;
	bool  bIntro=true; //We parse XML from the very start
	CPROC *pProc=NULL; //Procedure object is the direct result of parsing SQL code
  int iFileCnt=0;
	CUSTR src, sName, sTMP, sPath, sOwner;

	if(this->IdxFile().slen() <=0)
		throw CERR("ERROR: CDB::ParseFiles No index file name set!\n");

	try
	{
		CUSTR sDebug, sName, sVal;

		xpPState.open(this->IdxFile()); //Open XML source file

		this->InitIndexStack(stkTags); //Init stack obj with XML definitions

		while(xpPState.eof() == false)
		{
			if((pTAGS = this->SearchXMLFile(xpPState, stkTags, bIntro))!=NULL)
			{
				switch(stkTags.getStateID())
				{
				case STN_PROC: //procedural code found, then parse & analyze it...
					src   = pTAGS->getTagName();
					if(src == "proc")            //PROCESS 'PROC' XML TAG inside <procs>
					{
						//Throw standard XML parser error, if required property is missing
						sOwner = pTAGS->Props.sGetVal("owner");
						src    = pTAGS->Props.sGetVal("src");
						sName  = pTAGS->Props.sGetVal("name");

						if(sOwner.slen() <= 0 || sName.slen() <= 0  || src.slen() <= 0)
						{
							xpPState. throwErr("owner, src, and name properties are required in <proc> XML-Tag", ERR_SYNTAX);
							return -3;                          //(This is never reached)
						}

						//Parse this file and return a binary in-memory representation of its contents
						if((pProc=ParsePROCFile(this->sServer,this->sDB,sOwner,sName,this->sInpFile,src,pPrOpts,tbcTabs))!=NULL)
						{
							if(pProc->GetObjType() == eREFTRIG) iCntTrigs++;
							else
								if(pProc->GetObjType() == eREFPROC) iCntProcs++;
	
							sDebug = pProc->GetName();
		
							for(int i=0;i<pTAGS->Props.Size();i++) //additional properties
							{
								pTAGS->Props.GetProp(i, sName, sVal);
	
								if(sName != "name" && sName != "src" && sName != "type")
									pProc->m_prpProps.AddProp(sName, sVal);
							}

              prcPrcs.AddProc(pProc); //Add into collection of procs
		
              if(prcPrcs.FindPRCNode(pProc->GetServer(),pProc->GetDB()
                                    ,pProc->GetOwner(),pProc->GetName())==false)
							{
                CUSTR s("ERROR: Inserted Item in DB:");
                s += pProc->GetDB();
                s += ", Owner:";
                s += pProc->GetOwner();
                s += ", Name:";
								s += pProc->GetName();
								s += " lost in proc collection!";
		
								throw CERR(s); //Throw Procedure/Trigger lost in collection error!
							}
							iFileCnt++;
		
							FREE_OBJ(pProc);
						}
					}                   // END OF PROCESS 'PROC' XML TAG
				break;
				case STN_ROOT:
				if(pTAGS->getTagName() == "tab")
				{
					sOwner = pTAGS->Props.sGetVal("owner");
					if(sOwner.slen() <= 0)
					{
						xpPState.throwErr("The owner properties is required in <tab> XML-Tag", ERR_SYNTAX);
						return -4;                              //(This is never reached)
					}
          //consume this table tag
          tbcTabs.AddTableView(*pTAGS, this->sServer,this->sDB,sOwner, eREFTABLE);
				}

        if(pTAGS->getTagName() == "view") //consume view tag + parse SQLsource
				{
					sOwner = pTAGS->Props.sGetVal("owner");
					if(sOwner.slen() <= 0)
					{
						xpPState.throwErr("The owner properties is required in <view> XML-Tag", ERR_SYNTAX);
						return -4;                              //(This is never reached)
					}

					tbcTabs.AddTableView(*pTAGS, this->sServer,this->sDB, sOwner, eREFVIEW);

					//Get source code for view, if available, and parse it
					if(pTAGS->Props.sGetVal("src").slen() > 0)
					{
						//Parse file and highlight sql-source for this view here
						CUSTR sSrc(pTAGS->Props.sGetVal("src"));
						CUSTR sName(pTAGS->Props.sGetVal("name"));
						tbcTabs.ParseTabFile(sInpFile, sSrc, this->sServer, this->sDB, sOwner, sName, pPrOpts);
					}
				}

				//Copy properties such as DB Server, DATABASE etc...
				if(pTAGS->getTagName() == "parse_files")
				{
          //Throw standard XML parser error, if required property is missing
          if(pTAGS->Props.sGetVal("SERVER").slen() <= 0 ||
              pTAGS->Props.sGetVal("DB").slen()     <= 0 ||
              pTAGS->Props.sGetVal("owner").slen()  <= 0)
                xpPState.throwErr("SERVER, DB, owner required in <parse_files>! (e.g. SERVER=\"ZEUS\" DB=\"TITAN\" owner=\"dbo\")", ERR_SYNTAX);

          this->sServer    = pTAGS->Props.sGetVal("SERVER");
					this->sDB        = pTAGS->Props.sGetVal("DB");
 					this->sOwner     = pTAGS->Props.sGetVal("owner");
					this->prpDBProps = pTAGS->Props;
				}

        //Copy class definition/classification of database objects
        //Sample XML: <CLASS_DEF type="table" classify="replicate_source" icon="res/rep_src.png" text="Source of Replication" />
				if(pTAGS->getTagName() == "CLASS_DEF")
        {
          //Check Requirements here
          if(pTAGS->Props.sGetVal("type").slen() <= 0 || pTAGS->Props.sGetVal("classify").slen() <= 0 ||
            pTAGS->Props.sGetVal("icon").slen() <= 0  || pTAGS->Props.sGetVal("text").slen() <= 0)
            xpPState.throwErr("type, classify, icon, and text are required properties in <CLASS_DEF>!", ERR_SYNTAX);

          CPROP tProps;
          tProps.AddProp("icon",pTAGS->Props.sGetVal("icon")); //Copy these two properties for each class
          tProps.AddProp("text",pTAGS->Props.sGetVal("text"));

          if(pTAGS->Props.sGetVal("type").scmp_ci((const UCHAR*)"table")==0)
            TabClassDefs.Add(pTAGS->Props.sGetVal("classify"), tProps);

          if(pTAGS->Props.sGetVal("type").scmp_ci((const UCHAR*)"proc")==0)
            this->PrcClassDefs.Add(pTAGS->Props.sGetVal("classify"), tProps);
        }

        //Sample XML Tag: <CLASS type="table" classify="replicate_source" owner="dbo" name="TableName" />
				if(pTAGS->getTagName() == "CLASS" )
        {
          //Check Requirements here
          if(pTAGS->Props.sGetVal("type").slen() <= 0 || pTAGS->Props.sGetVal("classify").slen() <= 0 ||
            pTAGS->Props.sGetVal("owner").slen() <= 0  || pTAGS->Props.sGetVal("name").slen() <= 0)
            xpPState.throwErr("type, classify, owner, and name are required properties in <CLASS>!", ERR_SYNTAX);

          if(pTAGS->Props.sGetVal("type").scmp_ci((const UCHAR*)"table")==0)
            this->tbcTabs.AddTableClass(this->sServer, this->sDB,pTAGS->Props.sGetVal("owner")
                                       ,pTAGS->Props.sGetVal("name"), pTAGS->Props.sGetVal("classify"));
          else
          if(pTAGS->Props.sGetVal("type").scmp_ci((const UCHAR*)"proc")==0)
            this->prcPrcs.AddProcClass(this->sServer, this->sDB,pTAGS->Props.sGetVal("owner")
                                       ,pTAGS->Props.sGetVal("name"), pTAGS->Props.sGetVal("classify"));
        }
        break;
				case STN_HTMLROOT: //HTMLROOT tag found, find web site info and store it
					src   = pTAGS->getTagName();  //for later use in memory
					if(src == "backlink")  //PROCESS 'backlink' XML TAG inside <html_root>
					{
						sRootLinkURL = pTAGS->Props.sGetVal("link");
						sRootLink    = pTAGS->Props.sGetVal("text");
					}
				break;
				case STN_PROGOPTS:           //Additional program-options
					src = pTAGS->getTagName();
					if(src == "dbobj_docs")   //Get documentation folder to dbobj docus
					{
						sTMP = s::sGetCWD();
    				sTMP = s::GetAbsPath(sTMP, this->sInpFile);

						src = pTAGS->Props.sGetVal("folder");
						sPath = s::GetAbsPath(sTMP, src);

						if(sPath.slen()>0)            //Make sure path is terminated
						if(sPath[sPath.slen()-1] != DIR_DELIMITER) sPath +=(UCHAR)DIR_DELIMITER;

            printf("     Include: Documentation Folder -> %s\n",(const char *)sPath);
						this->sHTMLDocuPath = sPath;
					}
				}                //End of switch statement
			}
			FREE_OBJ(pTAGS); //Free tag data used in last loop...
		}                 //while loop from main-parser

    //Resolve reference to unknown tab-objects (couldn't decide for view or table up to here)
    //Resolve table collection first and use the result in the proc collection
    //to resolve dependencies to unknown objects...
    tbcTabs.ResolveUnknownTables(this->sServer, this->sDB);
    prcPrcs.ResolveUnknownTables(tbcTabs, this->sServer, this->sDB); //Resolve assumed user entries

    //Remove Unknown Tables that hold no dependency
    tbcTabs.RemoveAssumedTables();

    //Resolve assumed users, if this is possible for unresolved deps
    prcPrcs.ResolveUnknownProcs(this->sServer, this->sDB);

    //Resolve flag for external/internal/undefined Database Calls
    prcPrcs.ResolveExtIntDBCalls(this->sServer, this->sDB);

    //Copy trigger information to table objects
    prcPrcs.Trigger2Tables(tbcTabs, this->sServer, this->sDB);

    //Create a proc object+unique ID for each call to an unknown proc
    //This way we can later cycle through all db objects regardless of their
    //status of definition, this will then include (un)known tables/procs
    prcPrcs.CreateUnknownProcEntries();
    tbcTabs.CreateUnknownTabEntries(prcPrcs);
  }
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: CDB::Parsefiles\n");
		return -2;
	}
	catch(...) //Progress Error and throw to caller
	{
		xpPState.throwErr("UNKNOWN ERROR ooccured in CDB::Parsefiles!!!\n");
		return -3;
	}
		
	return iFileCnt;
}

#undef STN_PROC
#undef STN_CLASS_DEF

#define _HTML_TABLE_FRAME "<table id=\"NoBord\" width=\"100%%\"><tr>\n"

int CDB::PrintHTML(COpts *pPrOpts, int & iCntProcs, int & iCntTrigs, int & iCntTabs)
{
	if(pPrOpts==NULL) return -1;

  if(this->sOutPutDir.slen() <=0)
		throw CERR("ERROR: CDB::PrintHTML No HTML OutPut-Directory set!\n");

  int iFileCnt=0;
	THIS_DB thisDB; //A short_cut structur to capsule useful information

	thisDB.sServer       = this->sServer;
	thisDB.sDB           = this->sDB;
	thisDB.sDBOwner      = this->sOwner;
  thisDB.sRootLinkURL  = this->sRootLinkURL;
  thisDB.sRootLink     = this->sRootLink;
  thisDB.sHTMLDocuPath = this->sHTMLDocuPath;

   //Class definitions used to pop-up Icon and descriptions for each class
  //when outlining things later in HTML
  thisDB.TabClassDefs  = this->TabClassDefs;
  thisDB.PrcClassDefs  = this->PrcClassDefs;

  try
	{ CUSTR sDir(this->GetOutPutDir()); //Get path to SQL source
    CUSTR s;

		CUSTR sSQLLinkPath(s::GetRelPath(sDir, this->sInpFile));
		CUSTR sFile;

		// >>>>>>>>>>>>>>>>>>>>> Write index file <<<<<<<<<<<<<<<<<<<<<<<<<<<
		sFile = s::CreateFileName(this->GetOutPutDir(), "index", ".html");
		FILE *fpOut;
		
		if((fpOut = fopen(sFile, "wb"))!=NULL)
		{
			CUSTR sName, sVal;
			int i=0;

			s = this->sServer + "/" + this->sDB;
			fprintf(fpOut, "%s\n<html>\n\t<head>\n\t\t<title>%s (NextLevel Index)</title>\n"
			             , DOCTYPE_TAG, (const char *)s);
			fprintf(fpOut, "\t<link rel=\"shortcut icon\" href=\"res/nlogo.jpg\">\n");
			fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/style.css\">\n");

			fprintf(fpOut, "<link rel=\"stylesheet\" type=\"text/css\" href=\"res/office_xp.css\" title=\"Office XP\" />\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenu.js\"></script>\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"res/jsdomenubar.js\"></script>\n");
			fprintf(fpOut, "<script type=\"text/javascript\" src=\"menuconf.js\"></script>\n");

			fprintf(fpOut, "<META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\">\n");
			fprintf(fpOut, "<META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\">\n");

      fprintf(fpOut, "</head>\n<body onload=\"initjsDOMenu(\'%s\')\">\n", (const char *)s);

			//Link to procedures that are external to this database
			HTML_Externals(sDir, prcPrcs,tbcTabs,thisDB);
      iFileCnt++;

			//Link to procs, tables, and views that are unreferenced in this DB
			HTML_PrintUnrefDBObj(sDir,prcPrcs,tbcTabs, thisDB);
      iFileCnt++;

			fprintf(fpOut, _HTML_TABLE_FRAME);

			//Write back to root hyperlink ???
			if(sRootLinkURL.slen() > 0 && sRootLink.slen() > 0)
			{
				fprintf(fpOut, "<td valign=\"top\" align=\"left\">\n"); //>>>>>>>>>>

        fprintf(fpOut, "<a href=\"%s\"><img border=\"0\"", (const char *)sRootLinkURL);
        fprintf(fpOut, " src=\"res/back.jpg\" title=\"%s\"></a>",(const char *)sRootLink);
				fprintf(fpOut, "\n</td>\n");
			}
			// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			fprintf(fpOut, "<td valign=\"top\" align=\"left\">\n");
			time_t ltime;  time( &ltime ); //Print local time and date

			//Write date of construction and properties if available ...
			fprintf(fpOut, "<small><small>Date=\"%s\"", ctime( &ltime ) );
			for(i=0;i<prpDBProps.Size();i++)
			{
				prpDBProps.GetProp(i, sName, sVal);
				fprintf(fpOut, " %s=\"%s\"",(const UCHAR *)sName, (const UCHAR *)sVal);
			}
			fprintf(fpOut, "</small></small>\n");
			//Get some basic counting statistics
			int iCntTables = tbcTabs.iCntTabObjects(eREFTABLE, false);
			int iCntViews  = tbcTabs.iCntTabObjects(eREFVIEW,  false);
			int iCntProcs  = prcPrcs.iCntPrcObjects(eREFPROC,  false);
			int iCntTrigs  = prcPrcs.iCntPrcObjects(eREFTRIG,  false);

			fprintf(fpOut, "<br><small><small>Tables: %i, Views: %i\n", iCntTables, iCntViews);
			fprintf(fpOut, ", Procedures: %i, Triggers: %i</small></small></td>\n", iCntProcs, iCntTrigs);

			fprintf(fpOut, "<td valign=\"top\" align=\"right\">\n"); //>>>>>>>>>>
			fprintf(fpOut, "<div id=\"staticMenu\"></div></td>\n");
			fprintf(fpOut, "</tr></table>\n");

      fprintf(fpOut, "<center>\n\n");
			WriteHTMLIndex(fpOut, &prcPrcs, &tbcTabs, thisDB);
      iFileCnt++;

      // >>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			// Write name of generator and print logo of tool >>>>>>>>>>>>>>>>>>
      // >>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			fprintf(fpOut, "<p><small><a href=\"https://github.com/Dirkster99/NextLevel\">");
			fprintf(fpOut, "<img border=\"0\" width=\"32\" height=\"32\" src=\"res/nl_logo.jpg\" align=\"middle\"></a>");
			fprintf(fpOut, "Generated by NextLevel Version %s Build %i</small></p>\n",
										 (const char *)pPrOpts->sVers, pPrOpts->iBuild);
			fprintf(fpOut, "</center></body></html>");

			FCLOSE(fpOut);
		}
		// >>>>>>>>>>>>>>>>>>>>> Write index file <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		// >>>>>>>>>>>>>>>>>>>>> Write Table and Proc files <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		iCntTabs = tbcTabs.HTML_PrettyPrintTabs(tbcTabs,prcPrcs,thisDB,this->sOutPutDir);

    // Generate HTML for procedural database objects (stored procedures and triggers)
    prcPrcs.ppPrintCGs(tbcTabs, this->sInpFile, prcPrcs,
                        sSQLLinkPath, thisDB, this->sOutPutDir, &iCntProcs, &iCntTrigs);

		if(pPrOpts->bGenWinHelp == true) //Generate WinHelp Header files
		{
			WriteWinHelpHeaderFiles(thisDB,tbcTabs, prcPrcs, prpDBProps, this->sOutPutDir);
		}
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: CDB::PrintHTML\n");
		return -2;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR ooccured in CDB::PrintHTML!!!\n");
		return -3;
	}
	iFileCnt += iCntProcs + iCntTrigs + iCntTabs;

	return iFileCnt;
}

#define E_SRC "CDB::FindUnknownObjects"
//Get Dependencies to objects that are external to this DB
//========================================================
CDEPS *CDB::FindExtDBObjects(CPITEM *pitDBs)
{
	try
	{ //Collect external references to unknowns
		CDEPS *pdpExt, *pdpExtPrc;

		//Add DB external tabs and procs to collections
		pdpExt    = tbcTabs.FindUnknownObjects(2, this->sServer, this->sDB, pitDBs);
		pdpExtPrc = prcPrcs.FindUnknownObjects(2, this->sServer, this->sDB, pitDBs);

		if(pdpExt != NULL)
		{
			*pdpExt += pdpExtPrc;
			delete pdpExtPrc;
			pdpExtPrc = NULL;
		}

		return pdpExt;
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: %s\n", E_SRC);
		return false;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR",0,E_SRC);
		return false;
	}

	return NULL;
}
#undef E_SRC

//Resolve outbound dependencies across Database limits (step 1)
UINT CDB::ResolveForeignDBDeps(CDEPS * pdpDB)
{
	CUSTR sAbsPath;        //Build Absolute path to output directory for this Database
  sAbsPath = s::sGetCWD(); //Get Absolute path to current working directory

  sAbsPath = s::GetAbsPath(sAbsPath, this->GetOutPutDir());

  THIS_DB thisDB; //A short_cut structur to capsule useful information
	thisDB.sDB      = this->sDB;
	thisDB.sDBOwner = this->sOwner;
	thisDB.sServer  = this->sServer;

  UINT iRet=0;
	iRet += tbcTabs.ResolveForeignDBDeps(sAbsPath, thisDB, pdpDB);
	iRet += prcPrcs.ResolveForeignDBDeps(sAbsPath, thisDB, pdpDB);

  return iRet;
}

//Resolve outbound dependencies across Database limits (Step 2)
UINT CDB::SetForeignDBDeps(CDEPS * pdpDB)
{
	CUSTR sAbsPath;        //Build Absolute path to output directory for this Database
  sAbsPath = s::sGetCWD(); //Get Absolute path to current working directory

  sAbsPath = s::GetAbsPath(sAbsPath, this->GetOutPutDir());

  THIS_DB thisDB; //A short_cut structur to capsule useful information
	thisDB.sDB      = this->sDB;
	thisDB.sDBOwner = this->sOwner;
	thisDB.sServer  = this->sServer;

  UINT iRet=0;

  iRet += tbcTabs.SetForeignDBDeps(thisDB, pdpDB);
  iRet += prcPrcs.SetForeignDBDeps(thisDB, pdpDB, tbcTabs);

  return iRet;
}

//Find all external objects that depend on a given object and return their access codes
UINT CDB::FindAllOutboundDeps(CDEPS & dpRet
                             ,const CUSTR & sServer,const CUSTR & sDB
                             ,const CUSTR & sOwner, const CUSTR & sName)
{
  CUSTR sAbsOutPath(s::GetAbsPath(s::sGetCWD(),this->sOutPutDir));

  UINT iRet  = tbcTabs.FindAllOutboundDeps(dpRet,sServer,sDB,sOwner,sName, sAbsOutPath);
       iRet += prcPrcs.FindAllOutboundDeps(dpRet,sServer,sDB,sOwner,sName, sAbsOutPath);

 return iRet;
}

// Add the given dependencies to the dependencies addressed by: pcDB.pcOwner.pcName
//==================================================================================>
void CDB::AddOutboundDeps(CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB
                         ,const UCHAR *pcOwner,const UCHAR *pcName, const eREFOBJ & eObjType)
{
  switch(eObjType)
  {
  case eREFTABVIEW:
  case eREFTABLE:       //Dependence on database table
  case eREFVIEW:       //Dependence on database view
    this->tbcTabs.AddOutboundDeps(dpDepIn,pcServer,pcDB,pcOwner,pcName);
    break;
  case eREFPROC:    //Dependence on database procedure
  case eREFTRIG:   //Dependence on database trigger
    this->prcPrcs.AddOutboundDeps(dpDepIn,pcServer,pcDB,pcOwner,pcName);
    break;
  default:
    break;
  }
}

void CDB::SetDBProps(THIS_DB & thisDB)
{
  thisDB.sDB      = this->sDB;
  thisDB.sDBOwner = this->sOwner;   //Name of database and owner
	thisDB.sServer  = this->sServer; //Name of server database is running on

  //Write back to root hyperlink ???
	//URL and content for href to link to root page of NextLevel pages
	//in the XML import this is the <backlink .../> tag inside <htmlroot>
	thisDB.sRootLinkURL = this->sRootLinkURL;
  thisDB.sRootLink    = this->sRootLink;
}
