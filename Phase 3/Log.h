/**
 * Log.h
 * 
 * Contains a print function that can be easily disabled for Ultima.cpp, and
 * enabled for UnitTest.cpp.
 */

#pragma once

// 0 = Disabled | 1 = Enabled
#define DEBUG 0
#if DEBUG
  #define LOG(x) std::cout << x
#else
  #define LOG(x)
#endif