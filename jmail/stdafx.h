// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(JMAIL_STDAFX_H)
#define JMAIL_STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream.h>
#include <process.h>
#include <afx.h>
#include <vector>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>

class errors {
public:
  char err[80];
  errors(const char *l){ strcpy(err,l); }
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

#include "config.h"
#include "color.h"
#include "remote.h"
#include "jmail.h"
#include "window.h"
#include "hydra.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // JMAIL_STDAFX_H
