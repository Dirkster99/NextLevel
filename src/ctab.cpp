
#include "ctab.h"
#include "custr.h"
#include "defs.h"
#include "cerr.h"

#include "copts.h"

//Style highlighting for references to procedures or triggers
#define HTML_PROC_INTRO "<SPAN CLASS=\""CSS_PNM"\">"
#define HTML_PROC_OUTRO "</SPAN>"

// Name of file to reference for external db objects
extern const char EXTERN_CALLS_FNAME[];

void CTAB::Init(const UCHAR *pcServer,   //Name of Database Server object lives on
                const UCHAR *pcDB,      //Name of Database object lives on
                const UCHAR *pcOwner,  //Name of owner of this object
                const UCHAR *pcNameIn,
                const UCHAR *pcUpd,
                const UCHAR *pcIns,
                const UCHAR *pcDel,
                const eREFOBJ etbObjType)
{
  static int iTabIdCounter = 0;  //Generate a unique id for each table/view object

  iDataSz=-1, iIndexSz=-1;
  iKeyID         = iTabIdCounter++;

  this->sServer   = pcServer;
  this->sDB       = pcDB;       //Name of DB Obect lives in
  this->sOwner    = pcOwner;   //Name of owner of this object
  this->sObjName  = pcNameIn; //Copy name
  this->eObjType  = etbObjType;

  this->sTargFileName = this->sObjName.LowerCase() + ".html"; //Generate HTML Target Name

  //Copy trigger values
  if(pcUpd!=NULL) pcUpdTrig = s::sdup(pcUpd);
  else pcUpdTrig=NULL;

  if(pcIns!=NULL) pcInsTrig = s::sdup(pcIns);
    else pcInsTrig=NULL;

  if(pcDel!=NULL) pcDelTrig = s::sdup(pcDel);
    else pcDelTrig=NULL;
}

CTAB::CTAB( const UCHAR *pcServer,
            const UCHAR *pcDB,      //Name of DB Obect lives in
            const UCHAR *pcOwner,  //Name of owner of this object
            const UCHAR *pcName,
            const UCHAR *pcUpd,
            const UCHAR *pcIns,
            const UCHAR *pcDel,
            const eREFOBJ etbObjType)
:CDP(), pcDelTrig(NULL), pcInsTrig(NULL), pcUpdTrig(NULL),
  ppColumns(NULL), iColumnsSz(0), iColumnsActSz(0) //Size & Actual size of Columns Arrays
{
  Init(pcServer,pcDB,pcOwner,pcName,pcUpd, pcIns, pcDel, etbObjType);
}

void CTAB::FreeThis()
{
	if(pcUpdTrig != NULL) //Resolve double refs to same string...
	{
		if(pcUpdTrig == pcInsTrig) pcInsTrig = NULL;
		if(pcUpdTrig == pcDelTrig) pcDelTrig = NULL;

		delete [] pcUpdTrig;
	}

	if(pcInsTrig != NULL) //Resolve double refs to same string...
	{
		if(pcInsTrig == pcDelTrig) pcDelTrig = NULL;

		delete [] pcInsTrig;
	}

	delete [] pcDelTrig;

	pcUpdTrig = pcInsTrig = pcDelTrig = NULL;

	//Copy columns and their properties for this table
	if(ppColumns != NULL && iColumnsActSz > 0)
	{
		for(int i=0;i<iColumnsActSz;i++)
		{
			if(ppColumns[i] !=NULL) delete ppColumns[i]; //Delete each Column
		}
		delete [] ppColumns;               //Delete collection of columns
	}
	iColumnsSz = iColumnsActSz = 0;
}

CTAB::~CTAB()
{
	FreeThis();
}

