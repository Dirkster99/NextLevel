
//This class stores and resolve XML document type declarations
//

#ifndef DTD_H
#define DTD_H

#include "ctag.h"
#include "custr.h"
#include "cstrm.h"

#include "chartab.h"

class CTAG; //compiling RawTag and DTD

//DocType declaration can be internal or external
//RDT_INTERNAL Internal doctypes end at ]>
//RDT_EXTERNAL External doctypes end at end of file
//
//Each type has different formatting in termination
//
typedef enum RDT{ RDT_INTERNAL=1, RDT_EXTERNAL=2};

class CDTD
{
public:
	CDTD();  //Constructor and destructor
	~CDTD();

	//Document Declaration was found
	//consume this decl and return to caller
	void ParseDOCTYPEDecl(CSTRM *pxsInp);

	int ResolveRefs(CUSTR & s);

private:
	void Reset();

	CTAG *ProcessDTDTag(CSTRM *pxsInp); //Process CDTD Tag from stream and obj

	//Parse a doctype declaration that is stated internally in an XML doc
	//
	void ParseDocType(const RDT & rdtType, CSTRM *pxsInp);

	//This try is used to store and resolve references such as '&gt;' or '&lt;'
	TNODE *ptrRooRefs;
	int      iRefMaxSz; //Length of largest reference stored in TRIE
};

#endif

