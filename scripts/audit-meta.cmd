@echo off
setlocal
pushd "%~dp0.."
python scripts/audit-meta.py %*
set "exit_code=%ERRORLEVEL%"
popd
exit /b %exit_code%
