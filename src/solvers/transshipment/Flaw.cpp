#include "Flaw.hpp"

using namespace graph_analysis::algorithms;

namespace templ {
namespace solvers {
namespace transshipment {

std::string Flaw::toString(size_t indent) const
{
    std::stringstream ss;
    std::string hspace(indent,' ');
    switch(violation.getType())
    {
        case ConstraintViolation::MinFlow:
                ss << hspace << "Minflow violation in timeline:" << std::endl
                    << hspace << "    missing: " << violation.getDelta() << std::endl
                    << hspace << "    context:" << std::endl
                    << hspace << "        previous requirement: " << previousFtr.toString(indent + 12)
                    << hspace << "        current requrirement: " << ftr.toString(indent + 12) << std::endl
                    << hspace << "        previous interval: " << previousFtr.getInterval().toString() << std::endl
                    << hspace << "        current interval: " << ftr.getInterval().toString() << std::endl
                    ;
                    //<< " timeline: " << std::endl
                    //<< roleTimeline.toString()
                    //<< std::endl;
                break;
        case ConstraintViolation::TotalMinFlow:
                ss << hspace << "TotalMinflow violation in timeline:" << std::endl
                    << hspace << "    missing: " << violation.getDelta() << std::endl
                    << hspace << "    context:" << std::endl
                    << hspace << "        previous requirement: " << previousFtr.toString(indent + 12)
                    << hspace << "        current requrirement: " << ftr.toString(indent + 12) << std::endl
                    << hspace << "        previous interval: " << previousFtr.getInterval().toString() << std::endl
                    << hspace << "        current interval: " << ftr.getInterval().toString() << std::endl
                    ;
                    //<< " timeline: " << std::endl
                    //<< roleTimeline.toString()
                    //<< std::endl;
                break;
        case ConstraintViolation::TransFlow:
                ss << hspace << "Transflow violation in timeline:" << std::endl
                    << hspace << "    missing: " << violation.getDelta() << std::endl
                    << hspace << "    context:" << std::endl
                    << hspace << "        current requirement: " << ftr.toString(indent + 12) << std::endl
                    << hspace << "        subsequent requirement: " << subsequentFtr.toString(indent + 12)
                    << hspace << "        current interval: " << ftr.getInterval().toString() << std::endl
                    << hspace << "        subsequent interval: " << subsequentFtr.getInterval().toString() << std::endl
                    ;
                    //<< " timeline: " << std::endl
                    //<< roleTimeline.toString()
                    //<< std::endl;
                break;
        case ConstraintViolation::TotalTransFlow:
                ss << hspace << "TotalTransflow violation in timeline:" << std::endl
                    << hspace << "    missing: " << violation.getDelta() << std::endl
                    << hspace << "    context:" << std::endl
                    << hspace << "        current requirement: " << ftr.toString(indent + 12) << std::endl
                    << hspace << "        subsequent requirement: " << subsequentFtr.toString(indent + 12)
                    << hspace << "        current interval: " << ftr.getInterval().toString() << std::endl
                    << hspace << "        subsequent interval: " << subsequentFtr.getInterval().toString() << std::endl
                    ;
                    ;
                break;

    }
    return ss.str();
}

} // end namespace transshipment
} // end namespace solvers
} // end namespace templ
