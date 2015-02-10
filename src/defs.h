
#ifndef DEFS_328593764857345
#define DEFS_328593764857345

#define TOOL_NAME     "NextLevel"
#define TOOL_BUILD    51
#define SOFT_VERSION  "2.1"

//Size of a integer number as string (char) representation
#define NUM_SZ 15

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

//#ifndef WIN32
//#ifndef _bool
//	#define _bool
//	typedef unsigned int bool;
//#endif
//
//#ifndef _BOOL
//	#define _BOOL
//	typedef unsigned int BOOL;
//#endif
//#endif

#ifndef _UINT
typedef unsigned int UINT;
#define _UINT
#endif

#ifndef _UCHAR
typedef unsigned char UCHAR;
#define _UCHAR
#endif

#ifndef _ULONG
	#define _ULONG
	typedef unsigned long ULONG;
#endif

#ifndef FCLOSE
#define FCLOSE(f) if(f!=NULL){fclose(f); f=NULL;}
#endif

#ifndef FREE_PTR
#define FREE_PTR(f) if(f!=NULL){free(f); f=NULL;}
#endif

/*This Macro returns true if a Flag (bit) is set, or false otherwise*/
#ifndef FLAG
	#define FLAG(lMask, _FLAG) ((lMask & _FLAG) != 0)
#endif

/*This Macro returns true if a Flag (bit) is NOT set, or false ...  */
#ifndef NFLAG
	#define NFLAG(lMask, _FLAG) ((lMask & _FLAG) == 0)
#endif

/*This Macro returns true if a Flag (bit) is set, or false otherwise*/
#ifndef SETFLAG
	#define SETFLAG(lMask, _FLAG) (lMask = (lMask | _FLAG))
#endif

/* This Macro unsets a bit in a mask */
#ifndef UNSETFLAG
	#define UNSETFLAG(lMask, _FLAG) (lMask = (lMask & (~_FLAG)))
#endif

//C++ Macros
#ifndef FREE_OBJ
#define FREE_OBJ(f) if(f!=NULL){delete f; f=NULL;}
#endif

//APPLICATION SPECIFIC STUFF >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Definitions for HTML/CSS COLOR STYLES
//These definitions are used to map color highlightings from HTML into
//a CSS Style file via '<SPAN class="..."> </SPAN>' HTML-commands
//===================================================================>
#define CSS_CMT  "cmt"   //ANSI-C and SQL Style comment highlighting
#define CSS_STR  "str"  //SQL string highlighting ' ... ' and " ... "

// SQL keyword highlighting style (e.g.: SELECT, INSERT INTO etc...)
#define CSS_KWD "kwd"

// SQL procedure name highlighting style  (e.g.: exec <procedure_name>)
#define CSS_PNM "pnm"

// Default color for any object that falls outside of
//the normal classification (This is normally not used at all,
//but available as fall back option for highlighting of unknown objects)
#define CSS_DFLT "dflt"

#define CSS_VIEW  "viw"     //Color highlighting for references to views
#define CSS_TABLE  "tbl"   //Color highlighting for references to tables

// SQL cursor name highlighting style
//(e.g.: declare <cursor_name>) cursor for ... for read only
#define CSS_CURSOR "crsr"

// Color highlighting for references to unknown tables
// This is for references to tables outside of the database
// or tables that do not exist anymore (or never did)
#define CSS_UNKNOWN_TABLE "utbl"
#define CSS_UNKNOWN_PROC  "uproc"

//Some HTML TAG Definitions
#define DOCTYPE_TAG "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"

#endif
