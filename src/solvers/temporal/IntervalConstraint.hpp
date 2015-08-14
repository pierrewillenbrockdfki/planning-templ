#ifndef TEMPL_SOLVERS_INTERVAL_CONSTRAINT_HPP
#define TEMPL_SOLVERS_INTERVAL_CONSTRAINT_HPP

#include <graph_analysis/Edge.hpp>
#include <templ/solvers/temporal/point_algebra/TimePoint.hpp>

#define T_INTERVALCONSTRAINT(x) boost::dynamic_pointer_cast<templ::solvers::IntervalConstraint>(x)

namespace templ {
namespace solvers {
namespace temporal {

/**
 * An Interval Constraint represents an edge in the constraint network identified by an interval
 * \ which represents the constraint between two variables
 */
class IntervalConstraint : public graph_analysis::Edge
{
private:
    double lowerBound;
    double upperBound;

public:

    typedef boost::shared_ptr<IntervalConstraint> Ptr;

    /**
     * Default constructor for an interval constraint
     */
    IntervalConstraint(point_algebra::TimePoint::Ptr source, point_algebra::TimePoint::Ptr target, double lowerBound, double upperBound);

    virtual ~IntervalConstraint() {}

    /**
     * Get the class name of this constraint
     * \return classname
     */
    virtual std::string getClassName() const { return "IntervalConstraint"; }

    /**
     * Get stringified object
     * \return string repr
     */
    virtual std::string toString() const;

    /**
     * Get the source vertex/variable of this interval constraint
     */
    point_algebra::TimePoint::Ptr getSourceVariable() { return boost::dynamic_pointer_cast<point_algebra::TimePoint>( getSourceVertex()); }

    /**
     * Get the target vertex/variable of this interval constraint
     */
    point_algebra::TimePoint::Ptr getTargetVariable() { return boost::dynamic_pointer_cast<point_algebra::TimePoint>( getTargetVertex()); }

    void setLowerBound(double bound) { lowerBound = bound; }

    void setUpperBound(double bound) { upperBound = bound; }
    /**
     * Get the lower bound of this constraint
     */
    double getLowerBound() { return lowerBound; }

    /**
     * Get the upper bound of this constraint
     */
    double getUpperBound() { return upperBound; }

protected:

    /// Make sure cloning works for this constraint
    virtual graph_analysis::Edge* doClone() { return new IntervalConstraint(*this); }

};

typedef std::vector<IntervalConstraint::Ptr> IntervalConstraintList;

} // end namespace temporal
} // end namespace solvers
} // end namespace templ

#endif // TEMPL_SOLVERS_INTERVAL_CONSTRAINT_HPP
