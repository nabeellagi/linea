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
  -sEXPORTED_FUNCTIONS=_malloc,_free,_det,_det_status,_solve_gauss,_solve_gauss_status,_solve_lu,_solve_lu_status,_linear_regression,_linear_regression_status ^
  -sEXPORTED_RUNTIME_METHODS=HEAPF64,ccall,cwrap,UTF8ToString
if errorlevel 1 (
    echo [FAIL] WASM build failed.
    exit /b 1
)

if not exist "..\public\wasm" mkdir "..\public\wasm"
copy /Y "build\mat.js" "..\public\wasm\mat.js" >nul
if errorlevel 1 (
    echo [FAIL] Could not copy mat.js to public\wasm.
    exit /b 1
)
copy /Y "build\mat.wasm" "..\public\wasm\mat.wasm" >nul
if errorlevel 1 (
    echo [FAIL] Could not copy mat.wasm to public\wasm.
    exit /b 1
)

echo [OK] Built and copied mat.js and mat.wasm to public\wasm.
exit /b 0