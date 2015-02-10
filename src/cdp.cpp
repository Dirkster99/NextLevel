
#include "cdp.h"
#include "custr.h"
#include "cerr.h"

void CDP::CopyThis(const CDP & cdpIn)
{
  this->sServer   = cdpIn.sServer;
  this->sDB       = cdpIn.sDB;
  this->sOwner    = cdpIn.sOwner;
  this->sObjName  = cdpIn.sObjName;
  this->eObjType  = cdpIn.eObjType;
  this->iFlags    = cdpIn.iFlags;
}

//Standard constructor
CDP::CDP():eObjType(eREFUNKNOWN),iFlags(0)
{}

//Copy constructor
CDP::CDP(const CDP & cdpIn)
{
  CopyThis(cdpIn);
}

CDP & CDP::operator=(const CDP *pdpIn)
{
  if(pdpIn==NULL)      //Initialize to default if copy is not available
  {                    //This should never happen, but if it does, it
    sServer   = "";    //should avoid crashing the program and debugging the problem...
    sDB       = "";
    sOwner    = "";
    sObjName  = "";
    eObjType  = eREFUNKNOWN;
    iFlags    = 0;
  }
  else
    *this = *pdpIn;   //Use copy operator above

  return *this;
}

CDP & CDP::operator=(const CDP & dpIn)
{
  CopyThis(dpIn);

  return *this;
}


//We refer to same database object
//if all strings can be matched case-insensitively
//(except for those note matched intentionally)
//This is a useful function if components of the object
//reference are unknown and must be found otherwise...
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CDP::bMatchRef(const CDP & refIn, const eMATCH_WTO eWTOFilter ) const
{
  switch(eWTOFilter)
  {
  case eWTO_SERVER: //Attempt a Match without name of server
    if(this->sOwner.scmp_ci(refIn.sOwner)   == 0 &&
      this->sDB.scmp_ci(refIn.sDB)         == 0 &&
      //this->sServer.scmp_ci(refIn.sServer) == 0 &&
      this->sObjName.scmp_ci(refIn.sObjName)   == 0
      )
      return true;
    break;
  case eWTO_DB: //Attempt a Match without name of database
    if(this->sOwner.scmp_ci(refIn.sOwner)   == 0 &&
      //this->sDB.scmp_ci(refIn.sDB)         == 0 &&
      this->sServer.scmp_ci(refIn.sServer) == 0 &&
      this->sObjName.scmp_ci(refIn.sObjName)   == 0
      )
      return true;
    break;
  case eWTO_OWNER: //Attempt a Match without name of owner of database object
    if(//this->sOwner.scmp_ci(refIn.sOwner)   == 0 &&
      this->sDB.scmp_ci(refIn.sDB)         == 0 &&
      this->sServer.scmp_ci(refIn.sServer) == 0 &&
      this->sObjName.scmp_ci(refIn.sObjName)   == 0
      )
      return true;

  default:
  case eWTO_MT_ALL: //return match only, if all four components (server.db.owner.name) are matched
    if(this->sOwner.scmp_ci(refIn.sOwner)   == 0 &&
      this->sDB.scmp_ci(refIn.sDB)         == 0 &&
      this->sServer.scmp_ci(refIn.sServer) == 0 &&
      this->sObjName.scmp_ci(refIn.sObjName)   == 0
      )
      return true;
    break;
  }

  return false;
}


void CDP::Debug(FILE *fp) const //Debugging function for error tracing only
{
	fprintf(fp, "%s.%s.%s.%s, ", (const char *)this->sServer,(const char *)this->sDB,
                               (const char *)this->sOwner,(const char *)this->sObjName);

	fprintf(fp, "TYPE: ");
  switch(this->eObjType)
  {//Reference to unknown object
	case eREFUNKNOWN: fprintf(fp, "UNKNOWN");
		break;

  //Reference to a table or view, we don't know, yet (no definition seen, yet)
  case eREFTABVIEW: fprintf(fp, "TABVIEW");
		break;
  //Reference to a table
  case eREFTABLE: fprintf(fp, "TABLE");
		break;
  //Reference to a view
	case eREFVIEW: fprintf(fp, "VIEW");
		break;
  //Reference to a trigger
	case eREFTRIG: fprintf(fp, "TRIG");
		break;
  //Reference to a stored procedure
	case eREFPROC: fprintf(fp, "PROC");
		break;

  default:
    fprintf(fp, "!!! WARNING !!! UNKNOWN TYPE (%i)", this->eObjType);
	}

  fprintf(fp, " iFlags: %i\n", this->iFlags);
}

void CDP::DEBUG_Obj_Type(FILE *fpOut, const eREFOBJ clss)
{
  switch(clss)
  {    //Reference to a table or view, we don't know, yet (no definition seen, yet)
	case eREFTABVIEW: fprintf(fpOut, "eREFTABVIEW");
		break;          
  case eREFTABLE:   fprintf(fpOut, "eREFTABLE"); //Reference to a table
		break;          
	case eREFVIEW:    fprintf(fpOut, "eREFVIEW"); //Reference to a view
		break;

  case eREFPROC: fprintf(fpOut, "eREFPROC"); //Reference to a stored procedure
		break;
  case eREFTRIG: fprintf(fpOut, "eREFTRIG"); //Reference to a trigger
		break;

  case eREFUNKNOWN: //Reference to unknown object
  default:          fprintf(fpOut, "eREFUNKNOWN");
	}
}

