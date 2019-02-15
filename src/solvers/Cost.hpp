#ifndef TEMPL_SOLVERS_COST_HPP
#define TEMPL_SOLVERS_COST_HPP

#include "../symbols/constants/Location.hpp"
#include "../Role.hpp"
#include <organization_model/OrganizationModelAsk.hpp>
#include <organization_model/vocabularies/OM.hpp>

namespace templ {
namespace solvers {

class Cost
{
public:
    Cost(const organization_model::OrganizationModelAsk& organizationModelAsk,
            const owlapi::model::IRI& mobilityFunctionality =
            organization_model::vocabulary::OM::resolve("MoveTo"),
            double feasibilityCheckTimeoutInMs = 20000);

    /**
     * Compute the travel distance from a given path using the norm of the
     * segment distances
     */
    static double getTravelDistance(const symbols::constants::Location::PtrList& path);

    /**
     * Estimate the travel time from a particular location to a destination for
     * a given coalition
     */
    double estimateTravelTime(const symbols::constants::Location::Ptr& from,
            const symbols::constants::Location::Ptr& to,
            const Coalition& coalition);

    /**
     * Estimate the reconfiguration cost from a particular CoalitionStructure
     * to a particular coalition structure
     * \return cost
     */
    double estimateReconfigurationCost(const CoalitionStructure& from,
        const CoalitionStructure& to);

private:
    organization_model::OrganizationModelAsk mOrganizationModelAsk;
    organization_model::Resource::Set mMoveToResource;
    double mFeasibilityCheckTimeoutInMs;
};

} // end namespace solvers
} // end namespace templ
#endif // TEMPL_SOLVERS_COST_HPP
