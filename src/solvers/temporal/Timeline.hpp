#ifndef TEMPL_TIMELINE_HPP
#define TEMPL_TIMELINE_HPP

#include <templ/ConstraintList.hpp>
#include <templ/TemporalAssertion.hpp>

namespace templ {
namespace solvers {
namespace temporal {

/**
 * \class Timeline
 * \brief A timeline is a chronicle for a single state variable x
 * It serves as partial specification of a function that gives
 * the value of the state variable over time
 *
 * \see Chronicle
 */
class Timeline
{
    StateVariable mStateVariable;

    TemporalAssertionList mTemporalAssertions;
    ConstraintList mConstraints;

public:
    /**
     * Check if timeline is consistent, i.e. (Automated Planning p 331 Def.
     * 14.8)
     * - no pair of assertions in the timeline conflicts
     *   i.e. no specification leaves two possibly distinct values of the same
     *   state variable at the same time
     * - every pair of assertions is either disjoint or refers to the same value
     *   and/or the same timepoint
     *
     */
    bool isConsistent() const;
};

} // end namespace temporal
} // end namespace solvers
} // end namespace templ
#endif // TEMPL_TIMELINE_HPP
