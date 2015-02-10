
#include "parse.h"
#include "custr.h"
#include "cpitem.h"
#include "ctab.h"

#include <stdlib.h>

int paSkipUntil(FILE *fpInput, UCHAR *pc)
{
	int iState = 0;
	UCHAR c=0;

	while(iState <= 0 && !feof(fpInput))
	{
		if(pc[0] != c)
		{
			while((c = getc(fpInput)) != pc[0] && !feof(fpInput) )
				;
		}

		if(feof(fpInput)) return false;

		while(!feof(fpInput))
		{
			if(pc[iState+1] == 0 ) return true;

			c = getc(fpInput);

			if(pc[iState+1] == c )
				iState++;
			else
			{
				iState = 0;
				break;      /**go back to outer loop**/
			}
		}
	}
	
	if(pc[iState+1] == 0 && c == pc[iState])
		return true; /** Match FOUND: Stop Skipping **/

	return false;
}

//
//Parse an SQL Keyword and skip multible comments until one non-space
//keyword accumulates in the input buffer pcBuf
//==================================================================>
int GetKeyWord(UCHAR *pcBuf, int iBufSz, FILE *fpIn, CPATRN & hlPatrn)
{
RestartSrch:
  int	iRdSz=0;
	bool bEndKeyword;
	UCHAR c;

	// Read until EOF or non-space character
	while((c = getc(fpIn)) <= ' ' && !feof(fpIn))
		;

	if(feof(fpIn)) return -1;

	pcBuf[iRdSz++] = c;
	pcBuf[iRdSz] = 0;

	bEndKeyword = false;
	while(bEndKeyword == false && !feof(fpIn) && iRdSz < iBufSz)
	{
		c = getc(fpIn);
		
		if(feof(fpIn)) return -1;

		if(c > ' ' && s::schr((UCHAR *)",;-=+|*/{[()]}", c ) == NULL)
		{
			pcBuf[iRdSz++] = c;
			pcBuf[iRdSz] = 0;
		}
		else
		{
			if(iRdSz > 0)
			{
				if((pcBuf[iRdSz-1] == '-' && c == '-') //Found a comment?
         ||(pcBuf[iRdSz-1] == '/' && c == '*'))
        {
          long lOffBgn = ftell(fpIn)-2;

          //one line comment: '-- This is a comment'
				  if(pcBuf[iRdSz-1] == '-' && c == '-')
				  {
					  pcBuf[--iRdSz] = 0;
					  paSkipUntil(fpIn, (UCHAR *)"\n");
				  }

				  // found an ANSI C-Style comment?
				  if(pcBuf[iRdSz-1] == '/' && c == '*')
				  {
					  pcBuf[--iRdSz] = 0;
					  paSkipUntil(fpIn, (UCHAR *)"*/");
				  }

          hlPatrn.AddPatrn2(lOffBgn, ftell(fpIn), hlComment);

          if(iRdSz == 0)
            goto RestartSrch; //Buffer still empty -> Continue search
        }

        bEndKeyword = true;
			}
		}
	} // End of Keyword Search

	return iRdSz;
}

//A reference to (an external) database object can be build from
//a number of strings delimited be '.' characters SQL allows white
//spaces between these items and this functions attempts to find
//all items belonging to one reference (despite the silly definition)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CompleteDBObjRef(UCHAR *pcBuf, FILE *fpIn, const int iBufSz)
{
	if(feof(fpIn)) return;
	UCHAR c;

	int iRdSz = s::slen(pcBuf);

	c=' ';

	//This makes sure that the parser starts correctly, we need this,
	//because the parser hunts from '.' to '.', so missing a '.' is means
	//missing a DBObj_Ref ;(
	if(iRdSz > 0)                        
	if(pcBuf[iRdSz-1] == '.') c = '.';

	long lOffBack = ftell(fpIn); //Skip back here if reference was complete
	int iRdSzBack = s::slen(pcBuf);
	fseek(fpIn, lOffBack-1, SEEK_SET);

	for(int i=0 ;!feof(fpIn) && i < 4; i++)
	{
		while(c <= ' ' && !feof(fpIn) && iBufSz > iRdSz)
			pcBuf[iRdSz++] = c =fgetc(fpIn);
	
		if(c != '.') //reference is complete up to here -> skip back and exit
		{
			fseek(fpIn, lOffBack, SEEK_SET);
			if(i>0)
				pcBuf[iRdSzBack-1]='\0';
			else
				pcBuf[iRdSzBack]='\0';
			return;
		}
	
		c=' ';                      //Skip whites and get additional part of reference
		while(c <= ' ' && !feof(fpIn) && iBufSz > iRdSz)
			pcBuf[iRdSz++]=c=fgetc(fpIn);
	
		if(c == '.') //Found a '..' statement so we need another attempt to accumm
		{            //the complete reference before the end of this loop
			c=' ';                 	//Skip whites and get additional part of reference
			while(c <= ' ' && !feof(fpIn) && iBufSz > iRdSz)
				pcBuf[iRdSz++]=c=fgetc(fpIn);
		}

		//Complete this part of DB reference
		while(c > ' ' && !feof(fpIn) && iBufSz > iRdSz
          && s::schr((UCHAR *)")}],;//-",c) == NULL)
		{ //Don't add terminating character
			pcBuf[iRdSz++] = c = fgetc(fpIn);
		}
		pcBuf[iRdSz]='\0';
		lOffBack = ftell(fpIn); //Skip back here if reference was complete
		iRdSzBack = iRdSz;
	}

	//Rewind stream by one character so we don't miss it
	fseek(fpIn, ftell(fpIn)-1, SEEK_SET);
}

/* Get Trigger Definition

create trigger [owner.]trigger_name
on ([owner.]table_name | DATABASE .. Table_name)
{for {insert , update , delete}
as SQL_statements
**********************************************/
bool GetTriggerDef(CUSTR sServerIn,CUSTR sDBIn,CUSTR sUserIn, UCHAR *pcBuf, int iBufSz,
                   FILE *fpIn, CPROC *prc)
{
	int iSz, iRefFlags=0;
	long lFileOff;

	if((iSz=GetKeyWord(pcBuf, iBufSz, fpIn, prc->hlPatrn))<=0) return false;

	if(s::scmp_ci_uc(pcBuf, "on") != 0) return false;

	lFileOff = ftell(fpIn)- 3;
	prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+2, hlSQLKeyword);

	//We expect to find the name of the table right here
	if((iSz=GetKeyWord(pcBuf, iBufSz, fpIn, prc->hlPatrn))<=0) return false;

	CompleteDBObjRef(pcBuf, fpIn, iBufSz);
	CUSTR sDBObj(pcBuf), sServer(sServerIn), sDB(sDBIn), sUser(sUserIn); //resolve access to db object
  CDP::ParseRef2DBOBJ(sServer, sDB, sUser, sDBObj, iRefFlags);

  prc->sOnTable = sDBObj;
  iSz = s::slen(pcBuf);
  lFileOff = ftell(fpIn) - (iSz + 1);
  prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+iSz, hlTableName,pcBuf
                        ,sServer, sDB, sUser, sDBObj, iRefFlags);

	if((iSz=GetKeyWord(pcBuf, iBufSz, fpIn, prc->hlPatrn))<=0) return false;

	if(s::scmp_ci_uc(pcBuf, "for") != 0) return false;

	lFileOff = ftell(fpIn)- 4;
	prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+3, hlSQLKeyword);

	while(!feof(fpIn))
	{
		if((iSz=GetKeyWord(pcBuf, iBufSz, fpIn, prc->hlPatrn))<=0) return false;
		else
		{
			if(s::scmp_ci_uc(pcBuf, "as")==0)  //End of Event definition
			{
				lFileOff = ftell(fpIn)- 3;
				prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+2, hlSQLKeyword);
				return true;
			}

			if(s::scmp_ci_uc(pcBuf, "delete")==0)
			{
				SETFLAG(prc->evOnEvents, evDELETE);
				lFileOff = ftell(fpIn)- 7;
				prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+6, hlSQLKeyword);
			}
			else
			{
				if(s::scmp_ci_uc(pcBuf, "insert")==0)
				{
					SETFLAG(prc->evOnEvents, evINSERT);
					lFileOff = ftell(fpIn)- 7;
					prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+6, hlSQLKeyword);
				}
				else
				{
					if(s::scmp_ci_uc(pcBuf, "update")==0)
					{
						SETFLAG(prc->evOnEvents, evUPDATE);
						lFileOff = ftell(fpIn)- 7;
						prc->hlPatrn.AddPatrn2(lFileOff , lFileOff+6, hlSQLKeyword);
					}
					else
					{
						prc->evOnEvents = evUNKNOWN; //Unknown event -> return error
						return false;
					}
				}
			}
		}
	}

	return false;
}

//------------------------------------------------------------------>
//Coder parser <X><Y><Z><X><Z><Y><Z><X><Y><X><Y><Z><X><Z><Y><Z><X><Y>
//------------------------------------------------------------------>

#define OFFSET_SZ 10

#define  STRING_ITEM 1 //Last item parsed was a string
#define COMMENT_ITEM 2 //Last item parsed was a comment

static void SkipString(const UCHAR c, FILE *fpInput, CPATRN & hlPatrn, int iLastString)
{
	UCHAR cTmp[2];
	long iOffBegin, iOffEnd;

	cTmp[0] = c, cTmp[1] = '\0';

	iOffBegin = ftell(fpInput)-1;  //Note start of string in source file
	paSkipUntil(fpInput, cTmp);   //Skip string
	iOffEnd   = ftell(fpInput);  //Note end of string

	if(iLastString == STRING_ITEM)
		hlPatrn.AddExtendedPatrn2(iOffBegin, iOffEnd,hlString);
	else
		hlPatrn.AddPatrn2(iOffBegin, iOffEnd,hlString);
}

static int SkipWhiteSpaces(UCHAR &c, FILE *fpIn, int & iLastItem, int & iLastString
           	       ,patrTYPE & patrMastre, int & iCreateState
                   ,CPITEM & patr             //Return recognized items 2 caller
                   ,CPATRN & hlPatrn
                   )
{
	while((c = getc(fpIn)) <= ' ' && !feof(fpIn)) //Read until EOF or non-space char
		;

	if(patrMastre == patrExec)      //Exec statements must be able to parse an equal char
	{
		if(patr.GetSz()>0 && c == '(')    //Disable parsing dynamic SQL here and now
		if(s::scmp_ci(patr.GetStrITEM(patr.GetSz()-1),(const UCHAR *)"exec")==0)
		{
			const long lFileOff = patr.GetOffset(patr.GetSz()-1);
			hlPatrn.AddPatrn2(lFileOff , lFileOff+4, hlSQLKeyword); //Highligh SQL_Key
			patr.Reset();
			iCreateState = 0;
			patrMastre   = patrUnknown;      //Reset back to unknown state
			return 1;                       //continue in file stream
		}


		if(s::schr((UCHAR *)",;({[<>]})+|", c ) != NULL)
		{
			iLastItem = iLastString = 0;  //Found non-whitespace after last comment or string
			                             //get next character from input stream
			return 1;                   //continue in file stream
		}
	}
	else
	{                         //Updates can also be update functions >update_func<
		if(patr.GetSz()>0 && c == '(')
		if(s::scmp_ci(patr.GetStrITEM(patr.GetSz()-1),(const UCHAR *)"update")==0)
		{
			const long lFileOff = patr.GetOffset(patr.GetSz()-1);
			hlPatrn.AddPatrn2(lFileOff , lFileOff+6, hlSQLKeyword); //Highligh SQL_Key
			patr.Reset();
			iCreateState = 0;
			patrMastre   = patrUnknown;      //Reset back to unknown state
			return 1;                       //continue in file stream
		}

		if(s::schr((UCHAR *)",;({[<>]})+=|", c ) != NULL)
		{
			iLastItem = iLastString = 0;  //Found non-whitespace after last comment or string
			                             //get next character from input stream
			return 1;                   //continue in file stream
		}
	}

	if(feof(fpIn)) //Ran out of input in the middle of something???
	{
		patr.Reset();
		return(-1);
	}

	if(c == '\'' || c == '\"')   // found an SQL Style "string" or 'string'?
	{
		SkipString(c, fpIn, hlPatrn, iLastString);

		iLastItem = 0;  //Found non-whitespace after last comment statement
		iLastString = STRING_ITEM;
		return 1;                  //continue in file stream
	}
	return 0;
}

