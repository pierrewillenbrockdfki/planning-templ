#ifndef TEMPL_PLANNING_SOLVERS_TEMPORAL_INTERVAL_HPP
#define TEMPL_PLANNING_SOLVERS_TEMPORAL_INTERVAL_HPP

#include <templ/solvers/temporal/point_algebra/TimePoint.hpp>
#include <templ/solvers/temporal/point_algebra/TimePointComparator.hpp>

namespace templ {
namespace solvers {
namespace temporal {

/**
 * Create an interval
 */
class Interval
{
    point_algebra::TimePoint::Ptr mpFrom;
    point_algebra::TimePoint::Ptr mpTo;

    point_algebra::TimePointComparator mTimePointComparator;

public:
    Interval(const point_algebra::TimePoint::Ptr& from,
            const point_algebra::TimePoint::Ptr& to,
            const point_algebra::TimePointComparator& comparator);

    point_algebra::TimePoint::Ptr getFrom() const { return mpFrom; }
    point_algebra::TimePoint::Ptr getTo() const { return mpTo; }

    bool operator<(const Interval& other) const { return lessThan(other); }
    bool operator==(const Interval& other) const { return equals(other); }


    bool distinctFrom(const Interval& other) const;
    bool lessThan(const Interval& other) const;
    bool equals(const Interval& other) const;

    std::string toString() const;
};

} // end namespace temporal
} // end namespace solvers
} // end namespace templ
#endif // TEMPL_SOLVERS_TEMPORAL_INTERVAL_HPP
