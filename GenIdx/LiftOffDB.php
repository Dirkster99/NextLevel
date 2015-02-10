<?php

  /*
  */

  /* Use this file to download information from an SQL server database.
   * This information can then later be converted into XML and be imported to NextLevel
   *
   * See USAGE section for required parameters
   * Requirements:
   * - isql installed and configured in below in the $QUERY_TOOL variabale
   * - login with select permission on database catalog tables of target database
   *
   * Refer to the nextlevel sample database project ZEUS/TITAN for more details.
   *
   * This php script has been developed on Windows
   */

  //Use DBConn.inc file to store user account and password <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  //Refer to ReadMe.txt for contents of this file (You need to create this on 1st time use)
  require 'Q:\DBConn.inc';

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN

  if ($argc < 5 || in_array($argv[1], array('--help', '-help', '-h', '-?')))
  {
    print "USAGE:\n";
    print "------\n";
    print "\n";

    print "Use this batch file to generate an XML index file in NextLevel format:\n";
    print "\n";
    print "arg[1] with Name of Database Server (e.g. ZEUS)\n";
    print "arg[2] with Name of Database (e.g. TITAN)\n";
    print "arg[3] with Name of Database Owner (e.g. dbo)\n";
    print "arg[4] target directory for downloaded SQL files\n";
    print "\n";

    exit(254);
  }

  $SERVER    = $argv[1];    //Copy Base parameters and go...
  $DB        = $argv[2];   //
  $OWNER     = $argv[3];  //
  $SQLDirOut = $argv[4]; //

  $path_parts    = pathinfo($_SERVER['SCRIPT_FILENAME']); //Find out where Script runs
  $HerePath      = "{$path_parts["dirname"]}/";          //

  $QUERY_TOOL    = 'isql';                              //Query tool to be used
  $sTMPQueryFile = "{$SQLDirOut}exec.sql";             //query file being executed against SERVER.DB
  $ResultSet     = "{$SQLDirOut}query_result.txt";    //temporary file for query results
  $PROC_IDX      = "{$SQLDirOut}idx.txt";            //Index of procedures and triggers
  $TAB_IDX       = "{$SQLDirOut}TabCat.txt";        //Index of Table/View Catalog 
  $TAB_CAT_IDX   = "{$SQLDirOut}TabCatIdx.txt";    //Table/View Catalog in plan text format
  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


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
      
      fprintf($fpOutHandle, "\n");  //This just for readability
      fclose($handle);
      fclose($fpOutHandle);
    }
  
    if($iState <= 0) //No result set, not even an empty one, so we exit
      exit("\n\n::ParseRAWSQL ERROR: No data found in RAW input file: $InpFile State: {$iState}\n\n\n");
  }

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //Download index ASCII file from Database SQL Server >>>>>>>>><<<<<<<<<<<<<<<<<<<
  print "Creating index of procedures...\n";

  if (!$handle = fopen($sTMPQueryFile, 'w'))
    exit("ERROR >>>PROC_IDX Cannot open file {$sTMPQueryFile}");

  // Write content to our opened file.
  if(fwrite($handle, "USE {$DB}\ngo\n\n") == FALSE)
    exit("ERROR >>>PROC_IDX Cannot write to file: $sTMPQueryFile");

  fprintf($handle, "set nocount on --Disable message (x rows affected)\n");
  fprintf($handle, ' select distinct so.name, so.type, su.name, so.crdate, so.loginame');
  fprintf($handle, ' from sysobjects so, sysusers su');
  fprintf($handle, ' where');
  fprintf($handle, '  (    so.type in ("TR", "V")');         //Get all Triggers and Views
  fprintf($handle, '       and so.uid = su.uid');
  fprintf($handle, '  )or');
  fprintf($handle, '  ( so.type in ("P")');
  fprintf($handle, '    and so.name not like "%%_tmp%%"');  // Filter Temporary Procedures
  fprintf($handle, '    and so.uid = su.uid)');
  fprintf($handle, "\ngo\n");
  //--order by type, name
  fclose($handle);

  //Run query against database and proces results below...
  `$QUERY_TOOL -S{$SERVER} -U{$DB_USER} -P{$DB_PASSWRD}  -w2500 -i"{$sTMPQueryFile}" -o"{$PROC_IDX}"`;

  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //Download index ASCII file from Database SQL Server >>>>>>>>><<<<<<<<<<<<<<<<<<<
  print "Creating index of tables/views...\n";

  if (!$handle = fopen($sTMPQueryFile, 'w'))
    exit("ERROR >>>TAB_IDX Cannot open file {$sTMPQueryFile}");

  // Write content to our opened file.
  if(fwrite($handle, "USE {$DB}\ngo\n\n") == FALSE)
    exit("ERROR >>>TAB_IDX Cannot write to file: $sTMPQueryFile");

  // Get a listing of all tables in the db
  fprintf($handle, "set nocount on --Disable message (x rows affected)\n");
  fprintf($handle, ' select "INFO"    as "table_qualifier"');
  fprintf($handle, '         ,su.name as "table_owner"');
  fprintf($handle, '         ,so.name as "table_name"');
  fprintf($handle, '         ,case so.type when "U" then "TABLE"');
  fprintf($handle, '                       when "V" then "VIEW"');
  fprintf($handle, '                       else so.type end as "table_type"');
  fprintf($handle, '   from sysobjects so, sysusers su');
  fprintf($handle, '  where so.type in ("U", "V")');
  fprintf($handle, '    and su.uid  = so.uid');
  fprintf($handle, "\ngo\n");
  fclose($handle);

  `$QUERY_TOOL -S{$SERVER} -U{$DB_USER} -P{$DB_PASSWRD}  -w2500 -i"{$sTMPQueryFile}" -o"{$TAB_IDX}"`;
  
  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  // Get each table/View SQL Definition and stick into a plain ASCII File
  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  $iState       = 0;
  $iCntSQLFiles = 0;

  // In our example we're opening $filename in write mode.
  if (!$fpQuery = fopen($sTMPQueryFile, 'w'))
    exit("ERROR >>GetTABIDX Cannot open file {$sTMPQueryFile}");

  // Write content to our opened file.
  if(fwrite($fpQuery, "USE {$DB}\ngo\n\n") == FALSE)
    exit("ERROR >>GetTABIDX Cannot write to file: {$sTMPQueryFile}");

  if(file_exists($TAB_IDX) && is_readable($TAB_IDX))
  {
    if(filesize($TAB_IDX) <= 0)
      exit("\n\nERROR: ASCII INDEX FILE : {$argv[6]} CONTAINS NO DATA.\n\n\n");

    $handle = fopen($TAB_IDX, "r");
    while (!feof($handle))
    {
      $buffer = fgets($handle, 4096);
      $chars  = preg_split('/ /', $buffer, -1, PREG_SPLIT_NO_EMPTY);

      //this state should hold data so we attempt to process it or exit processing right here
      if($iState == 2)
      {
        if(strlen($chars[0]) > 0 && strlen($chars[1]) > 0
        && strlen($chars[2]) > 0 && strlen($chars[3]) > 0)
        {
          //Here we process each reference to a Table/View object to find:
          //its size and column definitions
          list($sTag, $sDBObjOwner, $sDBObj, $sDBObjType) = $chars;

          if($sTag != "INFO") continue;

          //PRINT "$sDBObjOwner.$sDBObj\n";
          fprintf($fpQuery,"set nocount on --Disable message (x rows affected )\n");
      
          if($sDBObjType == "TABLE" ||$sDBObjType == "VIEW")
          {
            if($sDBObjType == "TABLE")
            {
              fprintf($fpQuery,"print 'BEGIN_SPACE $sDBObjType {$sDBObjOwner} {$sDBObj}'\n");
              fprintf($fpQuery,"exec sp_spaceused '{$sDBObjOwner}.{$sDBObj}'\n");
              fprintf($fpQuery,"print 'END_SPACE $sDBObjType {$sDBObjOwner} {$sDBObj}'\n");
            }
      
            fprintf($fpQuery,"print 'BEGIN $sDBObjType {$sDBObjOwner} {$sDBObj}'\n");
            fprintf($fpQuery,"exec sp_columns '{$sDBObj}', '{$sDBObjOwner}'\n");
            fprintf($fpQuery,"print 'END $sDBObjType {$sDBObjOwner} {$sDBObj}'\n");
          }
      
          fwrite($fpQuery, "go\n\n"); //Write final go for each query to keep results small

          $iCntSQLFiles = $iCntSQLFiles + 1;
          continue;
        }
        else $iState = 3;
      }

      if($iState == 1)
      {
        //this state is table delimiter line '--- ---' => which we skip unseen
        $iState = $iState + 1;
        continue;
      }

      if($iState == 0
         && strcasecmp($chars[0],"table_qualifier") == 0
         && strcasecmp($chars[1],"table_owner")     == 0
         && strcasecmp($chars[2],"table_name")      == 0
         && strcasecmp($chars[3],"table_type")      == 0)
      {
        //next state is table delimiter line '--- ---' => which we skip
        $iState = $iState + 1;
        continue;
      }
    }
    fclose($handle);

    if($iCntSQLFiles <= 0 && iState == 0) //No result set, not even an empty one, so we exit
      exit("\n\nERROR: No data found in ASCII index file: $TAB_IDX State: $iState\n\n\n");
  }
  else
    exit("\n\nERROR: Can not open plain ASCII index file: $TAB_IDX\n\n\n");

  // >>> Query all table objects that are replicated away from here
  fprintf($fpQuery,"set nocount on --Disable message (x rows affected)\n");
  fprintf($fpQuery,"PRINT \"BEGIN_SRC_REP_TABLES\"\n");
  fprintf($fpQuery,"select \"INFO\"    as \"table_qualifier\"\n");
  fprintf($fpQuery,", user_name(o.uid) as \"table_owner\"\n");
  fprintf($fpQuery,"  , o.name as \"table_name\"\n");
  fprintf($fpQuery," from sysobjects o, sysusers su\n");
  fprintf($fpQuery,"     ,master.dbo.spt_values v, master.dbo.spt_values x, master.dbo.sysmessages m\n");
  fprintf($fpQuery,"where o.type in (\"U\") --, \"V\") -- looking for user tables only\n");
  fprintf($fpQuery,"  and o.sysstat & -32768 != 0\n");
  fprintf($fpQuery,"  and su.uid  = o.uid\n");
  fprintf($fpQuery,"  and o.sysstat & 2063 = v.number\n");
  fprintf($fpQuery,"  and ((v.type = \"O\" and o.type != \"XP\") or\n");
  fprintf($fpQuery,"       (v.type = \"O1\" and o.type = \"XP\"))\n");
  fprintf($fpQuery,"  and v.msgnum = m.error\n");
  fprintf($fpQuery,"  and isnull(m.langid, 0) = 0              -- @sptlang\n");
  fprintf($fpQuery,"  and m.error between 17100 and 17199\n");
  fprintf($fpQuery,"  and x.type = \"R\"\n");
  fprintf($fpQuery,"  and o.userstat & -32768 = x.number\n");
  fprintf($fpQuery,"PRINT \"END_SRC_REP_TABLES\"\n");
  fprintf($fpQuery,"go\n");

  fclose($fpQuery);

  print "{$iCntSQLFiles} Tables objects found.... querying DDL (table size and column definition)\n";
  //Get index of tables/views (this inclused column definitions and size of indexes and data)
  `$QUERY_TOOL -S{$SERVER} -U{$DB_USER} -P{$DB_PASSWRD}  -w2500 -i"{$sTMPQueryFile}" -o"{$TAB_CAT_IDX}"`;
  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
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

        $chars = preg_split('/ /', $buffer, -1, PREG_SPLIT_NO_EMPTY);

        //this state should hold data so we attempt to process it
        //or exit processing right here
        if($iState == 2)
        {
          if(strlen($chars[0]) > 0 && strlen($chars[1]) > 0
          && strlen($chars[2]) > 0)
          {
            //Here we process each reference to a database object and download proc/tric/view defs
            list($sDBObj, $sDBObjType, $sDBObjOwner) = $chars;
  
            printf("{$iCntSQLFiles}> DBOBJ: $sDBObjOwner.$sDBObj TYPE: $sDBObjType\n");
            GetProc($sTMPQueryFile, $DB, $sDBObj, $sDBObjOwner);

            `$QUERY_TOOL -S{$SERVER} -U{$DB_USER} -P{$DB_PASSWRD}  -w2500 -i"{$sTMPQueryFile}" -o"{$ResultSet}"`;

            //Post processing step is required, because
            //1> Line format in database is not as the oeiginal format
            //2> Control sequences need to be translated if DB Server is UNIX and Client is Windows (or vice versa)
            //Output file to directory Out/<Owner>_<Object-Name>.sql
            `dos2unix {$ResultSet} > NUL`;
            ParseRAWSQL($ResultSet, "{$SQLDirOut}{$sDBObjOwner}_{$sDBObj}.sql");

            if(filesize("$SQLDirOut{$sDBObjOwner}_{$sDBObj}.sql") <= 0)
            {
              print "ERROR: FILE \"$SQLDirOut{$sDBObjOwner}_{$sDBObj}.sql\" has length of 0!";
              exit;
            }

            $iCntSQLFiles = $iCntSQLFiles + 1;
            continue;
          }
          else $iState = 3;
        }

        if($iState == 1)
        {
          //this state is table delimiter line '--- ---' => which we skip unseen
          $iState = $iState + 1;
          continue;
        }

        //This is the first line of query result, we match this and change into next state
        if($iState == 0 && strcasecmp($chars[0],"name") == 0
        && strcasecmp($chars[1],"type") == 0
        && strcasecmp($chars[2],"name") == 0)
        {
          //next state is table delimiter line '--- ---' => which we skip
          $iState = $iState + 1;
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
