#if !defined(AFX_STDAFX_H__FB3F734B_F9A0_43B6_B834_085D7659CD01__INCLUDED_)
#define AFX_STDAFX_H__FB3F734B_F9A0_43B6_B834_085D7659CD01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma warning(disable: 4244 4554 4800)
#endif // _MSC_VER > 1000

#define _WIN32_IE 0x0500
#define FD_SETSIZE 1024
#define NOMINMAX
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define atoll _atoi64

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <boost/algorithm/string.hpp>
#include <sys/stat.h>
#include <afxsock.h>		// MFC socket extensions
#include <cassert>
#include <fstream>
#include <io.h>
#include <list>
#include <map>
#include <natupnp.h>
#include <set>
#include <shlwapi.h>
#include <sstream>
#include <string>
#include <vector>
#include "windows/ETSLayout.h"
#include "bt_misc.h"
#include "sha1.h"
#include "virtual_binary.h"

using namespace boost;
using namespace std;

inline __int64 max(__int64 a, __int64 b)
{
	return a > b ? a : b;
}

inline __int64 min(__int64 a, __int64 b)
{
	return a < b ? a : b;
}

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_STDAFX_H__FB3F734B_F9A0_43B6_B834_085D7659CD01__INCLUDED_)
