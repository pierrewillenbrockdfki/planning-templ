#include "TemporalConstraintNetwork.hpp"
#include <graph_analysis/WeightedEdge.hpp>
#include <graph_analysis/algorithms/FloydWarshall.hpp>

using namespace templ::solvers::temporal::point_algebra;
using namespace graph_analysis;


namespace templ {
namespace solvers {
namespace temporal {

TemporalConstraintNetwork::TemporalConstraintNetwork()
    : mpDistanceGraph( new graph_analysis::lemon::DirectedGraph() )
{
}

TemporalConstraintNetwork::~TemporalConstraintNetwork()
{
}

// stp(N) is generated by upper-lower bounds of range on disjunctive intervals
void TemporalConstraintNetwork::stp()
{
	BaseGraph::Ptr graph = mpDistanceGraph->copy();
	
	TemporalConstraintNetwork tcn;
	
	EdgeIterator::Ptr edgeIt = graph->getEdgeIterator();
	
	IntervalConstraint::Ptr edge;
	double max, min;
	while (edgeIt->next())
	{
		// for each edge of the temp.const.network, check each interval and find the lowest lower bound and the biggest upper bound
		edge = boost::dynamic_pointer_cast<IntervalConstraint>( edgeIt->current() );
		max = 0; min = std::numeric_limits<double>::infinity();
		std::vector<Bounds> v = edge->getIntervals();
    	std::vector<Bounds>::iterator it = v.begin();
    	while (it!=v.end())
    	{
    		if (it->getLowerBound() < min) min = it->getLowerBound();
    		if (it->getUpperBound() > max) max = it->getUpperBound();
    		it++;
    	}
    	// create a new interval constraint using only the interval [min,max] 
    	IntervalConstraint::Ptr i(new IntervalConstraint(edge->getSourceTimePoint(),edge->getTargetTimePoint()));
    	i->addInterval(Bounds(min,max));
		tcn.addIntervalConstraint(i);
	}
	// update mpDistanceGraph with the one that we just created (tcn)
	mpDistanceGraph = tcn.mpDistanceGraph->copy();
}

// the intersection between a temporal constraint network and a simple temporal constraint network
graph_analysis::BaseGraph::Ptr TemporalConstraintNetwork::intersection(graph_analysis::BaseGraph::Ptr other)
{
	// complex temporal constraint network
	BaseGraph::Ptr graph0 = mpDistanceGraph->copy();

	// simple temporal constraint network
	BaseGraph::Ptr graph1 = other->copy();

	// the intersection result
	TemporalConstraintNetwork tcn;

	EdgeIterator::Ptr edgeIt1 = graph1->getEdgeIterator();
	TimePoint::Ptr source,target;
	IntervalConstraint::Ptr edge0,edge1;
	
	// iterate through the simple temporal constraint network
	while (edgeIt1->next()) 
	{
		edge1 = boost::dynamic_pointer_cast<IntervalConstraint>( edgeIt1->current() );
		source = edge1 -> getSourceTimePoint();
		target = edge1 -> getTargetTimePoint();
		std::vector<Bounds> x = edge1->getIntervals();
		std::vector<Bounds>::iterator interval = x.begin();
		EdgeIterator::Ptr edgeIt0 = graph0->getEdgeIterator();
		// for each edge from the simple temporal constraint network; iterate through the complex temporal constraint network
		while (edgeIt0->next())
		{
			// if we find two edges (one from each graph) which have the same source and target variables
			// then we look at their upper and lower bounds
			edge0 = boost::dynamic_pointer_cast<IntervalConstraint>( edgeIt0->current() );
			if (target == edge0->getTargetTimePoint() && source == edge0->getSourceTimePoint())
			{

				std::vector<Bounds> v = edge0->getIntervals();
    			std::vector<Bounds>::iterator it = v.begin();
    			IntervalConstraint::Ptr i(new IntervalConstraint(source, target));
    			while (it!=v.end())
    			{
					/* edge1: [x--------------y]
				   		edge0: 	 [a-------b]
				   		=> add:  [a-------b]
					*/    
					if (interval->getLowerBound() <= it->getLowerBound() && interval->getUpperBound() >= it->getUpperBound())
					{
						i->addInterval(Bounds(it->getLowerBound(),it->getUpperBound()));
					}
					else
					/*  edge1: [x------------y]
						edge0:      [a------------b]
						=> add:     [a-------y] 
					*/
					if (interval->getLowerBound() <= it->getLowerBound() && interval->getUpperBound() <= it->getUpperBound())
					{
						if (it->getLowerBound() <= interval->getUpperBound())
						{
							i->addInterval(Bounds(it->getLowerBound(),interval->getUpperBound()));
						}
					}
					else
					/*  edge1:        [x-----------y]
				    	edge0:    [a---------b]
				    	=> add:       [x-----b]
					*/
				    if (interval->getLowerBound() >= it->getLowerBound() && interval->getUpperBound() >= it->getUpperBound())
					{
						if (it->getUpperBound() >= interval->getLowerBound())
						{
							i->addInterval(Bounds(interval->getLowerBound(),it->getUpperBound()));
						}
					}
					else
					/*  edge1:     [x----y]
						edge0:  [a-----------b]
						=> add:    [x----y]
					*/
					if (interval->getLowerBound() >= it->getLowerBound() && interval->getUpperBound() <= it->getUpperBound())
					{
						i->addInterval(Bounds(interval->getLowerBound(),interval->getUpperBound()));
					}
					// otherwise:
					/*  edge1:  [x----y]
						edge0:            [a----b]
						or
						edge1:            [x----y]
						edge0:  [a----b]
						=> we don't add anything
					*/
					it++;
				}
				tcn.addIntervalConstraint(i);
			}
		}
	}
	mpDistanceGraph = tcn.mpDistanceGraph->copy();
	// update mpDistanceGraph with the one that we just created (tcn)
	return mpDistanceGraph;
}

// Change A ------[lowerBound,uppperBound]------> B into:
/* 
 *           A --- weight:   upper bound --> B
 *           B --- weight: - lower bound --> A
 *	Upper and lower bounds of each interval are added as edges in forward and backward direction between two edges
 *   Returns the weighted graph
**/
graph_analysis::BaseGraph::Ptr TemporalConstraintNetwork::toWeightedGraph()
{
	BaseGraph::Ptr graph(new graph_analysis::lemon::DirectedGraph());
	BaseGraph::Ptr tcn = mpDistanceGraph->copy();
	EdgeIterator::Ptr edgeIt = tcn->getEdgeIterator();

	while (edgeIt->next())
    {
    	IntervalConstraint::Ptr edge0 = boost::dynamic_pointer_cast<IntervalConstraint>( edgeIt->current() );

    	std::vector<Bounds> v = edge0->getIntervals();
        std::vector<Bounds>::iterator it = v.begin();
    	while (it!=v.end())
    	{
        	WeightedEdge::Ptr edge1(new WeightedEdge(it->getUpperBound()));
        	edge1->setSourceVertex(edge0->getSourceTimePoint());
        	edge1->setTargetVertex(edge0->getTargetTimePoint());
        	graph->addEdge(edge1);

        	WeightedEdge::Ptr edge2(new WeightedEdge(-it->getLowerBound()));
        	edge2->setSourceVertex(edge0->getTargetTimePoint());
        	edge2->setTargetVertex(edge0->getSourceTimePoint());
        	graph->addEdge(edge2);
        	it++;
        }
    }

    return graph;
}

//compute the minimal network of a simple temporal network using Floyd-Warshall
void TemporalConstraintNetwork::minNetwork()
{
	// Input: a simple temporal constraint network
	TemporalConstraintNetwork tcn;
	// Change its graph into a weighted graph and then apply Floyd-Warshall
	BaseGraph::Ptr graph = (toWeightedGraph())->copy();

	BaseGraph::Ptr oldGraph = mpDistanceGraph->copy();
	algorithms::DistanceMatrix distanceMatrix = algorithms::FloydWarshall::allShortestPaths(graph, [](Edge::Ptr e) -> double
                {
                    return boost::dynamic_pointer_cast<WeightedEdge>(e)->getWeight();
                });
	EdgeIterator::Ptr edgeIt = oldGraph->getEdgeIterator();

	// Change again the computed graph into a graph using Interval Constraint representation
	TimePoint::Ptr v1,v2;
	while (edgeIt->next())
	{
		IntervalConstraint::Ptr edge = boost::dynamic_pointer_cast<IntervalConstraint>(edgeIt->current());
		v1 = edge->getSourceTimePoint();
		v2 = edge->getTargetTimePoint();

		double distance12 = distanceMatrix[std::pair<TimePoint::Ptr, TimePoint::Ptr>(v1,v2)];
		double distance21 = distanceMatrix[std::pair<TimePoint::Ptr, TimePoint::Ptr>(v2,v1)];
		distance21 = (-1)*distance21;
		IntervalConstraint::Ptr i(new IntervalConstraint(v1, v2));
		i->addInterval(Bounds(distance21,distance12));
		tcn.addIntervalConstraint(i);	
	}
	// update mpDistanceGraph with the one that we just created (a simple temporal constraint network with the shortest paths computed)
	mpDistanceGraph = tcn.mpDistanceGraph->copy();
}

