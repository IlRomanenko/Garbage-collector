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

using namespace std;



#ifdef _DEBUG

#ifndef LOG_DEF
//ofstream LOG(stderr);
#define LOG cerr
#define LOG_DEF
#endif

#else

#define LOG 

#endif