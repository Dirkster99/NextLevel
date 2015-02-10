
//Database Server implementation class
//We collect all information relating to various database objects in this class
#ifndef CSERVER_328593764857345__234234
#define CSERVER_328593764857345__234234

#include "cdb.h"       //xml relational database classes
#include "cpitem.h"   //cpitem class
#include "custr.h"   //string class

//Relational Database Server object to store various sorts of DB information within
//The class name suggests that we need more than one c++ cserver class instance
//to model more than one server: not so! -> a server class simply encapsulates
//each CDB instance. CDB instances themself refer already to an actual DB Server
//CSERVER is, thus, just a (memory, comunications) managing component between CDB instances
//
//CSERVER and CDB are, technically, not to be confused with real Server and DB instances!
//
//1> Because a C++ CSERVER instance does not have a name,
//2> Each CDB instance of one CSERVER instance can belong to a different server
//   (verify CDB.sDB, CDB.sServer properties)
//
//(the above points are usually inpossible on a real DB Server)
class CSERVER
{
public:
	CSERVER();          //Class Constructors and Destructors
	~CSERVER();

	CSERVER(const CSERVER & dbIn)
	{
		throw CERR("Copy constructor for CSERVER NOT implemented, yet!!!\n");
	}

	CSERVER & operator=(const CSERVER *pDB)
	{
		throw CERR("Equal operator for CSERVER NOT implemented, yet!!!\n");
	}

	CSERVER & operator=(const CSERVER & dbObj)
	{
		throw CERR("Equal operator for CSERVER NOT implemented, yet!!!\n");
	}

  //Parse SQL source codes and analyse for one relational database
  int ParseInput(COpts & prgOptions, const CUSTR & sOutputPath, const CUSTR & sIdxInput
                ,int & iTrigsOut, int & iProcsOut);

  int WriteAllDBOutput(COpts & prgOptions);

  //=============================================================>
  // Import XML index pointing to multible XML index files
  // Each describing DDL information for one relational database
  // Return Value: Negative -> Indicates Error/Warning
  //               Zero     -> Indicates Success
  //=============================================================>
  int ProcessXMLSrvFile(COpts & prgOptions, const CUSTR & sXMLSRVInput);

  void ResolveDBDeps(); //Resolve externl deps between databases

  //Find a database by its name of server and database
  bool hFind(const UCHAR *pcServer, const UCHAR *pcDB) const;
  CDB & hGetEntry(const UCHAR *pcServer, const UCHAR *pcDB);
private:
  //======================================================================>
	//A collection of database sitting physcally on a server
  //Each database in this collection can reside on a different server
  //We hash the database-object name and keep objects in on different servers
  //in each chained list, this way we keep track of same database names on a
  //nuber of (different) servers
	typedef struct h_dbr
	{
		//Name of Server and database
		CUSTR sServName, sDB; //Name of Database  is used as hash-key
                         //Servername is a secondary key in linked list
		CDB *pDBR;

		struct h_dbr *pNext; //Link to next item

		h_dbr():pDBR(NULL),pNext(NULL) {}
		h_dbr(const UCHAR *pcServName,const UCHAR *pcDB,const CDB *pDBRIn)
		 :pDBR(NULL),pNext(NULL)
		{
			pDBR       = new CDB(*pDBRIn);
			sServName  = pcServName;
			sDB        = pcDB;
		}
		~h_dbr()
		{
			delete pDBR;
			//pNext(NULL) -> doing this will destroy whole list so we do this from outside
		}
	} H_DBR;

	int iDbrRootSz;     //Size of root array
	H_DBR **pphDBRRt;  //Root of table collection

	//Private Private Private Hashing functions
	unsigned int hGetKey(const UCHAR *word) const;
	void hFreeColl();
	bool hAddEntry(const UCHAR *pcServName, const UCHAR *pcDB, const CDB *pDB);

  //=========================== END OF HASHING COLLECTION FUNCTIONs -------------->
  //XML Parser State engine
  void InitIndexStack(CSTAK & stkTags);
  int ManageXMLState(int iThisSt, CTAG *prtNext);
  CTAG *SearchXMLFile(CPARS & xpPState, CSTAK & stkTags, bool & bIntro);

  //Find names of all databases stored in the server object
  CPITEM *hGetUniqueDBs() const;

  //Find all external objects that depend on a given object and return their access codes
  UINT FindAllOutboundDeps(CDEPS & dpRet,const CUSTR & sServer,const CUSTR & sDB
                                        ,const CUSTR & sOwner, const CUSTR & sName) const;

  void AddOutboundDeps(CDEPS & dpDepIn,const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner
                                      ,const UCHAR *pcName, const eREFOBJ  eObjType);
};

#endif
