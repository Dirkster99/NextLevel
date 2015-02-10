// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// NextLevel.cpp : Defines the main entry point for the application.
//

#include <stdio.h>
#include <string.h>

#include "cpitem.h"

#include "copts.h"
#include "cserver.h"
#include "defs.h"

#include<stdio.h>

//String case insenstiv comparison is not ANSI-C standard, is it?
#ifdef WIN32
  #define strcasecmp(x, y) stricmp(x, y)
#endif

int main(int argc, char* argv[])
{
  try
  {
	int	bNotSupported = false, bVers = false;
  CUSTR sIdxInput, sOutputPath, sSrvInput;
	COpts *pPrOps = new COpts(); //Object to handle program options at run time

	while(bNotSupported == false && argc > 1)
	{
		// Output html files into this directory
		if( argc >= 3 && strcasecmp(argv[1], "-DirOut") == 0 )
		{
      sOutputPath = argv[2];
			argc -= 2, argv += 2;
			continue;
		}

		// Print version information and exit, if nothing else is wanted
		if( argc >= 2 && strcasecmp(argv[1], "-v") == 0 )
		{
      printf("%s Relational Database Tool Version %s (Build %i)\n",
              TOOL_NAME, SOFT_VERSION, TOOL_BUILD);
			
			printf("Copyright 2004-2006 by Dirkster99\n");

			bVers = true;
			argc -= 1, argv += 1;
			continue;
		}

		// Accept this XML index file to parse inputs and create outputs
		if( argc >= 3 && strcasecmp(argv[1], "-srv") == 0 )
		{
      sSrvInput = argv[2];
			argc -= 2, argv += 2;
			continue;
		}

		// Accept this XML index file to parse inputs and create outputs
		if( argc >= 3 && strcasecmp(argv[1], "-idx") == 0 )
		{
      sIdxInput = argv[2];
			argc -= 2, argv += 2;
			continue;
		}

		// Generate Header Files for WinHelp Workshop (Free Tool from MS)
		if( argc >= 2 && strcasecmp(argv[1], "-WinHelp") == 0 )
		{
			pPrOps->bGenWinHelp = true;

			argc -= 1, argv += 1;
			continue;
		}

		bNotSupported = true;
	}

	//Check minimal parameter settings
  if((sOutputPath.slen() == 0 || sIdxInput.slen()==0) //-idx and -DirOut Option
     && sSrvInput.slen() == 0)                       //-src Option
	{
		bNotSupported = true;
		if(bVers == false)
      fprintf(stderr, "ERROR: REQUIRED -idx or -DirOut option not set.\n");
	}

	if(bNotSupported == true)
	{
		fprintf(stderr, "\nNextLevel is a Callgraph/Dependency Tool for T-SQL Trigger, Procedures and DB Tables/Views\n");
		fprintf(stderr, "-idx    %s -idx <XML index file pointing to DDL Files for one database>\n", TOOL_NAME);
		fprintf(stderr, "        e.g.:   %s -idx idx.xml\n\n",TOOL_NAME);

		fprintf(stderr, "-DirOut %s -DirOut <Target path for HTML output files>\n", TOOL_NAME);
		fprintf(stderr, "        e.g.: %s -DirOut C:\\TMP\\\n\n",TOOL_NAME);

		fprintf(stderr, "-srv    %s -srv <XML file pointing to multible XML files for DDL of Databases>\n", TOOL_NAME);
		fprintf(stderr, "        e.g.:   %s -srv srv.xml\n\n",TOOL_NAME);

		fprintf(stderr, "\nNote: Either (-idx and -DirOut) or -srv are required parameters\n");
		
		fprintf(stderr, "\n-v       To print version info\n");
		fprintf(stderr, "-WinHelp To generate header files for the MS HTML Help Workshop\n");
		fprintf(stderr, "(visit www.dipedia.org/nl for extended documentation)\n");
		return(-1);
  }

  //Relational Database Server object to store various sorts of DB information within
  //(CSERVER and CDB are, technically, not to be confused with real Server and DB instances!)
  CSERVER srvDBServ;

  //Import one XML index describing DDL information one relational database
  if(sOutputPath.slen()>0 && sIdxInput.slen()>0)
  {
    printf("\n>>> %02i Processing DB Pointer into XML DB\n",1);
    printf("        Item: %s\n",(const char *)sIdxInput);
    printf("    Location: %s\n",(const char *)sOutputPath);

    int iTrigs=0, iProcs=0;

    if(srvDBServ.ParseInput(*pPrOps, sOutputPath, sIdxInput,iTrigs, iProcs) == 0)
      srvDBServ.WriteAllDBOutput(*pPrOps);
  }

  //Import XML index pointing to multible XML index files
  //each describing DDL information for one relational database
  if(sSrvInput.slen() > 0)
  {
    srvDBServ.ProcessXMLSrvFile(*pPrOps, sSrvInput);

    //1> Resolve all outbound dependencies for each DB stored in the server object
    //2> Add inbound dependencies to each object stored in each DB on the server object
    srvDBServ.ResolveDBDeps();
    srvDBServ.WriteAllDBOutput(*pPrOps);
  }

  delete pPrOps; //Free program options (input & output objects)
  }
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
    printf("CodeRef: ::nextlevel\n");
		return -2;
	}
	catch(...) //Progress Error and throw to caller
	{
    printf("UNKNOWN ERROR occured in ::NextLevel.");
  }

  return 0;
}