static void SkipAndMarkComment(UCHAR *pcUntil, int *piLastItem, FILE *fpInput, CPATRN & hlRoot)
{
	int iOffBegin, iOffEnd;

	iOffBegin = ftell(fpInput)-2;   //Note start of comment in source file
	paSkipUntil(fpInput, pcUntil);  //Skip comment
	iOffEnd   = ftell(fpInput);     //Note end of comment

	if(piLastItem == NULL)
		hlRoot.AddPatrn2(iOffBegin, iOffEnd,hlComment);
	else
	{
		if(*piLastItem == COMMENT_ITEM)
			hlRoot.AddExtendedPatrn2(iOffBegin, iOffEnd,hlComment);
		else
			hlRoot.AddPatrn2(iOffBegin, iOffEnd,hlComment);

		*piLastItem = COMMENT_ITEM;
	}
}


UCHAR pcLimit[] = ",;-+=|*/{[()]}";
UCHAR pcEndCm[] = "*/";

static int SkipWhiteSpace2(UCHAR &c, FILE *fpIn,
                           int & iLastItem, int & iLastString,
                           CPATRN & hlPatrn  //Highlighting pattern for this file
                          )
{
	iLastItem=iLastString=0;

	UCHAR cLast;

	while(!feof(fpIn))
	{
		cLast = c;
		c     = getc(fpIn);
		
		if(feof(fpIn)) return -1; //Ran out of input in the middle of something???

		// found a comment such as '-- This is a comment'
		if((cLast == '-' && c == '-') || (cLast == '/' && c == '*'))
		{
			if(cLast == '-' && c == '-')
				SkipAndMarkComment((UCHAR *)"\n", &iLastItem, fpIn, hlPatrn);

			// found an ANSI C-Style comment such as '// This is a comment'
			if(cLast == '/' && c == '*')
				SkipAndMarkComment(pcEndCm, &iLastItem, fpIn, hlPatrn);
		}
		else //We could throw an exception here if c == "-" && cLast != "-" && cLast > " "
		{
			if(c > ' ' && c != '-' && c != '/') return 1;
		}
	} // End of inner while loop --> Keyword Searcher

	return -1;
}

int FindKeyword(bool & bEndKeyword, FILE *fpInput, UCHAR *pcBuf, const int iBufSz
               ,CPITEM & patr   //Return collection of recognized items 2 caller
               ,CPATRN & hlPatrn //Highlighting Pattern object
               ,int   & iLastItem
               ,UCHAR & c)         //First character of word to be red
{
	int iRdSz	= 0;

	// We have a non-space -> Fill Buffer until nxt white space or buffer overflow
	iRdSz=0;
	pcBuf[iRdSz++] = c;
	pcBuf[iRdSz] = 0;
	
	bEndKeyword = false;
	while(bEndKeyword == false && !feof(fpInput) && iRdSz < iBufSz)
	{
		c = getc(fpInput);
		
		if(feof(fpInput)) //Ran out of input in the middle of something???
		{
			patr.Reset();
			return -1;
		}

		if(c > ' ' && s::schr(pcLimit, c ) == NULL)
		{
			pcBuf[iRdSz++] = c;
			pcBuf[iRdSz] = 0;
		}
		else
		{
			if(iRdSz > 0)
			{
				// found a comment such as '-- This is a comment'
				if(pcBuf[iRdSz-1] == '-' && c == '-')
				{
					pcBuf[--iRdSz] = 0;
					SkipAndMarkComment((UCHAR *)"\n", &iLastItem, fpInput, hlPatrn);
					break;
				}

				// found an ANSI C-Style comment such as '// This is a comment'
				if(pcBuf[iRdSz-1] == '/' && c == '*')
				{
					pcBuf[--iRdSz] = 0;
					SkipAndMarkComment(pcEndCm, &iLastItem, fpInput, hlPatrn);
					break;
				}

				bEndKeyword = true;  //Found somethin' to look at (cont. further below)
			}
		}
	} // End of inner while loop --> Keyword Searcher
	
	return iRdSz;
}

static bool FindNextToken(FILE *fpIn, long & lOffset,     //Input Stream from file
                         UCHAR & c, UCHAR *pcBuf, int iBufSz,
                         CPATRN & hlPatrn,  //Highlighting pattern for this file
                         int & iRdSz)
{
	if(feof(fpIn)) return false;
	UCHAR cLast;

	iRdSz = 0;
	pcBuf[iRdSz] = '\0';
	c     = ' ';

	while(!feof(fpIn) && iRdSz < iBufSz)
	{
		cLast = c;
		c     = getc(fpIn);

		//We skip comments but do highlight them
		if((cLast == '-' && c == '-') || (cLast == '/' && c == '*'))
		{
			if(cLast == '-' && c == '-') // something as '-- This is a comment'?
				SkipAndMarkComment((UCHAR *)"\n", NULL, fpIn, hlPatrn);
			else
			if(cLast == '/' && c == '*') //ANSI C-Style comment?
				SkipAndMarkComment(pcEndCm, NULL, fpIn, hlPatrn);

			cLast = c = ' ';
			continue;                  //Restart while loop
		}

		if(iRdSz > 0)
		{
			if(c <= ' ' || s::schr((UCHAR *)",;=+|*{[()]}", c ) != NULL)
				goto Exit; //white space after content found
		}
		else
			if(c <= ' ') continue; //Skip leading white spaces

		if(c == '/' || c == '-') continue; //Skip comments

		if(iRdSz == 0) lOffset = ftell(fpIn)-1;

		pcBuf[iRdSz++] = c; //Buffer content and keep going
		pcBuf[iRdSz] = 0;
	}

Exit:
	if(s::schr((UCHAR *)",;=+|*{[()]}", pcBuf[0]) != NULL && pcBuf[1]=='\0')
	{
		c = pcBuf[0];                       //reset c and rewind stream by one character
		fseek(fpIn, ftell(fpIn), SEEK_SET); //so we don't miss it
		pcBuf[0] = '\0';
	}
	
	return true;
}

#define IGNORE_AND_LOAD_NEXT 0
#define END_OF_LIST_FOUND    1
#define NOT_END_OF_LIST      2

//See if we have a end of the FROM list or not
//by looking into a table of reserved SQL keywords
int bEndOfFromList(COpts *pPrOpts, UCHAR *pcBuf, long lOffset, CPATRN & hlPatrn)
{
	if(s::scmp_ci_uc(pcBuf, "holdlock"  ) == 0 || 
	   s::scmp_ci_uc(pcBuf, "noholdlock") == 0 ||
	   s::scmp_ci_uc(pcBuf, "shared"    ) == 0
	  )
	{
		long lOffEnd = lOffset + s::slen(pcBuf);
		hlPatrn.AddPatrn2(lOffset , lOffEnd, hlSQLKeyword); //Mark SQL keyword
		pcBuf[0] = '\0';
		return IGNORE_AND_LOAD_NEXT;
	}

	if(pPrOpts->hKeyWords.Get_i(pcBuf)==true)  //if they are in the table
		return END_OF_LIST_FOUND;

	return NOT_END_OF_LIST;
}

//>>> left_table [inner | left [outer] | right [outer]]
//>>> join right_table
//>>> on left_column_name = right_column_name
//>>> This is the syntax tree we are attempting to parse

//INNER JOIN <TABLE_NAME>
//LEFT JOIN  <TABLE_NAME>
//RIGHT JOIN <TABLE_NAME>
//LEFT OUTER JOIN <TABLE_NAME>
//RIGHT OUTER JOIN <TABLE_NAME>
//These SQL keywords are parsed and recognized in an SQL 92 FROM extend
//> without terminating the FROM state, other SQL keywords will terminate it !!!
#define SQL92_FROMKEY_SZ 43
const UCHAR cSQL92_FROMKEY[SQL92_FROMKEY_SZ][11] =
{
 "BETWEEN","BIT","COALESCE","INNER","JOIN","LEFT","LIKE","NOT","ON","OR","OUTER"
,"REAL","DECIMAL","NUMERIC","DATETIME","DATE","TIMESTAMP","TIME","YEAR","CHAR"
,"INT","VARCHAR","TINYBLOB","TINYTEXT","BLOB","TEXT","MEDIUMBLOB"
,"MEDIUMTEXT","LONGBLOB","LONGTEXT","ENUM",	"inner", "outer", "on", "and"
,"or", "between", "null", "not","exists", "IS","substring","convert"
};

typedef struct sql92state
{
	int iPrevState, iCurrState;
	CUSTR sPrevKeyword, sCurrKeyword;
	int iLevel;

} SQL92STATE;

#define s92ParseTreeSZ 17
SQL92STATE s92ParsTree[s92ParseTreeSZ] =
{
// At the beginning, iPrevState=0 and sPrevKeyword=""
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
{0,1,     "", "INNER", 0}, //First keyword thar could encounered are these
{0,2,     "", "LEFT" , 0},
{0,3,     "", "RIGHT", 0},

{1,4, "INNER", "JOIN", 1},
{4,5, "JOIN",      "", 2},  //Parser 'terminates' with table name -> Reset State

{2,5, "LEFT", "JOIN",  1},
{2,6, "LEFT", "OUTER", 1},

{5,7, "JOIN", "", 2},       //Parser 'terminates' with table name -> Reset State
{6,8, "OUTER", "JOIN",  2},
{8,9, "JOIN", "",  3},      //Parser 'terminates' with table name -> Reset State

{3,10, "RIGHT", "JOIN", 1},
{3,11, "RIGHT", "OUTER", 1},

{10,12, "JOIN", "", 2},     //Parser 'terminates' with table name -> Reset State
{11,13, "OUTER", "JOIN", 2},

{13,14, "JOIN", "", 3},     //Parser 'terminates' with table name -> Reset State
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
{ 0,15,     "", "JOIN", 0},
{15,16,     "JOIN", "", 1},
};

#define TABLE_NAME 0
#define NEXT_STATE 1
#define UNRECOGNIZ 2
// Find next state node and messsage whether we fond a table name or not
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FindNextSQL92State(int & idxCurrState, const UCHAR *pcBuf,
                       SQL92STATE *ps92ParsTree, int iTreeSZ)
{
	if(idxCurrState != -1)
	{
		for(int j=0; j<iTreeSZ; j++)      //Set state engine to next state node
		{
			if(s92ParsTree[j].iLevel     == (ps92ParsTree[idxCurrState].iLevel+1))
			if(s92ParsTree[j].iPrevState == ps92ParsTree[idxCurrState].iCurrState)
			if(s::scmp_ci(pcBuf, s92ParsTree[j].sCurrKeyword) == 0)
			{
				//printf("DEBUG ANY_ITEM BUF %s %s %s %i",pcBuf, (const char *)s92ParsTree[j].sPrevKeyword, (const char *)s92ParsTree[j].sCurrKeyword,s92ParsTree[j].sCurrKeyword.slen());
				idxCurrState = j;
				return NEXT_STATE;
			}
		}

		for(int j=0; j<iTreeSZ; j++)      //Set state engine to next state node
		{
			if(s92ParsTree[j].iLevel == (ps92ParsTree[idxCurrState].iLevel+1))
			if(s92ParsTree[j].iPrevState == ps92ParsTree[idxCurrState].iCurrState)
			if(s::scmp_ci(ps92ParsTree[idxCurrState].sCurrKeyword, s92ParsTree[j].sPrevKeyword) == 0)
			if(s92ParsTree[j].sCurrKeyword.slen() == 0)
			{
				//printf("DEBUG TABLE_NAME BUF %s %s %s %i",pcBuf, (const char *)s92ParsTree[j].sPrevKeyword, (const char *)s92ParsTree[j].sCurrKeyword,s92ParsTree[j].sCurrKeyword.slen());
				idxCurrState = -1;
				return TABLE_NAME;
			}
		}
	}
	else
	{
		for(int j=0; j<s92ParseTreeSZ; j++)   //Set state engine to first state node
		{
			if(s92ParsTree[j].iLevel == 0)
			if(s::scmp_ci(pcBuf, s92ParsTree[j].sCurrKeyword)==0)
			{
				idxCurrState = j;
				return NEXT_STATE;
			}
		}
	}

	return UNRECOGNIZ;
}


