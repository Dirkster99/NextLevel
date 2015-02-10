@ECHO OFF
REM Build Script for project in Windows/cmd using MinGW32 as required environment
REM For Build on Linux or other Unixes, replace: Makefile.mingw32 with Makefile.linux
REM
REM mingw32-make CFG=debug -f Makefile.mingw32 clean
mingw32-make CFG=debug -f Makefile.mingw32

REM mingw32-make CFG=release -f Makefile.mingw32 clean
REM mingw32-make CFG=release -f Makefile.mingw32

pause
