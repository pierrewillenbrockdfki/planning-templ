#pragma once

#include <gecode/minimodel.hh>
#include <templ/solvers/Variable.hpp>
#include <templ/Symbol.hpp>
#include <templ/Role.hpp>

namespace templ {
namespace solvers {
namespace csp {
namespace utils {

class Formatter
{
public:
    /**
     * Print a set of array in matrix form
     */
    static std::string toString(const std::vector<Gecode::IntVarArray>& arrays, const std::vector<Symbol::Ptr>& fluents, const std::vector<Variable::Ptr>& variables, const std::vector<std::string>& arrayLabels);

    /**
     * Print an array in matrix form
     */
    static std::string toString(const Gecode::IntVarArray& array, const std::vector<Symbol::Ptr>& fluents, const std::vector<Variable::Ptr>& variables);

};

} // end namespace utils
} // end namespace csp
} // end namespace solvers
} // end namespace templ
