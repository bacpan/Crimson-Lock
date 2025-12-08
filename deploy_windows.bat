@echo off
REM Crimson Lock - Comprehensive Windows Deployment & Installation Script
REM Combines portable deployment and system installation into one unified script

echo Crimson Lock - Windows Deployment & Installation
echo ================================================

REM Get current directory (project root)
set PROJECT_DIR=%cd%
set BUILD_DIR=%PROJECT_DIR%\build

echo.
echo Please select deployment option:
echo 1. Create Portable Package (no admin required)
echo 2. Install to System (requires admin privileges) 
echo 3. Both - Create Portable then Install to System
echo.
set /p DEPLOY_CHOICE="Enter your choice (1-3): "

if "%DEPLOY_CHOICE%"=="1" goto :PORTABLE_ONLY
if "%DEPLOY_CHOICE%"=="2" goto :SYSTEM_ONLY
if "%DEPLOY_CHOICE%"=="3" goto :BOTH
echo Invalid choice. Defaulting to portable package.
goto :PORTABLE_ONLY

:BOTH
call :CREATE_PORTABLE
if %errorlevel% neq 0 exit /b %errorlevel%
goto :INSTALL_SYSTEM

:PORTABLE_ONLY
call :CREATE_PORTABLE
goto :END

:SYSTEM_ONLY
REM Check for Administrator privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required!
    echo Right-click and "Run as Administrator"
    pause
    exit /b 1
)
REM Check if portable exists, if not create it first
if not exist "Crimson-Lock-Windows-Portable\CrimsonLock.exe" (
    echo Portable package not found. Creating it first...
    call :CREATE_PORTABLE
    if %errorlevel% neq 0 exit /b %errorlevel%
)
goto :INSTALL_SYSTEM

REM ==========================================
REM PORTABLE PACKAGE CREATION
REM ==========================================
:CREATE_PORTABLE
echo.
echo ==========================================
echo  CREATING PORTABLE PACKAGE
echo ==========================================

REM Check for executable
set EXECUTABLE_PATH=
if exist "%BUILD_DIR%\Release\CrimsonLock.exe" (
    set EXECUTABLE_PATH=%BUILD_DIR%\Release\CrimsonLock.exe
    set BUILD_TYPE=Release
) else if exist "%BUILD_DIR%\Debug\CrimsonLock.exe" (
    set EXECUTABLE_PATH=%BUILD_DIR%\Debug\CrimsonLock.exe
    set BUILD_TYPE=Debug
) else if exist "%BUILD_DIR%\CrimsonLock.exe" (
    set EXECUTABLE_PATH=%BUILD_DIR%\CrimsonLock.exe
    set BUILD_TYPE=MinGW
) else (
    echo No executable found!
    echo Please build the application first
    echo Try: .\run.sh
    pause
    exit /b 1
)

echo Found executable: %EXECUTABLE_PATH%

REM Create deployment directory
set DEPLOY_DIR=%PROJECT_DIR%\Crimson-Lock-Windows-Portable
if exist "%DEPLOY_DIR%" rmdir /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%"

REM Copy main executable
copy "%EXECUTABLE_PATH%" "%DEPLOY_DIR%\CrimsonLock.exe" >nul
echo Copied main executable

REM Try to use windeployqt for automatic Qt dependency deployment
echo Searching for windeployqt...
set WINDEPLOYQT_FOUND=0

REM First try to find Qt in PATH
where windeployqt.exe >nul 2>&1
if %errorlevel%==0 (
    echo Found windeployqt in PATH
    windeployqt.exe --force --no-translations --no-system-d3d-compiler --no-opengl-sw --compiler-runtime "%DEPLOY_DIR%\CrimsonLock.exe" >nul
    set WINDEPLOYQT_FOUND=1
) else (
    REM Try common Qt installation paths
    for %%p in (
        "C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe"
        "C:\Qt\5.14.2\msvc2017_64\bin\windeployqt.exe"
        "C:\Qt\5.13.2\msvc2017_64\bin\windeployqt.exe"
        "C:\Qt\5.12.12\msvc2017_64\bin\windeployqt.exe"
        "C:\Qt\Tools\QtCreator\bin\windeployqt.exe"
    ) do (
        if exist %%p (
            echo Found windeployqt at %%p
            %%p --force --no-translations --no-system-d3d-compiler --no-opengl-sw --compiler-runtime "%DEPLOY_DIR%\CrimsonLock.exe" >nul
            set WINDEPLOYQT_FOUND=1
            goto :WINDEPLOYQT_DONE
        )
    )
)

