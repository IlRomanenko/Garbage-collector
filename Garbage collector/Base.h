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



#define LOG cerr

#ifdef _DEBUG

#include <vld.h> //Visual Leak Detector

#define PARANOID //special for GC memory_buffer (see destructor)

#else

#endif
