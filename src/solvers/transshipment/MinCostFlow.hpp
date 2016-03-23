#ifndef TEMPL_SOLVER_TRANSSHIPMENT_MINCOSTFLOW_HPP
#define TEMPL_SOLVER_TRANSSHIPMENT_MINCOSTFLOW_HPP

#include <vector>
#include <graph_analysis/BaseGraph.hpp>
#include <graph_analysis/algorithms/MultiCommodityMinCostFlow.hpp>
#include <templ/Mission.hpp>
#include <templ/solvers/csp/RoleTimeline.hpp>
#include <templ/SpaceTimeNetwork.hpp>

namespace templ {
namespace solvers {
namespace transshipment {

struct Flaw
{
    ConstraintViolation violation;
    Role affectedRole;

    csp::FluentTimeResource previousFtr;
    csp::FluentTimeResource ftr;
    csp::FluentTimeResource subsequentFtr;

    Flaw(const graph_analysis::algorithms::ConstraintViolation& violation, 
        const Role& role)
        : violation(violation)
        , role(role)
    {}
};

struct MinCostFlowStatus
{
    graph_analysis::BaseGraph::Ptr flowGraph;
    std::vector<graph_analysis::algorithms::ConstraintViolation> violations;
    uint32_t commodities;
};

class MinCostFlow
{
    /**
     *
     * \param commodities number of existing immobile resources (i.e. roles in
     * this planners context)
     */
    MinCostFlow::MinCostFlow(const Mission& mission,
            const std::map<Role, csp::RoleTimeline>& timelines,
            SpaceTimeNetwork* spaceTimeNetwork,
            uint32_t commodities);
    /**
     *  Translating the space time network into the mincommodity representation,
     *  i.e. the flow graph
     *  This will fill the BipartiteGraph to allow the mapping of the space-time graph
     *  to the flow graph
     *
     *  The flow graph will contain vertices of the type
     *  graph_analysis::algorithms::MultiCommodityMinCostFlow::edge_t::Ptr
     *  and
     *  graph_analysis::algorithms::MultCommodityMinCostFlow::vertex_t::Ptr
     *
     * \param commodities Number of commodities that should be taken into
     * consideration for the underlying MultiCommodityMinCostFlow problem
     */
    BaseGraph::Ptr createFlowGraph(uint32_t commodities);

    /** 
     *  Set the commodity supply and demand 
     *  Since the general transport network is constructed from mobile systems,
     *  that means supply and demand comes from the requirements of immobile systems.
     *  This function thus sets the  'start','end' and '(transition) waypoints' for all
     *  immobile systems
     *
     *  Operates on the flow graph through the mapping of the bipartite graph
     */
    void setCommoditySupplyAndDemand();

    MinCostFlowStatus compute();

    /**
     * After the flow optimization has taken place -- update the space time
     * network, i.e. the locations, with information about the roles
     * Update the spaceTimeNetwork from the data of the flowGraph using the reverse mapping and adding
     * the corresponding (and new) roles
     */
    void updateRoles(const BaseGraph::Ptr& flowGraph);

    //    const std::map<Role, RoleTimeline>& timelines,
    //    const Mission& mission,
    //    SpaceTimeNetwork* spaceTimeNetwork,
    //    uint32_t commodities);
    //
    std::vector<Flaw> computeFlaws(const graph_analysis::algorithms::MultiCommodityMinCostFlow&) const;

    std::vector<templ::solvers::csp::FluentTimeResource>::const_iterator 
        getFluent(const templ::solvers::csp::RoleTimeline& roleTimeline,
            const SpaceTimeNetwork::tuple_t::Ptr& tuple);

private:
    Mission mMission;
    std::map<Role, csp::RoleTimeline> mTimelines;
    std::vector<Role> mCommoditiesRoles;

    SpaceTimeNetwork mpSpaceTimeNetwork;
    // Store the mapping between flow graph and space time network
    graph_analysis::BipartiteGraph mBipartiteGraph;
};

} // end namespace transshipment
} // end namespace solvers
} // end namespace templ
#endif // TEMPL_SOLVER_