// =========================================================================
// Description     : Private copy function used by equality operator & cctor
// Argument        : const CTAB & tbIn
// =========================================================================
void CTAB::CopyThis(const CTAB & tbIn)
{
  CDP::CopyThis(tbIn);

  iDataSz         = tbIn.iDataSz;  //Copy size of table information
  iIndexSz        = tbIn.iIndexSz;
	sTargFileName   = tbIn.sTargFileName;
	sSource         = tbIn.sSource;
	m_prpTableProps = tbIn.m_prpTableProps;
	hlPatrn         = tbIn.hlPatrn;
	m_dpDBObjDeps   = tbIn.m_dpDBObjDeps;
  m_csClasses     = tbIn.m_csClasses;

	iKeyID = tbIn.iKeyID;

	// ______________________________________________________________
	if(tbIn.pcUpdTrig != NULL) //Resolve double refs to same string...
	{
		pcUpdTrig = s::sdup(tbIn.pcUpdTrig);

		if(tbIn.pcUpdTrig == tbIn.pcInsTrig) pcInsTrig = pcUpdTrig;
		if(tbIn.pcUpdTrig == tbIn.pcDelTrig) pcDelTrig = pcUpdTrig;
	}

	if(tbIn.pcInsTrig != NULL && pcInsTrig == NULL) //Resolve double refs to same string...
	{
		pcInsTrig = s::sdup(tbIn.pcInsTrig);
    
		if(tbIn.pcInsTrig == tbIn.pcDelTrig) pcDelTrig = pcInsTrig;
	}

	if(tbIn.pcDelTrig != NULL && pcDelTrig == NULL)
    pcDelTrig = s::sdup(tbIn.pcDelTrig);
	// ______________________________________________________________

	//Copy columns and their properties for this table <<<<<<<<<<<<<<
	if(tbIn.ppColumns != NULL && tbIn.iColumnsActSz > 0)
	{
		if((ppColumns = new COLS * [tbIn.iColumnsActSz])!=NULL)
		{
			for(int i=0;i<tbIn.iColumnsActSz;i++)
			{
				if((ppColumns[i] = new COLS())!=NULL)
					ppColumns[i]->clColumn = tbIn.ppColumns[i]->clColumn; //Copy Column properties
			}
			iColumnsSz = iColumnsActSz = tbIn.iColumnsActSz;
		}
	}
}

CTAB::CTAB(const CTAB & tbIn) //Copy CTor
:CDP(), pcDelTrig(NULL), pcInsTrig(NULL), pcUpdTrig(NULL),
  ppColumns(NULL), iColumnsSz(0), iColumnsActSz(0) //Size & Actual size of Columns Arrays
{
  this->CopyThis(tbIn);
}

CTAB::CTAB()
:CDP(),	pcDelTrig(NULL), pcInsTrig(NULL), pcUpdTrig(NULL),
	ppColumns(NULL), iColumnsSz(0), iColumnsActSz(0) //Size & Actual size of Columns Arrays
{
	Init();
}

CTAB & CTAB::operator=(const CTAB & tabObj)
{
	FreeThis(); //Free data and copy new data from pTab

  this->CopyThis(tabObj);

	return *this;
}

CTAB & CTAB::operator=(const CTAB *pTab)
{
	FreeThis(); //Free data and copy new data from pTab
	if(pTab!=NULL) *this = *pTab;

	return *this;
}

//
//Add Trigger plus event information to table object ...
//------------------------------------------------------->
#define INIT_COL_SZ 10
void CTAB::AddColumn(const CPROP & cpInput)
{
	if(ppColumns == NULL) //Init on first call
	{
		if((ppColumns = new COLS * [INIT_COL_SZ])!=NULL)
		{
			iColumnsSz    = INIT_COL_SZ;
			iColumnsActSz = 1;
			if((ppColumns[0] = new COLS())!=NULL)
				ppColumns[0]->clColumn = cpInput;
		}
		else
		{
			iColumnsSz    = 0;
			iColumnsActSz = 0;
		}
	
		return;
	}

	//Need to allocate new array of pointers and grow from there...
	if(iColumnsActSz >= iColumnsSz)    //Grow array of strings
	{
		COLS **ppTmp;
	
		if((ppTmp = new COLS * [INIT_COL_SZ + iColumnsSz])==NULL)
			return; //Can't grow further :(

		for(int i=0;i<iColumnsSz;i++){ ppTmp[i] = ppColumns[i]; }

		delete [] ppColumns;                //Get rid of old array pointers

		ppColumns  = ppTmp;
		iColumnsSz = INIT_COL_SZ + iColumnsSz;
	}

	if(iColumnsActSz < iColumnsSz) //Grow array of patterns
	if((ppColumns[iColumnsActSz] = new COLS())!=NULL)
	{
		ppColumns[iColumnsActSz]->clColumn = cpInput;
		iColumnsActSz++;
	}
}
#undef INIT_COL_SZ

