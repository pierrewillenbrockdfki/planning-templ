#include "Mission.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <iterator>

#include <owlapi/Vocabulary.hpp>
#include <owlapi/io/OWLOntologyIO.hpp>
#include <organization_model/Algebra.hpp>
#include "solvers/FluentTimeResource.hpp"
#include "solvers/temporal/point_algebra/TimePointComparator.hpp"
#include "solvers/temporal/QualitativeTemporalConstraintNetwork.hpp"
#include "symbols/object_variables/LocationCardinality.hpp"
#include "symbols/object_variables/LocationNumericAttribute.hpp"
#include "solvers/csp/TemporalConstraintNetwork.hpp"
#include "io/MissionWriter.hpp"
#include "io/MissionRequirements.hpp"

namespace templ {

namespace pa = solvers::temporal::point_algebra;

Mission::Mission()
{}

Mission::Mission(const organization_model::OrganizationModel::Ptr& om, const std::string& name)
    : mpTemporalConstraintNetwork(new solvers::temporal::QualitativeTemporalConstraintNetwork())
    , mpRelations(graph_analysis::BaseGraph::getInstance())
    , mpOrganizationModel(om)
    , mOrganizationModelAsk(om)
    , mName(name)
    , mpTransferLocation(new symbols::constants::Location("transfer-location"))
    , mpLogger(new utils::Logger())
{
    // add all functionalities (which could be requested)
    //
    owlapi::model::IRIList list = mOrganizationModelAsk.ontology().allSubClassesOf(organization_model::vocabulary::OM::Functionality());
    mRequestedResources.insert(mRequestedResources.begin(), list.begin(), list.end());
}

Mission::Mission(const Mission& other)
    : mpTemporalConstraintNetwork()
    , mpRelations()
    , mpOrganizationModel(other.mpOrganizationModel)
    , mOrganizationModelAsk(other.mOrganizationModelAsk)
    , mName(other.mName)
    , mModelPool(other.mModelPool)
    , mRoles(other.mRoles)
    , mModels(other.mModels)
    , mPersistenceConditions(other.mPersistenceConditions)
    , mConstraints(other.mConstraints)
    , mRequestedResources(other.mRequestedResources)
    , mTimeIntervals(other.mTimeIntervals)
    , mTimePoints(other.mTimePoints)
    , mObjectVariables(other.mObjectVariables)
    , mConstants(other.mConstants)
    , mConstantsUse(other.mConstantsUse)
    , mpTransferLocation(other.mpTransferLocation)
    , mScenarioFile(other.mScenarioFile)
    , mpLogger(other.mpLogger)
    , mDataPropertyAssignments(other.mDataPropertyAssignments)
{

    if(other.mpRelations)
    {
        mpRelations = other.mpRelations->clone();
    }

    if(other.mpTemporalConstraintNetwork)
    {
        ConstraintNetwork::Ptr constraintNetwork = other.mpTemporalConstraintNetwork->clone();
        mpTemporalConstraintNetwork = dynamic_pointer_cast<solvers::temporal::QualitativeTemporalConstraintNetwork>(constraintNetwork);
    }
}

void Mission::setOrganizationModel(const organization_model::OrganizationModel::Ptr& organizationModel)
{
    mpOrganizationModel = organizationModel;
    mOrganizationModelAsk = organization_model::OrganizationModelAsk(organizationModel);
}

void Mission::applyOrganizationModelOverrides()
{
    DataPropertyAssignment::apply(mpOrganizationModel, mDataPropertyAssignments);
}

void Mission::setAvailableResources(const organization_model::ModelPool& modelPool)
{
    mModelPool = modelPool;
    refresh();
}

void Mission::refresh()
{
    mRoles.clear();
    mModels.clear();

    mModels = mModelPool.getModels();
    mRoles = Role::toList(mModelPool);

    // Update the ask object based on the model pool and applying the functional
    // saturation bound
    if(!mpOrganizationModel)
    {
        throw std::invalid_argument("templ::Mission::refresh: organization model is not set, so cannot refresh");
    }
    mOrganizationModelAsk = organization_model::OrganizationModelAsk(mpOrganizationModel, mModelPool, true /*functional saturation bound*/);
}

symbols::ObjectVariable::Ptr Mission::getObjectVariable(const std::string& name, symbols::ObjectVariable::Type type) const
{
    using namespace ::templ::symbols;
    std::set<ObjectVariable::Ptr>::const_iterator cit = mObjectVariables.begin();
    for(; cit != mObjectVariables.end(); ++cit)
    {
        ObjectVariable::Ptr variable = *cit;
        if(type != ObjectVariable::UNKNOWN && variable->getObjectVariableType() == type && variable->getInstanceName() == name)
        {
            return variable;
        }
    }
    throw std::invalid_argument("templ::Mission::getObjectVariable: variable with name '" + name + "'"
            " and type '" + ObjectVariable::TypeTxt[type] + "' not found");
}

symbols::ObjectVariable::Ptr Mission::getOrCreateObjectVariable(const std::string& name, symbols::ObjectVariable::Type type) const
{
    using namespace ::templ::symbols;

    ObjectVariable::Ptr variable;
    try {
         variable = getObjectVariable(name, type);
    } catch(const std::invalid_argument& e)
    {
        variable = ObjectVariable::getInstance(name, type);
    }
    return variable;
}

void Mission::prepareTimeIntervals()
{
    using namespace solvers::temporal;

    if(!mpTemporalConstraintNetwork->isConsistent())
    {
        throw std::runtime_error("templ::Mission: provided temporal constraint network is not consistent");
    }

    mTimeIntervals.clear();
    std::vector<PersistenceCondition::Ptr>::const_iterator cit =  mPersistenceConditions.begin();
    for(;cit != mPersistenceConditions.end(); ++cit)
    {
        PersistenceCondition::Ptr pc = *cit;
        Interval interval(pc->getFromTimePoint(), pc->getToTimePoint(), point_algebra::TimePointComparator(mpTemporalConstraintNetwork));

        std::vector<Interval>::const_iterator tit = std::find(mTimeIntervals.begin(), mTimeIntervals.end(), interval);
        if(tit == mTimeIntervals.end())
        {
            mTimeIntervals.push_back(interval);
        } else {
            LOG_DEBUG_S << "TimeInterval: " << tit->toString() << " already in list";
        }
    }

    std::set<solvers::temporal::point_algebra::TimePoint::Ptr> uniqueTimepoints;
    for(const solvers::temporal::Interval& interval : mTimeIntervals)
    {
        uniqueTimepoints.insert( interval.getFrom() );
        uniqueTimepoints.insert( interval.getTo() );
    }

    mTimePoints.clear();
    mTimePoints.insert(mTimePoints.begin(), uniqueTimepoints.begin(), uniqueTimepoints.end());
}

void Mission::validateForPlanning() const
{
    if(!mpOrganizationModel)
    {
        throw std::runtime_error("templ::Mission::validate no organization model hase been set.");
    }

    validateAvailableResources();
    if(!mpTemporalConstraintNetwork->isConsistent())
    {
        throw std::runtime_error("templ::Mission::validate provided temporal constraint network is not consistent");
    }

    if(mTimeIntervals.empty())
    {
        throw std::runtime_error("templ::Mission::validate: no time intervals defined");
    }
}

solvers::temporal::TemporalAssertion::Ptr Mission::addResourceLocationCardinalityConstraint(
            const symbols::constants::Location::Ptr& location,
            const solvers::temporal::point_algebra::TimePoint::Ptr& fromTp,
            const solvers::temporal::point_algebra::TimePoint::Ptr& toTp,
            const owlapi::model::IRI& resourceModel,
            size_t cardinality,
            owlapi::model::OWLCardinalityRestriction::CardinalityRestrictionType type
)
{
    if(!mpOrganizationModel)
    {
        throw std::runtime_error("templ::Mission::addResourceLocationCardinalityConstraint: mission has not been initialized with organization model");
    }

    using namespace owlapi;
    using namespace ::templ::symbols;
    // Make sure constant is known
    requireConstant(location);
    ObjectVariable::Ptr locationCardinality(new object_variables::LocationCardinality(location, cardinality, type));

    // the combination of resource, location and cardinality represents a state variable
    // which needs to be translated into resource based state variables
    symbols::StateVariable rloc(ObjectVariable::TypeTxt[ObjectVariable::LOCATION_CARDINALITY],
            resourceModel.toString());

    solvers::temporal::TemporalAssertion::Ptr temporalAssertion = addTemporalAssertion(rloc, locationCardinality, fromTp, toTp);

    owlapi::model::IRIList::const_iterator rit =  std::find(mRequestedResources.begin(),
            mRequestedResources.end(),
            resourceModel);
    if(rit == mRequestedResources.end())
    {
        mRequestedResources.push_back(resourceModel);
    }

    mObjectVariables.insert(locationCardinality);
    return temporalAssertion;
}

solvers::temporal::TemporalAssertion::Ptr Mission::addResourceLocationNumericAttributeConstraint(
        const symbols::constants::Location::Ptr& location,
        const solvers::temporal::point_algebra::TimePoint::Ptr& fromTp,
        const solvers::temporal::point_algebra::TimePoint::Ptr& toTp,
        const owlapi::model::IRI& resourceModel,
        const owlapi::model::IRI& attribute,
        int32_t minInclusive,
        int32_t maxInclusive
        )
{
    using namespace owlapi;
    using namespace ::templ::symbols;

    requireConstant(location);
    ObjectVariable::Ptr numericAttribute(new object_variables::LocationNumericAttribute(
                location,
                attribute,
                minInclusive,
                maxInclusive));

    symbols::StateVariable rloc(ObjectVariable::TypeTxt[ObjectVariable::LOCATION_NUMERIC_ATTRIBUTE],
            resourceModel.toString());

    solvers::temporal::TemporalAssertion::Ptr temporalAssertion = addTemporalAssertion(rloc,
            numericAttribute,
            fromTp,
            toTp);

    mObjectVariables.insert(numericAttribute);
    return temporalAssertion;
}

solvers::temporal::TemporalAssertion::Ptr Mission::addTemporalAssertion(const symbols::StateVariable& stateVariable,
        const symbols::ObjectVariable::Ptr& objectVariable,
        const solvers::temporal::point_algebra::TimePoint::Ptr& fromTp,
        const solvers::temporal::point_algebra::TimePoint::Ptr& toTp)
{
    using namespace solvers::temporal;
    PersistenceCondition::Ptr persistenceCondition = PersistenceCondition::getInstance(stateVariable,
            objectVariable,
            fromTp,
            toTp);

    std::vector<solvers::temporal::PersistenceCondition::Ptr>::const_iterator pit = std::find_if(mPersistenceConditions.begin(), mPersistenceConditions.end(), [persistenceCondition](const PersistenceCondition::Ptr& p)
            {
                return *persistenceCondition == *p;
            });

    if(pit == mPersistenceConditions.end())
    {
        LOG_DEBUG_S << "Adding spatio-temporal constraint: " << stateVariable.toString()
            << " " << objectVariable->toString() << "@[" << fromTp->toString() << "--" << toTp->toString() <<"]" << std::endl;
        mPersistenceConditions.push_back(persistenceCondition);
    } else {
        LOG_DEBUG_S << "Already existing: " << (*pit)->toString();
        throw std::invalid_argument("templ::Mission::addTemporalAssertion: trying to re-add temporal assertion or"
                "trying to add redundant temporal assertion: '" + persistenceCondition->toString() + "'");
    }

    LOG_DEBUG_S << "Adding implicitly defined temporal constraint for time interval";
    pa::QualitativeTimePointConstraint::Ptr constraint = make_shared<pa::QualitativeTimePointConstraint>(fromTp, toTp, pa::QualitativeTimePointConstraint::Less);
    addConstraint(constraint);

    return persistenceCondition;
}

void Mission::addConstraint(const Constraint::Ptr& constraint)
{
    if(!hasConstraint(constraint))
    {
        using namespace solvers::temporal::point_algebra;
        mConstraints.push_back(constraint);

        if(constraint->getCategory() == Constraint::TEMPORAL_QUALITATIVE)
        {
            mpTemporalConstraintNetwork->addConstraint(constraint);
        }
    }
}

bool Mission::hasConstraint(const Constraint::Ptr& constraint) const
{
    return mConstraints.end() != std::find_if(mConstraints.begin(),
            mConstraints.end(), [&constraint](const Constraint::Ptr& other)
            {
                return *constraint == *other;
            });
}

std::string Mission::toString() const
{
    std::stringstream ss;
    ss << "Mission: " << mName << std::endl;
    ss << mModelPool.toString();

    ss << "    PersistenceConditions " << std::endl;
    std::vector<solvers::temporal::PersistenceCondition::Ptr>::const_iterator pit = mPersistenceConditions.begin();
    for(; pit != mPersistenceConditions.end(); ++pit)
    {
        ss << (*pit)->toString(8) << std::endl;
    }
    ss << "    Constraints " << std::endl;
    for(const Constraint::Ptr& c : mConstraints)
    {
        ss << c->toString(8) << std::endl;
    }

    return ss.str();
}

void Mission::save(const std::string& filename) const
{
    io::MissionWriter::write(filename, *this);
}

void Mission::validateAvailableResources() const
{
    for(const std::pair<owlapi::model::IRI, size_t>& pair : mModelPool)
    {
        if(pair.second > 0)
            return;
    }

    throw std::runtime_error("templ::Mission: mission has no available resources specified (not present or max cardinalities sum up to 0) -- \ndefined : model pool is "
            + mModelPool.toString());
}

void Mission::updateMaxCardinalities(solvers::FluentTimeResource& ftr) const
{
    organization_model::ModelPool maxCardinalities = ftr.getMaxCardinalities();
    organization_model::ModelPool maxAvailability = getAvailableResources();

    organization_model::ModelPool newMax = organization_model::Algebra::min(maxCardinalities, maxAvailability);
    ftr.setMaxCardinalities(newMax);
}

std::vector<solvers::FluentTimeResource> Mission::getResourceRequirements(const Mission::Ptr& mission)
{
    using namespace solvers;

    std::vector<FluentTimeResource> requirements;

    // Iterate over all existing persistence conditions
    // -- pick the ones relating to location-cardinality function
    using namespace templ::solvers::temporal;
    for(PersistenceCondition::Ptr p : mission->mPersistenceConditions)
    {
        symbols::StateVariable stateVariable = p->getStateVariable();
        if(stateVariable.getFunction() == symbols::ObjectVariable::TypeTxt[symbols::ObjectVariable::LOCATION_CARDINALITY] )
        {
            try {
                FluentTimeResource ftr = fromLocationCardinality( p, mission );
                // set upper bounds according to available resources
                mission->updateMaxCardinalities(ftr);
                requirements.push_back(ftr);
            } catch(const std::invalid_argument& e)
            {
                LOG_WARN_S << e.what();
            }
        }
    }

    // If multiple requirements exists that have the same interval
    // they can be compacted into one requirement
    FluentTimeResource::compact(requirements);
    return requirements;
}


// Get special sets of constants
std::vector<symbols::constants::Location::Ptr> Mission::getLocations(bool excludeUnused) const
{
    using namespace symbols;
    std::vector<constants::Location::Ptr> locations;
    std::set<Constant::Ptr>::const_iterator cit = mConstants.begin();
    for(; cit != mConstants.end(); ++cit)
    {
        const Constant::Ptr& constant = *cit;
        if(excludeUnused && mConstantsUse[constant] == 0)
        {
                continue;
        }

        if(constant->getConstantType() == Constant::LOCATION)
        {
            locations.push_back( dynamic_pointer_cast<constants::Location>(constant) );
        }
    }
    return locations;
}

symbols::constants::Location::Ptr Mission::getLocation(const std::string& name) const
{
    using namespace symbols::constants;
    return dynamic_pointer_cast<Location>( getConstant(name, symbols::Constant::LOCATION) );
}

solvers::temporal::point_algebra::TimePoint::PtrList Mission::getOrderedTimepoints() const
{
    solvers::temporal::point_algebra::TimePoint::PtrList timepoints = mTimePoints;
    solvers::temporal::QualitativeTemporalConstraintNetwork::Ptr qtcn = dynamic_pointer_cast<solvers::temporal::QualitativeTemporalConstraintNetwork>(mpTemporalConstraintNetwork);
    if(qtcn)
    {
        std::vector<solvers::temporal::point_algebra::TimePoint::Ptr> tps = solvers::csp::TemporalConstraintNetwork::getSortedList(*qtcn);
        return tps;
    } else {
        mpTemporalConstraintNetwork->sort(timepoints);
    }
    return timepoints;
}

solvers::temporal::point_algebra::TimePoint::Ptr Mission::getTimepoint(const std::string& name) const
{
    using namespace solvers::temporal::point_algebra;
    for(const TimePoint::Ptr& t : mTimePoints)
    {
        if(t->getLabel() == name)
        {
            return t;
        }
    }
    throw std::invalid_argument("templ::Mission::getTimepoint: no timepoint '" + name + "' exists");
}

const solvers::temporal::Interval& Mission::getTimeInterval(const std::string& from, const std::string& to) const
{
    for(const solvers::temporal::Interval& i : mTimeIntervals)
    {
        if(i.getFrom()->getLabel() == from
                && i.getTo()->getLabel() == to)
        {
            return i;
        }
    }

    throw std::invalid_argument("templ::Mission::getTimeInterval: no interval from '" + from + "'"
            " to '" + to + "' available -- make sure you called prepareTimeIntervals");
}

void Mission::requireConstant(const symbols::Constant::Ptr& constant)
{
    addConstant(constant);
    incrementConstantUse(constant);
}

uint32_t Mission::incrementConstantUse(const symbols::Constant::Ptr& constant)
{
    mConstantsUse[constant] += 1;
    return mConstantsUse[constant];
}

void Mission::addConstant(const symbols::Constant::Ptr& constant)
{
    using namespace symbols;
    std::set<Constant::Ptr>::const_iterator cit = std::find_if(mConstants.begin(), mConstants.end(), [constant](const symbols::Constant::Ptr& c)
            {
                return *constant == *c;
            });
    if(cit == mConstants.end())
    {
        mConstants.insert(constant);
    } else {
        if(*cit == constant)
        {
            // We found the constant -- nothing to do since it has already been
            // added
        } else {
            // We found the constant -- but pointers are different
            // This will lead to inconsitencies, so throw
            throw std::invalid_argument("templ::Mission::addConstant: constant '" +
                constant->toString() + "' already registered, but different pointer objects");
        }
    }
}

const symbols::Constant::Ptr& Mission::getConstant(const std::string& id, symbols::Constant::Type type) const
{
    using namespace symbols;
    std::set<Constant::Ptr>::const_iterator cit = mConstants.begin();
    for(; cit != mConstants.end(); ++cit)
    {
        const Constant::Ptr& constant = *cit;
        if(type != symbols::Constant::UNKNOWN && constant->getConstantType() == type)
        {
            if(constant->getInstanceName() == id)
            {
                return constant;
            }
        }
    }
    throw std::invalid_argument("templ::Mission: no constant named '" + id + "' of type '" + symbols::Constant::TypeTxt[type] + "' known");
}

graph_analysis::Edge::Ptr Mission::addRelation(const graph_analysis::Vertex::Ptr& source,
        const std::string& label,
        const graph_analysis::Vertex::Ptr& target)
{
    using namespace graph_analysis;
    Edge::Ptr edge(new Edge(source, target, label));
    mpRelations->addEdge(edge);
    return edge;
}

io::SpatioTemporalRequirement::Ptr Mission::getRequirementById(uint32_t id) const
{
    using namespace graph_analysis;
    VertexIterator::Ptr vertexIt = mpRelations->getVertexIterator();
    while(vertexIt->next())
    {
        io::SpatioTemporalRequirement::Ptr requirement = dynamic_pointer_cast<io::SpatioTemporalRequirement>(vertexIt->current());
        if(requirement && requirement->id == id)
        {
            return requirement;
        }
    }
    std::stringstream ss;
    ss << "templ::Mission::getRequirementById: could not find requirement with id '" << id << "'";
    throw std::invalid_argument(ss.str());
}

solvers::FluentTimeResource Mission::findRelated(const io::SpatioTemporalRequirement::Ptr& requirement, const solvers::FluentTimeResource::List& ftrs) const
{
    using namespace solvers::temporal::point_algebra;
    for(const solvers::FluentTimeResource& ftr : ftrs)
    {
        if(ftr.getLocation()->getInstanceName() == requirement->spatial.location.id)
        {
            solvers::temporal::Interval interval = ftr.getInterval();
            if(interval.getFrom()->getLabel() == requirement->temporal.from
                    && interval.getTo()->getLabel() == requirement->temporal.to)
            {
                return ftr;
            }
        }
    }

    std::stringstream ss;
    ss << "templ::Mission::findRelated: could not identify related fluent time resource for ";
    ss << " requirement with id '" << requirement->id << "'";
    throw std::invalid_argument(ss.str());
}

solvers::FluentTimeResource Mission::findRelatedById(uint32_t id, const solvers::FluentTimeResource::List& ftrs) const
{
    return findRelated( getRequirementById(id), ftrs );
}

solvers::FluentTimeResource Mission::fromLocationCardinality(const solvers::temporal::PersistenceCondition::Ptr& p, const Mission::Ptr& mission)
{
    using namespace templ::solvers::temporal;
    point_algebra::TimePointComparator timepointComparator(mission->getTemporalConstraintNetwork());

    const symbols::StateVariable& stateVariable = p->getStateVariable();
    owlapi::model::IRI resourceModel(stateVariable.getResource());
    symbols::ObjectVariable::Ptr objectVariable = dynamic_pointer_cast<symbols::ObjectVariable>(p->getValue());
    symbols::object_variables::LocationCardinality::Ptr locationCardinality = dynamic_pointer_cast<symbols::object_variables::LocationCardinality>(objectVariable);

    Interval interval(p->getFromTimePoint(), p->getToTimePoint(), timepointComparator);
    std::vector<Interval>::const_iterator iit = std::find(mission->mTimeIntervals.begin(), mission->mTimeIntervals.end(), interval);
    if(iit == mission->mTimeIntervals.end())
    {
        throw std::runtime_error("templ::Mission::fromLocationCardinality: could not find interval: '" + interval.toString() + "'");
    }

    owlapi::model::IRIList::const_iterator sit = std::find(mission->mRequestedResources.begin(), mission->mRequestedResources.end(), resourceModel);
    if(sit == mission->mRequestedResources.end())
    {
        throw std::runtime_error("templ::Mission::fromLocationCardinality: could not find service: '" + resourceModel.toString() + "'");
    }

    symbols::constants::Location::Ptr location = locationCardinality->getLocation();

    std::vector<symbols::constants::Location::Ptr> locations = mission->getLocations();
    std::vector<symbols::constants::Location::Ptr>::const_iterator lit = std::find(locations.begin(), locations.end(), location);
    if(lit == locations.end())
    {
        throw std::runtime_error("templ::solvers::csp::TransportNetwork::getResourceRequirements: could not find location: '" + location->toString() + "'");
    }

    using namespace solvers::csp;

    // Map objects to numeric indices -- the indices can be mapped
    // backed using the mission they were created from
    uint32_t timeIndex = std::distance(mission->mTimeIntervals.cbegin(), iit);
    solvers::FluentTimeResource ftr(mission,
            std::distance(mission->mRequestedResources.cbegin(), sit)
            , timeIndex
            , std::distance(locations.cbegin(), lit)
    );

    owlapi::model::OWLOntologyAsk ask = mission->getOrganizationModelAsk().ontology();
    if(ask.isSubClassOf(resourceModel, organization_model::vocabulary::OM::Functionality()))
    {
        // retrieve upper bound
        ftr.setMaxCardinalities( mission->mOrganizationModelAsk.getFunctionalSaturationBound(resourceModel) );

    } else if(ask.isSubClassOf(resourceModel, organization_model::vocabulary::OM::Actor()))
    {
        switch(locationCardinality->getCardinalityRestrictionType())
        {
            case owlapi::model::OWLCardinalityRestriction::MIN :
            {
                size_t min = ftr.getMinCardinalities().getValue(resourceModel, std::numeric_limits<size_t>::min());
                ftr.setMinCardinalities(resourceModel, std::max(min, (size_t) locationCardinality->getCardinality()) );
                ftr.setMaxCardinalities(resourceModel, std::numeric_limits<size_t>::max());
                break;
            }
            case owlapi::model::OWLCardinalityRestriction::MAX :
            {
                size_t max = ftr.getMaxCardinalities().getValue(resourceModel, std::numeric_limits<size_t>::max());
                ftr.setMaxCardinalities(resourceModel, std::min(max, (size_t) locationCardinality->getCardinality()));
                ftr.setMinCardinalities(resourceModel, 0);
                break;
            }
            default:
                break;
        }
    } else {
        throw std::invalid_argument("templ::Mission::fromLocationCardinality: Unsupported state variable: " + resourceModel.toString());
    }
    return ftr;
}

void Mission::saveInputData(const std::string& path) const
{
    boost::filesystem::path specDir(path);
    // create spec dir if neccessary
    if(boost::filesystem::create_directories(specDir) )
    {
        std::string scenarioFile = getScenarioFile();
        if(!scenarioFile.empty())
        {
            boost::filesystem::path from(scenarioFile);
            boost::filesystem::path to("");
            to += specDir;
            to += "/";
            to += from.filename();
            // https://svn.boost.org/trac/boost/ticket/10038
            //boost::filesystem::copy_file(from, to, boost::filesystem::copy_option::overwrite_if_exists);
            std::string cmd = "cp " + from.string() + " " + to.string();
            if( -1 == system(cmd.c_str()) )
            {
                throw std::runtime_error("templ::Mission::saveInputData: could not copy '"
                        + from.string() + "' to '" + to.string() + "'");
            }

            std::string filename = specDir.string() + "/organization_model.owl";
            owlapi::io::OWLOntologyIO::write(filename, getOrganizationModel()->ontology(), owlapi::io::RDFXML );
        } else {
            throw std::invalid_argument("templ::Mission::saveInputData: "
                    "missio has been autogenerated -- serialization not yet implemented");
        }
    }
}

} // end namespace templ