#define _SRC "parse:ParseSQL92FromExtend"
//This function consumes an SQL_92 FROM extend, first table name is already
//present in 'CPITEM patr' item collection and one of the keywords:
//'INNER', 'LEFT', or 'RIGHT' is present in pcBuf (fresh from input stream)
//=============================================================================>
static int ParseSQL92FromExtend(
	FILE *fpIn, UCHAR & c, UCHAR *pcBuf, int iBufSz,      //Input Stream from file
	long lOffset,
	CPITEM & patr,                             //Parse tree of recognized items
	CPATRN & hlPatrn,                        //Highlighting pattern for this file
	COpts *pPrOpts,                         //Programm options, keywords etc...
	const CUSTR sRefServer,const CUSTR sRefDB, const CUSTR sRefOwner,
  int iCnt,
	const CUSTR sRefDBObj)
{
	int iRdSz=0,i,j, idxCurrState=-1, iState=-1,iRefFlags=0;
	bool bFromValidSQL92, bCont;
	CUSTR sOwner,sThisServ, sThisDB, sDBObj;

	iState=FindNextSQL92State(idxCurrState, pcBuf, s92ParsTree, s92ParseTreeSZ);

	if(idxCurrState == -1)
	{
		printf("%s WARNING: Unrecognized Keyword %s in %s found!", _SRC, pcBuf, (const UCHAR *)sRefDBObj);
		return iCnt;
	}

	hlPatrn.AddPatrn2(lOffset, lOffset+s::slen(pcBuf), hlSQLKeyword);

	//printf(">>> DEBUG %s %s idx %i iState %i\n", (const UCHAR *)sRefDBObj, pcBuf, idxCurrState,iState);

	//Continue parsing until end of FROM extend or end of file
	for(i=0, bCont = true;!feof(fpIn) && bCont == true; i++, pcBuf[0]='\0')
	{
		if(FindNextToken(fpIn, lOffset, c, pcBuf, iBufSz, hlPatrn, iRdSz) == false)
			return iCnt;

		if(feof(fpIn)) break; //Exit for loop on end of file

		if(s::slen(pcBuf) <= 0) continue; //We still have file left but no buffer???

		//printf("%2i DEBUG \'%s\' \'%c\' idx %i iState %i\n", i, pcBuf, c,idxCurrState, iState);

		iState=FindNextSQL92State(idxCurrState, pcBuf, s92ParsTree, s92ParseTreeSZ);

		switch(iState)
		{
			case TABLE_NAME:
				CompleteDBObjRef(pcBuf, fpIn, iBufSz);
				CDP::ParseRef2DBOBJ((sThisServ=sRefServer),(sThisDB=sRefDB),(sOwner=sRefOwner),(sDBObj=pcBuf), iRefFlags);
				patr.AddString(sThisServ,sThisDB,sOwner,sDBObj,pcBuf,lOffset, iRefFlags);
				iCnt++;
				idxCurrState = -1;
			break;
			case NEXT_STATE:
				hlPatrn.AddPatrn2(lOffset, lOffset+s::slen(pcBuf), hlSQLKeyword);
			break;

			case UNRECOGNIZ:
				//Find and highlight a sql keyword that can form a part of the from extend
				for(j=0, bFromValidSQL92=false;
				    j<SQL92_FROMKEY_SZ && bFromValidSQL92==false; j++)
				{
					if(s::scmp_ci(pcBuf, cSQL92_FROMKEY[j])==0)
					{
						bFromValidSQL92=true;
						hlPatrn.AddPatrn2(lOffset, lOffset+s::slen(pcBuf), hlSQLKeyword);
					}
				}
		
				if(bFromValidSQL92==false)
				{
					if(bEndOfFromList(pPrOpts, pcBuf, lOffset, hlPatrn) == END_OF_LIST_FOUND)
					{
						c=' ';
						pcBuf[0]='\0';
						fseek(fpIn, lOffset, SEEK_SET); //Reset back to last keyword on strm
						bCont = false;
					}
				}

			break;
		}
	}
	//printf("<<< DEBUG %s\n\n", (const UCHAR *)sRefDBObj);

	return iCnt;
}
#undef _SRC
#undef JOIN1_SQL_92_SZ

#undef TABLE_NAME
#undef NEXT_STATE
#undef UNRECOGNIZ

#define CONTINUE_SQL_92_SZ 4
const UCHAR cCONT_SQL_92[CONTINUE_SQL_92_SZ][6] =
{
	"inner", "left", "right", "join"
};

#define _SRC "parse:ParseFromExtend"
//Parse the from extent in a seperate function
//This function consumes a T-SQL FROM Extent completely, unitl a keyword(or EOF)
//is encountered that does not belong to the from syntax (e.g. 'where')
//
//The function ParseSQL92FromExtend is called,if a valid SQL 92 keyword, such as
//'INNER', 'LEFT', or 'RIGHT' is encountered
//=============================================================================>
static int ParseFromExtend(FILE *fpIn,      //Input Stream from file
                            UCHAR & c,
                            UCHAR *pcBuf, int iBufSz,
                            long lOffset,
                            CPITEM & patr,        //Parse tree of recognized items
                             CPATRN & hlPatrn,  //Highlighting pattern for this file
                             COpts *pPrOpts,   //Programm options, keywords etc...
                             const CUSTR sRefServer,const CUSTR sRefDB,const CUSTR sRefOwner,
                             const CUSTR sRefDBObj) //For Debugging parser error
{
	int iCnt=0,iLastItem=0,iLastString=0,iRefFlags=0;
	CUSTR sDBObj;

	if(s::slen(pcBuf)<=0) //We need a table name right here
	{
		patr.Reset(); //Restart state engine, if none is here
		return iCnt;
	}

	long lOffBack = ftell(fpIn);

	try //pcBuf should contain the 'TABLE' from which we select
	{	 //The following while loop will/should consume the rest of the from clause
	CUSTR sThisServ(sRefServer),sThisDB(sRefDB),sOwner(sRefOwner);

	CompleteDBObjRef(pcBuf, fpIn, iBufSz);
	CDP::ParseRef2DBOBJ(sThisServ,sThisDB,sOwner,(sDBObj=pcBuf), iRefFlags);
	patr.AddString(sThisServ,sThisDB,sOwner,sDBObj,pcBuf,lOffset, iRefFlags);

	patr.epType = patrFROM;                   //Set type of recognized parser item
	iCnt++;

	int iRdSz=0, iRet;


	while(!feof(fpIn))
	{
		if(c == '(')     //Process index references unseen
		{
			while(c != ')' && !feof(fpIn)) c = getc(fpIn);
	
			SkipWhiteSpace2(c,fpIn,iLastItem,iLastString,hlPatrn);
		}
		//Skip rest of table reference (index enforcements, tab shortcut etc.)
		if(c != ',')
		{
			do
			{
				if(FindNextToken(fpIn, lOffset, c, pcBuf, iBufSz, hlPatrn, iRdSz) == false)
					return iCnt;
		
				if((iRet=bEndOfFromList(pPrOpts, pcBuf, lOffset, hlPatrn)) == true)
				{
					for(int i=0;i<CONTINUE_SQL_92_SZ;i++)
					{
						if( s::scmp_ci(cCONT_SQL_92[i], pcBuf) == 0)
						{
						if((iCnt = ParseSQL92FromExtend(fpIn,c,pcBuf,iBufSz,lOffset,patr,hlPatrn,
						                                pPrOpts,sRefServer,sRefDB, sRefOwner, iCnt, sRefDBObj)) < 0)
							goto SetBackExit;

						return iCnt;
						}
					}
					goto SetBackExit;
				}
			}
			while(iRet == IGNORE_AND_LOAD_NEXT);
	
			if(c == '(')     //Process index references unseen
			{
				while(c != ')' && !feof(fpIn)) c = getc(fpIn);
		
				SkipWhiteSpace2(c,fpIn,iLastItem,iLastString,hlPatrn);
			}
		}

		if(c <= ' ') SkipWhiteSpace2(c,fpIn,iLastItem,iLastString,hlPatrn);

		if(c == '(')     //Process index references unseen
		{
			while(c != ')' && !feof(fpIn)) c = getc(fpIn);
	
			SkipWhiteSpace2(c,fpIn,iLastItem,iLastString,hlPatrn);
		}

		//Have we got another table name to follow???
		if(c != ',' && s::scmp(pcBuf,(UCHAR *)",")!=0)
		{
			for(int i=0;i<CONTINUE_SQL_92_SZ;i++) //Attempt to match SQL 92 Keyword
			{
				if( s::scmp_ci(cCONT_SQL_92[i], pcBuf) == 0)
				{
				if((iCnt = ParseSQL92FromExtend(fpIn,c,pcBuf,iBufSz,lOffset,patr,hlPatrn,
				                    pPrOpts,sRefServer,sRefDB,sRefOwner, iCnt, sRefDBObj)) < 0)
					goto SetBackExit;

				return iCnt;
				}
			} //FOR LOOP will exit function if match was available...

			if(s::schr((UCHAR *)"jilrJILR",c)!=NULL) //Attempt to match SQL 92 Keyword 2nd
			{
				lOffset = ftell(fpIn)-1;   //Set back one character, get keyword & match
				fseek(fpIn,lOffset,SEEK_SET);

			  if(FindNextToken(fpIn,lOffset, c, pcBuf, iBufSz, hlPatrn, iRdSz) == false)
					return iCnt;
			}

			for(int i=0;i<CONTINUE_SQL_92_SZ;i++) //Attempt to match SQL 92 Keyword
			{
				if( s::scmp_ci(cCONT_SQL_92[i], pcBuf) == 0)
				{
				if((iCnt = ParseSQL92FromExtend(fpIn,c,pcBuf,iBufSz,lOffset,patr,hlPatrn,
				                    pPrOpts,sRefServer,sRefDB,sRefOwner, iCnt, sRefDBObj)) < 0)
					goto SetBackExit;

				return iCnt;
				}
			} //FOR LOOP will exit function if match was available...
			goto SetBackExit;
		}
	
		do
		{	if(FindNextToken(fpIn,lOffset, c, pcBuf, iBufSz, hlPatrn, iRdSz) == false)
				return iCnt;

			if((iRet = bEndOfFromList(pPrOpts, pcBuf, lOffset, hlPatrn)) == true)
			{
				for(int i=0;i<CONTINUE_SQL_92_SZ;i++)
				{
					if( s::scmp_ci(cCONT_SQL_92[i], pcBuf) == 0)
					{
					if((iCnt = ParseSQL92FromExtend(fpIn,c,pcBuf,iBufSz,lOffset,patr,hlPatrn,
					                        pPrOpts,sRefServer,sRefDB,sRefOwner,iCnt,sRefDBObj)) < 0)
						goto SetBackExit;

					return iCnt;
					}
				}
				goto SetBackExit;
			}
		}
		while(iRet == IGNORE_AND_LOAD_NEXT);

		CompleteDBObjRef(pcBuf, fpIn, iBufSz);
		CDP::ParseRef2DBOBJ((sThisServ=sRefServer),(sThisDB=sRefDB),(sOwner=sRefOwner),(sDBObj=pcBuf),iRefFlags);
		patr.AddString(sThisServ,sThisDB,sOwner,sDBObj,pcBuf, lOffset, iRefFlags);
    iCnt++;
	}

	return iCnt;
	}
	catch(CERR e)
	{
		CUSTR s(e.getsErrSrc());
		printf("CodeRef: %s in %s\n", (const UCHAR *)s, _SRC);
		fseek(fpIn, lOffBack, SEEK_SET);
		return -1;
	}
	catch(...) //Progress Error and throw to caller
	{
		printf("An unknown error occurred in: %s\n", _SRC);
		fseek(fpIn, lOffBack, SEEK_SET);
		return -1;
	}

SetBackExit:                //Set us back to where we were after last table name
	fseek(fpIn, lOffBack, SEEK_SET);
	return iCnt;
}
#undef CONTINUE_SQL_92
#undef _SRC

