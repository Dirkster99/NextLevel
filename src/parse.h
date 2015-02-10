

#ifndef PARSE_2634328593764857345__234234
#define PARSE_2634328593764857345__234234

#include "cproc.h"
#include "cscol.h"
#include "cpitem.h"
#include "copts.h"
#include "ctabcol.h"

#include <stdio.h>

int paSkipUntil(FILE *fpInput, UCHAR *pc);

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//Find a statement such as 'create procedure x'
//or create 'create trigger x' and return x
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
CPROC *ParseObjDef(const UCHAR *pcDB, const UCHAR *pcOwner,
                   UCHAR *pcBuf, int iBufSz, CSHCOL & hKeyWords, FILE *fpInput);

//Parse contents of Trigger/Stored Procedure and return in-memory structure
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CPROC *ParsePROCFile(const UCHAR *pcServer,const UCHAR *pcDB
                    ,const UCHAR *pcOwner,const UCHAR *pcName
                    ,CUSTR sSrcPath, const UCHAR *pcSrcFName,COpts *pPrOpts, CTABCOL & tbcTabs);

//-_--_--_--_--_--_--_--_--_--_--_--_--_->
//
//Parse SQL View definition file and return data ...
//__-=__-=__-=__-=__-=__-=__-=__-=__-=__-=__-=__-=__-=>
void ParseViewFile(const CUSTR sFPath, const CUSTR sFName, CTAB & tblView, COpts *pPrOpts,
                   CTABCOL & tbcTabs, const UCHAR *pcServer, const UCHAR *pcDB);

int Convert2KB(CUSTR sSz, CUSTR sPart);
CUSTR ConvertKB2UsefulSz(int iKBSz);

#endif

