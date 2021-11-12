//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA
//_/_/ Autocatalytic Endogenous Reflective Architecture
//_/_/ 
//_/_/ Copyright (c) 2018-2021 Jeff Thompson
//_/_/ Copyright (c) 2018-2021 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018 Jacqueline Clare Mallett
//_/_/ Copyright (c) 2018-2021 Icelandic Institute for Intelligent Machines
//_/_/ http://www.iiim.is
//_/_/ 
//_/_/ Copyright (c) 2012 Jan Koutnik
//_/_/ Center for Analysis and Design of Intelligent Agents
//_/_/ Reykjavik University, Menntavegur 1, 102 Reykjavik, Iceland
//_/_/ http://cadia.ru.is
//_/_/ 
//_/_/ Part of this software was developed by Eric Nivel
//_/_/ in the HUMANOBS EU research project, which included
//_/_/ the following parties:
//_/_/
//_/_/ Autonomous Systems Laboratory
//_/_/ Technical University of Madrid, Spain
//_/_/ http://www.aslab.org/
//_/_/
//_/_/ Communicative Machines
//_/_/ Edinburgh, United Kingdom
//_/_/ http://www.cmlabs.com/
//_/_/
//_/_/ Istituto Dalle Molle di Studi sull'Intelligenza Artificiale
//_/_/ University of Lugano and SUPSI, Switzerland
//_/_/ http://www.idsia.ch/
//_/_/
//_/_/ Institute of Cognitive Sciences and Technologies
//_/_/ Consiglio Nazionale delle Ricerche, Italy
//_/_/ http://www.istc.cnr.it/
//_/_/
//_/_/ Dipartimento di Ingegneria Informatica
//_/_/ University of Palermo, Italy
//_/_/ http://diid.unipa.it/roboticslab/
//_/_/
//_/_/
//_/_/ --- HUMANOBS Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without
//_/_/ modification, is permitted provided that the following conditions
//_/_/ are met:
//_/_/ - Redistributions of source code must retain the above copyright
//_/_/   and collaboration notice, this list of conditions and the
//_/_/   following disclaimer.
//_/_/ - Redistributions in binary form must reproduce the above copyright
//_/_/   notice, this list of conditions and the following disclaimer 
//_/_/   in the documentation and/or other materials provided with 
//_/_/   the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its
//_/_/   contributors may be used to endorse or promote products
//_/_/   derived from this software without specific prior 
//_/_/   written permission.
//_/_/   
//_/_/ - CADIA Clause: The license granted in and to the software 
//_/_/   under this agreement is a limited-use license. 
//_/_/   The software may not be used in furtherance of:
//_/_/    (i)   intentionally causing bodily injury or severe emotional 
//_/_/          distress to any person;
//_/_/    (ii)  invading the personal privacy or violating the human 
//_/_/          rights of any person; or
//_/_/    (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
//_/_/ CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
//_/_/ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//_/_/ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//_/_/ DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//_/_/ CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//_/_/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//_/_/ SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//_/_/ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//_/_/ WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//_/_/ NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//_/_/ OF SUCH DAMAGE.
//_/_/ 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef winepi_h
#define winepi_h

// do not uncomment this switch; implementation incomplete!
//#define WINEPI_SERIAL

#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <utility> // pair
#include <fstream>
#include <iostream> // cout
#include <ctime> // clock_t

#include "../submodules/CoreLibrary/CoreLibrary/base.h"
#include <../r_code/object.h>

//#include "correlator.h"
typedef uint64 timestamp_t;
typedef P<r_code::Code> event_t;

// Added to handle operator overloading and map references
inline bool operator<(event_t lhs, event_t rhs) { return (r_code::Code*)lhs < (r_code::Code*)rhs; }
inline bool operator>(event_t lhs, event_t rhs) { return lhs < rhs; }


struct event_less
{
  bool operator()(const event_t lhs, const event_t rhs)  const { return lhs < rhs; }
};

struct event_pair
{
  bool operator()(std::pair<event_t, size_t> lhs, std::pair<event_t, size_t> rhs) const { return lhs.first < rhs.first; }
};

