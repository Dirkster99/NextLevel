
#ifndef PRETTYPRINT_2634328593764857345__234234
#define PRETTYPRINT_2634328593764857345__234234

#include "custr.h"
#include "cpatrn.h"

#include "cproccol.h"
#include "ctabcol.h"

#include "DBDefs.h"

//Convert number to string and return formated version of table id
//=================================================================>
void FormatTableKey(hlTYPE hlTyp, const int iKey, UCHAR *pcKey, const int iKeySz);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Link to procedures, tables, and views that are unreferenced in this DB
//Print Unreferenced Database Objects HTML page on output stream
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HTML_PrintUnrefDBObj(CUSTR & sOutPutDir,CPRCCOL & prcPrcs,
                          CTABCOL & tbcTabs, const THIS_DB thisDB);

//Write information about external database procedures
//Print External Objecta HTML page on output stream
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HTML_Externals(CUSTR sOutDir,CPRCCOL & prcPrcs,CTABCOL & tbcTbs,
                    const THIS_DB thisDB);

//Write index page with links to other items
void WriteHTMLIndex(FILE *fpOut,CPRCCOL *prcPrcs,CTABCOL *ptbTabs,const THIS_DB & thisDB);

//Highlight SQL Source texts in HTML
//
void HTML_HighLightSQL(const CPRCCOL & prcPrcs, const CTABCOL & tbcTabs
                      ,const CDEPS  & dpDBObjDeps    //Database Proc/Tric Object Deps
                      ,CPATRN & hlPatrn       //Highlighting for recognized objects
                      ,const CUSTR   & sThisOwner          //Name of owner for this object
                      ,const CUSTR   & sThisObjName       //Name of this object
                      ,const eREFOBJ & ObjType
                      ,FILE *fpSQLSrc          //Input SQL source file
                      ,FILE *fpOut            //HTML output file
                      ,const THIS_DB & thisDB
                      ,const CUSTR   & sAbsOutPath);

// Generate WinHelp Header files
// Input Parameter: Programm options
// Output:          Header Files for Windows Help Workshop
//===============_______________________---------------------->>>
void WriteWinHelpHeaderFiles(const THIS_DB thisDB
                            ,CTABCOL & tbcTabs,CPRCCOL & prcPrcs
                            ,CPROP & prpDBProps
                            ,CUSTR sOutputPath  //Output path for HTML Files
                            );

// Name of file to link for DB objects that reference external objects
extern const char EXTERN_CALLS_FNAME[];

// Name of file to link for unreferenced DB objects
extern const char UNREFERENCED_OBJECTS[];

//link to nextlevel online pages in resource directory
extern const char HELP_IDX_FNAME[];

bool IncludeDocuFile(FILE *fpOut, const char *pcHTMLDocuPath
                    ,const char *pcOwner, const char *pcDBOBJName);

//Format a reference to a database object such that it is most useful
//We show the database name if access an external database
//We show the owner if it is not the owner of the database
//------------------------------------------------------------------>
CUSTR FormatUsefulDBObjRef(const UCHAR *pcServer,const UCHAR *pcObjDB,const UCHAR *pcOwner,
                           const UCHAR *pcName ,const THIS_DB thisDB);

//Attempt to resolve external links to other databases (indexing more than one DB)
CUSTR sResolveExtDBLink(const CPRCCOL & prcPrcs, const CTABCOL & tbcTabs
                       ,const THIS_DB & thisDB
                       ,const CUSTR   & sThisOwner          //Name of owner for this object
                       ,const CUSTR   & sThisObjName       //Name of this object
                       ,const eREFOBJ & ObjType
                       ,const CUSTR   & sAbsOutPath
                       ,const CUSTR   & sServer      //
                       ,const CUSTR   & sDB         //We look to resolve a dependency to this DB
                       ,const CUSTR   & sOwner
                       ,const CUSTR   & sDBObj
                       ,const CUSTR   & sInLink   //Return this, if link's not available
                       ,eREFOBJ       & eObjClass    //return view/table/proc or unknown
                      );

//Write custom javascript menu-bar to navigate inside a database and across to other DBs
void JS_PrintMenuConf(const THIS_DB & thisDB, const CUSTR & sOutDir, CPITEM *pitServDBs);

#endif