//----------------------------------------------------------------------->
//Add highlighting pattern and dependency information for SQL statements
//such as: update, insert, delete ... etc
//----------------------------------------------------------------------->
#define _SRC "Parse::AddPattern"
static void AddPattern(CPITEM & patr,       //Parse tree of recognized items
                        CPROC & prcRet,    //Highlighting pattern for this file
                     CTABCOL  & tbcTabs)  //Name of database proc object resides in
{
	long lOffBeg, lOffEnd;
	int idx,iFlags;
	const UCHAR *pcTabProcName=NULL;

	eREFOBJ dpoPrc = prcRet.GetObjType();

  if(dpoPrc != eREFTRIG && dpoPrc != eREFPROC)
		throw CERR("Type of SQL PROC must be Trigger or Procedure!!!",0,_SRC);

  CDP::Handle_Requi_Params(prcRet.GetServer(),prcRet.GetDB(),prcRet.GetOwner(),prcRet.GetName(), _SRC);

	//Dep-obj used further below to tell a table/view you depend on this proc/trig
	CDEP dpDBOBj("", "", "", "", dpoPrc,eAT_UNKNOWN, 0,""); //resolve access to db obj
	CUSTR sDBObj, sServer, sDB, sUser;
	//This is just a shortcut to make refering to this PROC easy as a pie :)
  CUSTR sThisDBObj(prcRet.GetName()), sThisServ(prcRet.GetServer())
       ,sThisDB(prcRet.GetDB()), sThisUser(prcRet.GetOwner());

	try
	{
	switch(patr.epType)
	{
	case patrFROM:
		if(patr.GetSz() <= 0) break;                   //Have we got a FROM keyword?

		lOffBeg = patr.GetOffset(0);                  //Mark recognized FROM keyword
		lOffEnd = patr.GetOffset(0) + s::slen(patr.GetStrITEM(0));
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlSQLKeyword);

		for(idx=1; idx<patr.GetSz(); idx++)
		{
			pcTabProcName = patr.GetStrITEM(idx, sServer, sDB, sUser, sDBObj,iFlags);

			if(s::slen(pcTabProcName) >= 0)
			if(pcTabProcName[0] != '#')
			{
				lOffBeg = patr.GetOffset(idx);                 //Highlight name of table
				lOffEnd = lOffBeg + s::slen(pcTabProcName);
	
				if(prcRet.GetObjType() == eREFTRIG) //Redirect select statements on inserted/deleted
				{
					if(s::scmp_ci_uc(pcTabProcName, "inserted") == 0 ||
					   s::scmp_ci_uc(pcTabProcName, "deleted" ) == 0)
					{
						sDBObj = prcRet.sOnTable;
					}
				}
				prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableFrom,pcTabProcName
				                                   ,sServer,sDB, sUser, sDBObj,iFlags);
	
				//Store Fact: Procedure implements SELECT on a table in TabCOL
        dpDBOBj.SetName(sThisServ,sThisDB,sThisUser,sThisDBObj,dpoPrc);
				dpDBOBj.eacAccess  = eAT_SELECT;
        dpDBOBj.iFlags     = iFlags;              //Set additional flags for this reference
				tbcTabs.AddTableDepend(sServer,sDB,sUser,sDBObj,eREFUNKNOWN,&dpDBOBj);

				//2nd Store fact in PROC-COL, refer to table or view, we don't know, yet
        dpDBOBj.SetName(sServer,sDB,sUser,sDBObj,eREFTABVIEW);
				prcRet.m_dpDBObjDeps.AddDep(&dpDBOBj);
			}
		} //End of FOR Loop
	break;

	case patrDEAL_CU:
		lOffBeg = patr.GetOffset(0);                    //Mark recognized keywords
		lOffEnd = patr.GetOffset(1) + s::slen(patr.GetStrITEM(1));

		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlSQLKeyword);

		pcTabProcName = patr.GetStrITEM(2);             //Map into UsageCU highlighting
		lOffBeg = patr.GetOffset(2);                   //Highlight datatype of decl
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlUsageCU, pcTabProcName);
	break;

	case patrUSAG_CU:                            //parsing SQL Cursor statement
		pcTabProcName = patr.GetStrITEM(0);
		lOffBeg = patr.GetOffset(0);              //Mark recognized keyword + datatype of decl
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlSQLKeyword);

		pcTabProcName = patr.GetStrITEM(1);
		lOffBeg = patr.GetOffset(1);                   //Highlight datatype of decl
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlUsageCU, pcTabProcName);
	break;

	case patrDECLARE:                          //e.g. DECLARE VARIABLE other than cursor
		pcTabProcName = patr.GetStrITEM(0);
		lOffBeg = patr.GetOffset(0);              //Mark recognized keyword + datatype of decl
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword);

		pcTabProcName = patr.GetStrITEM(2);
		lOffBeg = patr.GetOffset(2);                   //Highlight datatype of decl
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword);
	break;

	case patrDECL_CU:                           //e.g. DECLARE Select CURSOR
		pcTabProcName = patr.GetStrITEM(0);
		lOffBeg = patr.GetOffset(0);              //Highlight declare keyword
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword);

		pcTabProcName = patr.GetStrITEM(1);      //Highlight name of cursor
		lOffBeg = patr.GetOffset(1);
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlDECL_CUName, pcTabProcName);

		lOffBeg = patr.GetOffset(2);                 //Highlight cursor for keywords
		lOffEnd = patr.GetOffset(3) + s::slen(patr.GetStrITEM(3));
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword);
	break;

	case patrUPDATEStats: //e.g. update statistics
		pcTabProcName = patr.GetStrITEM(patr.GetSz()-1, sServer, sDB, sUser, sDBObj,iFlags);
		idx = 0;
		lOffBeg = patr.GetOffset(idx);
		if(patr.GetSz() == 2)
			lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx)); //optimize highlighting
		else
		{
			idx = patr.GetSz()-2;
			lOffEnd = patr.GetOffset(idx) + s::slen(patr.GetStrITEM(idx));
		}

		//Mark recognized keyword(s) We need table_name for on_event_link
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlTableUpdStats,pcTabProcName
		                                      ,sServer,sDB, sUser, sDBObj,iFlags);

		idx = patr.GetSz()-1;                       //Highlight name of table
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlTableName, pcTabProcName
		                                   ,sServer,sDB, sUser, sDBObj,iFlags);

		//Store Fact: Procedure implements UPDATE on a table 1st in TabCOL
    dpDBOBj.SetName(sThisServ,sThisDB,sThisUser,sThisDBObj,dpoPrc);
		dpDBOBj.eacAccess  = eAT_UPDATEStats;
    dpDBOBj.iFlags     = iFlags;              //Set additional flags for this reference
		tbcTabs.AddTableDepend(sServer,sDB,sUser,sDBObj,eREFUNKNOWN,&dpDBOBj);

    //2nd Store fact in PROC-COL, table or view we don't know
    dpDBOBj.SetName(sServer,sDB,sUser,sDBObj,eREFTABVIEW);
		prcRet.m_dpDBObjDeps.AddDep(&dpDBOBj);
	break;

	case patrExec: //EXEC <Procedure_Name>
		pcTabProcName = patr.GetStrITEM(patr.GetSz()-1, sServer,sDB,sUser,sDBObj, iFlags);
		idx = 0;
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword); //Mark SQL keyword

		if(s::slen(pcTabProcName) <= 0) break;

		idx = patr.GetSz()-1;         //Highlight name of a procedure
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(pcTabProcName);
		prcRet.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlProcedure,pcTabProcName
		                                    ,sServer,sDB,sUser,sDBObj,iFlags);

		//PROC type is based on the theorie that triggers
		//can not be part of an exec statement
		prcRet.m_dpDBObjDeps.AddDep(sServer,sDB,sUser,sDBObj, eREFPROC ,eAT_EXEC,iFlags);
	break;

	case patrTRUNC_TAB: //Truncate TABLE STATEMENT
		pcTabProcName = patr.GetStrITEM(patr.GetSz()-1, sServer,sDB, sUser, sDBObj,iFlags);

		idx = 0;
		lOffBeg = patr.GetOffset(idx);
		if(patr.GetSz() == 2)
			lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx)); //optimize highlighting
		else
		{
			idx = patr.GetSz()-2;
			lOffEnd = patr.GetOffset(idx) + s::slen(patr.GetStrITEM(idx));
		}
		//Mark recognized keyword(s) We need table_name to link to that table
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTruncTable, pcTabProcName
		                                       ,sServer,sDB,sUser,sDBObj,iFlags);

		idx = patr.GetSz()-1;                       //Highlight name of table
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableName,pcTabProcName
		                                     ,sServer,sDB,sUser,sDBObj,iFlags);

		//Store Fact: Procedure implements DELETE on a table 1st in TabCOL
    dpDBOBj.SetName(sThisServ,sThisDB,sThisUser,sThisDBObj,dpoPrc);
		dpDBOBj.eacAccess  = eAT_TRUNC_TAB;
    dpDBOBj.iFlags     = iFlags;              //Set additional flags for this reference
		tbcTabs.AddTableDepend(sServer,sDB,sUser,sDBObj,eREFUNKNOWN,&dpDBOBj);

    //2nd Store fact in PROC-COL, table or view we don't know
    dpDBOBj.SetName(sServer,sDB,sUser,sDBObj,eREFTABVIEW);
		prcRet.m_dpDBObjDeps.AddDep(&dpDBOBj);
	break;

	case patrDELETE: //DELETE [FROM] TABLE
		pcTabProcName = patr.GetStrITEM(patr.GetSz()-1, sServer,sDB,sUser,sDBObj, iFlags);

		idx = 0;
		lOffBeg = patr.GetOffset(idx);
		if(patr.GetSz() == 2)
			lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx)); //optimize highlighting
		else
		{
			idx = patr.GetSz()-2;
			lOffEnd = patr.GetOffset(idx) + s::slen(patr.GetStrITEM(idx));
		}
		//Mark recognized keyword(s) We need table_name for on_event_link
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableDel, pcTabProcName
		                                   ,sServer,sDB, sUser, sDBObj,iFlags);

		idx = patr.GetSz()-1;                       //Highlight name of table
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableName,pcTabProcName
		                                   ,sServer,sDB, sUser, sDBObj,iFlags);

		//Store Fact: Procedure implements DELETE on a table 1st in TabCOL
    dpDBOBj.SetName(sThisServ,sThisDB,sThisUser,sThisDBObj,dpoPrc);
		dpDBOBj.eacAccess  = eAT_DELETE;
    dpDBOBj.iFlags     = iFlags;              //Set additional flags for this reference
    tbcTabs.AddTableDepend(sServer,sDB,sUser,sDBObj,eREFUNKNOWN,&dpDBOBj);

    //2nd Store fact in PROC-COL, table or view we don't know
    dpDBOBj.SetName(sServer,sDB,sUser,sDBObj,eREFTABVIEW);
		prcRet.m_dpDBObjDeps.AddDep(&dpDBOBj);
	break;

	case patrUPDATE: //UPDATE TABLE
		pcTabProcName = patr.GetStrITEM(patr.GetSz()-1, sServer,sDB,sUser,sDBObj, iFlags);

		idx = 0;
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));

		//Mark recognized keyword > We need table_name for on_event_link
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableUpd, pcTabProcName
		                                   ,sServer,sDB, sUser, sDBObj,iFlags);

		idx = patr.GetSz()-1;         //Highlight name of table
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));
		prcRet.hlPatrn.AddPatrn2(lOffBeg,lOffEnd,hlTableName,pcTabProcName
		                                ,sServer,sDB, sUser, sDBObj,iFlags);

		//Store Fact: Procedure implements UPDATE on a table
    dpDBOBj.SetName(sThisServ,sThisDB,sThisUser,sThisDBObj,dpoPrc);
		dpDBOBj.eacAccess  = eAT_UPDATE;
    dpDBOBj.iFlags     = iFlags;              //Set additional flags for this reference
		tbcTabs.AddTableDepend(sServer,sDB,sUser,sDBObj,eREFUNKNOWN,&dpDBOBj);

    //2nd Store fact in PROC-COL, table or view we don't know
    dpDBOBj.SetName(sServer,sDB,sUser,sDBObj,eREFTABVIEW);
		prcRet.m_dpDBObjDeps.AddDep(&dpDBOBj);
		break;

	case patrINSERT: //INSERT [INTO] TABLE
		pcTabProcName = patr.GetStrITEM(patr.GetSz()-1, sServer,sDB,sUser,sDBObj, iFlags);

		idx = 0;
		lOffBeg = patr.GetOffset(idx);
		if(patr.GetSz() == 2)
			lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx)); //optimize highlighting
		else
		{
			idx     = patr.GetSz() - 2;
			lOffEnd = patr.GetOffset(idx) + s::slen(patr.GetStrITEM(idx));
		}
		//Mark recognized keyword(s)  > We need table_name for on_event_link
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableIns, pcTabProcName
		                                   ,sServer,sDB, sUser, sDBObj,iFlags);

		idx = patr.GetSz() - 1;         //Highlight name of table
		lOffBeg = patr.GetOffset(idx);
		lOffEnd = lOffBeg + s::slen(patr.GetStrITEM(idx));
		prcRet.hlPatrn.AddPatrn2(lOffBeg , lOffEnd, hlTableName,pcTabProcName
		                                    ,sServer,sDB,sUser,sDBObj, iFlags);

    dpDBOBj.SetName(sThisServ,sThisDB,sThisUser,sThisDBObj,dpoPrc);
		dpDBOBj.eacAccess  = eAT_INSERT;       //Store Fact: PROC implements insert
    dpDBOBj.iFlags     = iFlags;              //Set additional flags for this reference
		tbcTabs.AddTableDepend(sServer,sDB,sUser,sDBObj,eREFUNKNOWN,&dpDBOBj);

    //2nd Store fact in PROC-COL, table or view we don't know
    dpDBOBj.SetName(sServer,sDB,sUser,sDBObj,eREFTABVIEW);
		prcRet.m_dpDBObjDeps.AddDep(&dpDBOBj);
	break;

	default:
	break;
	}
	}
	catch(CERR e)
	{
		CUSTR s(e.getsErrSrc());
		printf("CodeRef: %s in %s (DBObj: %s)\n", (const UCHAR *)s, _SRC, (const char*)prcRet.GetName());
		printf("DEBUG INFO: '%s.%s.%s' State: %i\n",(const char*)sDB,(const char*)sUser,
	                                            (const char*)sDBObj,patr.epType);
		e.printfMsg();

		//throw CERR(e);
		return;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR", 0, _SRC);
	}
}
#undef _SRC

