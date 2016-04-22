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

using namespace std;



#ifdef _DEBUG

    ofstream LOG(stderr);

#else

#define LOG 

#endif