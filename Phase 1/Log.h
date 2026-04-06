/**
 * Log.h
 * 
 * Contains a print function that can be easily disabled for Ultima.cpp, and
 * enabled for UnitTest.cpp.
 */

#pragma once

#define DEBUG 0 // 0 = Disabled | 1 = Enabled
#if DEBUG
  #define LOG(x) std::cout << x
#else
  #define LOG(x)
#endif