//
//Parse an Exec SQL Statement keyword by keyword and state by state
//-------------------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static bool ParseExec(int *piCreateState, const int iRdSz, UCHAR *pcBuf
                     ,FILE *fpIn, const int iBufSz, const long lFileOff
										 ,CPITEM & patr,CUSTR sRefServ,CUSTR sRefDB, CUSTR sRefOwner,CUSTR sDBObj)
{
  int iRefFlags=0;

	switch(*piCreateState)
	{
	// pcBuf should hold the name of the procedure call OR
	// an SQL variable assigned to the return code: e.g.: exec @iRet = p_get_THIS
	case 1:
		if(iRdSz <= 0)
		{
			patr.Reset(); //Reset State Engine to Start
			return false;
		}

		if(pcBuf[0] != '@')
		{
			patr.epType = patrExec;  //Set type of recognized item
			sDBObj = pcBuf;

			CompleteDBObjRef(pcBuf, fpIn, iBufSz);
			CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,sDBObj, iRefFlags);
			//Add name of function to string array, thats the reference as seen in SQL
			patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf, lFileOff, iRefFlags);
			return true;
		}

		patr.AddString("","","","",pcBuf, lFileOff,0);
		*piCreateState += 1;            // contimue search for '= p_get_THIS'
		break;

	case 2:
		if(iRdSz <= 0)
		{
			patr.Reset(); //Reset State Engine to Start
			return false;
		}

		if(s::scmp_ci_uc(pcBuf, "=") == 0)
		{
			patr.AddString("","","","",pcBuf, lFileOff,0);
			*piCreateState += 1;
		}
		else
		{
			*piCreateState=0;
			patr.Reset();    //Reset State Engine to Start
		}
		break;

	// pcBuf should hold the name of the procedure call following the equal char
	case 3:
		patr.epType = patrExec;  //Set type of recognized item

		CompleteDBObjRef(pcBuf, fpIn, iBufSz);
		CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
		//Add name of function to string array, plus reference as seen in SQL Source
		patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj, pcBuf, lFileOff, iRefFlags);
		return true;
		
	default:
		return false; //All unrecognized states are bad and will reset the engine
	}

	return true;
}

//
//Parse a TRUNCATE TABLE SQL Statement keyword by keyword and state by state
static bool ParseTruncTab(int *piCreateState,
                          int & iRdSz, UCHAR *pcBuf,const int iBufSz,FILE *fpIn,
                          const long lFileOff,
                          CPITEM & patr,
                          CUSTR sRefServ,CUSTR sRefDB,CUSTR sRefOwner,CUSTR sDBObj)
{
  int iRefFlags=0;

  if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset();  //Reset State Engine to Start
		return false;
	}

	switch(*piCreateState)
	{
	//pcBuf should contain the 'TABLE' from which we delete or the 'FROM' keyword
	case 1:
		if(s::scmp_ci_uc(pcBuf,"TABLE") == 0) //We need the table keyword right here
		{
			patr.AddString("","","","",pcBuf,lFileOff,0);
			*piCreateState += 1;        // contimue search for 'TABLE_NAME' -> Ref-Dep
			return true;
		}

		return false;
		break;

	case 2: //pcBuf is expected to hold the table name/reference
		if(pcBuf[0] != '#')
		{
			patr.epType = patrTRUNC_TAB;  //Set type of recognized item
	
			CompleteDBObjRef(pcBuf, fpIn, iBufSz);
			iRdSz=s::slen(pcBuf);
			CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
			patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf,lFileOff, iRefFlags);
			return true;
		}

    patr.Reset(); //Reset result state engine (because we macthed this but ignore it)
		return false;

  default:        //Unrecognized states will reset the engine, so we pass this
		return false;
	}

	return true;
}

//
//Parse a DELETE [FROM] TABLE SQL Statement keyword by keyword and state by state
static bool ParseDELETE(int *piCreateState,
                        int & iRdSz, UCHAR *pcBuf,const int iBufSz,FILE *fpIn,
                        const long lFileOff,
                        CPITEM & patr,
                        CUSTR sRefServ,CUSTR sRefDB,CUSTR sRefOwner,CUSTR sDBObj)
{
  int iRefFlags=0;

  if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset();  //Reset State Engine to Start
		return false;
	}

	switch(*piCreateState)
	{
	//pcBuf should contain the 'TABLE' from which we delete or the 'FROM' keyword
	case 1:
		if(s::scmp_ci_uc(pcBuf,"FROM") != 0)
		{
			patr.epType = patrDELETE;  //Set type of recognized item

			CompleteDBObjRef(pcBuf, fpIn, iBufSz);
			CDEP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
			patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf,lFileOff, iRefFlags);
			return true;
		}

		patr.AddString("","","","",pcBuf,lFileOff,0);
		*piCreateState += 1; // contimue search for 'TABLE_NAME' -> Ref-Dep
		break;

	case 2:
		if(s::scmp_ci_uc(pcBuf,"FROM") != 0)
		{
			patr.epType = patrDELETE;  //Set type of recognized item
	
			CompleteDBObjRef(pcBuf, fpIn, iBufSz);
			CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
			patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf,lFileOff, iRefFlags);
			return true;
		}

	default:        //Unrecognized states will reset the engine, so we pass this
		return false;
	}

	return true;
}

//
//Parse a INSERT [INTO] TABLE SQL Statement keyword by keyword and state by state
//
static bool ParseINSERT(int *piCreateState,
												const int iRdSz,
												UCHAR *pcBuf, const int iBufSz,
												FILE *fpInput,
												const long lFileOff,
												CPITEM & patr,
												CPATRN & hlPatrn,
                        CUSTR sRefServ,CUSTR sRefDB, CUSTR sRefOwner, CUSTR sDBObj)
{
	if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset();  //Reset State Engine to Start
		return false;
	}

	UCHAR c;
	int iLastItem=0,iLastString=0, iRefFlags=0;
	CPATRN ptrmTmp;
	long lFOff;

	switch(*piCreateState)
	{
	//pcBuf should contain the 'TABLE' from which we delete or the 'FROM' keyword
	case 1:
		if(s::scmp_ci_uc(pcBuf,"INTO") != 0)
		{
			patr.epType = patrINSERT;                    //Set type of recognized item
			CompleteDBObjRef(pcBuf, fpInput, iBufSz);
			CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
			patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf,lFileOff, iRefFlags);
			return true;
		}

		patr.AddString("","","","",pcBuf,lFileOff,0);
		*piCreateState += 1; // contimue search for '= p_get_ID'
		break;

	case 2:           //pcBuf should contain the 'TABLE' into which we insert into
		patr.epType = patrINSERT;                      //Set type of recognized item
		CompleteDBObjRef(pcBuf, fpInput, iBufSz);
		CDEP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
		patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf,lFileOff, iRefFlags);//Add name of table

		//Implement skip field list (, , ,), if given, pop back, if not >>>>><<<<<<<
		lFOff = ftell(fpInput);

		SkipWhiteSpace2(c,fpInput,iLastItem,iLastString,ptrmTmp);

		if(c == '(')
		{
			hlPatrn += ptrmTmp;

			while(c != ')' && !feof(fpInput))
				SkipWhiteSpace2(c,fpInput,iLastItem,iLastString,hlPatrn);
		}
		else
			fseek(fpInput, lFOff , SEEK_SET); //pop back to where we were >>>>>>><<<<<

		return true;

	default:
		return false; //All unrecognized states are bad and will reset the engine
	}

	return true;
}

//
//Parse a Declare statement
//
static bool ParseDECLARE(patrTYPE *ppatrMastre,int *piCreateState,const int iRdSz,
												UCHAR *pcBuf, const long lFileOff, CPITEM & patr)
{
	if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset();  //Reset State Engine to Start
		return false;
	}

	switch(*piCreateState)
	{
	//pcBuf should contain a name of a variable or the keyword "cursor"
	case 1:
		*piCreateState += 1;                  //Add name of function to string array
		patr.AddString("","","","",pcBuf, lFileOff,0);
		return true;
	break;
	//pcBuf should contain a name of a variable or the keyword "cursor"
	case 2:
		patr.AddString("","","","",pcBuf, lFileOff,0);
		if(s::scmp_ci_uc(pcBuf,"CURSOR") == 0)
		{
			*piCreateState += 1;
			*ppatrMastre    = patrDECL_CU; //SQL cursor declaration
			return true;
		}
		else
		{
			patr.epType = patrDECLARE;
			return true;
		}
	}
	return false;      //All unrecognized states are bad and will reset the engine
}

//
//Parse a UPDATE TABLE SQL Statement keyword by keyword and state by state
//
static bool ParseDECL_CU(int iCreateState, const int iRdSz, UCHAR *pcBuf,
                         const long lFileOff, CPITEM & patr)
{
	if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset(); //Reset State Engine to Start
		return false;
	}

	switch(iCreateState)
	{
	//pcBuf should contain: the name of the 'CURSOR' we are looking at
	case 3:
		if(s::scmp_ci_uc(pcBuf,"FOR") == 0)
		{
			patr.AddString("","","","",pcBuf, lFileOff,0);     //Add Name of cursor
			patr.epType = patrDECL_CU;                   //Set type of recognized item
		
			return true;
		}
	}

	return false;
}

//
//Parse a deallocate cursor statement state by state
//===================================================>
static bool ParseDEAL_CU(int *piCreateState, const int iRdSz, UCHAR *pcBuf,
												 const long lFileOff, CPITEM & patr)
{
	if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset();  //Reset State Engine to Start
		return false;
	}

	switch(*piCreateState)
	{
	//pcBuf should contain the 'CURSOR' statement, otherwise we are looking at something else
	case 1:
		if(s::scmp_ci_uc(pcBuf,"CURSOR") == 0)
		{
			patr.AddString("","","","",pcBuf,lFileOff,0);
			*piCreateState += 1;                     //continue search
			return true;
		}
		break;

	//pcBuf should contain the name of the CURSOR that has been deallocated here
	case 2:
		patr.epType = patrDEAL_CU;                 //Set type of recognized item
		patr.AddString("","","","",pcBuf,lFileOff,0);  //Add name of cursor

		return true;
	}

	return false;
}

