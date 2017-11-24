#ifndef TEMPL_SIMPLE_CONSTRAINT_HPP
#define TEMPL_SIMPLE_CONSTRAINT_HPP

#include <graph_analysis/Edge.hpp>
#include "../Constraint.hpp"

namespace templ {
namespace constraints {

class SimpleConstraint :  public virtual Constraint, public graph_analysis::Edge
{
public:
    typedef shared_ptr<Constraint> Ptr;

    /* Default constructor for a constraint
     */
    SimpleConstraint(Type type = UNKNOWN);

    /**
     * Constructor for a constraint using source and target
     */
    SimpleConstraint(Type type, const Variable::Ptr& source, const Variable::Ptr& target);

    /**
     * Deconstructor
     */
    virtual ~SimpleConstraint();

    /**
     * Get the class name of this constraint
     * \return classname
     */
    virtual std::string getClassName() const override { return "SimpleConstraint"; }

    /**
     * Get stringified object
     * \return string repr
     */
    virtual std::string toString() const override;

    /**
     * Get stringified object
     * \return string repr
     */
    virtual std::string toString(uint32_t indent) const override;

    /**
     * Get the source vertex/variable of this constraint
     */
    Variable::Ptr getSourceVariable() const { return dynamic_pointer_cast<Variable>( getSourceVertex()); }

    /**
     * Get the target vertex/variable of this constraint
     */
    Variable::Ptr getTargetVariable() const { return dynamic_pointer_cast<Variable>( getTargetVertex()); }

protected:
    /// Make sure cloning works for this constraint
    graph_analysis::Edge* getClone() const override { return new SimpleConstraint(*this); }
};

} // end namespace constraints
} // end namespace templ
#endif // TEMPL_SIMPLE_CONSTRAINT_HPP
