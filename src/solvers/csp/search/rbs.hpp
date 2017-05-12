/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2014
 *     Guido Tack, 2012
 *
 *  Last modified:
 *     $Date: 2016-04-19 17:19:45 +0200 (Di, 19 Apr 2016) $ by $Author: schulte $
 *     $Revision: 14967 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/search/support.hh>
#include <gecode/search/meta/dead.hh>

namespace Gecode { namespace Search { namespace Meta {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop*
  stop(Stop* so);

  /// Create restart engine
  GECODE_SEARCH_EXPORT Engine*
  engine(Space* master, Stop* stop, Engine* slave,
         const Search::Statistics& stat, const Options& opt,
         bool best);

}}}

namespace Gecode { namespace Search {

  /// A TemplRBS engine builder
  template<class T, template<class> class E>
  class TemplRbsBuilder : public Builder {
    using Builder::opt;
  public:
    /// The constructor
    TemplRbsBuilder(const Options& opt);
    /// The actual build function
    virtual Engine* operator() (Space* s) const;
  };

  template<class T, template<class> class E>
  inline
  TemplRbsBuilder<T,E>::TemplRbsBuilder(const Options& opt)
    : Builder(opt,E<T>::best) {}

  template<class T, template<class> class E>
  Engine*
  TemplRbsBuilder<T,E>::operator() (Space* s) const {
    return build<T,TemplRBS<T,E> >(s,opt);
  }

}}

namespace Gecode {

  template<class T, template<class> class E>
  inline
  TemplRBS<T,E>::TemplRBS(T* s, const Search::Options& m_opt) {
    if (m_opt.cutoff == NULL)
      throw Search::UninitializedCutoff("TemplRBS::TemplRBS");
    Search::Options e_opt(m_opt.expand());
    Search::Statistics stat;
    e_opt.clone = false;
    e_opt.stop  = Search::Meta::stop(m_opt.stop);
    Space* master;
    Space* slave;
    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      master = NULL;
      slave  = NULL;
      e = new Search::Meta::Dead(stat);
    } else {
      master = m_opt.clone ? s->clone() : s;
      slave  = master->clone(true,m_opt.share_rbs);
      MetaInfo mi(0,0,0,NULL,NoGoods::eng);
      slave->slave(mi);
      e = Search::Meta::engine(master,e_opt.stop,Search::build<T,E>(slave,e_opt),
                               stat,m_opt,E<T>::best);
    }
  }


  template<class T, template<class> class E>
  inline T*
  templ_rbs(T* s, const Search::Options& o) {
    TemplRBS<T,E> r(s,o);
    return r.next();
  }

  template<class T, template<class> class E>
  SEB
  templ_rbs(const Search::Options& o) {
    if (o.cutoff == NULL)
      throw Search::UninitializedCutoff("rbs");
    return new Search::TemplRbsBuilder<T,E>(o);
  }


}

// STATISTICS: search-meta