//
//Parse a UPDATE TABLE SQL Statement keyword by keyword and state by state
//
static bool ParseUPDATE(patrTYPE *ppatrMastre, int iCreateState,const int iRdSz,
                        UCHAR *pcBuf, const int iBufSz,FILE *fpIn,
                        const long lFileOff, CPITEM & patr,
                        CUSTR sRefServ,CUSTR sRefDB, CUSTR sRefOwner)
{
  int iRefFlags=0;
	CUSTR sDBObj;

	if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset(); //Reset State Engine to Start
		return false;
	}

	switch(iCreateState)
	{
	//pcBuf should contain:
	//1> the name of the 'TABLE' on which we update
	//2> or the 'statistics' keyword (in which case we highlight SQL and continue)
	case 1:
		if(s::scmp_ci_cu("statistics", pcBuf) == 0)
		{
			patr.AddString("","","","",pcBuf, lFileOff,0);  //Add sql keyword
			*ppatrMastre = patrUPDATEStats;  //Update statistics keyword -> Set new target
		}
		else
		{
			patr.epType = patrUPDATE;  //Set type of recognized item

			CompleteDBObjRef(pcBuf, fpIn, iBufSz);
			CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
			patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf, lFileOff, iRefFlags);
		}

		return true;
	}

	return false;
}

//
//Parse a UPDATE TABLE SQL Statement keyword by keyword and state by state
//-------------------------==========================================>>>>>>>
static bool ParseUPDATEStats(int iCreateState, const int iRdSz, UCHAR *pcBuf,
                             const int iBufSz,FILE *fpIn,
                             const long lFileOff, CPITEM & patr,
                             CUSTR sRefServ,CUSTR sRefDB, CUSTR sRefOwner, CUSTR sDBObj)
{
  int iRefFlags=0;

  if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset(); //Reset State Engine to Start
		return false;
	}

	switch(iCreateState)
	{
	//pcBuf should contain:
	//1> the name of the 'TABLE' on which we run the update statistics command
	case 1:
		CompleteDBObjRef(pcBuf, fpIn, iBufSz);
		CDP::ParseRef2DBOBJ(sRefServ,sRefDB,sRefOwner,(sDBObj=pcBuf), iRefFlags);
		patr.AddString(sRefServ,sRefDB,sRefOwner,sDBObj,pcBuf, lFileOff,0); //Add name of table

		patr.epType = patrUPDATEStats;  //Set type of recognized item
		return true;
	}

	return false;
}

//
//Parse a UPDATE TABLE SQL Statement keyword by keyword and state by state
//
static bool ParseOPEN_CU(int iCreateState, const int iRdSz, UCHAR *pcBuf,
                        const long lFileOff, CPITEM & patr)
{
	if(iRdSz <= 0)   //No info -> no deal
	{
		patr.Reset(); //Reset State Engine to Start
		return false;
	}

	//pcBuf should contain: the name of the cursor we are about to open
	if(iCreateState == 1)
	{
		patr.AddString("","","","",pcBuf, lFileOff,0); //Add name of cursor
		patr.epType = patrUSAG_CU;               //Set type of recognized item
		
		return true;
	}

	return false;
}

typedef struct first_state
{
	UCHAR pcString[12];
	patrTYPE patrStart;

} FIRST_STATE;

// <<<============--------------- First states and their keywords ----=====>>>
#define StartStatsSz 14

FIRST_STATE StartStats[StartStatsSz] =
{
	 {"exec"   ,patrExec}
	,{"execute",patrExec}
	,{"call"   ,patrExec}  //Sybase IQ dialekt
	,{"delete" ,patrDELETE}
	,{"update" ,patrUPDATE}
	,{"insert" ,patrINSERT}
	,{"declare",patrDECLARE} //declare variable or cursor (see patrDECL_CU)

	,{"open"       ,patrUSAG_CU} //using cursor statements
	,{"fetch"      ,patrUSAG_CU}
	,{"close"      ,patrUSAG_CU}
	,{"deallocate" ,patrDEAL_CU}    //deallocating cursor statement
	,{"from"       ,patrFROM}      //FROM Clause (used in select, update, delete statements)
	,{"truncate"   ,patrTRUNC_TAB} //TRUNCATE TABLE STATEMENT
	,{"for"   ,patrFOR_UPDATE}    //TRUNCATE TABLE STATEMENT
};

