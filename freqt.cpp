/*
   Copyright (C) 2003 Taku Kudo, All rights reserved.
   This is free software with ABSOLUTELY NO WARRANTY.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA
*/
  
#include <iostream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <iterator>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "config.hpp"
#include "freqt.hpp"



class Freqt {

private:

  std::vector < nodes_t > transaction;
  std::vector < std::string > pattern;
  unsigned int minsup;
  unsigned int minpat;
  unsigned int maxpat;
  bool weighted;
  bool enc;
  bool where;
  std::ostream *os;
  READER_FUNC reader_func;

  std::istream& read (std::istream &is)
  {
    std::string line;
    while (std::getline (is, line)) {
      if (line.empty() || line[0] == ';') continue;
      transaction.resize (transaction.size()+1);
      this->reader_func (line, transaction[transaction.size()-1]);
    }
    return is;
  }

  unsigned int support (const projected_t &projected)
  {
    if (weighted) return projected.locations.size(); // weighted
    unsigned int old = 0xffffffff; // INT_MAX
    unsigned int size = 0; 
    for (unsigned int i = 0; i < projected.locations.size(); ++i) {
      if (projected.locations[i].first != old) ++size;
      old = projected.locations[i].first;
    }

    return size;
  }

  void report (const projected_t &projected)
  {
    unsigned size  = 0;
    unsigned int sup = projected.support;
    unsigned int wsup = projected.locations.size();

    for (unsigned int i = 0; i < pattern.size(); ++i) 
      if (pattern[i] != ")") ++size;

    if (size < minpat) return;

    if (where) {
      *os << "<pattern>\n";
      *os << "<support>" << sup << "</support>\n";
      *os << "<wsupport>" << wsup << "</wsupport>\n";
      *os << "<what>";
    } else {
      *os << sup << " " << wsup << " " << size << " ";
    }

    if (enc) {
      for (unsigned int i = 0; i < pattern.size(); ++i) {
	if (i != 0) *os << ' ';
	*os << pattern[i];
      }
    } else {
      int n = 0;
      for (unsigned int i = 0; i < pattern.size(); ++i) {
	if (pattern[i] == ")") {
	  *os << ')';
	  --n;
	} else {
	  ++n;
	  *os << '(' << pattern[i];
	}
      }
      for (int i = 0; i < n; ++i) *os << ')';
    }

    if (where) {
      *os << "</what>\n<where>";
      for (unsigned int i = 0; i < projected.locations.size(); ++i) {
	if (i != 0) *os << ' ';
	  *os << projected.locations[i].first;
      }
      *os << "</where>\n</pattern>";
    }
    
    *os << '\n';

    return;
  }

  void prune (std::map <std::string, projected_t>& candidate)
  {
    for (std::map <std::string, projected_t>::iterator l = candidate.begin (); 
         l != candidate.end(); ) {
      unsigned int sup= support (l->second);
      if (sup < minsup) {
        std::map <std::string, projected_t>::iterator tmp = l;
        tmp = l;
        ++tmp;
        candidate.erase (l);
        l = tmp;
      } else {
	l->second.support = sup;
        ++l;
      }
    }

    return;
  }

  void project (const projected_t &projected)
  {
    if (pattern.size() >= maxpat) return;

    int depth = projected.depth;
    std::map <std::string, projected_t> candidate;

    for (unsigned int i = 0; i < projected.locations.size(); ++i) {
      unsigned int  id = projected.locations[i].first;
      int          pos = projected.locations[i].second;
      std::string prefix = "";

      for (int d = -1 ; d < depth && pos != -1; ++d) {

	int start    = (d == -1) ? transaction[id][pos].child : transaction[id][pos].sibling;
	int newdepth = depth - d;
	for (int l = start; l != -1; l = transaction[id][l].sibling) {
	  std::string item = prefix + " " + transaction[id][l].val;
	  projected_t &tmp = candidate[item];
	  tmp.locations.push_back (std::make_pair <unsigned int, int> (id, l));
	  tmp.depth = newdepth;
	}

	if (d != -1) pos = transaction[id][pos].parent;
	prefix += " )";
      }
    }

    prune (candidate);

    for (std::map <std::string, projected_t>::iterator l = candidate.begin (); 
         l != candidate.end(); ++l) {
      unsigned int size = pattern.size();
      tokenize<std::string>(l->first, std::back_inserter(pattern));
      report (l->second);
      project (l->second);
      pattern.resize (size);
    }

    return;
  }

public:

  void run (std::istream &_is, std::ostream &_os,
	    unsigned int _minsup,
	    unsigned int _minpat,
	    unsigned int _maxpat,
	    bool _weighted,
	    bool _enc,
	    bool _where,
        READER_FUNC _reader_func)
  { 
    minsup   = _minsup;
    minpat   = _minpat;
    maxpat   = _maxpat;
    enc      = _enc;
    weighted = _weighted;
    where    = _where;
    os       = &_os;
    reader_func = _reader_func;
    read (_is);

    std::map <std::string, projected_t> freq1;
    for (unsigned int i = 0; i < transaction.size(); ++i) 
      for (unsigned int j = 0; j < transaction[i].size(); ++j) 
	freq1[transaction[i][j].val].locations.push_back (std::make_pair <unsigned int, int> (i, j));

    prune (freq1);
     
    if (where) *os << "<?xml version=\"1.0\"?>\n<results>\n";

    for (std::map <std::string, projected_t>::iterator l = freq1.begin (); 
         l != freq1.end(); ++l) {
      l->second.depth = 0;
      pattern.push_back (l->first);
      report  (l->second);
      project (l->second);
      pattern.resize (pattern.size()-1);
    }
    if (where) *os << "</results>\n";

    return;
  }

  Freqt (): minsup (1), minpat (1), maxpat (0xffffffff), 
            weighted (false), enc (false), where (false), os(&std::cout) {};
  ~Freqt () {};
};

int main (int argc, char **argv)
{
  extern char *optarg;
  unsigned int minsup = 1;
  unsigned int minpat = 1;
  unsigned int maxpat = 0xffffffff;
  bool enc = false;
  bool where = false;
  bool weighted = false;
  unsigned int format = 0;

  int opt;
  while ((opt = getopt(argc, argv, "weWM:m:L:f:")) != -1) {
    switch(opt) {
    case 'm':
      minsup = atoi (optarg);
      break;
    case 'M':
      minpat = atoi (optarg);
      break;
    case 'L':
      maxpat = atoi (optarg);
      break;
    case 'f':
      format = atoi (optarg);
      break;
    case 'e':
      enc = true;
      break;
    case 'w':
      where = true;
      break;
    case 'W':
      weighted = true;
      break;
    default:
     std::cerr << "Usage: " << argv[0] 
           << " [-m minsup] [-M minpat] [-L maxpat] [-e] [-W] < data .." << std::endl;
     std::cerr << "Version: " << CONST_VERSION        
           << " (Build at " << CONST_COMF_DATETIME << ")" << std::endl;

      return -1;
    }
  }

  READER_FUNC reader_func = str2node;
  switch(format){
      case 0:
          break;
      case 1:
          break;
      default:
        std::cerr << "Unknown format type" << std::endl;
        return -1;
  };

  Freqt freqt;
  freqt.run (std::cin, std::cout, 
	     minsup, minpat, maxpat, weighted, enc, where, reader_func);

  return 0;
}
