@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo.
echo ========================================
echo  1. Building and running unit tests
echo ========================================
echo.

if not exist build mkdir build

set SRCS=
for /r src %%f in (*.cpp) do set SRCS=!SRCS! "%%f"
for /r tests %%f in (*.cpp) do set SRCS=!SRCS! "%%f"

em++ %SRCS% -std=c++17 -O1 -Isrc -Itests -o build\tests.js
if errorlevel 1 (
    echo [FAIL] Unit test compilation failed.
    exit /b 1
)

node build\tests.js
if errorlevel 1 (
    echo [FAIL] Unit tests failed. Skipping report generation.
    exit /b 1
)

set NATIVE_SRCS=
for /r src %%f in (*.cpp) do set NATIVE_SRCS=!NATIVE_SRCS! "%%f"

echo.
echo ========================================
echo  2. Generating linear-solver reports
echo ========================================
echo.

if exist tools\generate_solver_outputs.cpp (
    g++ !NATIVE_SRCS! tools\generate_solver_outputs.cpp -std=c++17 -O2 -Isrc -o build\generate_solver_outputs.exe
    if errorlevel 1 (
        echo [FAIL] Solver report generator compilation failed.
        exit /b 1
    )
    build\generate_solver_outputs.exe
    if errorlevel 1 (
        echo [FAIL] Solver report generation failed.
        exit /b 1
    )
)

REM Keep the determinant stage when the determinant add-on from the earlier guide is installed.
if exist tools\generate_determinant_outputs.cpp (
    echo.
    echo ========================================
    echo  3. Generating determinant reports
    echo ========================================
    echo.
    g++ !NATIVE_SRCS! tools\generate_determinant_outputs.cpp -std=c++17 -O2 -Isrc -o build\generate_determinant_outputs.exe
    if errorlevel 1 (
        echo [FAIL] Determinant report generator compilation failed.
        exit /b 1
    )
    build\generate_determinant_outputs.exe
    if errorlevel 1 (
        echo [FAIL] Determinant report generation failed.
        exit /b 1
    )
)

echo.
echo ========================================
echo  4. Generating linear-regression reports
echo ========================================
echo.

if not exist tools\generate_regression_outputs.cpp (
    echo [FAIL] Missing tools\generate_regression_outputs.cpp
    exit /b 1
)

g++ !NATIVE_SRCS! tools\generate_regression_outputs.cpp -std=c++17 -O2 -Isrc -o build\generate_regression_outputs.exe
if errorlevel 1 (
    echo [FAIL] Regression report generator compilation failed.
    exit /b 1
)

build\generate_regression_outputs.exe
if errorlevel 1 (
    echo [FAIL] Regression report generation failed.
    exit /b 1
)

echo.
echo ========================================
echo  5. Compiling LaTeX reports to PDF
echo ========================================
echo.

REM Check that the Python script exists.
if not exist tools\latex_to_pdf.py (
    echo [FAIL] Missing tools\latex_to_pdf.py
    exit /b 1
)

REM Check that Python is available in PATH.
where python >nul 2>nul
if errorlevel 1 (
    echo [FAIL] Python was not found in PATH.
    exit /b 1
)

REM Compile all LaTeX .txt reports into PDFs.
python tools\latex_to_pdf.py
if errorlevel 1 (
    echo [FAIL] LaTeX-to-PDF conversion failed.
    exit /b 1
)

echo [OK] LaTeX reports compiled successfully.

echo.
echo ========================================
echo  ALL TESTS AND REPORTS COMPLETED
echo  Reports: tests\out\
echo ========================================
exit /b 0
