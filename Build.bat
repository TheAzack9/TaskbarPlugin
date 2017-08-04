@echo off
setlocal enabledelayedexpansion enableextensions

title Plugin Build Script

rem TODO: fetch more variables from variables.h
rem Example skin name, Plugin name (which has to be the same as plugin project name!), 

echo.
echo.
echo "Building Plugin"
echo --------------------

set Project=Source\Plugin

echo Finding version for plugin
for /f "delims=" %%x in (%Project%\version.h) do ( 
	echo %%x|findstr "PLUGIN_VERSION" >nul 2>&1
	if not errorlevel 1 (
		set Version=%%x
	)

	echo %%x|findstr "PLUGIN_AUTHOR" >nul 2>&1
	if not errorlevel 1 (
		set Author=%%x
	)

	echo %%x|findstr "PLUGIN_NAME" >nul 2>&1
	if not errorlevel 1 (
		set Plugin=%%x
	)

	echo %%x|findstr "EXAMPLE_SKIN_PATH" >nul 2>&1
	if not errorlevel 1 (
		set SkinPath=%%x
	)
	
	echo %%x|findstr "EXAMPLE_SKIN_NAME" >nul 2>&1
	if not errorlevel 1 (
		set SkinName=%%x
	)
)

if not DEFINED Version (
		echo Could not determine Version
		echo Try running build with version number as first argument to override
		goto END
	)  

if not DEFINED Author (
		echo Could not determine Author
		echo Try running build with author as second argument to override
		goto END
	)

set Version=%Version:#define PLUGIN_VERSION =%
set "Version=%Version:*"=%
echo Version found: %Version%

set Author=%Author:#define PLUGIN_AUTHOR =%
set "Author=%Author:*"=%
echo Author found: %Author%

set Plugin=%Plugin:#define PLUGIN_NAME =%
set "Plugin=%Plugin:*"=%
echo Plugin found: %Plugin%

set SkinPath=%SkinPath:#define EXAMPLE_SKIN_PATH =%
set "SkinPath=%SkinPath:*"=%
echo SkinPath found: %SkinPath%

set SkinName=%SkinName:#define EXAMPLE_SKIN_NAME =%
set "SkinName=%SkinName:*"=%
echo SkinName found: %SkinName%

echo.

set ReleaseLocation=Releases\%Version%
if exist "%ReleaseLocation%" echo Release already built for Version %Version%, Aborting & goto END

echo Finding msbuild.exe

SET msbuildLoc="msbuild.exe"
if not exist %msbuildLoc% (
	echo.
	echo Notice: msbuild.exe not in environment variables, falling back to "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\msbuild.exe"
	echo.
	SET msbuildLoc="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\msbuild.exe"
)

if not exist %msbuildLoc% echo ERROR: msbuild.exe not found, searched in %msbuildLoc% & goto END

set MSBUILD=%msbuildLoc% /nologo^
 /p:Configuration=Release^
 /t:rebuild^
 /v:q

echo Building x86 version of plugin
call %MSBUILD% /p:Platform=Win32 /p:IntDir="%~dp0%Project%\Intermediate\Release\x86" /p:OutDir="%~dp0%ReleaseLocation%\x86" /m "Source\SDK-CPP.sln" > "BuildLog.txt"
if not %ERRORLEVEL% == 0 echo   ERROR %ERRORLEVEL%: Build failed & goto END

echo.

echo Building x64 version of plugin
call %MSBUILD% /p:Platform=x64 /p:IntDir="%~dp0%Project%\Intermediate\Release\x64" /p:OutDir="%~dp0%ReleaseLocation%\x64" /m "Source\SDK-CPP.sln" > "BuildLog.txt"
if not %ERRORLEVEL% == 0 echo   ERROR %ERRORLEVEL%: Build failed & goto END

echo Removing build log
del "BuildLog.txt"

echo Packing rmskin:
call "PluginPacker.exe" "Create Skin" "%Plugin%" "%Author%" "%Version%" "%SkinName%" "%ReleaseLocation%\%Plugin% %SkinName%.rmskin" "%SkinPath%/" "%ReleaseLocation%\x86\%Plugin%.dll" "%ReleaseLocation%\x64\%Plugin%.dll"

if errorlevel 1 (
	echo Could not pack skin, Aborting build
	rmdir /S /Q "%~dp0%ReleaseLocation%/"
	goto END
	)

echo Packing redist:
call "PluginPacker.exe" "Create Redist" "%Plugin%" "%ReleaseLocation%\%Plugin% Redist.zip" "%ReleaseLocation%\x86\%Plugin%.dll" "%ReleaseLocation%\x64\%Plugin%.dll"

if errorlevel 1 (
	echo Could not pack redist, Aborting build
	rmdir /S /Q "%~dp0%ReleaseLocation%/"
	goto END
	)

echo Finished sucessfully
:END
pause