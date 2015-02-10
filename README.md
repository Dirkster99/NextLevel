
# Relational Database Tool NextLevel V2.0

## Introduction

NextLevel is a relational database tool for those who work with SQL
projects that encompass many procedures, triggers and of course
views and tables. Nextlevel generates a set of HTML files which
are similar to those generated by javadoc or doxygen. EACH SQL
procedure or trigger is parsed and checked for dependencies,
-each unique dependency is noted with a link at the head of the
file and within the source code. Simply clicking on an item of
reveals the definition and dependencies of that database object
and one is thus, able to browse the database catalog and analyse
its technical aspects rather than the browsing the data of the DB.

NextLevel is thus, particularly useful if your database project includes
a lot of SQL program source code, while it is not so useful if your
project consists of tables and views only.

### Review Demo

Download the zip container and have a look at a sample implementation:

- NextLevel/TITAN/TITAN_HTML/index.html
- INTERFACE/INTERFACE_HTML/index.html

to get a basic understanding of the results that can be produced with this tool.

The implemented demo sample assumes that you have 2 database servers called
**SUN** and **ZEUS**. There is a database called **INTERFACE** on **SUN** and
and a database called **TITAN** on **ZEUS**. Both are somehow linked through
their SQL processing logic and this tool can be used to visualize this on the
**Connections** page which is available from the top-right corner menu on each
page.

Each page inside each database documents the dependencies to other database objects
and lists the source code which can be used to travel to the next database object
in a matter of a click.

Be sure to check out all pages and Help/About information available in these
pages to answer questions about this project and what it does.

### Release Notes

- nextlevel can be build as release or debug version, the first is ment
  for production while the latter is useful for GDB & Co

- SQL 92 Syntax for INNER/OUTER JOIN is not supported
  (because its not implemented, yet)


### Building nextlevel on Windows (requires MinGW installed)

Change into nextlevel project folder and build project with:

mingw32-make -f Makefile.mingw32 CFG=release
mingw32-make -f Makefile.mingw32 CFG=debug

(This will create temporary object files in c:\tmp\nextlevel)


### Sample usage on Windows executed from within the nextlevel project folder

bin\nextlevel.exe -idx TITAN\TITAN_SQL\idx.xml  -DirOut TITAN\TITAN_HTML -WinHelp

(see nextlevel\TITAN\WinBuildHTML.bat for a detailed description)

This will create HTML files in the TITAN\TITAN_HTML output-directory.
To view these files, open your favourite browser and load:
nextlevel\TITAN\TITAN_HTML\index.html

The -WinHelp option will generate header files for the Microsoft Help Workshop,
this tool can be used to nicely bundle all HTML files in one container.

### Building nextlevel on Linux

Before building nextlevel create a tempory file directory
(or edit OBJ_DIR in Makefile.linux):

/tmp/nextlevel/release
/tmp/nextlevel/debug

Change into nextlevel project folder and build project with:

make -f Makefile.linux CFG=release
make -f Makefile.linux CFG=debug

### Using nextlevel on Linux

bin/nextlevel -idx TITAN/TITAN_SQL/idx.xml  -DirOut TITAN/TITAN_HTML

This will create HTML files in the TITAN/TITAN_HTML output-directory.
To view these files, open your favourite browser and load:
nextlevel/TITAN/TITAN_HTML/index.html

### Change Log

---
Monday, 26. June 2006
           BugFix
           -> Minor change, added 2 HTML TAGS:
           
           
           in every generated <HEADER> Tag to prevent a browser from caching
           generated HTML Tags. This is a useful feature for using the generated
           pages on a web-server, because a web-browser may see an outdated (cached)
           page, while coding (and or db structure) may have changed since last
           visiting the page.

---
Wednesday, 22. March 2006
           BugFix
           -> Fixed Bug in CTABCOL::HTML_PrintViewAccess having to do with a
              view dependency linking to a view in an external database

---
Sunday, 5. March 2006
           Bug-Fix
           -> Parser Recovery from 'Truncate Table #TempTable' was faulty
              (We ignore references to temporary tables)
           -> Create Trigger Statement with "on DB .. TableName" was parsed
              incorrectly

---
Friday, 10. Februar 2006
           Adjusting HTML printouts to support CSS (Cascading Style Sheets)
           in all table printed.

---
Monday, 30. Januar 2006
           Database objects can now be classified, associated with an icon
           and a description (which is shown as hover text of the icon).

           You can verify this mechanism in the TITAN/INTERFACE sample databases
           through tables that are a source of replication. In particular,
           these samples are about tables classified as source of a replication
           rule via an outside replication mechanism such as a replication server.

---
Thursday, 17. Januar 2006
           BugFix in computation of linkage relative directories
           (multiple levels of relative links were computed in-correctly)

---
Wednesday, 28. December 2005
           Starting internal re-design: All database objects should be modelled
           with one universal approach, to implement this I am implementing a
           new class called CDP, which identifies a DB Object (or a reference to
           one) by its type, server.database.owner.ObjectName! There is also space
           for additional flags and class-specific gimics, but the idea is that
           ctab, cproc, and other database related classes will inherite from
           CDP and thus support the same way of identification all around the
           whole program :) phew, all objects are now owner and server compliant :)

---
05.12.2005 From now on I use two central terms - outbound and inbound dependencies -
           when talking about dependencies between database objects in detail.
           An outbound dependency of object x is a dependency that results from
           accessing an object y from within object x
           (e.g.: A stored procedur GetID that selects from a table GEN_ID
                  is said to have an outbound dependency on GEN_ID).

           An inbound dependency for an object x results from an object y
           accessing object x (see above example, here GEN_ID has an inbound
           dependency from GetID).

       >>> Bug Fix in CPRCCOL and TABCOL class, outbound dependencies are lost,
           if the owner between two dependent database objects is different
           (in above example GetID and GEN_ID belonging, for example to
           dbo.GetID and rs.GEN_ID).
           
           Such a dependence is not trivial to resolve, because dbo.GetID can
           hold, for example, a statement such as: SELECT * FROM GEN_ID
           and although, the statement is valid, and although it is clear what
           table is meant (if their is no other table with that name under any
           other user) the resolution is still difficult, mainly, because database
           objects arrive one by one at parsing time.

           The dependency algorithm works now such that all dependencies to db
           objects are initially pointed to those objects that have the same user
           as database object from which the dependency is outgoing. Later, when
           all objects are parsed, these assumed dependencies are checked, whether
           they point at a valid object, and whether there is a better resolution.
           The dependency is then reset when a better, but still unique resolution
           is possible. (Phew) NextLevel now fully supports database Objects with
           same names but different owner and resolving interdependencies between
           these things :)
           

---
04.12.2005 Bug Fix in CPRCCOL class, procedures with same name belonging to
           different owners are now stored in dependence of their Database,
           owner, and name (this far, only database and name were checked)

---
29.11.2005 Introduced CSERVER class to provide a mechanism for downloading
           and linking more than one database(s) sitting on several DB Servers
