
#include "cproc.h"

// Name of file to reference for external db objects
extern const char EXTERN_CALLS_FNAME[];

//<<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>>
//Create unique number for this instance
//<<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>>
void CPROC::CreateID()
{
	static int ProcID = 0;
	
	ID = ProcID++;
}

//<<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>>
//Special constructor for name & type of proc/trig
//<<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>>
CPROC::CPROC(const CUSTR & sServerIn, const CUSTR & sDBIn
            ,const CUSTR & sOwnerIn, const CUSTR & sName, const eREFOBJ & eClass)
:CDP(),evOnEvents(evUNKNOWN)
{
  this->sServer   = sServerIn;
  this->sDB       = sDBIn;
  this->sOwner    = sOwnerIn;
  this->sObjName  = sName;
  this->eObjType  = eClass;

  this->sTargFileName = this->sObjName.LowerCase() + ".html";
  CreateID();
}

//<<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>>
//Standard DTor
//<<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>>
CPROC::~CPROC()
{
}

//<<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>>
//Standard CTor (is private, and thus, not used)
//<<<>><<<>>>><<<>><<<>>>><<<>><<<>>>><<<>><<<>>>>
CPROC::CPROC():CDP(),evOnEvents(evUNKNOWN)
{
	CreateID();
}

void CPROC::CopyThis(const CPROC & prcIn)
{
  CDP::CopyThis(prcIn);
  this->ID            = prcIn.ID;
  this->sTargFileName = prcIn.sTargFileName;
  this->sOnTable      = prcIn.sOnTable;
  this->sSrcFile      = prcIn.sSrcFile;
  this->evOnEvents    = prcIn.evOnEvents;
  this->m_csClasses   = prcIn.m_csClasses;

  this->hlPatrn       = prcIn.hlPatrn;
  this->m_dpDBObjDeps = prcIn.m_dpDBObjDeps;
  this->m_prpProps    = prcIn.m_prpProps;
}

CPROC::CPROC(const CPROC & prcIn):CDP()
{
  CopyThis(prcIn);
}

CPROC & CPROC::operator=(const CPROC & prcIn)
{
  CopyThis(prcIn);

	return *this;
}

CPROC & CPROC::operator=(const CPROC *pPtr)
{
	if(pPtr != NULL) *this = *pPtr; //Copy data

	return *this;
}

void CPROC::operator+=(const CPROC & prcIn)
{
	//sServer,sDB,sOwner,sName Key Info should not be written over !!!!
  //1> Overwrite owner only if there is not one already present
  //2> Try to find something better than unknown,
	//   otherwise stick with type of proc
	if(sOwner.slen()==0) sOwner = prcIn.sOwner;

	if(eObjType == eREFUNKNOWN) eObjType = prcIn.eObjType;

	if(sOnTable.slen()==0) sOnTable   = prcIn.sOnTable;

	if(sSrcFile.slen()==0) sSrcFile   = prcIn.sSrcFile;

	evOnEvents    |= prcIn.evOnEvents;

  this->m_csClasses.AddCopyFrom(&prcIn.m_csClasses);

	hlPatrn       += prcIn.hlPatrn;
	m_dpDBObjDeps += prcIn.m_dpDBObjDeps;
}

CPROC CPROC::operator+(const CPROC & prcIn) const
{
	CPROC ret(*this); //Create new object to return to caller

	ret += prcIn;

	return ret;
}

//Debug Database Object Information
//
void CPROC::DebugPROC(FILE *fpOut) const
{
  fprintf(fpOut, " DB_OBJ: %s.%s.%s.%s ID %i (%i Calls)\n"
        ,(const UCHAR *)sServer,(const UCHAR *)sDB,(const UCHAR *)sOwner,(const UCHAR *)sObjName
        ,ID, m_dpDBObjDeps.CountAccTypes(eAT_EXEC));

	fprintf(fpOut, "DB_OBJ_CLASS: ");
	DEBUG_Obj_Type(fpOut, this->eObjType);
	fprintf(fpOut, "\n");

	if(this->eObjType == eREFTRIG)
	{
		bool bPrint=false;

		fprintf(fpOut, "    ON_TABLE: %s\n", (const UCHAR *)sOnTable);
		fprintf(fpOut, "      EVENTS: ");

		if FLAG(evOnEvents, evUPDATE)      //Pretty-Print Events for this Trigger
		{
			fprintf(fpOut, "update"); bPrint=true;
		}

		if FLAG(evOnEvents, evINSERT)
		{
			if(bPrint== true) fprintf(fpOut, ", ");

			fprintf(fpOut, "insert"); bPrint=true;
		}

		if FLAG(evOnEvents, evDELETE)
		{
			if(bPrint== true) fprintf(fpOut, ", ");

			fprintf(fpOut, "delete");
		}

		fprintf(fpOut, "\n");
		m_dpDBObjDeps.Debug(fpOut);
	}
}

CUSTR CPROC::GetTargFileName() const
{
	if(this->eObjType == eREFUNKNOWN)
	{
		CUSTR s(EXTERN_CALLS_FNAME);

		s += "#p";
		s += ID;
		return s;                         //Reference to externals page plus id anker
	}

	return sTargFileName;
}

//*****************************************************************
//****************************************************************
//**************************************************************
