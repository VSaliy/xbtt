#if !defined(AFX_STDAFX_H__934FECC7_0914_4243_9D96_05910F2591B5__INCLUDED_)
#define AFX_STDAFX_H__934FECC7_0914_4243_9D96_05910F2591B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)

#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <vector>

using namespace std;

#ifdef WIN32
#define for if (0) {} else for

#pragma comment(lib, "ws2_32.lib")
#else
#include <stdint.h>
#endif
#include "socket.h"
#include "virtual_binary.h"

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_STDAFX_H__934FECC7_0914_4243_9D96_05910F2591B5__INCLUDED_)