struct event_compare
{
  bool operator() (const event_t e1, const event_t e2) const
  {
    return e1->get_oid() < e2->get_oid();
  }
};


struct Candidate {
  std::map<int, event_t> G_; // mapping from [1..] to event_types
#ifdef WINEPI_SERIAL
  std::multimap<int, int> R; // list of tuples of type (int,int)
#endif
  std::map<event_t, size_t, event_less> type_count_;
  int block_start_;
  int event_count_;
  int freq_count_;
  timestamp_t inwindow_;


  Candidate() {
    init();
  }

  // -- jm made parameter const
  Candidate(const event_t& A) {
    init();
    set(1, A);
  }

  void init() {
    block_start_ = 0;
    event_count_ = 0;
    freq_count_ = -1;
    inwindow_ = 0;
  }

  Candidate& operator=(const Candidate& e) {
    G_.clear();
    G_.insert(e.G_.begin(), e.G_.end());
#ifdef WINEPI_SERIAL
    R.clear();
    R.insert(e.R.begin(), e.R.end());
#endif
    type_count_.clear();
    type_count_.insert(e.type_count_.begin(), e.type_count_.end());
    block_start_ = e.block_start_;
    event_count_ = e.event_count_;
    freq_count_ = e.freq_count_;
    inwindow_ = e.inwindow_;
    return *this;
  }

  bool operator==(const Candidate& e) const {
    return G_ == e.G_;
  }

  bool operator<(const Candidate& e) const {
    std::map<int, event_t>::const_iterator it = G_.begin(), it2 = e.G_.begin();
    //jm added explicit comparison from correlator.cpp -presumably where this was
    // being handled by default in previous c++ versions.
    for (; it != G_.end() && it2 != e.G_.end(); ++it, ++it2)
      if (it->second->get_oid() < it2->second->get_oid())
        return true;
      else if (it->second->get_oid() > it2->second->get_oid())
        return false;
    return (it2 != e.G_.end());
  }

  size_t size() const {
    return G_.size();
  }

  event_t& get(int i) {
    return G_.find(i)->second;
  }


  void set(int i, const event_t& x) {
    // if(G_.find(i) != G_.end())
    // --type_count_[G_[i]];
    ++type_count_[x];
    G_[i] = x;
  }
  /*
      bool order(int x, int y) {
          std::pair<std::multimap<int,int>::iterator,std::multimap<int,int>::iterator> its = R.equal_range(x);
          for(std::multimap<int,int>::iterator it = its.first; it != its.second; ++it)
              if(it->second == y)
                  return true;
          return false;
      }
  */
  std::string toString() const {
    std::stringstream ss;
    ss << "<";
    bool comma = false;
    for (std::map<int, event_t>::const_iterator it = G_.begin(); it != G_.end(); ++it) {
      if (comma)
        ss << ", ";
      else
        comma = true;
      ss << it->second->get_oid() /*<< it->first*/;
    }
#ifdef WINEPI_SERIAL
    for (std::multimap<int, int>::const_iterator it = R.begin(); it != R.end(); ++it)
      ss << ", " << G_.find(it->first)->second /*<< it->first*/ << "->" << G_.find(it->second)->second /*<< it->second*/;
#endif
    ss << ">";
    return ss.str();
  }
};

struct Rule {
  Candidate lhs_;
  Candidate rhs_;
  double conf_;

  Rule(const Candidate& lhs_, const Candidate& rhs_, double conf_) : lhs_(lhs_), rhs_(rhs_), conf_(conf_) {}

  Rule& operator=(const Rule& r) {
    lhs_ = r.lhs_;
    rhs_ = r.rhs_;
    conf_ = r.conf_;
    return *this;
  }

  std::string toString() {
    std::stringstream ss;
    ss << lhs_.toString() << " implies " << rhs_.toString() << " with conf " << conf_;
    return ss.str();
  }
};

// a sequence is a tuple (s, starttime, endtime) where s is a list of tuples (timestamp,event)
// a candidate is a tuple (G,R,block_start)
//   where G is a dict: int -> event_types
//         R is a list of (int,int) tuples
//         block_start is an index