//A reference to a database object can contain a user/owner definition
//and/or references to objects in other databases (string containing the 
//name of the other database)
//
//We look at these cases here and attempt to extract three items:
//
//Name of Server        - Name of database server we are looking at
//Name of Database      - Database where the accessed object resides on
//User/Role             - User being employed in this statement
//Database-Object Name  - Database object being accessed
//
//Default Values for name of server, database, and owner must be set by caller
//and will be over-written, if passed in raw reference (through sDBOBJ) contains
//this information (example for sDBOBJ: "ZEUS.TITAN.dbo.ADDRESS")
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CDP::ParseRef2DBOBJ(CUSTR & sServer, CUSTR & sDatabase, CUSTR & sUser, CUSTR & sDBOBJ, int & iFlag)
{
	int i=sDBOBJ.FindSubString("..");

  iFlag=0; //Reset all flag settings

	             //Parsing statement refering to database object in another DB
	if(i>0)     // <Database>'..'<DB-Object> 
	{          //Get name of database being accessed
		        //Get name of db object being accessed in other database
		CUSTR s(sDBOBJ);
		sDatabase = sDBOBJ.Mid(0, i-1);
		sDBOBJ    = sDBOBJ.Mid(i+1);

		sDatabase.Trim();
		sDBOBJ.Trim();

		if(sDatabase.slen()<=0 || sDBOBJ.slen()<=0)
		{
      printf("Warning: Error triming \'%s\' \'%s\' \'%s\'\n", (const char *)s,
         (const char *)sDatabase,(const char *)sDBOBJ);
      printf("CodeRef: CDEP::ParseRef2DBOBJ\n");
		}

    //We do not have information about a particular owner of this object
    //So, we assume the owner of the database to be the correct owner for now
    //Later, we can resolve this dependence to a different owner, if there is
    //no such object under the DB Owner but exactly one under another owner
    iFlag |= mFLAG_ASSUMED_OWNER;
	}
	else
	{
		CUSTR s(sDBOBJ);
		i=s.FindCharFromRight('.');        //Found access statement to this object
		if(i<1)
		{
			s.Trim();                        //Get last part of reference and return
			sDBOBJ = s;
      iFlag |= mFLAG_ASSUMED_OWNER;  //Assume this owner as owner of refered obj
			return;
		}
		sDBOBJ = s.Mid(i+1);
		sDBOBJ.Trim();

		s = s.Mid(0,i);
		i=s.FindCharFromRight('.');     //Found access statement with this user
		if(i<1)
		{
			s.Trim();
			sUser = s;
			return;
		}
		sUser = s.Mid(i+1);
		sUser.Trim();

		s = s.Mid(0,i);
		s.Trim();
		if(s.slen()<=0) return;
                            //What's left over is (could be a) reference to a database or
                           //<Server.database> => We attemt to resolve this, as well!
    i=s.FindCharFromRight('.');     //Found access statement with this database name
		if(i<1)
		{
			s.Trim();
			sDatabase = s;
			return;
		}

		sDatabase = s.Mid(i+1);
		sDatabase.Trim();

		s = s.Mid(0,i);
		s.Trim();
		if(s.slen()<=0) return; //Whats left over from here on is the server name
                            //So, we return it and quit this line of action
    sServer = s;         
		sServer.Trim();
	}
}

//This is just a sort of Assert function to make sure required params are used as required
//(We don't use assert() though)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CDP::Handle_Requi_Params(const CUSTR & sServ, const CUSTR & sDB,const CUSTR & sOwn,const CUSTR & sName
                             ,const CUSTR & LOC)
{
  CUSTR sErr;

  if(sServ.slen() <= 0) sErr = "SERVER FOR: ";

	if(sDB.slen() <= 0)   sErr = "DATABASE FOR: ";

	if(sOwn.slen() <= 0)  sErr = "OWNER FOR: ";

  if(sName.slen() <= 0) sErr = "DB_OBJNAME FOR: ";

  if(sErr.slen() > 0)
  {
    sErr += CDP::sGetRef(sServ,sDB,sOwn,sName);
    sErr += " IS EMPTY!!!";
    throw CERR(sErr, ERR_INTERNAL, LOC);
  }
}

CUSTR CDP::sGetRef(const CUSTR & sServer,const CUSTR & sDB
                  ,const CUSTR & sOwn   ,const CUSTR & sName)
{
  CUSTR sRet;

  sRet += sDB;
  sRet += ".";
  sRet += sServer;
  sRet += ".";
  sRet += sOwn;
  sRet += ".";
  sRet += sName;

  return sRet;
}
