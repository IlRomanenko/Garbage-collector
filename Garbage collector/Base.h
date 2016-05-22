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
#include <queue>
#include <cassert>

using namespace std;


class ISmartObject;
class AllocatedMemoryChunk;


#define LOG cerr

#ifdef _DEBUG

#include <vld.h> //Visual Leak Detector

#else

#endif
