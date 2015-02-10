
#ifndef CPATRN_328593764857345
#define CPATRN_328593764857345

#include "defs.h"
#include "custr.h"
#include "cerr.h"

typedef enum
{
	 hlUnknown    = 0
	,hlTableIns   = 1 //Insert Modif on a TABLE
	,hlTableUpd   = 2 //Update Modif on a TABLE
	,hlTableDel   = 3 //DELETE Modif on a TABLE
	,hlTruncTable = 4
	,hlTableName  = 5 //Table Name in SQL Database

	,hlProcedure     = 6  //Name of an SQL procedure (part of an exec statement)
	,hlComment       = 7  //Highlight a comment
	,hlString        = 8  //Highlight a string
	,hlSQLKeyword    = 9  //Keywords such as exec, delete, update, from  and so forth
	,hlTableUpdStats = 10 //Highlighting a uodate statistics keyword

	,hlDECL_CUName = 11  //Name of cursor
	,hlDECLARE     = 12  //Beginning of cursor declare
  ,hlDECLARE_TYP = 13  //Type of variable declaration
	,hlUsageCU     = 14  //Open cursor command
	
	,hlTableFrom   = 15  //FROM Clause used as part of select, delete, update...
} hlTYPE;

//This collection stores highlighting information sorted by file offsets
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class CPATRN
{
public:
	CPATRN();
	~CPATRN();

	CPATRN(const CPATRN & ptrIn);
	CPATRN & operator=(const CPATRN *pPtr);
	CPATRN & operator=(const CPATRN & ptrnIn);

	void FreeMe();

	//Add another color highlighting style to list of styles
	//------------------------------------------------------>
	void AddPatrn2(const int iOffBegin, const int iOffEnd,const hlTYPE hlPatrnType,const CUSTR & sRawRef=""
                ,const CUSTR & sServer="",const CUSTR & sDB="",const CUSTR & sOwner="",const CUSTR & sDBObj=""
                ,const int iFlags=0);

	//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	//create a highlighting pattern node with optimization on last
	//node added, if we add for example two comment highlightings in
	//a row then these can be merged into one highlighting...
	//So, we extend the last highlighting pattern if it did not change,
	//attach new highlighting otherwise
	//=================================================================>
	void AddExtendedPatrn2(const int iOffBegin,const int iOffEnd,const hlTYPE hlPatrnType,const CUSTR & sRawRef=""
	                      ,const CUSTR & sServer="",const CUSTR & sDB="",const CUSTR & sOwner="",const CUSTR & sDBObj=""
                        ,const int iFlags=0);

	//Simple List Iterator function....Start
	//-----------------------------------=====>
	void SetFirst();

  //Simple List Iterator function....Continue
  //<=====-----------------------------------
  bool GetNext(int & iOffBegin, int & iOffEnd, hlTYPE & hlPatrnType
              ,CUSTR & sServer,CUSTR & sDB,CUSTR & sOwner,CUSTR & sDBObj
              ,CUSTR & sRawRef, int & iFlags);

	//Simple List Search function for previous nodes of actual node
	//-----------------------------------==========================>
	bool GetFindPrev(const hlTYPE hlPatrnType, CUSTR & sName);

	//Simple List Search function for next nodes of actual node
	//-----------------------------------==========================>
	bool GetFindNext(const hlTYPE hlPatrnType, CUSTR & sName);

	void   operator+=(const CPATRN & ptrnIn);
	CPATRN operator+(const CPATRN & ptrnIn) const;

  //Sometimes failure is part of the algorithm, and this is fine, as long as,
  //failure correction is also part of the algorithm
  //So, here we call this to set a correct db_object_owner, if it was assumed wrongly
  //In some exotic cases the owner needs to be changed once all objects have been parsed
  //This is done to re-align aambigous reference to db objects belonging to a different owner
  //than the refering object
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  void ResetRef2AmbigousOwner(const UCHAR *pcServer, const UCHAR *pcDB
                             ,const UCHAR *pcOwner,const UCHAR *pcName
                             ,const UCHAR *pcNewOwner
                             ,const int iMask);       //Should be something like mFLAG_ASSUMED_OWNER
private:
	typedef struct patrn //Patrn structure is a simple linked list
	{
		int iOffBegin;        //Begin of pattern for color style highlighting
		int iOffEnd;         //End of pattern for color style highlighting
		hlTYPE hlPatrnType; //Type of highlighting pattern

		CUSTR sServer; //If hlPatrnType referes to a dbobj then here is the name of the Database Server
		CUSTR sDB;     //If hlPatrnType referes to a dbobj then here is the name of the Database
    CUSTR sOwner; //owner,name of that object highlighted (proc or table)
    CUSTR sName;

    CUSTR sRawRef; //This is the reference as seen in SQL source file
    int iFlags;

		struct patrn *pNext;
		struct patrn *pPrev;

		patrn():iOffBegin(0),iOffEnd(0),hlPatrnType(hlUnknown),iFlags(0),pNext(NULL),pPrev(NULL)
		{}

	} PATRN;

	PATRN *phlFirst, *phlLast; //highlighting pattern root
	PATRN *phlNext;           //This is use for iteration over highlightings

	//Copy List of patterns and offsets from one list into another
	//-------------------------------=============================>
	void CopyList(const PATRN *phlSrc);

	//Create a new highlighting pattern node...
	//------------------------------------------->
	PATRN *hlCreateNode2(const int iOffBegin, const int iOffEnd
                      ,const hlTYPE hlPatrnType, const CUSTR & sServer, const CUSTR & sDB
                      ,const CUSTR & sOwner, const CUSTR & sDBOBj, const CUSTR & spcRawRef
                      ,const int iFlags, PATRN *pPrev=NULL);
};

/*****************************************************************/
/****************************************************************/
/***************************************************************/

#endif