// x
// x Parse procedure/trigger SQL source code and return recognized items
// x
// x Note: PATR and PATRN_R do not always contain the same information since,
// x       for example, comments are highlighted (in PATRN_R), but never
// x       returned as recognized items (in PATR)
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#define _SRC "PARSE::paGetNxtItem"
int paGetNxtItem(UCHAR *pcBuf, int iBufSz //Output buffer e.g. 10k memory
                ,FILE *fpInput           //Input Stream from file
                ,CPITEM & patr          //Return collection of recognized items 2 caller
                ,CPROC & prcRet        //Proc object
                ,COpts *pPrOpts       //Programm options, keywords etc...
                ,CTABCOL & tbcTabs
                ,const UCHAR *pcServer
                ,const UCHAR *pcDB)    //Name of database object resides in
{
	UCHAR c;
	int iRdSz, iCreateState=0,y;
	bool bEndKeyword = false;
  int iLastItem = 0, iLastString = 0; //States to optimize parsing for comments & strings
	patrTYPE patrMastre = patrUnknown;  //A state for a statement we are attempting to parse

	try
	{
	//Default values for references to object that depend on this object
	CUSTR sRefServ(prcRet.GetServer()), sRefDB(pcDB),sRefOwner(prcRet.GetOwner()), sDBObj;
  CDP::Handle_Requi_Params(sRefServ,sRefDB,sRefOwner,prcRet.GetName(), _SRC);

	patr.Reset(); //Reset result state engine

	while(!feof(fpInput)) //Parser main loop, in each loop we extract a keyword,
	{                    //or group of words and do something with that
		sRefServ=pcServer, sRefDB=pcDB, sRefOwner=prcRet.GetOwner(), sDBObj="";

		while((y = SkipWhiteSpaces(c,fpInput,iLastItem,iLastString,patrMastre,
		                                    iCreateState,patr,prcRet.hlPatrn)) == 1)
			;

		if(y < 0) return false; //End of file or other error condition...

		iLastString = 0;
		if((iRdSz=FindKeyword(bEndKeyword, fpInput, pcBuf, iBufSz,patr ,prcRet.hlPatrn,iLastItem ,c))< 0)
			return false;
		
		if(bEndKeyword == true)    //Found a keyword to look at, then switch some states!
		{
			//Get File offset for keyword item we are about to analyze
			const long lFileOff = ftell(fpInput) - (iRdSz+1);

			iLastItem = 0;  //Found non-whitespace after last comment statement

			switch(patrMastre) //Initiate first state for each Mastre state, A Mastre
			{                  //is for example an 'EXEC' or 'DELETE' statement, In
			case patrUnknown:  //other words, the 1st word of that statement is what
				for(y=0;y<StartStatsSz && patrMastre == patrUnknown;y++) //we match here
				{
					if(s::scmp_ci(pcBuf,StartStats[y].pcString) == 0)
					{
						if(StartStats[y].patrStart == patrUPDATE)
						{
							if( c != '(') //Updates can also be update functions >update_func<
							{
								patrMastre = StartStats[y].patrStart;
								iCreateState=1;
								patr.AddString("","","","",pcBuf, lFileOff,0);//Highlight SQL keyword
							}
						}
						else
						{
							patrMastre = StartStats[y].patrStart;
							iCreateState=1;
							patr.AddString("","","","",pcBuf, lFileOff,0);  //Highlight SQL keyword
						}
					}
				}

				if(patrMastre == patrUnknown)             //Highlight single sql keyword
				if(pPrOpts->hKeyWords.Get_i(pcBuf) == true)  //if they are in the table
				{
					prcRet.hlPatrn.AddPatrn2(lFileOff , lFileOff+iRdSz, hlSQLKeyword);
				}
			break;
			          //Parse Exec Statement or reset to initial state on syntax error
			case patrExec:
				if(ParseExec(&iCreateState,iRdSz,pcBuf,fpInput,iBufSz, lFileOff,patr,
				                                   sRefServ,sRefDB, sRefOwner, sDBObj) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;
			case patrTRUNC_TAB:                             //TRUNCATE TABLE STATEMENT
				if(ParseTruncTab(&iCreateState,iRdSz,pcBuf,iBufSz,fpInput,lFileOff,patr,
				                                      sRefServ,sRefDB, sRefOwner,sDBObj) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;
			case patrDELETE: //Parse DELETE Statement or reset to initial state on syntax error
				if(ParseDELETE(&iCreateState,iRdSz,pcBuf, iBufSz,fpInput,lFileOff,patr,
				                             sRefServ,sRefDB,sRefOwner,sDBObj) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;
			case patrUPDATE: //Parse UPDATE Statement or reset to initial state on syntax error
				if(ParseUPDATE(&patrMastre,iCreateState,iRdSz,pcBuf,iBufSz,fpInput,
				                              lFileOff,patr,sRefServ,sRefDB,sRefOwner) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrUPDATEStats: //Parse UPDATE Statement or reset to initial state on syntax error
        if(ParseUPDATEStats(iCreateState,iRdSz,pcBuf,iBufSz,fpInput,lFileOff,patr,
                            sRefServ,sRefDB,sRefOwner,sDBObj) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;
			
			case patrINSERT: //Parse INSERT Statement or reset to initial state on syntax error
				if(ParseINSERT(&iCreateState,iRdSz,pcBuf,iBufSz,fpInput,lFileOff,patr,
				               prcRet.hlPatrn, sRefServ,sRefDB,sRefOwner,sDBObj) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrDECLARE: //Parse INSERT Statement or reset to initial state on syntax error
				if(ParseDECLARE(&patrMastre,&iCreateState,iRdSz,pcBuf,lFileOff,patr)==false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrDECL_CU: //Parse INSERT Statement or reset to initial state on syntax error
				if(ParseDECL_CU(iCreateState,iRdSz,pcBuf, lFileOff,patr) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrUSAG_CU: //Parse INSERT Statement or reset to initial state on syntax error
				if(ParseOPEN_CU(iCreateState,iRdSz,pcBuf, lFileOff,patr) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrDEAL_CU:
				if(ParseDEAL_CU(&iCreateState,iRdSz,pcBuf, lFileOff,patr) == false)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrFROM:
				iLastItem=iLastString=0;
				if(ParseFromExtend(fpInput, c, pcBuf, iBufSz, lFileOff,patr,prcRet.hlPatrn,
				                         pPrOpts,sRefServ,sRefDB,sRefOwner,prcRet.GetName())<=0)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;

			case patrFOR_UPDATE: //This is just to disambiguate from for update cursor
				if(s::scmp_ci(pcBuf, (const UCHAR *)"update")==0 ||
           s::scmp_ci(pcBuf, (const UCHAR *)"read")==0)
				{
					const long lFOff = patr.GetOffset(patr.GetSz()-1);
					prcRet.hlPatrn.AddPatrn2(lFOff , lFOff+3, hlSQLKeyword); //"FOR" KeyWrd
					prcRet.hlPatrn.AddPatrn2(lFileOff , lFileOff+6, hlSQLKeyword); //"UPDATE" KeyWrd
					patr.Reset();
					iCreateState = 0;
					patrMastre   = patrUnknown;
				}
			}
		}

		//Found an interesting sequence of keywords
		if(patrMastre != patrUnknown && patr.epType == patrMastre)
		{
			if((patrMastre == patrDELETE || patrMastre == patrUPDATE || patrMastre == patrINSERT)
			   && patr.GetSz()>0)
			{
				//Here we want to filter references to temporary tables such as #TEMP
				//We found a statement, but we choose to ignore it by marking it as unknown
				const UCHAR *pc = patr.GetStrITEM(patr.GetSz()-1);
				if(pc[0] == '#') patr.epType = patrUnknown;
			}

			AddPattern(patr,prcRet,tbcTabs); //Get Highlighting patterns
			return(true);
		}

		iRdSz = 0;        // Reset input buffer
		pcBuf[iRdSz] = 0;
	}                   //End of Main while Loop
	if(feof(fpInput)) return(false);

	//We should never get here, but return what we got in case we got here anyway
	AddPattern(patr, prcRet, tbcTabs); //Get Highlighting patterns
	}
	catch(CERR e)
	{
		printf("CodeRef: %s\n", _SRC);
		e.printfMsg();
    printf("DEBUG INFO: DB Object: %s.%s.%s.%s Buf: %s\n"
            ,(const char *)prcRet.GetServer(),(const char *)prcRet.GetDB()
            ,(const char *)prcRet.GetOwner(), (const char *)prcRet.GetName(),pcBuf);

		if(patrMastre >=0 && patrMastre < StartStatsSz)
			printf("DEBUG INFO: State: %s\n", StartStats[patrMastre].pcString);
		else
			printf("DEBUG INFO: Unknown State: %i\n", patrMastre);

		//throw CERR(e);
		return false;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR", 0, _SRC);
	}

	return(true);
}
#undef _SRC

//----------------------------------------------------------------------->
//Add Highlight pattern information for update, insert, delete statements
//----------------------------------------------------------------------->
#define _SRC "parse::AddViewPattern"
static void AddViewPattern(CPITEM & patr,    //Parse tree of recognized items
                           CTAB & tblView,  //Highlighting pattern for this file
                           CTABCOL & tbcTabs)
{
	long lOffBeg, lOffEnd;
	int idx,iFlags;
	const UCHAR *pcTabProcName=NULL;
	CUSTR sServ, sDB, sUser, sDBObj;
	CUSTR sThisServ(tblView.GetServer()),sThisDB(tblView.GetDB())
       ,sThisUser(tblView.GetOwner()), sThisDBObj(tblView.GetName()); //Just a handy dandy shortcut

  CDP::Handle_Requi_Params(sThisServ,sThisDB,sThisUser, sThisDBObj, _SRC);

	try
	{
	if(patr.epType == patrFROM)
	if(patr.GetSz() > 0)
	{
		lOffBeg = patr.GetOffset(0);                 //Highlight & add tab/view name
		lOffEnd = patr.GetOffset(0) + s::slen(patr.GetStrITEM(0));
		tblView.hlPatrn.AddPatrn2(lOffBeg ,lOffEnd,hlSQLKeyword);  //Mark "FROM" keyword

		//Store Fact: View implements SELECT on another table or !!!another view!!!
		pcTabProcName = patr.GetStrITEM(1, sServ, sDB, sUser, sDBObj,iFlags);

		tblView.m_dpDBObjDeps.AddDep(sServ, sDB, sUser,sDBObj, eREFTABVIEW, eAT_SELECT_BY_VIEW, iFlags);
		
		//FACT: A View can implement ONLY SELECTs on another view or table
		CDEP dpsObj(sThisServ, sThisDB, sThisUser,sThisDBObj, eREFVIEW, eAT_SELECT, iFlags,"");
		tbcTabs.AddTableDepend(sServ,sDB,sUser,sDBObj, eREFUNKNOWN, &dpsObj);

		for(idx=1; idx<patr.GetSz(); idx++)
		{
			pcTabProcName = patr.GetStrITEM(idx, sServ, sDB, sUser, sDBObj,iFlags);

			if(s::slen(pcTabProcName) <= 0) continue;

			lOffBeg = patr.GetOffset(idx);                 //Highlight name of table
			lOffEnd = lOffBeg + s::slen(pcTabProcName);

			tblView.hlPatrn.AddPatrn2(lOffBeg,lOffEnd,hlTableFrom,pcTabProcName
			                         ,sServ,sDB, sUser, sDBObj,iFlags);

			//Store Fact: View implements SELECT on another table or view
			tblView.m_dpDBObjDeps.AddDep(sServ,sDB,sUser,sDBObj, eREFTABVIEW, eAT_SELECT_BY_VIEW, iFlags);
			
			//FACT: A View can implement ONLY SELECTs on another view or table
			CDEP dpsObj1(sThisServ,sThisDB, sThisUser,sThisDBObj, eREFVIEW, eAT_SELECT, iFlags,"");
			tbcTabs.AddTableDepend(sServ, sDB, sUser, sDBObj, eREFUNKNOWN, &dpsObj1);
		} //End of FOR Loop
	}
	}
	catch(CERR e)
	{
		printf("CodeRef: %s\n", _SRC);
		throw CERR(e);
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR", 0, _SRC);
	}
}
#undef _SRC

// <<<============--------------- First View states and their keywords ----=====>>>
#define StartViewStatsSz 1

FIRST_STATE StartViewStats[StartViewStatsSz] =
{
	{"from"       ,patrFROM}   //FROM Clause (used in select, update, delete statements)
};

// ***
// * Parse procedure/trigger SQL source code and return recognized items
// *
// * Note: PATR and PATRN_R do not always contain the same information since,
// *       for example, comments are highlighted (in PATRN_R), but never
// *       returned as recognized items (in PATR)
// ***
#define _SRC "parse::paGetNxtViewItem"
int paGetNxtViewItem(UCHAR *pcBuf, int iBufSz, //Output buffer e.g. 10k memory
                     FILE *fpInput,            //Input Stream from file
                     CPITEM & patr,           //Return collection of recognized items 2 caller
                     CTAB & tblRet,           //View object
                     COpts *pPrOpts,          //Programm options, keywords etc...
                     CTABCOL & tbcTabs,
                     const UCHAR *pcServer, const UCHAR *pcDB)
{
	UCHAR c;
	int iRdSz, iCreateState=0,y;
	bool bEndKeyword = false;
  int iLastItem = 0, iLastString = 0; //States to opimize parsing for comments & strings
	patrTYPE patrMastre = patrUnknown;  //A state for a statement we are attempting to parse

	//Default values for references to object that depend on this object
	CUSTR sRefOwner(tblRet.GetOwner()), sRefServ(pcServer), sRefDB(pcDB), sDBObj;
  CDP::Handle_Requi_Params(sRefServ,sRefDB,sRefOwner,tblRet.GetName(), _SRC);

	try
	{
	patr.Reset(); //Reset result state engine

	while(!feof(fpInput))
	{
		while((y = SkipWhiteSpaces(c,fpInput,iLastItem,iLastString,patrMastre,
		                                    iCreateState,patr,tblRet.hlPatrn)) == 1)
			;

		if(y < 0) return false; //End of file or other error condition...

		iLastString = 0;
		if((iRdSz=FindKeyword(bEndKeyword, fpInput, pcBuf, iBufSz,patr ,tblRet.hlPatrn,iLastItem ,c))< 0)
			return false;
		
		if(bEndKeyword == true)    //Found a keyword to look at, then switch some states!
		{
			//Get File offset for keyword item we are about to analyze
			const long lFileOff = ftell(fpInput) - (iRdSz+1);

			iLastItem = 0;  //Found non-whitespace after last comment statement

			switch(patrMastre)       //Initiate first state for each Mastre state
			{                        //A Mastre is for example an 'EXEC' or 'DELETE' statement
			case patrUnknown:        //In other words, the first word of that statement is what
				for(y=0;y<StartViewStatsSz && patrMastre == patrUnknown;y++)          //we match here
				{
					if(s::scmp_ci(pcBuf,StartViewStats[y].pcString) == 0)
					{
						if(StartViewStats[y].patrStart == patrUPDATE)
						{
							if( c != '(') //Updates can also be update functions
							{
								patrMastre = StartViewStats[y].patrStart;
								iCreateState=1;
								patr.AddString("","","","",pcBuf, lFileOff,0); //Highlight single sql keywords
							}
						}
						else
						{
							patrMastre = StartViewStats[y].patrStart;
							iCreateState=1;
							patr.AddString("","","","",pcBuf, lFileOff,0); //Highlight single sql keywords
						}
					}
				}

				if(patrMastre == patrUnknown)                 //Highlight single sql keywords
				if(pPrOpts->hKeyWords.Get_i(pcBuf) == true)  //if they are in the table
				{
					tblRet.hlPatrn.AddPatrn2(lFileOff , lFileOff+iRdSz, hlSQLKeyword);
				}
			break;

			case patrFROM:
				iLastItem=iLastString=0;
				if(ParseFromExtend(fpInput, c, pcBuf, iBufSz, lFileOff,patr,tblRet.hlPatrn,
                           pPrOpts, pcServer,sRefDB,sRefOwner,tblRet.GetName())<=0)
				{
					patrMastre   = patrUnknown;
					iCreateState = 0;
				}
			break;
			default:
			break;
			}
		}

		//Found an interesting sequence of keywords
		if(patrMastre != patrUnknown && patr.epType == patrMastre)
		{
			if((patrMastre == patrDELETE || patrMastre == patrUPDATE || patrMastre == patrINSERT)
			   && patr.GetSz()>0)
			{
				//Here we want to filter references to temporary tables such as #TEMP
				//We found a statement, but we choose to ignore it by marking it as unknown
				const UCHAR *pc = patr.GetStrITEM(patr.GetSz()-1);
				if(pc[0] == '#')
				{
					patr.epType = patrUnknown;
				}
			}

			AddViewPattern(patr, tblRet, tbcTabs); //Get Highlighting patterns
			return(true);
		}

		iRdSz = 0;        // Reset input buffer
		pcBuf[iRdSz] = 0;
	}                   //End of Main while Loop
	if(feof(fpInput)) return(false);

	AddViewPattern(patr, tblRet, tbcTabs);  //Get Highlighting patterns
	}
	catch(CERR e){ printf("CodeRef: %s\n", _SRC); throw CERR(e); }
	catch(...)   { throw CERR("UNKNOWN ERROR", 0, _SRC);         }
	
	return(true);
}
#undef _SRC
#undef STRING_ITEM
#undef COMMENT_ITEM

// Find a statement such as 'create procedure x'
// or create 'create trigger x' and return x
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#define _SRC "PARSE::ParseObjDef"
CPROC *ParseObjDef(const UCHAR *pcServer,const UCHAR *pcDB, const UCHAR *pcOwner
                  ,UCHAR *pcBuf, int iBufSz, CSHCOL & hKeyWords, FILE *fpInput)
{
	UCHAR c;
	int iRdSz, bEnd = false, bEndKeyword = false, iCreateState=0;
	CPROC *prcRet;

	//We will insert the name later or not return this (promised)
	if((prcRet = new CPROC(pcServer,pcDB, pcOwner, "",eREFUNKNOWN))==NULL)
		goto OnErrorExit;

	while(bEnd == false && !feof(fpInput))
	{
		// Read until EOF or non-space character
		while((c = getc(fpInput)) <= ' ' && !feof(fpInput) )
			;

		if(feof(fpInput)) goto OnErrorExit;

		//We have a non-space->Fill Buffer until next white-space or buffer overflow
		iRdSz=0;
		pcBuf[iRdSz++] = c;
		pcBuf[iRdSz] = 0;
	
		bEndKeyword = false;
		while(bEndKeyword == false && !feof(fpInput) && iRdSz < iBufSz)
		{
			c = getc(fpInput);
			
			if(feof(fpInput)) goto OnErrorExit;

			if(c > ' ' && s::schr((UCHAR *)",;-+=|*/{[()]}", c ) == NULL)
			{
				pcBuf[iRdSz++] = c;
				pcBuf[iRdSz] = 0;
			}
			else
			{
				if(iRdSz > 0)
				{
					// found a comment such as '-- This is a comment'
					if(pcBuf[iRdSz-1] == '-' && c == '-')
					{
						int iLastItem=0;
						pcBuf[--iRdSz] = 0;
						SkipAndMarkComment((UCHAR *)"\n", &iLastItem, fpInput, prcRet->hlPatrn);
					}

					// found an ANSI C-Style comment?
					if(pcBuf[iRdSz-1] == '/' && c == '*')
					{
						int iLastItem=0;
						pcBuf[--iRdSz] = 0;
						SkipAndMarkComment(pcEndCm, &iLastItem, fpInput, prcRet->hlPatrn);
					}

					bEndKeyword = true;
				}
			}
		} // End of Keyword Search

		if(bEndKeyword == true)
		{
			if(hKeyWords.Get_i(pcBuf) == true)
			{
				long lOffEnd = ftell(fpInput)-1;
				long lOffBeg = lOffEnd-s::slen(pcBuf);

				prcRet->hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword);
			}

			switch(iCreateState)
			{
			case 0:
				if(s::scmp_ci_uc(pcBuf, "create") == 0 ||
					 s::scmp_ci_uc(pcBuf, "alter") == 0)
				{
					iCreateState = 1;
				}
				break;
			
			case 1:
				if(s::scmp_ci_uc(pcBuf, "procedure") == 0 ||
					 s::scmp_ci_uc(pcBuf, "proc") == 0)
					prcRet->SetObjType(eREFPROC);
				else
				{
					if(s::scmp_ci_uc(pcBuf, "trigger") == 0)
						prcRet->SetObjType(eREFTRIG);
				}

				if(prcRet->GetObjType() != eREFUNKNOWN)
					iCreateState = 2;
				else
					iCreateState = 0;

				break;

			case 2:        // pcBuf Should hold the name of the item being parsed
				return prcRet;
			default:
			break;
			}
		}

		iRdSz = 0;        // Reset input buffer
		pcBuf[iRdSz] = 0;
	}

	if(feof(fpInput)) goto OnErrorExit;

	return prcRet;

OnErrorExit:
		delete prcRet;
		return NULL;
}
#undef _SRC

static bool ParseViewDef(UCHAR *pcBuf, int iBufSz, CSHCOL & hKeyWords, FILE *fpInput,
                         CUSTR & sViewName, CTAB & tblView)
{
	UCHAR c;
	int iRdSz, bEnd = false, bEndKeyword = false, iCreateState=0;

	while(bEnd == false && !feof(fpInput))
	{
		// Read until EOF or non-space character
		while((c = getc(fpInput)) <= ' ' && !feof(fpInput) )
			;

		if(feof(fpInput)) goto OnErrorExit;

		// We have a non-space -> Fill Buffer until next
		//	white space or buffer overflow
		iRdSz=0;
		pcBuf[iRdSz++] = c;
		pcBuf[iRdSz] = 0;
	
		if(c != '(' && c != ')')           //Handle brackets as one keyword
			bEndKeyword = false;
		else
			bEndKeyword = true;

		while(bEndKeyword == false && !feof(fpInput) && iRdSz < iBufSz)
		{
			c = getc(fpInput);
			
			if(feof(fpInput)) goto OnErrorExit;

			if(c > ' ' && s::schr((UCHAR *)",;-+=|*/{[()]}", c ) == NULL)
			{
				pcBuf[iRdSz++] = c;
				pcBuf[iRdSz] = 0;
			}
			else
			{
				if(iRdSz > 0)
				{
					// found a comment such as '-- This is a comment'
					if(pcBuf[iRdSz-1] == '-' && c == '-')
					{
						int iLastItem=0;
						pcBuf[--iRdSz] = 0;
						SkipAndMarkComment((UCHAR *)"\n", &iLastItem, fpInput, tblView.hlPatrn);
					}

					// found an ANSI C-Style comment?
					if(pcBuf[iRdSz-1] == '/' && c == '*')
					{
						int iLastItem=0;
						pcBuf[--iRdSz] = 0;
						SkipAndMarkComment(pcEndCm, &iLastItem, fpInput, tblView.hlPatrn);
					}

					bEndKeyword = true;
				}
			}
		} // End of Keyword Search

		if(bEndKeyword == true)
		{
			if(hKeyWords.Get_i(pcBuf) == true)
			{
				long lOffEnd = ftell(fpInput)-1;
				long lOffBeg = lOffEnd-s::slen(pcBuf);

				tblView.hlPatrn.AddPatrn2(lOffBeg, lOffEnd, hlSQLKeyword);
			}

			switch(iCreateState)
			{
			case 0:
				if(s::scmp_ci_uc(pcBuf, "create") == 0 ||
					 s::scmp_ci_uc(pcBuf, "alter") == 0)
				{
					iCreateState = 1;
				}
				break;
			
			case 1:
				if(s::scmp_ci_uc(pcBuf, "view") == 0)
					tblView.SetObjType(eREFVIEW);

				if(tblView.GetObjType() != eREFUNKNOWN)
					iCreateState = 2;
				else
					iCreateState = 0;

				break;

			case 2:        // pcBuf Should hold the name of the view item being parsed
				sViewName = pcBuf;
				iCreateState = 3;
			break;

			case 3:        // pcBuf Should hold the name of the view item being parsed
				if(s::scmp_ci_uc(pcBuf, "as") == 0)
					return true;
				else           //Skip column definition unseen and search for 'as' keyword
				{
					// Read until EOF or non-space character
					while((c = getc(fpInput)) != ')' && !feof(fpInput) )
						;					

					iCreateState = 4;
				}
			break;

			case 4:        // pcBuf Should hold the name of the view item being parsed
				if(s::scmp_ci_uc(pcBuf, "as") == 0)
					return true;
				else
					return false; //Unknown object definition return error!
			}
		}

		iRdSz = 0;        // Reset input buffer
		pcBuf[iRdSz] = 0;
	}

	if(feof(fpInput)) goto OnErrorExit;

	return true;

OnErrorExit:
		return false;
}

#define RD_BUFSZ 1024 //Size of Input read buffer 10k

//-_--_--_--_--_--_--_--_--_--_--_--_--_->
//
//Parse SQL PROC File and return data ...
//__-__-__-__-__-__-__-__-__-__-__-__-__->
#define _SRC "PARSE::ParsePROCFile"
CPROC *ParsePROCFile(const UCHAR *pcServer,const UCHAR *pcDB,const UCHAR *pcOwner,const UCHAR *pcName
                    ,CUSTR sSrcPath, const UCHAR *pcSrcFName,COpts *pPrOpts, CTABCOL & tbcTabs)
{
	if(pPrOpts==NULL) return NULL;
  CDP::Handle_Requi_Params(pcServer,pcDB,pcOwner,pcName, _SRC);

	FILE *fpTrace;
	CPROC *prcRet=NULL;
  UCHAR *pcBuf = NULL;
  CUSTR sPathFile;
  int iRefFlags=0;

	try
	{
		sPathFile = sSrcPath + pcSrcFName;

		CPITEM paPState; //State engine for parser states

		if((fpTrace = fopen(sPathFile, "rb")) != NULL) //READ SQL Source for parser
		{
			UCHAR *pcBuf = new UCHAR[RD_BUFSZ];
				int iBufSz = RD_BUFSZ;

			//Extract header information (object name, type ...and such)
			if((prcRet = ParseObjDef(pcServer,pcDB, pcOwner, pcBuf, iBufSz, pPrOpts->hKeyWords, fpTrace))!=NULL)
			{
				CUSTR sServer(pcServer), sDB(pcDB), sUser(pcOwner), sDBObj(pcBuf);
				CDEP::ParseRef2DBOBJ(sServer,sDB,sUser,sDBObj, iRefFlags);  //resolve access to db object

        prcRet->SetServer(sServer);  //Reset these references, usually
        prcRet->SetDB(sDB);         //default values apply, but it is
        prcRet->SetOwner(sUser);   //also possible they don't, so we make sure
				prcRet->SetName(sDBObj);
        prcRet->sSrcFile = pcSrcFName;

				if(prcRet->GetObjType() == eREFTRIG)
				{
					if(GetTriggerDef(sServer,sDB,sUser,pcBuf, iBufSz, fpTrace, prcRet)==false)
						goto OnErrorExit;   //On Error this returns empty opbject definition
				}

				//Parse remaining file and build one level callgraph for trigger/proc
				if(prcRet->GetObjType() == eREFTRIG || prcRet->GetObjType() == eREFPROC)
				{
					while(paGetNxtItem(pcBuf,iBufSz,fpTrace,paPState,*prcRet,pPrOpts,
					      tbcTabs, pcServer,pcDB)== true)
					{
						//Something useful found ... do somehting with it ...or don't !?!?!?
					}
					paPState.Reset(); //Free states engine from memory
				}
			}

			delete [] pcBuf;
			FCLOSE(fpTrace)
		}
		else
		{
			CUSTR s("ERROR: Can not open File: ");
			s += sPathFile;
			s += "' !!!";

			throw CERR(s);
		}
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: Parse::ParsePROCFile\n");
		return NULL;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR",0,_SRC);
	}

	return prcRet;

OnErrorExit:
  printf("ERROR: Unknown DB Object: %s\n", (const char *)sPathFile);
	printf("CodeRef: Parse::ParsePROCFile\n");

  FCLOSE(fpTrace)
	delete  [] pcBuf;
	FREE_OBJ(prcRet);

	return NULL;
}
#undef _SRC

//-_--_--_--_--_--_--_--_--_--_--_--__-__-_-_-__-_--_->
//
//Parse SQL View definition file and return data ...
//__-__-__-__-__-__-__-__-__-__-__-_-__-__-_-_-__-_-__->
#define _SRC "PARSE::ParseViewFile"
void ParseViewFile(const CUSTR sFPath, const CUSTR sFName //Input Path and File-name
                  ,CTAB & tblView
                  ,COpts *pPrOpts
                  ,CTABCOL & tbcTabs
                  ,const UCHAR *pcServer,const UCHAR *pcDB
                  )
{
	if(pPrOpts==NULL) return;

	FILE *fpTrace;
  UCHAR *pcBuf = NULL;

  CDP::Handle_Requi_Params(pcServer,pcDB,tblView.GetOwner(),tblView.GetName(), _SRC);

	try
	{ //Build complete path to file & open it
		CUSTR sPathFile = s::sGetPath(sFPath) + sFName;

		CPITEM paPState; //State engine for parser states
		CUSTR sViewName;

		if((fpTrace = fopen(sPathFile, "rb")) != NULL)
		{
			UCHAR *pcBuf    = new UCHAR[RD_BUFSZ];
				int iBufSz    = RD_BUFSZ;
			tblView.sSource = sPathFile;

			if(ParseViewDef(pcBuf,iBufSz,pPrOpts->hKeyWords,fpTrace,sViewName,tblView)==true)
			{
				//Build a callgraph for trigger or procedures
				if(tblView.GetObjType() == eREFVIEW)
				{
					while(paGetNxtViewItem(pcBuf,iBufSz,fpTrace,paPState,tblView,pPrOpts,
					      tbcTabs, pcServer, pcDB)== true)
					{
						//Something useful found ... then do somehting with it ...or don't !?!?!?
					}
						paPState.Reset(); //Free states engine from memory
				}
			}
			else //Throw error if parser does not work for this SQL code
			{
				FCLOSE(fpTrace)
				delete  [] pcBuf;
				CUSTR s("Parse::ParseViewFile Unknown db object in file: '");
				s += sPathFile;
				s += "' !!!";
				throw CERR(s);
			}

			delete  [] pcBuf;
			FCLOSE(fpTrace)
		}
		else  //Throw error, if source for SQL was not found...
		{
			FCLOSE(fpTrace)
			delete  [] pcBuf;
			CUSTR s("Parse::ParseViewFile WARNING: Can not open View definition file: ");
			s += sPathFile;
			s += "' !!!";

			throw CERR(s);
		}
	}
	catch(CERR e)
	{
		printf("ERROR: %s\n", (const char *)e.getMsg());
		printf("CodeRef: Parse::ParseViewFile\n");
		return;
	}
	catch(...) //Progress Error and throw to caller
	{
		throw CERR("UNKNOWN ERROR ooccured in Parse::ParseViewFile!!!\n");
	}

	return;
}

#undef _SRC
#undef RD_BUFSZ

int Convert2KB(CUSTR sSz, CUSTR sPart)
{
	int iConv;

	iConv = atoi((const char *)sSz);             //Nothing to convert, just return
	if(sPart.scmp_ci((const UCHAR *)"kb")==0) return iConv;
	else
	//convert from MB 2 KB
	if(sPart.scmp_ci((const UCHAR *)"mb")==0) return (int)(iConv/1024);
	else
	//convert from MB 2 KB
	if(sPart.scmp_ci((const UCHAR *)"gb")==0) return (int)(iConv/1024/1024);
	else
	//convert from MB 2 KB
	if(sPart.scmp_ci((const UCHAR *)"tb")==0) return (int)(iConv/1024/1024/1024);

	return -1;
}

CUSTR ConvertKB2UsefulSz(int iKBSz)
{
	char cBuf[40];

	if(iKBSz < 512)
		sprintf(cBuf, "%i KB", iKBSz);
	else
	if(iKBSz < 512*1024)
		sprintf(cBuf, "%.2f MB", (((float)iKBSz)/1024));
	else
	if(iKBSz < 512*1024*1024)
		sprintf(cBuf, "%.2f GB", (((float)iKBSz)/1024/1024));
	else
	//if(iKBSz < 512*1024*1024*1024)
		sprintf(cBuf, "%.2f TB", (((float)iKBSz)/1024/1024/1024));

	return CUSTR(cBuf);
}
