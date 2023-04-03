@echo off

setlocal enabledelayedexpansion

:input_charname
set /p charname=Enter character name:
if "!charname!"=="" (
    echo Character name cannot be empty.
    goto input_charname
)

:input_password
set /p password=Enter password:
if "!password!"=="" (
    echo Password cannot be empty.
    goto input_password
)

echo Creating shortcut with name %charname%...

set shortcut="%cd%\%charname%.lnk"
set target="%cd%\bin\moac.exe"

if exist %shortcut% (
    echo Shortcut already exists. Overwriting...
    del %shortcut%
)

set /a options=0
set /p darkgui=Dark GUI? (y/n)
if /i "%darkgui%"=="y" set /a options+=1

set /p rightclick=Right-Click Menu? (y/n)
if /i "%rightclick%"=="y" set /a options+=2

set /p actionbar=Action (Icon) Bar, Enter to Chat? (y/n)
if /i "%actionbar%"=="y" set /a options+=4

set /p inventory=Three Inventory Rows (Smaller Bottom Window)? (y/n)
if /i "%inventory%"=="y" set /a options+=8

set /p moveequip=Moving Equipment Bar? (y/n)
if /i "%moveequip%"=="y" set /a options+=16

set /p bigbars=Big Health, Mana and mana bar? (y/n)
if /i "%bigbars%"=="y" set /a options+=32

set /p sound=Sound? (y/n)
if /i "%sound%"=="y" set /a options+=64

set /p font=Large Font? (y/n)
if /i "%font%"=="y" set /a options+=128

set /p fullscreen=True Full Screen? (y/n)
if /i "%fullscreen%"=="y" set /a options+=256

set /p oldmouse=Old Mouse Wheel Commands? (y/n)
if /i "%oldmouse%"=="y" set /a options+=512

set /p fastinv=Faster Inventory? (y/n)
if /i "%fastinv%"=="y" set /a options+=1024

set /p lessdelay=Less Command Delay? (y/n)
if /i "%lessdelay%"=="y" set /a options+=2048

powershell.exe -Command "$ws = New-Object -ComObject WScript.Shell; $s = $ws.CreateShortcut('%shortcut%'); $s.TargetPath = '%target%'; $s.Arguments = '-u %charname% -p %password% -d astoniaresurgence.com -o %options%'; $s.Save()"

echo Done.

