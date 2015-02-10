
#ifndef CDP_328593764857345
#define CDP_328593764857345

#include "cprop.h"
#include "defs.h"

//===========================================================================>>>
//== Additional Dependency Flags used in CDEP and CPATRN  =====================>>>
//===========================================================================>>>
//This flag concerns the owner of the db_object we are refering to. By default
//both objects at source and sink of a reference are _assumed_ to be the same.
//E.g., we can have a stored procedure TITAN.dbo.proc_1 refering to a table:
//
//TITAN.dbo.tab_1
//
//with: 1> SELECT * FROM tab1
//  or: 2> SELECT * FROM dbo.tab1
//  or: 3> SELECT * FROM TITAN..tab1
//
// Note that we have no explicit owner given in 1> and 2>, the owner could be
// dbo but does not have to be dbo. The reference should also resolve successfully
// if there is no object under the assumed owner, but (only one) under another owner
// (This case is then decided upon when all database objects are parsed and available
//  for resolution).
#define mFLAG_ASSUMED_OWNER  0x1 //iFlag Entries in CDBREF
#define mFLAG_UNKNOWN_OBJECT 0x2 //This flags a reference without given definition

typedef enum
{
  eREFUNKNOWN = 0 //Reference to unknown object
 ,eREFTABVIEW = 1 //Reference to a table or view, we don't know, yet (no definition seen, yet)
 ,eREFTABLE   = 2 //Reference to a table
 ,eREFVIEW    = 3 //Reference to a view
 ,eREFTRIG    = 4 //Reference to a trigger
 ,eREFPROC    = 5 //Reference to a stored procedure
}eREFOBJ;

typedef enum
{
  eWTO_MT_ALL = 0 //return match only, if all four components (server.db.owner.name) are matched
 ,eWTO_SERVER = 1 //Attempt a Match without name of server
 ,eWTO_DB     = 2 //Attempt a Match without name of database
 ,eWTO_OWNER  = 3 //Attempt a Match without name of owner of database object

}eMATCH_WTO;

//Define some Events on which SQL triggers may fire (this is used in CTAB & CPROC)
//This is used in CPROC and CTAB to identify code that fires on events on a table
//------------------------------------------------------------------------------<>
typedef enum{ evUNKNOWN=0, evDELETE=1, evINSERT=2, evUPDATE=4} EVENT_CL;

//List types of data modifications on table/views which are parsed and analyzed
#define dmDELETE      0x1  //Values are used in a bit mask to point
#define dmINSERT      0x2  //at one procedure doing more than class
#define dmUPDATE      0x4  //of  operation e.g. proc2: delete, insert on table x
#define dmUPDATEStats 0x8  //update statistics command implemented
#define dmSELECT      0x10 //Table names that appear in a FROM clause are handled
                           //as select statement access types (read only access)

//This is a simple container class to manage no simple references to
//database objects
//
//1-> CDP::ParseRef2DBOBJ decides about ASSUMING a particular object_owner or not...
class CDP
{
public:
  //Name of server, database, owner, and db object
	CUSTR sServer, sDB, sOwner,sObjName;

  //Type of database object we are refering to
  eREFOBJ eObjType;

  //These are additional flags to interpret parser results correctly
  int iFlags;

public:
  //Standard constructor/destructor
  CDP();
  virtual ~CDP(){}; //Make sure right dtor is called at run-time

  void CopyThis(const CDP & cdpIn);
  CDP(const CDP & cdpIn); //Copy constructor
  CDP(const CUSTR sServ, const CUSTR & sDBIn,const CUSTR & sOwn,const CUSTR & sName
     ,const eREFOBJ & eObjTypeIn, const int iFlagsIn=0) //Copy constructor
  {
    this->sServer  = sServ;
    this->sDB      = sDBIn;
    this->sOwner   = sOwn;
    this->sObjName = sName;
    this->eObjType = eObjTypeIn;
    this->iFlags   = iFlagsIn;
  }

  CDP & operator=(const CDP *pdpIn);
	CDP & operator=(const CDP & dpIn);

  //We refer to same database object, if all four strings:
  //sServer,sDB,sOwner,sDBObj can be matched case-insensitively
  //(except for those note matched intentionally)
  //This is a useful function, if components of the object-
  //reference are unknown and must be found otherwise...
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  bool bMatchRef(const CDP & refIn, const eMATCH_WTO eWTOFilter ) const;

  void Debug(FILE *fp) const; //Standard debugging function for error tracing only