 // check if 2 temporal constraint networks are the same
bool TemporalConstraintNetwork::areEqual(graph_analysis::BaseGraph::Ptr other)
{
	// check if the graphs have the same number of vertices and edges
	if (mpDistanceGraph->size() == other->size() && mpDistanceGraph->order() == other->order())
	{
		EdgeIterator::Ptr edgeIt1 = mpDistanceGraph -> getEdgeIterator();
		while (edgeIt1->next())
		{
			EdgeIterator::Ptr edgeIt2 = other -> getEdgeIterator();
			bool ok = false;
			// for each edge(source,target) from the first graph;
			// search for an edge with the same source and target vertices in the second graph
			while (edgeIt2->next())
			{
				// check each two edges if they have the same source;target;uppper and lower bounds
				// if not; then the graphs are not the same
				IntervalConstraint::Ptr i1 = boost::dynamic_pointer_cast<IntervalConstraint>(edgeIt1->current());
				IntervalConstraint::Ptr i2 = boost::dynamic_pointer_cast<IntervalConstraint>(edgeIt2->current());
				if (i1->getSourceTimePoint() == i2->getSourceTimePoint() && i1->getTargetTimePoint() == i2->getTargetTimePoint())
				{
					ok = true;
					// check if the constraints have the same number of intervals
					if (i1->getIntervalsNumber() == i2->getIntervalsNumber())
					{
						std::vector<Bounds> v = i1->getIntervals();
        				std::vector<Bounds>::iterator it = v.begin();
        				// for each interval in the first constratint, check if it is also in the second constraint
        				while (it!=v.end())
        				{
        					// if we find one which is not in both constraints then the graphs are not the same
        					if (i2->checkInterval(*it) == false) return false; 
        					it++;
        				}
        			}
        			else
        			{
        				return false;
        			}
        			break;
				} 
			}
			// we found one edge from graph1 which is not in graph2 => graphs are not the same
			if (ok == false) return ok;
		}
		return true;
	}
	else
	{
		return false;
	}
}

// Upper-Lower-Tightening Algorithm
void TemporalConstraintNetwork::upperLowerTightening()
{
	BaseGraph::Ptr oldGraph = mpDistanceGraph->copy();
	do
	{
		// oldGraph = newGraph
		oldGraph = mpDistanceGraph->copy();
		stp();
		minNetwork();
		// newGraph becomes the minimal network
		EdgeIterator::Ptr edgeIt = oldGraph->getEdgeIterator();
		// copy oldGraph into a temporal constraint network
		TemporalConstraintNetwork tcn;
		while (edgeIt->next())
		{
			IntervalConstraint::Ptr i = boost::dynamic_pointer_cast<IntervalConstraint>(edgeIt->current());
			std::vector<Bounds> v = i->getIntervals();
        	std::vector<Bounds>::iterator it = v.begin();
        	IntervalConstraint::Ptr n(new IntervalConstraint(i->getSourceTimePoint(),i->getTargetTimePoint()));
        	while (it!=v.end())
        	{
        		n->addInterval(Bounds(it->getLowerBound(),it->getUpperBound()));
        		it++;
        	}
			tcn.addIntervalConstraint(n);
		}
		// apply intersection for oldGraph and newGraph(minimal network)
		tcn.intersection(mpDistanceGraph);
		// newGraph becomes the resulted graph obtained from the intersection between the oldGraph and the minimal network 
		mpDistanceGraph = (tcn.mpDistanceGraph) -> copy();
	} while (!areEqual(oldGraph)); // until newGraph = oldGraph
}

int TemporalConstraintNetwork::getEdgeNumber()
{
	EdgeIterator::Ptr edgeIt = mpDistanceGraph->getEdgeIterator();
	int cnt=0;
	while (edgeIt->next()) cnt++;
	return cnt;
}

} // end namespace temporal
} // end namespace solvers
} // end namespace templ
