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
 * @file   schas.h
 * @brief  Schedule assembler: Text to binary representation of schedule.
 * @author Maksym Planeta mplaneta@os.inf.tu-dresden.de
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <schas.hpp>

using namespace std;

using schas::MemoryRange;
using schas::MemoryThunk;
using schas::MemoryHeader;
using schas::READ;
using schas::WRITE;

void thunk_scheduler(const char *schedule_file)
{
  // Thunk schedule represented as a map
  map<int, map<int, int> > t_map;
  ifstream tsched;
  tsched.open(schedule_file);

  int i = 0;
  std::string line;
  while (std::getline(tsched, line)) {
    std::istringstream iss(line);
    int pid, tid, cpu; // Process id, Thunk id, CPU number

    ++i;

    if (!(iss >> pid >> tid >> cpu)) {
      // error
      fprintf(stderr, "Error in line %d\n", i);
      throw std::runtime_error("Wrong thunk schedule format");
    }

    t_map[pid][tid] = cpu;
    // cout << pid << "  " << tid << "  "  << cpu << endl;
  }

  ofstream tsched_out;
  tsched_out.open(string(schedule_file) + ".bin", ofstream::out | ofstream::binary);
  for (auto &thread : t_map) {
    int size = t_map[thread.first].size();
    tsched_out.write((char *)&thread.first, sizeof thread.first);
    tsched_out.write((char *)&size, sizeof size);
    cout << thread.first << "(" << sizeof thread.first << ")  "
         << size << "(" << sizeof size << ")  "  << endl;
    cout << "== " << endl;
    for (auto &thunk : t_map[thread.first]) {
      cout << thunk.first << "  " << thunk.second << endl;
      tsched_out.write((char *)&thunk.first, sizeof thunk.first);
      tsched_out.write((char *)&thunk.second, sizeof thunk.second);
    }
    cout << "--- " << endl << endl;
  }
}

void page_scheduler(const char *schedule_file)
{
  // Thunk schedule represented as a map
  map<int, map<int, vector<MemoryRange> > > t_map;
  ifstream psched;
  psched.open(schedule_file);

  int i = 0;
  std::string line;
  while (std::getline(psched, line)) {
    std::istringstream iss(line);
    string type;
    int pid, tid; // Process id, Thunk id
    off_t start, end; // Memory range
    int cpu;  // Currently ignored
    int flags; // Currently ignored

    ++i;

    if (!(iss >> type >> pid >> tid >> start >> end >> cpu >> flags)) {
      // error
      fprintf(stderr, "Error in line %d\n", i);
      throw std::runtime_error("Wrong thunk schedule format");
    }

    t_map[pid][tid].push_back(MemoryRange(start, end, cpu, flags));
    cout << pid << "  " << tid << "  "  << start << "  " << end << "  " << cpu << "  " << flags << endl;
  }

  ofstream psched_out;
  psched_out.open(string(schedule_file) + ".bin", ofstream::out | ofstream::binary);
  for (auto &thread : t_map) {
    int size = t_map[thread.first].size();
    psched_out.write((char *)&thread.first, sizeof thread.first);
    psched_out.write((char *)&size, sizeof size);
    cout << thread.first << "(" << sizeof thread.first << ")  "
         << size << "(" << sizeof size << ")  "  << endl;
    cout << "== " << endl;
    for (auto &thunk : t_map[thread.first]) {
      int size = thunk.second.size();
      cout << ">> size " << size << endl;
      psched_out.write((char *)&size, sizeof size);
      for (auto &range : thunk.second) {
        cout << thunk.first << "  " << range << endl;
        psched_out.write((char *)&thunk.first, sizeof thunk.first);
        psched_out.write((char *)&range, sizeof thunk.second);
      }
    }
    cout << "--- " << endl << endl;
  }
}

void usage(const char *name)
{
  fprintf(stderr, "Usage: %s [-p <page_schedule>] [-t <thunk_schedule>] [-h]\n", name);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    usage(argv[0]);
    throw std::runtime_error("Wrong command line");
  }

  const char *tsched = NULL, *psched = NULL;
  bool work = false;
  int c;
  opterr = 0;

  while ((c = getopt(argc, argv, "t:p:h")) !=  -1) {
    switch (c) {
    case 't':
      tsched = strdup(optarg);
      work = true;
      break;
    case 'p':
      psched = strdup(optarg);
      work = true;
      break;
    case 'h':
      usage(argv[0]);
      work = true;
      break;
    case '?':
      if ((optopt == 'p') || (optopt == 't'))
        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint (optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      usage(argv[0]);
      abort ();
    }
  }

  if (!work || optind < argc) {
    usage(argv[0]);
    return 1;
  }

  if (tsched) {
    printf("Generating thunk schedule...\n");
    thunk_scheduler(tsched);
  }

  if (psched) {
    printf("Generating page schedule...\n");
    page_scheduler(psched);
  }

  return 0;
}