:WINDEPLOYQT_DONE
if %WINDEPLOYQT_FOUND%==1 (
    echo Qt dependencies deployed automatically
) else (
    echo Warning: windeployqt not found
    echo Manual Qt DLL copy may be required
)

REM Copy additional assets
if exist "%PROJECT_DIR%\assets" (
    mkdir "%DEPLOY_DIR%\assets"
    xcopy "%PROJECT_DIR%\assets\*" "%DEPLOY_DIR%\assets\" /E /I /H /Y >nul
    echo Copied application assets
)

REM Copy documentation
if exist "%PROJECT_DIR%\README.md" copy "%PROJECT_DIR%\README.md" "%DEPLOY_DIR%\" >nul
if exist "%PROJECT_DIR%\LICENSE" copy "%PROJECT_DIR%\LICENSE" "%DEPLOY_DIR%\" >nul

REM Create batch launcher
(
echo @echo off
echo REM Crimson Lock Launcher
echo cd /d "%%~dp0"
echo start "" "CrimsonLock.exe"
) > "%DEPLOY_DIR%\Launch-CrimsonLock.bat"

REM Create PowerShell launcher (more modern)
(
echo # Crimson Lock PowerShell Launcher
echo Set-Location -Path $PSScriptRoot
echo Start-Process -FilePath ".\CrimsonLock.exe" -WorkingDirectory $PSScriptRoot
) > "%DEPLOY_DIR%\Launch-CrimsonLock.ps1"

REM Test the deployment
echo Testing deployment...
"%DEPLOY_DIR%\CrimsonLock.exe" --version >nul 2>&1
if %errorlevel%==0 (
    echo Deployment test passed
) else (
    echo Warning: Deployment test failed - missing dependencies
)

REM Create ZIP package
echo Creating ZIP package...
powershell -command "Compress-Archive -Path '%DEPLOY_DIR%\*' -DestinationPath '%PROJECT_DIR%\Crimson-Lock-Windows-Portable.zip' -Force" >nul 2>&1
if exist "%PROJECT_DIR%\Crimson-Lock-Windows-Portable.zip" (
    echo ZIP package created: Crimson-Lock-Windows-Portable.zip
) else (
    echo Note: ZIP creation failed (PowerShell/7-Zip not available)
)

echo.
echo Portable package created successfully!
echo Location: %DEPLOY_DIR%
echo You can copy this folder to any Windows machine
echo.
exit /b 0

REM ==========================================
REM SYSTEM INSTALLATION
REM ==========================================
:INSTALL_SYSTEM
echo.
echo ==========================================
echo  INSTALLING TO SYSTEM
echo ==========================================

REM Check for Administrator privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required!
    echo Right-click and "Run as Administrator"
    pause
    exit /b 1
)

echo Administrator privileges confirmed

set INSTALL_SOURCE=%cd%
set PROGRAM_FILES_DIR=%ProgramFiles%\Crimson Lock
set START_MENU_DIR=%ProgramData%\Microsoft\Windows\Start Menu\Programs

REM Check if portable deployment exists
if not exist "Crimson-Lock-Windows-Portable\CrimsonLock.exe" (
    echo Portable deployment not found!
    echo Creating portable package first...
    call :CREATE_PORTABLE
    if %errorlevel% neq 0 exit /b %errorlevel%
)

echo Found portable deployment

REM Create installation directory
echo Creating installation directory...
if exist "%PROGRAM_FILES_DIR%" rmdir /s /q "%PROGRAM_FILES_DIR%"
mkdir "%PROGRAM_FILES_DIR%"

REM Copy all files from portable deployment
echo Installing application files...
xcopy "Crimson-Lock-Windows-Portable\*" "%PROGRAM_FILES_DIR%\" /E /I /H /Y >nul
if %errorlevel% neq 0 (
    echo Failed to copy application files
    pause
    exit /b 1
)

echo Application files installed

REM Check and install Visual C++ Redistributables if needed
echo Checking Visual C++ Redistributables...
set VCREDIST_NEEDED=0

REM Test if app runs (simple dependency check)
"%PROGRAM_FILES_DIR%\CrimsonLock.exe" --version >nul 2>&1
if %errorlevel% neq 0 set VCREDIST_NEEDED=1

if %VCREDIST_NEEDED%==1 (
    echo Installing Visual C++ Redistributables...
    REM Try to download and install VC++ Redistributables
    powershell -command "try { Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vc_redist.x64.exe' -OutFile '%TEMP%\vc_redist.x64.exe' -UseBasicParsing; Start-Process '%TEMP%\vc_redist.x64.exe' -ArgumentList '/quiet' -Wait; Remove-Item '%TEMP%\vc_redist.x64.exe' -Force; Write-Host 'VC++ Redistributables installed' } catch { Write-Host 'Failed to install VC++ Redistributables automatically' }" 2>nul
) else (
    echo Visual C++ Redistributables already available
)