//
//Add Trigger plus event information to table object ...
//------------------------------------------------------->
void CTAB::AddTrigEvents(const UCHAR *pcTrigName, const EVENT_CL evOnEvents)
{
	UCHAR *pcTmp = NULL;

	if(pcUpdTrig == NULL) //Don't overwrite existing trigger/event handler
	{
		if FLAG(evOnEvents, evUPDATE)
			pcTmp = pcUpdTrig = s::sdup(pcTrigName);
	}

	if(pcInsTrig == NULL)
	{
		if FLAG(evOnEvents, evINSERT) //Don't overwrite existing trigger/event handler
		{                            //Copy trigger name once and point to when available...
			if(pcTmp == NULL)
				pcTmp = pcInsTrig = s::sdup(pcTrigName);
			else
				pcInsTrig = pcTmp;
		}
	}

	if(pcDelTrig == NULL)
	{
		if FLAG(evOnEvents, evDELETE) //Don't overwrite existing trigger/event handler
		{                            //Copy trigger name once and point to when available...
			if(pcTmp == NULL)
				pcTmp = pcDelTrig = s::sdup(pcTrigName);
			else
				pcDelTrig = pcTmp;
		}
	}
}

//))))))))))))))))))))))))))((((((((((((((((((((((((((((((((((((((((((((((
//Print SQL Table definition with columns and their SQL datatype into HTML
//(((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))
void CTAB::HTML_PrintTableDefinition(FILE *fpOut)
{
	CSCOL csPropNames;
	int i,j;
	CUSTR sName, sVal;

	//Get unique names of column properties (except for required fields "name", "type")
	for(i=0;i<iColumnsActSz;i++)
	{
		int iPropSz = ppColumns[i]->clColumn.Size();
		for(j=0;j<iPropSz;j++)
		{
			ppColumns[i]->clColumn.GetProp(j, sName, sVal);

			if(sName != "name") //Add prop only, if it ain't required
			{
				if(sName != "type") csPropNames.AddString(sName);
			}
		}
	}

	fprintf(fpOut, "\n<table id=\"NoBord1\" cellPadding=\"4\" cellSpacing=\"0\">");
	fprintf(fpOut, "\n<thead><tr>");
	fprintf(fpOut, "\n<TD id=\"NoBord1Hd\">column</TD>");
	fprintf(fpOut, "\n<TD id=\"NoBord1Hd\">type</TD>");

	if(iColumnsActSz <= 0) //No column information available for this table
	{
		fprintf(fpOut, "\n</tr></thead></table>");
		return;
	}

	const UCHAR *pcProp = csPropNames.GetFirst();

	while(pcProp != NULL) //Print property names for each type in header of table
	{
		fprintf(fpOut, "\n<TD id=\"NoBord1Hd\">%s</TD>", pcProp);	
		pcProp = csPropNames.GetNext();
	}

	fprintf(fpOut, "\n</tr></thead>"); //End of table header

	//Print each column and their property
	for(i=0;i<iColumnsActSz;i++)
	{
		if((i & 1) != 0)           //Paint every second row gray
			fprintf(fpOut, "\n<tr BGCOLOR=\"#FFFFFF\">");
		else
			fprintf(fpOut, "\n<tr BGCOLOR=\"#DDDDDD\">");

		//print at least name and datatype of each column in table
		fprintf(fpOut,"<td id=\"NoBord1\">%s</td>", (const char *)ppColumns[i]->clColumn.sGetVal("name"));
		fprintf(fpOut,"<td id=\"NoBord1\">%s</td>", (const char *)ppColumns[i]->clColumn.sGetVal("type"));

		pcProp = csPropNames.GetFirst();

		while(pcProp != NULL)
		{
			sVal = ppColumns[i]->clColumn.sGetVal(pcProp);

			if(s::scmp_ci_uc(pcProp, "nulls") == 0)
			{
				if(sVal == "NOT NULL")
					sVal = "no";
				else
					sVal = "yes";
			}

			fprintf(fpOut,"<td id=\"NoBord1\">%s</td>", (const char *)sVal);
		
			pcProp = csPropNames.GetNext();
		}
		fprintf(fpOut, "\n</tr>");
	}

	fprintf(fpOut, "\n</table>");
}

