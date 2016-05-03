#pragma once
#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;



#ifdef _DEBUG

#define PARANOID //special for GC memory_buffer (see destructor)

#ifndef LOG_DEF
#define LOG cerr
#define LOG_DEF
#endif

#else

#define LOG 

#endif