REM Add to system PATH
echo Adding to system PATH...
for /f "tokens=2*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH 2^>nul') do set SYSTEM_PATH=%%b
echo %SYSTEM_PATH% | findstr /C:"%PROGRAM_FILES_DIR%" >nul
if %errorlevel% neq 0 (
    reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH /t REG_EXPAND_SZ /d "%SYSTEM_PATH%;%PROGRAM_FILES_DIR%" /f >nul
    echo Added to system PATH
) else (
    echo Already in system PATH
)

REM Create Start Menu shortcut
echo Creating Start Menu shortcut...
powershell -command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%START_MENU_DIR%\Crimson Lock.lnk'); $Shortcut.TargetPath = '%PROGRAM_FILES_DIR%\CrimsonLock.exe'; $Shortcut.WorkingDirectory = '%PROGRAM_FILES_DIR%'; $Shortcut.Description = 'Crimson Lock - Secure Password Manager'; $Shortcut.Save()" 2>nul
echo Start Menu shortcut created

REM Create Desktop shortcut  
echo Creating Desktop shortcut...
powershell -command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut([Environment]::GetFolderPath('CommonDesktopDirectory') + '\Crimson Lock.lnk'); $Shortcut.TargetPath = '%PROGRAM_FILES_DIR%\CrimsonLock.exe'; $Shortcut.WorkingDirectory = '%PROGRAM_FILES_DIR%'; $Shortcut.Description = 'Crimson Lock - Secure Password Manager'; $Shortcut.Save()" 2>nul
echo Desktop shortcut created

REM Register file associations for .crimson files
echo Registering file associations...
reg add "HKCR\.crimson" /ve /d "CrimsonLock.Vault" /f >nul
reg add "HKCR\CrimsonLock.Vault" /ve /d "Crimson Lock Vault File" /f >nul
reg add "HKCR\CrimsonLock.Vault\shell\open\command" /ve /d "\"%PROGRAM_FILES_DIR%\CrimsonLock.exe\" \"%%1\"" /f >nul
reg add "HKCR\CrimsonLock.Vault\DefaultIcon" /ve /d "%PROGRAM_FILES_DIR%\CrimsonLock.exe,0" /f >nul
echo File associations registered (.crimson files)

REM Create uninstaller
echo Creating uninstaller...
(
echo @echo off
echo echo Uninstalling Crimson Lock...
echo taskkill /f /im CrimsonLock.exe 2^>nul
echo rmdir /s /q "%PROGRAM_FILES_DIR%"
echo del "%START_MENU_DIR%\Crimson Lock.lnk" 2^>nul
echo del "%%PUBLIC%%\Desktop\Crimson Lock.lnk" 2^>nul
echo reg delete "HKCR\.crimson" /f 2^>nul
echo reg delete "HKCR\CrimsonLock.Vault" /f /reg:32 2^>nul
echo reg delete "HKCR\CrimsonLock.Vault" /f /reg:64 2^>nul
echo for /f "tokens=2*" %%%%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH 2^^^>nul'^) do set SYSTEM_PATH=%%%%b
echo setlocal EnableDelayedExpansion
echo set NEW_PATH=!SYSTEM_PATH:%PROGRAM_FILES_DIR%;=!
echo set NEW_PATH=!NEW_PATH:;%PROGRAM_FILES_DIR%=!
echo reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH /t REG_EXPAND_SZ /d "!NEW_PATH!" /f 2^>nul
echo echo Crimson Lock uninstalled successfully
echo pause
echo del "%%~f0"
) > "%PROGRAM_FILES_DIR%\Uninstall.bat"

REM Add uninstaller to registry
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CrimsonLock" /v DisplayName /d "Crimson Lock" /f >nul
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CrimsonLock" /v UninstallString /d "%PROGRAM_FILES_DIR%\Uninstall.bat" /f >nul
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CrimsonLock" /v DisplayIcon /d "%PROGRAM_FILES_DIR%\CrimsonLock.exe" /f >nul
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CrimsonLock" /v Publisher /d "bacpan" /f >nul
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CrimsonLock" /v DisplayVersion /d "1.0.0" /f >nul

echo.
echo System installation completed successfully!
echo - Application installed to: %PROGRAM_FILES_DIR%
echo - Start Menu shortcut created
echo - Desktop shortcut created  
echo - File associations registered (.crimson)
echo - Added to system PATH
echo - Uninstaller available in Add/Remove Programs
echo.
goto :END

:END
echo.
echo Deployment complete!
echo.
pause
