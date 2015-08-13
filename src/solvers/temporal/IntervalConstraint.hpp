#ifndef TEMPL_SOLVERS_INTERVAL_CONSTRAINT_HPP
#define TEMPL_SOLVERS_INTERVAL_CONSTRAINT_HPP

#include <templ/solvers/Constraint.hpp>

//#define T_INTERVALCONSTRAINT(x) boost::dynamic_pointer_cast<templ::solvers::IntervalConstraint>(x)

namespace templ {
namespace solvers {

/**
 * An Interval Constraint represents an edge in the constraint network identified by an interval
 * \ which represents the constraint between two variables
 */
class IntervalConstraint : public Constraint
{
private:
    double lowerBound;
    double upperBound;

public:

    typedef boost::shared_ptr<IntervalConstraint> Ptr;

    /**
     * Default constructor for a constraint
     */
    IntervalConstraint(Variable::Ptr source, Variable::Ptr target, double lowerBound, double upperBound);

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
     * Get the source vertex/variable of this constraint
     */
    Variable::Ptr getSourceVariable() { return boost::dynamic_pointer_cast<Variable>( getSourceVertex()); }

    /**
     * Get the target vertex/variable of this constraint
     */
    Variable::Ptr getTargetVariable() { return boost::dynamic_pointer_cast<Variable>( getTargetVertex()); }

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
    virtual graph_analysis::Edge* doClone() { return new Constraint(*this); }

};

typedef std::vector<IntervalConstraint::Ptr> IntervalConstraintList;

} // end namespace solvers
} // end namespace templ

#endif // TEMPL_SOLVERS_INTERVAL_CONSTRAINT_HPP
