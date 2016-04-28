echo off

rem This is the script I use on my Windows 7 virtual machine to build the public release of Bandage.
rem Bandage. It produces a directory ready for deployment. It requires that sample_LastGraph is in
rem the current directory.

rem These variables must be set to the correct paths and values for your system.
set QT_PATH=C:\Qt\5.6
set MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0
set MSVC_VERSION=msvc2015_64
set MSVC_VERSION_NUM=140
set GIT_PATH=C:\Program Files\Git\bin\git
set ZIP_PATH=C:\Program Files\7-Zip\7z.exe

rem Set up the MSVC compiler.
call "%MSVC_PATH%\VC\vcvarsall.bat" x86_amd64

rem Clone and build Bandage from the master branch.
call "%GIT_PATH%" clone https://github.com/rrwick/Bandage.git
call cd Bandage\
call "%QT_PATH%\%MSVC_VERSION%\bin\qmake.exe"
call "%QT_PATH%\..\Tools\QtCreator\bin\jom.exe"
call cd ..

rem Get the Bandage version number from the main.cpp file and replace its dots with underscores.
for /f %%i in ('findstr setApplicationVersion Bandage\program\main.cpp') do set VERSION_LINE=%%i
set VERSION_LINE=%VERSION_LINE:.=_%
set VERSION_LINE=%VERSION_LINE:"=;%
for /f "tokens=2 delims=;" %%G IN ("%VERSION_LINE%") DO set VERSION=%%G

rem Delete all of the source and build files
call move Bandage\release\Bandage.exe Bandage.exe
call rmdir Bandage\ /S /Q
call mkdir Bandage\
call move Bandage.exe Bandage\Bandage.exe

rem Add the necessary libraries so Bandage can be deployed.
call "%QT_PATH%\%MSVC_VERSION%\bin\windeployqt.exe" Bandage\Bandage.exe
call copy "%MSVC_PATH%\VC\redist\x64\Microsoft.VC%MSVC_VERSION_NUM%.CRT\msvcp%MSVC_VERSION_NUM%.dll" Bandage\msvcp%MSVC_VERSION_NUM%.dll
if exist "%MSVC_PATH%\VC\redist\x64\Microsoft.VC%MSVC_VERSION_NUM%.CRT\msvcr%MSVC_VERSION_NUM%.dll" call copy "%MSVC_PATH%\VC\redist\x64\Microsoft.VC%MSVC_VERSION_NUM%.CRT\msvcr%MSVC_VERSION_NUM%.dll" Bandage\msvcr%MSVC_VERSION_NUM%.dll
if exist Bandage\vcredist_x64.exe call del Bandage\vcredist_x64.exe

rem Zip Bandage with the sample graph and clean up.
call "%ZIP_PATH%" a -tzip Bandage_Windows_v%VERSION%.zip Bandage\ sample_LastGraph
call rmdir Bandage\ /S /Q
