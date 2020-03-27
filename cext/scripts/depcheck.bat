@ECHO OFF
REM %1: Path to dumpbin.exe
REM %2: Path to binary to process
REM %3: Expected dependencies
REM %4: Unexpected dependencies (optional)

REM `findstr` sets ERRORLEVEL to 0 is string is found, 1 if it's not.

ECHO Checking dependencies for %2

set expected=fail
set unexpected=fail

REM Look for expected dependencies
ECHO Checking for expected dependency: %3
%1 /DEPENDENTS %2 | findstr /M %3 >nul 2>&1
if %ERRORLEVEL%==0 set expected=pass
IF NOT %expected%==pass (
  ECHO Dependency %3 not found in %2 1>&2
  EXIT /B 1
)

REM Look for unexpected dependencies
ECHO Checking for unexpected dependency: %4
%1 /DEPENDENTS %2 | findstr /M %4 >nul 2>&1
if %ERRORLEVEL%==1 set unexpected=pass
IF NOT %expected%==pass (
  ECHO Unexpected dependency %4 found in %2 1>&2
  EXIT /B 1
)

ECHO Passed!
