
This project is simple XML parser, which can be used to extract
XML data from an XML file.

The parser is limited by its input buffer (default size 10k)
in XSTRM class. This means that no XML Tag ('< ... >') can be
longer than the buffer size since a buffer overflow will be
flagged otherwise (Err object is thrown).

This number does not apply to comments (which are ignored on
input) and leading white spaces in tag content. Note also for
tag content text ('<tag> ...tag content text... </tag>') - that
the text between two tags can not be longer than the input
buffer (minus trailing whitespaces).

TODO:

Log-History:

10 Feb, 2004

- A single XML file can now be searched with the XML parser :)
  and matches are printed with a translation to stdio

- Added a Dic class and a number of template classes in TList.h
  The template classes manage item references through one or two
  keys with simple linked list structures. The number of items in
  this structure is usually low (at most a few hundred) so linked
  lists are fine for this. We should use hash tables or splay trees
  if translations contain a lot more items. The TList.h file contains:

  - class KLst  A class that associates one key with one string
    This can be used to express the name of an application in
    a number of languages:
    Relationship is: <1>:1

    (key="en",Val="Dictionary"), (key="deu",Val="Wörterbuch")

  - class KKLst A class that associates two keys with one string
    This can be used to express the name of a language in a certain
    language:
    Relationship is: <1,1>:1

    (key1="en", key2="en", Val="English"), (key1="en", key2="deu", Val="German")
    (key1="deu", key2="deu", Val="Deutsch"), (key1="deu", key2="deu", Val="Englisch")

  - class KNLst A class that associates one key with a number
    of strings (This is how the transl tag is handled!):
    Relationship is: <1>:N

    (key1="en",  Val="translation", Val="translate", ...)
    (key1="deu", Val="übersetzung", Val="übersetzen", ...)


- The dic class manages language names, application names, and
  contents of transl tags (see ExtractTransl in idxbuild.c)

- UStr implements a simple input >> operator that can be used as
  sample code

28 Jan, 2004

- This version replaces the STL Stack with a custom class called
  Stak, which does not only stack'ify RawTags but keeps also track
  of state, state in turn can be used to identify the context of
  every XML tag :)

> Use stack for syntax check and propper XML processing (tick)

26 Jan, 2004

- This version uses an STL Stack container to accumulate tags and
  check their balance. The RawTag and PropBag classes did not have
  a copy constructor and equal operator :) - Make sure you have
  these for ALL objects that manage dynamic memory!!!

25 Jan, 2004

5> We use a set of the following classes now:

- UStr is a primitive class that handles unsigned char string
  manipulation function.

- Err is a class for Error objects that are used to propagate
  syntax error information and other run time problems to the
  outside world

- XSTRM is a simple file stream and read buffer class. This class
  handles an internal buffer which is also available for callers
  (used to accumulate strings when parsing). There is also a handy
  function called throwErr which will throw an Err object containing
  information about the current state (Line, Position etc.) of the
  input stream.

- PropBag is a simple class that handles XML tag properties. Each
  property has a name and a value, both have an index and can be
  freely set and reset durring the livetime of a PropBag object.

- The RawTag class handles XML Tag manipulation inside the running
  program. Tags are constructed from a UStr string and an optional
  typedescriptor. The string is parsed further if the descriptor
  was now set (or was set to TGP_UNKNOWN).
	
	Call RawTagTyp::getTagTyp() to determine what tag type the
	constructor of RawTag recognized. The constructor sets this type
	info to TGP_ERROR if the tag syntax was not aligned with the
	recognized type. The properties of a Tag are exposed via PropBag
  object inside of the RawTag class.

- DTD is a class that encapsulates XML Document Type Declaration
  This class has a function that will parse a DTD from an input
  stream - new '&' references are added to the current DTD decl!
	There is also a function for resolving string references such
	as &Auml; - This function is called from XPars::getNextRawTag()
	when parsing XML.

- The XPars class sits on top of the XSTRM, DTD and RawTag classes.
  XSTRM and DTD are used to parse new XML tags from input and RawTag
  is returned by XPars::getNextRawTag() to enable XML Tag processing.

5 Jan, 2004

1> XML tags are extracted and returned as raw strings in RawTag class
3> Simple error message system based on strings and iError is installed
2> Comments are skipped but syntax checked

4> Parse raw tags and identify names and types in RawTag class
   4.1 First character of tag name cannot be a white space or '<' or '>'
   4.2 Post-process tags to extract name and type from raw string
