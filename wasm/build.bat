@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

if not exist build mkdir build

set SRCS=
for /r src %%f in (*.cpp) do set SRCS=!SRCS! "%%f"

call em++ !SRCS! ^
  -std=c++17 -O2 -Isrc -o build\mat.js --no-entry ^
  -sMODULARIZE -sEXPORT_NAME=createMat ^
  -sALLOW_MEMORY_GROWTH=1 ^
  -sEXPORTED_FUNCTIONS=_malloc,_free ^
  -sEXPORTED_RUNTIME_METHODS=HEAPF64,ccall,cwrap,UTF8ToString
if errorlevel 1 (
    echo [FAIL] WASM build failed.
    exit /b 1
)

echo [OK] Built build\mat.js and build\mat.wasm
exit /b 0