  static void HTML_Color_Obj(FILE *fpOut
                            ,const eREFOBJ clss
                            ,const UCHAR *pcStr=NULL
                            ,const eREFOBJ clssShouldBe=eREFUNKNOWN) //Used when clss is unknown
  {
    switch(clss)
    {    //Reference to a table or view, we don't know, yet (no definition seen, yet)
    case eREFTABVIEW:
      if(pcStr == NULL)
        fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_UNKNOWN_TABLE);
      else
        fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>", CSS_UNKNOWN_TABLE, pcStr);
      break;          
    case eREFTABLE:   //Reference to a table
      if(pcStr == NULL)
        fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_TABLE);
      else
        fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>",CSS_TABLE,pcStr);
      break;          
    case eREFVIEW:    //Reference to a view
      if(pcStr == NULL)
        fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_VIEW);
      else
        fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>", CSS_VIEW, pcStr);
      break;

    case eREFPROC:                     //Reference to a stored procedure
    case eREFTRIG:                    //Reference to a trigger
      if(pcStr == NULL)
        fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_PNM);
      else
        fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>", CSS_PNM, pcStr);
      break;

    case eREFUNKNOWN: //Reference to unknown object
    default:
      switch(clssShouldBe)
      {
      case eREFUNKNOWN: //Reference to unknown object
        if(pcStr == NULL)
          fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_DFLT);
        else
          fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>", CSS_DFLT, pcStr);
      break;

    case eREFVIEW:    //Reference to a view
    case eREFTABLE:   //Reference to a table
    case eREFTABVIEW:
        if(pcStr == NULL)
          fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_UNKNOWN_TABLE);
        else
          fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>", CSS_UNKNOWN_TABLE, pcStr);
      break;
      case eREFPROC:                     //Reference to a stored procedure
      case eREFTRIG:                    //Reference to a trigger
        if(pcStr == NULL)
          fprintf(fpOut, "<SPAN CLASS=\"%s\">", CSS_UNKNOWN_PROC);
        else
          fprintf(fpOut, "<SPAN CLASS=\"%s\">%s</SPAN>", CSS_UNKNOWN_PROC, pcStr);
      break;
      }
    }
  }

  static void HTML_IconClassLink(FILE *fpOut, const eREFOBJ clss)
  {
    switch(clss)
    {               //Reference to a table
    case eREFTABLE: fprintf(fpOut, "<img src=\"res/table.jpg\" title=\"table\">");
      break;
                   //Reference to a view
    case eREFVIEW: fprintf(fpOut, "<img src=\"res/view.jpg\" title=\"view\">");
      break;

    case eREFPROC:   //Reference to a stored procedure
      fprintf(fpOut, "<img src=\"res/proc.jpg\" title=\"procedure\">");
      break;
    case eREFTRIG:   //Reference to a trigger
      fprintf(fpOut, "<img src=\"res/trigger.jpg\" title=\"trigger\">");
      break;

    case eREFTABVIEW:  //Reference to unknown table/view object
    case eREFUNKNOWN: //Reference to unknown object
    default:
      fprintf(fpOut, "<img src=\"res/radar.gif\" title=\"external/unknown table or view\">");
    }
  }

  static void DEBUG_Obj_Type(FILE *fpOut, const eREFOBJ clss);

  //A reference to a database object can contain a user/owner definition
  //and/or references to objects in other databases (string containing the 
  //name of the other database)
  //
  //We look at these cases here and attempt to extract at most four items:
  //
  //Name of Server        - Name of database server we are looking at
  //Name of Database      - Database where the accessed object resides on
  //User/Role             - User being employed in this statement
  //Database-Object Name  - Database object being accessed
  //
  //Default Values for name of server, database, and owner must be set by caller
  //and will be over-written, if passed in through raw reference (in sDBOBJ) contains
  //this information (example for sDBOBJ: "ZEUS.TITAN.dbo.ADDRESS")
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  static void ParseRef2DBOBJ(CUSTR & sServer, CUSTR & sDatabase, CUSTR & sUser, CUSTR & sDBOBJ, int & iFlag);

  //This is just a sort of Assert function to make sure required params are used as required
  //(We don't use assert() though)
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  static void Handle_Requi_Params(const CUSTR & sServ, const CUSTR & sDB
                                 ,const CUSTR & sOwn,const CUSTR & sName, const CUSTR & LOC);

  static CUSTR sGetRef(const CUSTR & sServer,const CUSTR & sDB
                      ,const CUSTR & sOwn,const CUSTR & sName);

};

//This is just a simple container to pass some header information
//from the calling Server/DB object to helper functions
typedef struct thisdb
{
	CUSTR sDB, sDBOwner;    //Name of database and owner
	CUSTR sServer;         //Name of server database is running on

  //Write back to root hyperlink ???
	CUSTR sRootLinkURL, sRootLink, sHTMLDocuPath;

  CPROPCOL TabClassDefs   //Class definitions used to pop-up Icon and descriptions for each class
          ,PrcClassDefs; //when outlining things later in HTML
}
THIS_DB;

#endif
