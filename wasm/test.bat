@echo off
setlocal enabledelayedexpansion
if not exist build mkdir build

set SRCS=
for /r src %%f in (*.cpp) do set SRCS=!SRCS! "%%f"
for /r tests %%f in (*.cpp) do set SRCS=!SRCS! "%%f"

em++ %SRCS% -std=c++17 -O1 -Isrc -Itests -o build\tests.js
if errorlevel 1 exit /b 1

node build\tests.js