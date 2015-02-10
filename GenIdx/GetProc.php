<?php
  /*
  */

  if ($argc < 4 || in_array($argv[1], array('--help', '-help', '-h', '-?')))
  {
    print "USAGE:\n";
    print "------\n";
    print "\n";

    print "Use this batch file to download procedural catalog information from your Sybase ASE Server:\n";
    print "\n";
    print "arg[1] with Name of Database Server (e.g. ZEUS)\n";
    print "arg[2] with Name of Database (e.g. TITAN)\n";
    print "arg[3] with Name of Index file containing Owner and object name (one pair seperated by space per line)\n";
    print "arg[4] target directory for downloaded SQL files (incl. final backslash!)\n";
    print "\n";

    exit(254);
  }

  //This is a php script that gives an idea of how catalog information with regard to
  //procedures and triggers can be downloaded from a Sybase ASE 12.5 database

  //The script accepts a file called 'Download.txt' as parameter,
  //in this file, there should be the name of the owner of a db object, and a name of a database object
  //(one pair per line)

  //This file requires dos2unix, isql, and of course php in your MS-DOS path variable

  //1> Use DBConn.inc file to store user account and password for database access <<<<<<<<<<<<<
  //   Refer to ReadMe.txt for contents of this file (You need to create this on 1st time use)
  //
  //Contents of DBConn.inc (required for login to database catalog access)
  //$DB_USER   ="MyLogin";
  //$DB_PASSWRD="shhht!";
  //
  require 'Q:\DBConn.inc';

  //2> Below, there is a section with all relevant variable settings:
  //Review all variable settings and configure to your needs
  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN

  $path_parts    = pathinfo($_SERVER['SCRIPT_FILENAME']); //Find out where Script runs,
  $HerePath      = "{$path_parts["dirname"]}/";          //so we can create a sub-dir

  //PARAMETER SECTION PARAMETER SECTION PARAMETER SECTION PARAMETER SECTION PARAMETER
  //Query tool to be used for database access (make sure the path is set correctly)
  $QUERY_TOOL    = 'isql';

  //PARAMETER SECTION PARAMETER SECTION PARAMETER SECTION PARAMETER SECTION PARAMETER

  //temp_query file being executed against SERVER.DB (leave this alone)
  $sTMPQueryFile = "{$SQLDirOut}__exec.sql";
  //temporary file for query results (leave this alone)
  $ResultSet     = "{$SQLDirOut}__query_result.txt";

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  $SERVER    = $argv[1];    //Copy Base parameters and go...
  $DB        = $argv[2];   //
  $PROC_IDX  = $argv[3];  //
  $SQLDirOut = $argv[4]; //

  //$SERVER        = "ZEUS";         //Name of database server
  //$DB            = "TITAN";        //Name of database on that server
  //$PROC_IDX      = "Download.txt"; //Index File of procedure and trigger names (one per line)
  //(Sub)-dir to download SQL source files into
  //(Script creates dir and runs only, if dir does not exist, yet)
  //$SQLDirOut     = "..\\..\\Download\\";

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //Write a query into file to download SQL source from database server
  //Prepand a use <DB> and go statement to each file being executed against the database
  //Also, write required code to list proc/trig definition from DB into a file
  function GetProc($sQueryFile, $sDataBaseName, $PROC_OBJ, $sOwner)
  {
    //We're opening outputfile in write mode
    if (!$handle = fopen($sQueryFile, 'w'))
      exit("ERROR ::GetProc Cannot open file $sQueryFile\n");

    // Write content to our opened file.
    if(fwrite($handle, "USE {$sDataBaseName}\ngo\n\n") == FALSE)
      exit("ERROR: in ::GetProc Cannot write to file: $sQueryFile");

    fprintf($handle,"set nocount on --Disable message (x rows affected )\n");
    fprintf($handle,"declare @sText varchar(1024), @sName varchar(64), @iCnt int, @sOWNER varchar(64)\n");
    fprintf($handle,"select @iCnt = 1, @sName=\"{$PROC_OBJ}\", @sOWNER=\"{$sOwner}\"\n");

    fprintf($handle,"select @sText = c.text\n");
    fprintf($handle,"  from sysobjects o, syscomments c, sysusers su\n");
    fprintf($handle," where o.name = @sName\n");
    fprintf($handle,"   and o.id = c.id\n");
    fprintf($handle,"   and c.colid=@iCnt\n");
    fprintf($handle,"   and o.uid  = su.uid\n");
    fprintf($handle,"   and su.name = @sOWNER\n");
    fprintf($handle," \n");
    fprintf($handle," while (@@rowcount > 0)\n");
    fprintf($handle," begin\n");
    fprintf($handle,"    print \"%%1! __END_OF__LINE__TAG__\",@sText\n");
    fprintf($handle,"\n");
    fprintf($handle,"    select @iCnt = @iCnt + 1\n");
    fprintf($handle,"\n");
    fprintf($handle,"    select @sText = c.text\n");
    fprintf($handle,"     from sysobjects o, syscomments c, sysusers su\n");
    fprintf($handle,"     where o.name = @sName\n");
    fprintf($handle,"       and o.id = c.id\n");
    fprintf($handle,"       and c.colid=@iCnt\n");
    fprintf($handle,"       and o.uid  = su.uid\n");
    fprintf($handle,"      and su.name = @sOWNER\n");
    fprintf($handle," end\n");

    fwrite($handle, "\n\ngo\n\n"); //Write final go
    fclose($handle);
  }

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //This function is needed, because Sybase ASE 12.5 stores procedural code in a special
  //format where lines are SPLIT on a certain length of byte sequences (eg. 255 Bytes)
  //Also, my isql version does not support selected columns longer than 255 characters.
  //To work around this, we use a special query (see above in GetProc) where we insert a
  //marker at each 'end of Sybase ASE line', The next function ParseRAWSQL looks for this
  //marker and unsplits the file back to its original human-readable format
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  function ParseRAWSQL($InpFile   //File to parse raw sql data from
                      ,$OutFile
                      )           //Return string with parsed and readable SQL Source
  {
    $iCnt    = 0;
    $iState = -2;

    if (!$fpOutHandle = fopen($OutFile, 'w'))
    {
       echo "::ParseRAWSQL ERROR: Cannot open output file ($OutFile) for writting.";
       exit -3;
    }

    if(file_exists($InpFile) && is_readable($InpFile))
    {
      if(filesize($InpFile) <= 0)
        exit("\n\nERROR: ASCII INDEX FILE : {$InpFile} CONTAINS NO DATA (Or is none existent).\n\n\n");
  
      $iState = -1;
      $bNext=0;
      $s="";
      $handle = fopen($InpFile, "r");
      while (!feof($handle))
      {
        $RawBuffer = fgets($handle, 4096);
        //Cut off additional line feed at end of line and do an awk like split
        $buffer = substr($RawBuffer, 0, strlen($RawBuffer)-1);
        $chars  = preg_split('/ /', $buffer, -1, PREG_SPLIT_NO_EMPTY);

       if(trim(end($chars)) == "__END_OF__LINE__TAG__")
        {
          if($bNext == 0)
          {
            $_t = substr($buffer,0,strlen($buffer)-22);
            $s  = "{$s}\n{$_t}";
          }
          else
          {
            $_t = substr($buffer,0,strlen($buffer)-22);
            $s  = "{$s}{$_t}";
          }
          $bNext = 1;
        }
        else
        {
          if($bNext == 0)
            $s = "{$s}\n{$buffer}";
                else
            $s = "{$s}{$buffer}";

          $bNext = 0;
        }

        if(strlen($s)>0) $iState = 1;

        fprintf($fpOutHandle, "%s",$s);
        $s="";
      }
      fclose($handle);
      fclose($fpOutHandle);
    }
  
    if($iState <= 0) //No result set, not even an empty one, so we exit
      exit("\n\n::ParseRAWSQL ERROR: No data found in RAW input file: $InpFile State: {$iState}\n\n\n");
  }

  if(file_exists($SQLDirOut))
    exit("Can not create sub-directory: {$SQLDirOut} (Rename or Remove existing file or directory).");

  if(mkdir("{$SQLDirOut}", 0700) == false)
    exit("Can not create sub-directory: {$SQLDirOut}.");
  
  $iState       = 0;
  $iCntSQLFiles = 0;

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  // Get each procedure/Trigger SQL Definition and stick it into a seperate file
  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  if(file_exists($PROC_IDX) && is_readable($PROC_IDX))
  {
    if(filesize($PROC_IDX) <= 0)
    {
      print "\n\nERROR: ASCII INDEX FILE : {$PROC_IDX} CONTAINS NO DATA.\n\n\n";
      exit -3;
    }

    $handle = fopen($PROC_IDX, "r"); //Start processing index of procedures/triggers/views
    while (!feof($handle))
    {
      $buffer = fgets($handle, 4096);
      $chars  = preg_split('/ /', $buffer, -1, PREG_SPLIT_NO_EMPTY);

      list($sOwner, $sDBObj) = $chars;
      $sDBObj                = trim($sDBObj);
      $sOwner                = trim($sOwner);

      //this state should hold data so we attempt to process it
      if(strlen($sDBObj) > 0)
      {
        //Here we process each reference to a database object and download proc/tric/view defs
        //list($sDBObj, $sDBObjType, $sDBObjOwner) = $chars;

        printf("{$iCntSQLFiles}> Downloading {$sDBObj}\n");
        GetProc($sTMPQueryFile, $DB, $sDBObj, $sOwner);

        `$QUERY_TOOL -S{$SERVER} -U{$DB_USER} -P{$DB_PASSWRD}  -w2500 -i"{$sTMPQueryFile}" -o"{$ResultSet}"`;

        //Post processing step is required, because
        //1> Line format in database is not as the original format
        //2> Control sequences need to be translated if DB Server is UNIX and Client is Windows (or vice versa)
        //Output file to directory Out/<Owner>_<Object-Name>.sql
        `dos2unix {$ResultSet} > NUL`;

        ParseRAWSQL($ResultSet, "{$SQLDirOut}{$sOwner}_{$sDBObj}.sql");

        if(filesize("{$SQLDirOut}{$sOwner}_{$sDBObj}.sql") <= 0)
        {
          print "ERROR: FILE \"{$chars}.sql\" has length of 0!";
          exit;
        }

        $iCntSQLFiles = $iCntSQLFiles + 1;
        continue;
      }
    }
    fclose($handle);

    if($iCntSQLFiles <= 0 && iState == 0) //No result set, not even an empty one, so we exit
    {
      print "\n\nERROR: No data found in ASCII index file: $PROC_IDX\n\n\n";
      exit -2;
    }
  }
  else
  {
    print "\n\nERROR: Can not open plain ASCII index file: $PROC_IDX\n\n\n";
    exit -2;
  }

?>
