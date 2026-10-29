# LINEA: WebAssembly engine
 
This folder holds everything behind LINEA's maths: the **C++ engine**, its **unit tests**, and the **scripts** that compile it to WebAssembly and then turn its LaTeX output into PDFs for checking.
Basically the backbone of Linea!

> **Just want to run the website?** You don't need anything here. The compiled engine is already committed at `public/wasm/`. See the [main README](../README.md). Come back to this guide when you want to **change the C++ code**, **run the tests**, or **rebuild the `.wasm` file**.
 
The scripts provided here are Windows batch files (`build.bat`, `test.bat`).

## What's in here
 
```
wasm/
├── build.bat            # Compile the engine to WASM and copy it into ../public/wasm/
├── test.bat             # Run unit tests, then generate LaTeX reports and PDFs
├── src/                 # The engine (C++17)
│   ├── common/          #   Fraction (exact arithmetic), Matrix type, LaTeX helpers, MAT_API macro
│   ├── lu/              #   LUP decomposition + forward/back substitution (shared by everything)
│   ├── determinants/    #   det(): LUP method, plus minor/cofactor cross-check for N ≤ 3
│   ├── solvers/         #   Equation parser, solve_gauss(), solve_lu()
│   ├── regression/      #   linear_regression(): least squares via the normal equations
│   └── basic/           #   (placeholder, currently empty)
├── tests/               # Unit tests (tiny self-made framework, no downloads needed)
│   ├── test_util.h      #   TEST(), CHECK(), CHECK_EQ() macros
│   ├── test_main.cpp    #   Runs every registered test and prints PASS/FAIL
│   ├── determinants/  solvers/  regression/  basic/
│   └── out/             #   Generated LaTeX reports (.txt) and PDFs, git-ignored
└── tools/
    ├── generate_determinant_outputs.cpp   # Write sample determinant reports
    ├── generate_solver_outputs.cpp        # Write sample solver reports (Gauss + LU)
    ├── generate_regression_outputs.cpp    # Write sample regression reports
    └── latex_to_pdf.py                    # Compile the reports to PDF (uses Tectonic)
```

---

## Prerequisites

| Task | You need |
| --- | --- |
| **Compile to WASM** | [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) |
| **Run unit tests** | Emscripten SDK + [Node.js](https://nodejs.org/) |
| **Generate LaTeX reports** (part of `test.bat`) | A native **GCC/MinGW-w64** `g++` with C++17 support. |
| **Compile reports to PDF** | [Python 3](https://www.python.org/downloads/), the `pypdf` package, and [Tectonic](https://tectonic-typesetting.github.io/) |

---

## How to compile to WASM
 
Run this:

```bat
wasm\build.bat
```
 
What it does:
 
1. Collects every `.cpp` under `src/`.
2. Compiles them with `em++` (C++17, `-O2`) into `build\mat.js` + `build\mat.wasm`.
3. Copies both files into **`../public/wasm/`**, which is where the Next.JS web app loads them from.

You should end with:
 
```
[OK] Built and copied mat.js and mat.wasm to public\wasm.
```
 
Then refresh the site (hard refresh, <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>R</kbd>, to skip the cached copy).
 
---

## How to test WASM

```bat
wasm\test.bat
```
 
It runs five stages and **stops at the first failure**:
 
| Stage | What happens |
| --- | --- |
| 1. Unit tests | Compiles `src/` + `tests/` with `em++` into `build\tests.js` and runs it with `node` |
| 2. Solver reports | Compiles `tools/generate_solver_outputs.cpp` with native `g++` and runs it |
| 3. Determinant reports | Same, with `generate_determinant_outputs.cpp` |
| 4. Regression reports | Same, with `generate_regression_outputs.cpp` |
| 5. PDF conversion | Runs `python tools\latex_to_pdf.py` |
 
If the unit tests fail in stage 1, the script stops there!
 