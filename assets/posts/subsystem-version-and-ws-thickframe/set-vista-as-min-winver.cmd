@echo off
cd /d %~dp0
if "%~1"=="" goto :bad_usage
title %~1
dumpbin /headers "%~1" ^
	| find "subsystem (Windows CUI)" >nul ^
	&& (set subs=console) ^
	|| (set subs=windows)

echo Subsystem detected: %subs%
editbin "%~1" /osversion:6.0 /subsystem:%subs%,6.0 || pause
exit/b

:bad_usage
echo Usage: %~nx0 [exe filepath]
pause
exit/b
