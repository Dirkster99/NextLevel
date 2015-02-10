
#ifndef CTAB_328593764857345
#define CTAB_328593764857345

#include "custr.h"
#include "cscol.h"
#include "cerr.h"
#include "defs.h"

#include "cprop.h"
#include "ctag.h"
#include "cpatrn.h"
#include "cdep.h"

class CTAB : private CDP
{
public:
	long iDataSz, iIndexSz;

  //Create a table object and return initialized contents
  CTAB( const UCHAR *pcServer,
        const UCHAR *pcDB,      //Name of DB object lives in
        const UCHAR *pcOwner,  //Name of owner of this object
        const UCHAR *pcName,
        const UCHAR *pcUpd,
        const UCHAR *pcIns,
        const UCHAR *pcDel,
        const eREFOBJ etbObjType);

	~CTAB();

  CTAB(const CTAB & tbIn);            //Copy CTor and assignment
	CTAB & operator=(const CTAB *pTab);
	CTAB & operator=(const CTAB & tabObj);

	const int GetID() const { return iKeyID;} 

	void AddColumn(const CPROP & cpInput);

	// Add Trigger plus event information into this table
	//------------------------------------------------------->
	void AddTrigEvents(const UCHAR *pcTrigName, const EVENT_CL evOnEvents);

	//Print Table information into HTML
	void HTML_PrintTable(FILE *fpOut, const CUSTR & sOutPutDir
                      ,const CUSTR & sUpdTrigSrc,const CUSTR & sDelTrigSrc,const CUSTR & sInsTrigSrc
                      ,const THIS_DB & thisDB);

	//Print SQL Table definition with columns into HTML
	void HTML_PrintTableDefinition(FILE *fpOut);

  bool bIsExternal()
	{
		if(iColumnsActSz > 0) return false;
		else
			return true;
	}

	const UCHAR *DelTrig(){ return pcDelTrig; }
	const UCHAR *InsTrig(){ return pcInsTrig; }
	const UCHAR *UpdTrig(){ return pcUpdTrig; }

  //PUBLIC PROPERTIES:
	CUSTR sSource;          //Link to source file, if supplied by caller
	CPROP m_prpTableProps; //Additional Properties for this table
	CPATRN hlPatrn;       //Root structure of pattern highlighting information (phlRoot)
	CDEPS m_dpDBObjDeps; //The table/view depends on these objects
	CSCOL m_csClasses;    //These are the classes in which this table/view object participates

	CUSTR GetServer()    const { return this->sServer;  }
	CUSTR GetDB()        const { return this->sDB;      }
	CUSTR GetOwner()     const { return this->sOwner;   }
	CUSTR GetName()      const { return this->sObjName; }
  eREFOBJ GetObjType() const { return eObjType; }

  void SetObjType(eREFOBJ eType)       {this->eObjType = eType;    }
  void SetOwner(const CUSTR & sOwnerIn){this->sOwner   = sOwnerIn; }

  //Get name of HTML target File associated with this table
	CUSTR GetTargFileName() const;

  void Debug(FILE *fp=stdout);

private:
	int iKeyID;  //Each table or view gets a unique number at creation time
	             //This number is used to link tables and operations on tables

	CUSTR sTargFileName; //Target HTML filename of the object in storage

	//All Three or just two triggers can point at the same string
	//This means both triggers are implemented in one procedure
	//e.g.: pcDelTrig == pcInsTrig == "T_InsertUpdate_IU"
	UCHAR *pcDelTrig; // Name of Delete Trigger for this table
	UCHAR *pcInsTrig; // Name of Insert Trigger for this table
	UCHAR *pcUpdTrig; // Name of Update Trigger for this table
  CTAB(); //Default constructor should never be used publicly

	typedef struct cols //This is where we store infos for each column of a table
	{
		CPROP clColumn;   //A column has certain properties, such as name, type, len etc...

		cols() //Standard Ctor for init2default
		{}
	}
	COLS;

	COLS **ppColumns;
	int iColumnsSz, iColumnsActSz; //Size & Actual used size of Columns Arrays

  void HTML_PrintTabTrigEvent(const UCHAR *pcName, const UCHAR *pcFName,
	                            const char *pcEvents, FILE *fpOut);

  void CopyThis(const CTAB & tbcIn); //Helper for Copy Constructor/=operator

  //Without telling there is no knowing, and so we init either a table
	//or a view, we just use eTUNKNOWN until there is a specific telling ...
	void Init(const UCHAR *pcServer=NULL,  //Name of Database Server object lives on
            const UCHAR *pcDB=NULL,      //Name of Database object lives on
            const UCHAR *pcOwner=NULL,  //Name of owner of this object
            const UCHAR *pcNameIn=NULL,
            const UCHAR *pcUpd=NULL,
            const UCHAR *pcIns=NULL,
            const UCHAR *pcDel=NULL,
            const eREFOBJ etbObjType=eREFTABVIEW);

  void FreeThis();
};

#endif