#define COL_ALIGN "align=\"right\" valign=\"top\""
#define COL_ALIGN1 "align=\"left\" valign=\"top\""
//
//Print name of a trigger with HTML linkage and event information
//==============================================================>
void CTAB::HTML_PrintTabTrigEvent(const UCHAR *pcName, const UCHAR *pcFName,
                                  const char *pcEvents, FILE *fpOut)
{
	fprintf(fpOut, "<tr><td %s>\n", COL_ALIGN);
	HTML_Color_Obj(fpOut, eREFTRIG);
	fprintf(fpOut, "</td><td %s><a href=\"%s\">%s%s%s</a>&nbsp;\n", COL_ALIGN1, pcFName,
	                                          HTML_PROC_INTRO, pcName,HTML_PROC_OUTRO);

	fprintf(fpOut, "<small>(ON: %s)</small></td></tr>\n", pcEvents);
}

//Print Header Information for each SQL Table/View in HTML
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CTAB::HTML_PrintTable(FILE *fpOut, const CUSTR & sOutPutDir
                          ,const CUSTR & sUpdTrigSrc,const CUSTR & sDelTrigSrc, const CUSTR & sInsTrigSrc
                          ,const THIS_DB & thisDB)
{	//Print intro table...
	fprintf(fpOut, "\n<table id=\"NoBord\" cellSpacing=\"0\" width=\"100%%\">\n");
	fprintf(fpOut, "<tr><td %s><table id=\"NoBord\">\n", COL_ALIGN1);

	fprintf(fpOut, "<tr><td %s>", COL_ALIGN);
	HTML_IconClassLink(fpOut, this->eObjType);
	fprintf(fpOut, "</td><td %s>\n", COL_ALIGN1);

	if(sSource.slen() > 0)
	{
		CUSTR s1(s::sGetPath(sSource));
		CUSTR sSQLLinkPath(s::GetRelPath(sOutPutDir, s1));

		sSQLLinkPath += s::sGetFileName(sSource);

		fprintf(fpOut, "<a href=\"%s\">%s%s%s</a>", (const char *)sSQLLinkPath,
		                     HTML_PROC_INTRO, (const char *)this->sObjName, HTML_PROC_OUTRO);
	}
	else
		fprintf(fpOut, "%s", (const char *)this->sObjName);

	if(m_prpTableProps.Size() > 0)
	{
		fprintf(fpOut, "<small>\n");
		CUSTR sName, sVal;
		for(int i=0;i<m_prpTableProps.Size();i++)
		{
			m_prpTableProps.GetProp(i, sName, sVal);
			fprintf(fpOut, " %s=\"%s\"",(const UCHAR *)sName, (const UCHAR *)sVal);
		}
		fprintf(fpOut, "</small>\n");
	}

	fprintf(fpOut, "</td></tr>\n");

	if(this->pcUpdTrig != NULL)
	{
		if(pcDelTrig != pcUpdTrig && pcInsTrig != pcUpdTrig)
			HTML_PrintTabTrigEvent(pcUpdTrig, sUpdTrigSrc, "UPDATE", fpOut);
		else
		if(pcDelTrig == pcUpdTrig && pcInsTrig != pcUpdTrig)
			HTML_PrintTabTrigEvent(pcUpdTrig, sUpdTrigSrc, "UPDATE, DELETE", fpOut);
		else
		if(pcDelTrig != pcUpdTrig && pcInsTrig == pcUpdTrig)
			HTML_PrintTabTrigEvent(pcUpdTrig, sUpdTrigSrc, "UPDATE, INSERT", fpOut);
		else
		if(pcDelTrig == pcUpdTrig && pcInsTrig == pcUpdTrig)
			HTML_PrintTabTrigEvent(pcUpdTrig, sUpdTrigSrc, "UPDATE, INSERT, DELETE", fpOut);
	}
	
	if(pcInsTrig != NULL)
	if(pcInsTrig != pcUpdTrig)
	{
		if(pcInsTrig != pcDelTrig)
			HTML_PrintTabTrigEvent(pcInsTrig, sInsTrigSrc, "INSERT", fpOut);
		else
			HTML_PrintTabTrigEvent(pcInsTrig, sInsTrigSrc, "INSERT, DELETE", fpOut);

		fprintf(fpOut, "\n");
	}
	
	if(pcDelTrig != NULL)
	if(pcDelTrig != pcUpdTrig)
	if(pcDelTrig != pcInsTrig)
	{
		HTML_PrintTabTrigEvent(pcDelTrig, sDelTrigSrc, "DELETE", fpOut);
	}
  
  if(this->m_csClasses.Sz() > 0)
  {
    fprintf(fpOut, "<tr><td colspan=\"2\">\n");

    const UCHAR *pcClassName = this->m_csClasses.GetFirst();

	  while(pcClassName != NULL) //Print property names for each type in header of table
	  {
      if(thisDB.TabClassDefs.Get_i(pcClassName) == true)
      {
        CPROP tPROP;
        thisDB.TabClassDefs.Get(pcClassName,tPROP);
        fprintf(fpOut, "\n<img src=\"%s\" title=\"%s\">",
                (const char*)tPROP.sGetVal("icon"), (const char*)tPROP.sGetVal("text"));
      }
		  pcClassName = m_csClasses.GetNext();
	  }
  
	  fprintf(fpOut, "</td></tr>\n");
  }

	fprintf(fpOut, "</table>\n");
	fprintf(fpOut,"</td>\n<td %s><div id=\"staticMenu\"></div></td>\n</tr>\n</table>\n\n",COL_ALIGN);
	//End of table intro
}
#undef HTML_PROC_INTRO
#undef HTML_PROC_OUTRO

#undef COL_ALIGN
#undef COL_ALIGN1

//Get name of HTML target File associated with this table
CUSTR CTAB::GetTargFileName() const
{
	if(this->eObjType == eREFUNKNOWN)  //Return external linking for this type of tab
	{
		CUSTR s(EXTERN_CALLS_FNAME);

		s += "#t";
		s += iKeyID;
		return s;                    //Reference to externals page plus id anker
	}

	return sTargFileName;
}

void CTAB::Debug(FILE *fp)
{
  CDP::Debug(fp);
  fprintf(fp, "TRIGGER DELETE: (0x%lx)%s\n", (long)pcDelTrig,pcDelTrig);
  fprintf(fp, "TRIGGER INSERT: (0x%lx)%s\n", (long)pcInsTrig,pcInsTrig);
  fprintf(fp, "TRIGGER UPDATE: (0x%lx)%s\n", (long)pcUpdTrig,pcUpdTrig);

  if(this->m_dpDBObjDeps.CountObjTypes(eREFUNKNOWN,true)>0)
  {
    fprintf(fp, "Dependencies:\n");
    this->m_dpDBObjDeps.Debug(fp);
  }
  fprintf(fp, "\n");
}
