@echo off
setlocal
pushd "%~dp0.."
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-book.ps1" %*
set "exit_code=%ERRORLEVEL%"
popd
exit /b %exit_code%
