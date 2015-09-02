// -*- C++ -*-

/*
   Author: Emery Berger, http://www.cs.umass.edu/~emery

   Copyright (c) 2007-8 Emery Berger, University of Massachusetts Amherst.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */
#ifndef _XGLOBALS_H_
#define _XGLOBALS_H_

#include "xdefines.h"
#include "xpersist.h"

#include <iostream>
using namespace std;

/// @class xglobals
/// @brief Maps the globals region onto a persistent store.
class xglobals : public xpersist<char, xdefines::MAX_GLOBALS_SIZE>{
public:

  xglobals(void) : xpersist<char, xdefines::MAX_GLOBALS_SIZE>(
      (void *)GLOBALS_START,
      (size_t)GLOBALS_SIZE) {
    // Make sure that we have enough room for the globals!
    assert(GLOBALS_SIZE <= xdefines::MAX_GLOBALS_SIZE);
    DEBUGF("GLOBALS_START is %lx, global_start %d\n",
           GLOBALS_START,
           __data_start);
  }
};

#endif // ifndef _XGLOBALS_H_
