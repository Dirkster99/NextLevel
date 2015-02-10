
This folder contains mainly 2 helpful PHP scripts for building a Nextlevel import
file structure. The Scripts are:

1> LiftOffDB.php
2> GenIdx.php

You need LiftOffDB.php only if you have no other way of downloading catalog information
from your database server. Use GenIdx.php if you changed the sample files for the sample
database project ZEUS/TITAN and want to rebuild HTML files.

The GenIdx.php script is required to build the XML import file for Nextlevel. The XML
import file stores additional program options and contains pointers to the files that
contain SQL code for procedures and triggers (see details below).

1> liftoffdb.php
----------------

Liftoffdb.php requires an include file that supplies the login and password
for the database server. Keep this file at a safe place or delete and recreate
it each time you run the script. The contents of the 'DBConn.inc' file are the
following:

<?php

  $DB_USER   ="MyLogin";   //Put name of Database Login here
  $DB_PASSWRD="shhht!";
?>


Required tools for using liftoffdb.php against Sybase ASE 12.5 Server are:

* Dos2Unix.exe     (you need this, if server and client run a different OS)
* isql.exe         (or any other command line tool to access your database)

The liftoffdb.php script requires Dos2Unix.exe (if you are on Windows, and
download from a Unix DB Server). The isql tool accepts as input an ASCII file
containing SQL code and outputs results as ASCII files. Make sure this tool
is available in the path variable.

2> GenIdx.php
-------------

The GenIdx script accepts a set of SQL files, 2 ASCII formated index files,
and information about the database name, its owner, and server name as parameter.

