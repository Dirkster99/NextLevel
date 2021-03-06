
//Database implementation class
//We collect all information relating to various database objects in this class
#ifndef DB_328593764857345__234234
#define DB_328593764857345__234234

#include "cstrm.h" //xml parser classes
#include "cstak.h"
#include "cpars.h"

#include "cprop.h"
#include "cproc.h"
#include "cerr.h"

#include "copts.h"
#include "ctabcol.h"
#include "cproccol.h"

class CDB
{
public:
	CDB();          //Class Constructors and Destructors
	~CDB();

	CDB(const CDB & dbIn);
	CDB & operator=(const CDB *pDB);
	CDB & operator=(const CDB & dbObj);

	//Function for parsing multible files by reading filenames
	//from an index file which is passed in PrOpts
	int ParseFiles(COpts *pPrOpts, int & iCntProcs, int & iCntTrigs);

	//Print callgraph and other information into linked HTML files
	int PrintHTML(COpts *pPrOpts, int & iCntProcs, int & iCntTrigs, int & iCntTabs);

	//Database properties are found in the XML index file (or not because these are optional)
	CPROP prpDBProps;

	void DEBUG_Tables(FILE *fp=stdout){ tbcTabs.DEBUG(fp); }
	void DEBUG_Procs(FILE *fp=stdout){  prcPrcs.DEBUG(fp); }

	//Read and write the output directory name
	//for output info generated by this program
	void OutPutDir(const char *pcDir){ sOutPutDir = pcDir; }
	CUSTR GetOutPutDir(){ return sOutPutDir; }

  //Read additional files from the indexfile where this string points to
	void IdxFile(const char *pcDir)
  {
    sIdxFile = pcDir;
    sInpFile = s::sGetPath(sIdxFile);
  }
	CUSTR IdxFile(){ return sIdxFile; }

  // Name of owner, database and server where db lives
  CUSTR 	GetOwnerName(){ return sOwner;}
  CUSTR 	GetDBName(){ return sDB;}
  CUSTR 	GetServerName(){ return sServer;}

  //Get Dependencies to objects that are external to this DB
  //========================================================
  CDEPS *FindExtDBObjects(CPITEM *pitDBs);

  //Resolve inbound/outbound dependencies across Database limits
  UINT ResolveForeignDBDeps(CDEPS * pdpDB);
  //Resolve inbound/outbound dependencies across Database limits (Step 2)
  UINT SetForeignDBDeps(CDEPS * pdpDB);

  //Find all external objects that depend on a given object and return their access codes
  UINT FindAllOutboundDeps(CDEPS & dpRet
                          ,const CUSTR & sServer,const CUSTR & sDB
                          ,const CUSTR & sOwner, const CUSTR & sName);

  //========================================================================================>
  void AddOutboundDeps(CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB
                      ,const UCHAR *pcOwner,const UCHAR *pcName, const eREFOBJ & eObjType);

  //Set database properties (like name,owner etc) so caller can work with this
  void SetDBProps(THIS_DB & thisDB);
private:
  void CopyThis(const CDB & dbIn); //Helper for Copy Constructor/=operator

  CTABCOL tbcTabs;  //Collection of Table/View Objects
	CPRCCOL prcPrcs; //Collection of procedure/trigger objects

	//Functions to support parsing the index.xml file for database definitions
	void InitIndexStack(CSTAK & stkTags);
	CTAG *SearchXMLFile(CPARS & xpPState, CSTAK & stkTags, bool & bIntro);
	int ManageXMLState(int iThisSt, CTAG *prtNext);

	//URL and content for href to link to root page of NextLevel pages
	//in the XML import this is the <backlink .../> tag inside <htmlroot>
	CUSTR sRootLinkURL, sRootLink;

	CUSTR sOwner, sDB, sServer; // Name of database and server where db lives

  CUSTR sOutPutDir    //Directory for output files
	                         //Database objects can be documented in a seperate folder using
                          //files that are named like the database objects
                         //"<owner>_<object_name>.html"... here we store the
                        //full path to these html-files
       ,sHTMLDocuPath  //Directory of files to be included for documentation
       ,sInpFile      //Input Path for SQL Files
       ,sIdxFile;    //Path and Name of input index.xml (and sql input files)

  CPROPCOL TabClassDefs   //Class definitions used to pop-up Icon and descriptions for each class
          ,PrcClassDefs; //when outlining things later in HTML
};

#endif
