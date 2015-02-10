@ECHO OFF

REM USE THIS BATCH FILE on WINDOWS to rebuild the ZEUS/TITAN database sample
REM REQUIRED: 1> YOU NEED PHP to be installed on your local machine as scripting host
REM              should you need to rebuild the idx.xml file

REM           2> You need the Microsoft Help Workshop to be installed should you need
REM              to rebuild the chm file


REM Normally, Files in the HTML SUB-DIRECTORY should be deleted
REM but I don't dare doing this on someone elses computer, so uncomment it
REM if you know what it does, otherwise some unreferenced HTML files will accumulate
REM if you happen to work with this sample...
REM del TITAN_HTML\*.* /Q

REM 1> Rebuild the XML index file which points to all SQL sources and has table/view defs
php ..\GenIdx\GenIdx.php INTERFACE_SQL\idx.txt INTERFACE_SQL\TabCat.txt SUN INTERFACE dbo header.xml > INTERFACE_SQL\idx.xml

REM Build HTML Documents for the TITAN Sample DATABASE
..\bin\nextlevel -idx INTERFACE_SQL\idx.xml -DirOut INTERFACE_HTML\ -WinHelp

REM 2> The following command will work only if you have Microsofts
REM Help Workshop installed, if not, go and get it from the MS Website (its free)
REM You also need a path variable entry pointing to hhc.exe
REM (restart the computer after setting the path on XP)
hhc INTERFACE_HTML\SUN_INTERFACE.hhp

PAUSE
