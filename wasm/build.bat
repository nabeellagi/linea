@echo off
setlocal enabledelayedexpansion
if not exist build mkdir build

set SRCS=
for /r src %%f in (*.cpp) do set SRCS=!SRCS! "%%f"

em++ %SRCS% ^
  -std=c++17 -O2 -Isrc -o build\mat.js --no-entry ^
  -sMODULARIZE -sEXPORT_NAME=createMat ^
  -sEXPORTED_FUNCTIONS=_malloc,_free ^
  -sEXPORTED_RUNTIME_METHODS=HEAPF64,ccall,cwrap,UTF8ToString