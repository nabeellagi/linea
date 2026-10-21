#pragma once
#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
  #define MAT_API extern "C" EMSCRIPTEN_KEEPALIVE
#else
  #define MAT_API extern "C"
#endif

enum Status { OK = 0, ERR_DIM = 1, ERR_SINGULAR = 2, ERR_NULL = 3 };

inline int idx(int r, int c, int cols) { return r * cols + c; }