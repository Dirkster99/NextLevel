
//
//Database object for procedural code such as procedures & triggers
//
#ifndef CPROC_328593764857345
#define CPROC_328593764857345

#include "defs.h"
#include "custr.h"
#include "cscol.h"
#include "cpatrn.h"
#include "cdep.h"
#include "cprop.h"

#include <stdio.h>

//*****************************************************************
//****************************************************************
//***************************************************************
//
// This class manages code dependencies for a trigger or procedure
// (here, both are refered to as procs)
// We collect information to answer the following issues:
//
// 1> What proc calls another proc?
// 2> What Table is being modified by which proc
// 3> Which proc is calling external database procs?
//
class CPROC : private CDP
{
public:
  void CopyThis(const CPROC & prcIn);
  CPROC(const CUSTR & sServerIn, const CUSTR & sDBIn
       ,const CUSTR & sOwnerIn, const CUSTR & sName, const eREFOBJ & eClass);
	~CPROC();

  CPROC(const CPROC & prcIn);            //CTor and equal operator
	CPROC & operator=(const CPROC *pPtr);
	CPROC & operator=(const CPROC & prcObj);

	void  operator+=(const CPROC & prcIn);
	CPROC operator+(const CPROC & prcIn) const;

  int getID() const {return ID;} //Get unique identifier

  void DebugPROC(FILE *fpOut) const;

//PUBLIC PROPERTIES:
	CPROP m_prpProps;   // Additional properties of this procedure

	CUSTR sOnTable;      // Table Name for Trigger Objects...
	CUSTR sSrcFile;      // SQL Source File that was parsed to find dependencies etc...
	
  //Events that trigger a table trigger (delete, insert, update) stored in a bit-mask
  int   evOnEvents;

	CPATRN hlPatrn; //Root structure of pattern highlighting information (phlRoot)

	//The trigger/procedure depends on these table/view/trigger/proc objects
	CDEPS m_dpDBObjDeps;

	CUSTR   GetServer()  const { return sServer;  }
	CUSTR   GetDB()      const { return sDB;      }
	CUSTR   GetOwner()   const { return sOwner;   }
  CUSTR   GetName()    const { return sObjName; }
  eREFOBJ GetObjType() const { return eObjType; }
	CSCOL m_csClasses;    //These are the classes in which this table/view object participates

  void SetObjType(const eREFOBJ & eType){this->eObjType = eType; }
	void SetName(const UCHAR *pcName)
	{
		sObjName      = pcName;
		sTargFileName = sObjName.LowerCase() + ".html";
	}

	void SetServer(const UCHAR *pcServer){ sServer = pcServer;}
	void SetDB(const UCHAR *pcDB)        { sDB = pcDB;        }
	void SetOwner(const UCHAR *pcOwner)  { sOwner = pcOwner;  }

  CUSTR GetTargFileName() const; //Get HTML file name for this SQL Trigger/stored procedure

private:
	CPROC();              //Standard constructor should neveer be used here
	CUSTR sTargFileName; //This is the target HTML filename of the object in storage
// depricated DBOBJ_CLASS edbClass;    // Type of DB Object Trigger, Proc etc...
// depricated CUSTR sName;        // Name of Database object
// depricated CUSTR sDB;        // Name of Database where object resides in
// depricated CUSTR sOwner;    // Name of owner of database object
	int ID;         //Unique identifier for this procedure/trigger

	void CreateID();
};

//*****************************************************************
//****************************************************************
//**************************************************************

#endif
