// PlatheaDLL.cpp: definisce le funzioni esportate per l'applicazione DLL.


#include "stdafx.h"
#include <utility>
#include <limits.h>
#include "PlatheaDLL.h"

// DLL internal state variables:

#include <Windows.h>
#include <iostream>
#include <sstream>

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s << "\n";                   \
   OutputDebugStringW( os_.str().c_str() );  \
}


void system_loadconfigurationfile() {


}


