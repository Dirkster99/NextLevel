<?php

  /* Use This file to convert information downloaded from an SQL server database
   * into XML file format. The script expects 2 ASCII formated files, here called:
   *
   * tabcat.txt  -> contains information about table column design and space used
   * idx.txt     -> contains a list of all database objects , owners, and optional information
   *
   * Refer to the comments below and TITAN_SQL/idx.txt and TITAN_SQL/tabcat.txt
   *          in the nextlevel sample database project ZEUS/TITAN for more details
   *
   * This php script has been developed on Windows
   */

  //Check input parameter
  if ($argc < 5 || in_array($argv[1], array('--help', '-help', '-h', '-?')))
  {
    print "USAGE: php Genidx.php arg 1 2 3 4 5\n";
    print "------\n";
    print "\n";

    print "Use this batch file to generate an XML index file in NextLevel format:\n";
    print "\n";
    print "Set arg[1] with filename to extract procedure catalog information from (e.g. index.txt)\n";
    print "Set arg[2] with filename to extract table catalog information from (e.g. TabCat.txt)\n";
    print "\n";
    print "Set arg[3] with Name of Database Server (e.g. ZEUS)\n";
    print "Set arg[4] with Name of Database (e.g. TITAN)\n";
    print "Set arg[5] with Name of Database Owner (e.g. dbo)\n";
    print "\n";
    print "(Optional)\n";
    print "Set arg[6] with Name of XML ROOT FILE (define optional parameters in NextLevel)\n";
    print "Set arg[7] with Name of XML FOOT FILE (define optional parameters in NextLevel)\n";

    exit -1;
  }

  //Convert ASCII Table index file into XML Nextlevel index format
  //Script expects to see a file with something like the following (with more lines though)
  //
  // name                           type name                           crdate                     loginame                      
  // ------------------------------ ---- ------------------------------ -------------------------- ------------------------------
  // p_statistic_relevance          P    dirk                                  Aug 22 2005  6:18PM fritz                         
  // p_statistic_irrelevance        P    peter                                 Aug 22 2005  6:18PM klaus                         
  // ...
  //
  // Column description:
  // 1 -> name is the name of a database object
  // 2 -> type is the type of the database object 'V' - View, 'P' - Procedure, 'TR' - Trigger
  // 3 -> name is the name of the owner of the object
  // 4 -> crdate is the date of creation
  // 5 -> loginame is the login name of the person who created an object
  //
  function GenXMLProcEntries($IdxFile, &$iProcCnt, &$iTrigCnt)
  {
    $State=0;
    $TabColPos    = array();
    $TabColLength = array();
    $TabColSz = 0;
    $EntryCnt=0;

    $iProcCnt=0;
    $iTrigCnt=0;

    //Generate XML pointer-information for procedures and triggers
    if(!file_exists($IdxFile) || !is_readable($IdxFile))
    {
      print "\n\nERROR: ::GenXMLProcEntries Cannot open ASCII index file: {$IdxFile} \n\n\n";
      exit -2;
    }

    $handle = fopen($IdxFile, "r");
    while (!feof($handle))
    {
      $buffer = fgets($handle, 4096);
      $chars = preg_split('/ /', $buffer, -1, PREG_SPLIT_NO_EMPTY);
  
      if($State == 0)                //&& $4 == "crdate" && $5 == "loginame")
    	if($chars[0] == "name" && $chars[1] == "type" && $chars[2] == "name")
      {
        $State = 1; //Enter next state
        continue;
      }
        
      if($State == 1) //parse table header line to find out how wide each column is
      {             //line looks something like this '------ ----- -----'
        $State   = 2;     //Enter next state
  
        $str=$buffer;
        $c=" ";
        for($i=0;$i<=strlen($str);$i++)
        {
          $cNext = substr($str, $i, 1);
          if ($cNext != $c && $cNext == "-") //Find beginnning of a line ' -----'
          {
            $TabColPos[$TabColSz]    = $i;  //Note position and length for each column
            $TabColLength[$TabColSz] = strlen($chars[$TabColSz]);
            $TabColSz++;
          }
    
          $c = $cNext;
        }
  
        if($TabColSz < 3 || $TabColSz > 5)
        {
          exit("Wrong number of query columns found (expected 3 or 5): {$$TabColSz}");
        }
        continue;
      }
  
      if($State == 2 && count($chars) >= 3)
      {
      	if($chars[1] == "P" || $chars[1] == "TR")
      	{
      	  if($chars[1] == "TR") $iTrigCnt++;  //Statistics

      	  if($chars[1] == "P") $iProcCnt++;

          $EntryCnt++;

          if(count($chars) >= 5)
          {
            //Get optional parameters, if available
            $sCreation = trim(substr($buffer, $TabColPos[3], $TabColLength[3]));  //Time of create
            $sCreator  = trim(substr($buffer, $TabColPos[4], $TabColLength[4])); //Login_name of creator
    
            print "  <proc name=\"{$chars[0]}\" src=\"{$chars[2]}_{$chars[0]}.sql\" type=\"{$chars[1]}\" owner=\"{$chars[2]}\" creation=\"$sCreation\" creator=\"$sCreator\"/>\n";
          }
          else
            print "  <proc name=\"{$chars[0]}\" src=\"{$chars[2]}_{$chars[0]}.sql\" type=\"$chars[1]\" owner=\"$chars[2]\"/>\n";
      	}
      }
    }
    fclose($handle);

    return $EntryCnt;
  }

  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  // GenXMLTabEntries converts a list of table columns and size information into XML format
  // It expects to see a ASCII formated file produced by the following SQL commands:
  //
  // print 'BEGIN_SPACE TABLE DB_OBJECT_OWNER DB_OBJECT'
  // exec sp_spaceused 'DB_OBJECT_OWNER.DB_OBJECT'
  // print 'END_SPACE TABLE DB_OBJECT_OWNER DB_OBJECT'
  //
  // print 'BEGIN TABLE DB_OBJECT_OWNER DB_OBJECT'
  // exec sp_columns 'DB_OBJECT', 'DB_OBJECT_OWNER'
  // print 'END TABLE DB_OBJECT_OWNER DB_OBJECT'
  //
  // -> where DB_OBJECT_OWNER is the name of the owner of a database object
  //          DB_OBJECT is the name of a database object
  //
  // -> The keyword 'TABLE' should be replaced with 'VIEW' when processing views
  // -> The 1st three lines around sp_spaceused are optional (not useful for views)
  //
  // -> Refer to sample database project ZEUS/TITAN for more information
  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  function GenXMLTabEntries($IdxFile, &$iCntTables, &$iCntViews)
  {
  	$str="";
  	$iStep=0;
  	$iCntTables=0;
  	$iCntViews=0;
  	$Table="";
  	$Owner="";
  
    $TableSpace="";
    $OwnerSpace="";
    $DateSpace=-1;
    $IndexSpace=-1;
    $IndexPart ="";
    $DatePart  ="";
  
    $handle = fopen($IdxFile, "r");
    while (!feof($handle))
    {
      $buffer = fgets($handle, 4096);
      if( strlen(trim($buffer)) <= 0) continue;

      $chars = preg_split('/ /', $buffer, -1, PREG_SPLIT_NO_EMPTY);

      for($i=0;$i<count($chars);$i++) //Subtract unwanted white spaces
        $chars[$i] = trim($chars[$i]);

      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX Start of a SRC_REP_TAB INFO
      if($chars[0] == "BEGIN_SRC_REP_TABLES")
      {
        $str        = trim($chars[0]);       //Copy name of state for later processing steps
        $iStep      = 1;
        $TableSpace = trim($chars[3]);
        $OwnerSpace = trim($chars[2]);
        $DateSpace  = -1;
        $IndexSpace = -1;
        $IndexPart ="";
        $DatePart  ="";

        continue;
      }

      if($chars[0] == "END_SRC_REP_TABLES") //End of REP_SRC_TABLE INFO
      {
        $iStep     = 0;
        $str       = "";
        continue;
      }

      if($str == "BEGIN_SRC_REP_TABLES" && $iStep >= 1)
      {
        $iStep++;

        if($iStep >= 4) // Filter for column infos name, and size columns are required
        {                
          //Print this to define an icon and a string for this class of object
          if($iStep == 4)
            printf("\n\n<CLASS_DEF type=\"table\" classify=\"replicate_source\" icon=\"res/rep_src.png\" text=\"Source of Replication\" />\n");

          if(trim($chars[0]) == "INFO")
          {
            $RepOwner   = trim($chars[1]);  //Owner of source table that is replicated
            $RepTabName = trim($chars[2]);  //Name of source table that is replicated

            printf("<CLASS type=\"table\" classify=\"replicate_source\" owner=\"%s\" name=\"%s\" />\n"
                   ,$RepOwner, $RepTabName);
            continue;
          }
        }
      }
      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX End of a SRC_REP_TAB INFO XXXXXX

      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX Start of a new TABLE SPACE INFO
      if($chars[0] == "BEGIN_SPACE" && $chars[1] == "TABLE")
      {
        $str        = trim($chars[0]);       //Copy name of state for later processing steps
        $iStep      = 1;
        $TableSpace = trim($chars[3]);
        $OwnerSpace = trim($chars[2]);
        $DateSpace  = -1;
        $IndexSpace = -1;
        $IndexPart ="";
        $DatePart  ="";

        continue;
      }

      #End of Table/View DDL
      if($chars[0] == "END_SPACE")
      {
        $iStep     = 0;
        $str       = "";
        #TableSpace=""
        #OwnerSpace=""
        #DateSpace =-1
        #IndexSpace=-1
        #IndexPart =""
        #DatePart  =""
        
        continue;
      }

      if($str == "BEGIN_SPACE" && strlen($TableSpace)>0 && strlen($OwnerSpace)>0 && $iStep >= 1)
      {
        $iStep++;
    
        if($iStep == 4)
        {                // Filter for column infos name, and size columns are required
          if(trim($chars[0]) == $TableSpace)
          {
            $DateSpace = trim($chars[4]);  //Space used for data on this table
            $DatePart  = trim($chars[5]);  //Size stated in KB, MB, GB ...
            $IndexSpace= trim($chars[6]);  //Space used for index(es) on this table
            $IndexPart = trim($chars[7]);  //Size stated in KB, MB, GB ...

            continue;
          }
        }
      }
      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX End of a Table SPACE INFO XXXXXX

    	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX Start of a new Table/View DDL
    	if($chars[0] == "BEGIN" && ($chars[1] == "TABLE" || $chars[1] == "VIEW"))
    	{
    		$s    = $chars[1]; //Copy owner and table/view name
    		$Owner= $chars[2];
    		$Table= $chars[3];
    		
    		$iStep=1;
    
    		if($s == "TABLE")
        {
    			print "\n<tab name=\"$Table\" owner=\"$Owner\">\n";
    
          if($TableSpace == $Table && $OwnerSpace == $Owner)
          {
            print "  <tabsize data=\"$DateSpace\" datapart=\"$DatePart\" index=\"$IndexSpace\" indexpart=\"$IndexPart\" />\n";
          }
        }
    		else
        {
      		if($s == "VIEW")
      		{
            //print "\n<view name=\"" Table "\" owner=\"" Owner "\"" " src=\"" Table ".sql\"" ">"
            print "\n<view name=\"$Table\" owner=\"$Owner\" src=\"{$Owner}_{$Table}.sql\">\n";
          }
        }
    
    		continue;
    	}
    
      if(($s == "TABLE" || $s == "VIEW") && $iStep == 1)
      {
                         # Filter for column infos
        if($chars[2] == $Table)  #name, Type and length are required arguments
        if(strlen($chars[3]) > 0 && strlen($chars[5]) >0 && strlen($chars[7]) > 0)
        {
          print "  <col name=\"$chars[3]\" type=\"$chars[5]\" len=\"$chars[7]\"";
      
          if($chars[5] == "numeric")
            print " prec=\"$chars[6]\" scale=\"$chars[8]\"";
    
          if($chars[10] == "0")
            print " nulls=\"NOT NULL\"";
      
          print " />\n";
          continue;
        }
      }
    
    	#End of Table/View DDL
    	if($chars[0] == "END" && $chars[1] == $s && $chars[2] == $Owner && $chars[3] == $Table)
    	{
    		if($s == "TABLE")
    		{
    			$iCntTables++;
    			print "</tab>";
    		}
    
    		if($s == "VIEW")
    		{
    			$iCntViews++;
    			print "</view>";
    		}
    
    		$s="";
    		$iStep=0;
    		$Table="";
    		$Owner="";
    		
    		continue;
    	}
    
    	if(($s == "TABLE" || $s == "VIEW") && $iStep == 1)
    	{
                                 // Filter for column infos
    		if($chars[2] == $Table)  //name, Type and length are required arguments
    		if(strlen($chars[3]) > 0 && strlen($chars[5]) >0 && strlen($chars[6]) >0)
    		{
    			print "<col name=\"$chars[3]\" type=\"$chars[5]\" len=\"$chars[7]\"\n";
    	
    			if($chars[5] == "numeric")
            print " prec=\"$chars[6]\" scale=\"$chars[8]\"\n";
    
    			if($chars[10] == "0")
            print " nulls=\"NOT NULL\"\n";
    	
    			print " />";
    			continue;
    		}
    	}
       // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX End of a Table/View DDL XXXXXX
    } // END OF WHILE for main file loop
    fclose($handle);

    return($iCntTables + $iCntViews);
  }

  //Open a file and print its contents to standard output
  //Parameter: Name of file to print contents for
  function PrintFileContents($FileName)
  {
    if(file_exists($FileName) && is_readable($FileName))
    {
      $handle = fopen($FileName, "r");
      while (!feof($handle))
      {
          $buffer = fgets($handle, 4096);
          echo $buffer;
      }
      fclose($handle);
      echo $read;
    }
    else
    {
      print "\n\nERROR ::PrintFileContents Cannot open Optional XML ROOT FILE: {$FileName} \n\n\n";
      exit -2;
    }
  }

  // MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX
  // MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX
  // MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX MAIN X MAIN XX
  $IdxFile = $argv[1];    //This is where the base plain text files are
  $TabFile = $argv[2];   //We convert them into plain XML to generate
  $SERVER  = $argv[3];  //plain HTML with NextLevel (from plain SQL)
  $DB      = $argv[4]; //Its all very plain :)
  $OWNER   = $argv[5];

  $path_parts = pathinfo($_SERVER['SCRIPT_FILENAME']); //Find out where Script runs
  $HerePath   = "{$path_parts["dirname"]}/";

  //Print Header of XML file
  print "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
  print "<?NextLevel version=\"1.1\" date=\"November, 2004\"?>\n";
  print "<!-- Database CallGraph Utility Application File -->\n";
  print "<!-- Generated with: php GenIdx.php idx.txt {$SERVER} {$DB} {$OWNER} > index.xml -->\n";
  print "<parse_files SERVER=\"{$SERVER}\" DB=\"{$DB}\" owner=\"{$OWNER}\">\n";

  //Read and print optional XML ROOT FILE
  //(STOP Processing, if parameter is set, but file not found)
  if($argc >= 7) PrintFileContents($argv[6]);

  //Generate pointers to SQL source files for procedures and triggers
  $iProcCnt=0; $iTrigCnt = 0;
  print "<procs>\n";
  $iSz = GenXMLProcEntries($IdxFile, $iProcCnt, $iTrigCnt);
  print "</procs>\n";

  //Generate Table and View entries (latter with pointers to source files)
  $iTabCnt=0; $iViewCnt = 0;
  $iSz = GenXMLTabEntries($TabFile, $iTabCnt,$iViewCnt);
  print "  <!-- $iTabCnt tables and $iViewCnt views found -->\n";
  print "  <!-- XXXXXXXXXXX $iProcCnt procedures and $iTrigCnt triggers found XXXXXX -->\n";

  if($argc >= 8) PrintFileContents($argv[7]);

	print "</parse_files>\n"; // End of XML file
?>
