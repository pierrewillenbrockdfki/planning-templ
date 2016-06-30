#include "Logger.hpp"
#include <sstream>
#include <boost/filesystem.hpp>
#include <base-logging/Logging.hpp>
#include <owlapi/io/OWLOntologyIO.hpp>
#include <templ/Mission.hpp>

namespace templ {

Logger::Logger(const base::Time& time, const std::string& baseDirectory, bool useSessions)
    : mTime(time)
    , mBaseDirectory(baseDirectory)
    , mUseSessions(useSessions)
    , mSessionId(0)
{
}

std::string Logger::filename(const std::string& filename) const
{
    std::stringstream ss;
    ss << mBaseDirectory << "/" << mTime.toString(base::Time::Seconds) << "-templ/";
    if(mUseSessions)
    {
        ss << "/" << mSessionId;
    }
    if(boost::filesystem::create_directories( boost::filesystem::path(ss.str())) )
    {
        LOG_DEBUG_S << "Created directory: '" << ss.str() << "'";
    }

    ss << "/" << filename;
    return ss.str();
}

std::string Logger::getBasePath() const
{
    std::stringstream ss;
    ss << mBaseDirectory << "/" << mTime.toString(base::Time::Seconds) << "-templ/";
    return ss.str();
}

void Logger::saveInputData(const Mission& mission) const
{
    std::stringstream ss;
    ss << getBasePath() << "/specs";
    boost::filesystem::path specDir(ss.str());
    // create spec dir if neccessary
    if(boost::filesystem::create_directories(specDir) )
    {
        std::string scenarioFile = mission.getScenarioFile();
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
                throw std::runtime_error("templ::Logger::saveInputData: could not copy '"
                        + from.string() + "' to '" + to.string() + "'");
            }

            std::string filename = specDir.string() + "/organization_model.owl";
            owlapi::io::OWLOntologyIO::write(filename, mission.getOrganizationModel()->ontology(), owlapi::io::RDFXML );
        } else {
            throw std::invalid_argument("templ::Logger::saveInputData: "
                    "missio has been autogenerated -- serialization not yet implemented");
        }
    }
}

} // end namespace templ