struct Sequence {
  std::multimap<timestamp_t, event_t> seq_;
  timestamp_t start_;
  timestamp_t end_;

  Sequence() {}

  Sequence(std::multimap<timestamp_t, event_t>& seq_)
    : seq_(seq_.begin(), seq_.end())
    , start_(seq_.begin()->first)
    , end_(seq_.rbegin()->first + 1)
  {}

  template<class InputIterator>
  void init(InputIterator it, InputIterator last) {
    seq_.clear();
    seq_.insert(it, last);
    start_ = seq_.begin()->first;
    end_ = seq_.rbegin()->first + 1;
  }

  void addEvent(timestamp_t t, event_t ev) {
    seq_.insert(std::make_pair(t, ev));  //jm make_pair conversion
  }

  std::string toString() {
    std::stringstream ss;
    ss << "<";
    bool comma = false;
    for (std::multimap<timestamp_t, event_t, event_compare>::iterator it = seq_.begin(); it != seq_.end(); ++it) {
      if (comma)
        ss << ", ";
      else
        comma = true;
      ss << it->first << ":" << it->second->get_oid();
    }
    ss << "}, s:" << start_ << ", e:" << end_ << ">";
    return ss.str();
  }
};




class WinEpi {
public:

  Sequence seq_;
  int win_;
  double min_fr_;
  double min_conf_;
  int max_size_;

  std::set<event_t, event_compare> event_types_;

  WinEpi(/*std::multimap<timestamp_t,event_t>& seq, int win, double min_fr, double min_conf, int max_size = -1*/);

  // initialize the sequence; iterator must return pairs
// template<class InputIterator> void setSeq(InputIterator it, InputIterator end);
  template<class InputIterator>
  void setSeq(InputIterator it, InputIterator end) {
    seq_.init(it, end);
    for (; it != end; ++it) {
      event_types_.insert(it->second);
    }
  }

  void setParams(int win, double min_fr, double min_conf, int max_size = -1);

  void algorithm_1(std::vector<Rule>& out);
  void algorithm_2(std::vector<std::vector<Candidate> >& F);
  void algorithm_3(std::vector<Candidate>& F, int el, std::vector<Candidate>& C);
  void algorithm_4(std::vector<Candidate>& C, double min_fr, std::vector<Candidate>& F);
  double fr(Candidate& a);

  template<class K, class V>
  static void subsets(std::map<K, V>& in, std::vector<std::map<K, V> >& out) {
    size_t nIn = in.size();
    size_t nOut = 1 << nIn;
    out.resize(nOut);
    std::map<K, V>::iterator it = in.begin();
    for (size_t i = 0; i < nIn; ++i, ++it) {
      size_t chunk_size = 1 << (nIn - i - 1);
      for (size_t chunk_start = 0; chunk_start < nOut; chunk_start += 2 * chunk_size)
        for (size_t j = chunk_start; j < chunk_start + chunk_size; ++j)
          out[j].insert(*it);
    }
  }

#ifdef WINEPI_SERIAL
  static void restrict(std::multimap<int, int>& in, std::map<int, event_t>& seq, std::multimap<int, int>& out){
      for (std::multimap<int,int>::iterator it = in.begin(); it != in.end(); ++it)
          if (seq.find(it->first) != seq.end() && seq.find(it->second) != seq.end())
              out.insert(*it);
  }
#endif

    static void strict_subcandidates(Candidate& a, std::vector<Candidate>& out) {

    if (a.size() < 2)
      return;

    out.resize((1 << a.size()) - 2);

    std::vector<std::map<int, event_t> > subseqv(1 << a.size());
    subsets(a.G_, subseqv);
    std::vector<std::map<int, event_t> >::iterator it = subseqv.begin();
    std::vector<std::map<int, event_t> >::iterator end = subseqv.end();
    size_t i = 0;
    for (++it, --end; it != end; ++it, ++i) {
#ifdef WINEPI_SERIAL
      std::multimap<int, int> R;
      restrict(a.R, *it, R);
      out[i].R.insert(R.begin(), R.end());
#endif
      for (std::map<int, event_t>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        out[i].set(it2->first, it2->second);
    }
  }
};

#endif
