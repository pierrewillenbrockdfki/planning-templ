#ifndef TEMPL_EVENT_HPP
#define TEMPL_EVENT_HPP

#include <templ/Symbol.hpp>
#include <templ/solvers/temporal/point_algebra/TimePoint.hpp>
#include <templ/solvers/temporal/TemporalAssertion.hpp>

namespace templ {
namespace solvers {
namespace temporal {

/**
 * \class Event
 * \brief "An event, denoted as x@t:(v_1,v_2) specifies the instantaneous change of the value of
 * x from v_1 to _v2 at time t, with v_1 != v_2"
 */
class Event : public TemporalAssertion
{
    friend class PersistenceCondition;

protected:
    Symbol::Ptr mpFromValue;
    Symbol::Ptr mpToValue;
    
    // Time constant or time variable
    point_algebra::TimePoint::Ptr mpTimepoint;

    bool refersToSameValue(const boost::shared_ptr<Event>& other, const point_algebra::TimePointComparator& comparator) const;
    bool refersToSameValue(const boost::shared_ptr<PersistenceCondition>& other, const point_algebra::TimePointComparator& comparator) const;

    bool disjointFrom(const boost::shared_ptr<Event>& other, const point_algebra::TimePointComparator& comparator) const;
    bool disjointFrom(const boost::shared_ptr<PersistenceCondition>& other, const point_algebra::TimePointComparator& comparator) const;

public:
    typedef boost::shared_ptr<Event> Ptr;

    static Event::Ptr getInstance(const symbols::StateVariable& stateVariable,
            const Symbol::Ptr& from,
            const Symbol::Ptr& to,
            const point_algebra::TimePoint::Ptr& timepoint);

    Event(const symbols::StateVariable& stateVariable,
            const Symbol::Ptr& from,
            const Symbol::Ptr& to,
            const point_algebra::TimePoint::Ptr& timepoint);

    Symbol::Ptr getFromValue() const { return mpFromValue; }
    Symbol::Ptr getToValue() const { return mpToValue; }

    point_algebra::TimePoint::Ptr getTimePoint() const { return mpTimepoint; }

    std::string toString() const;
};

} // end namespace temporal
} // end namespace solvers
} // end namespace templ
#endif // TEMPL_EVENT_HPP
