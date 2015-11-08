#pragma once

/*
   Author: Maksym Planeta

   Copyright (c) 2015 Maksym Planeta, TU Dresden.

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

/*
 * @file   xsched.h
 * @brief  Facility for runtime measurements.
 * @author Maksym Planeta mplaneta@os.inf.tu-dresden.de
 */

#include "xthread.h"

class xsched {
  /*** process local ***/

  xthread &_thread;
  // Pointer to common schedule
  int *_thunk_schedule;
  size_t _sched_size;
  // Pointer to thread local piece of schedule
  int *_thread_schedule;

  int get_affinity(int thunk);
public:

  xsched(xthread &thread);

  void updateThread();

  int getCPU();

  void trigger();
};
