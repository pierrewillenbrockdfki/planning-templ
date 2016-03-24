#include <templ/io/MissionReader.hpp>
#include <templ/MissionPlanner.hpp>

using namespace templ;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: %s <mission> <organization-model> [<min-number-of-solutions>]\n", argv[0]);
        exit(-1);
    }

    int minimumNumberOfSolutions = 1;
    if(argc == 4)
    {
        minimumNumberOfSolutions = atoi(argv[3]);
        std::cout << "Minimum number of requested solutions: " << minimumNumberOfSolutions << std::endl;
    }

    std::string missionFilename = argv[1];
    std::string organizationModelFilename = argv[2];

    using namespace templ;
    Mission mission = io::MissionReader::fromFile(missionFilename);
    printf("%s\n",mission.toString().c_str());

    using namespace organization_model;
    OrganizationModel::Ptr organizationModel = OrganizationModel::getInstance(organizationModelFilename);
    mission.setOrganizationModel(organizationModel);
    
    MissionPlanner missionPlanner(mission);

    std::vector<Plan> solutions = missionPlanner.execute(minimumNumberOfSolutions);
    std::cout << "The following solutions have been found: " << Plan::toString(solutions) << std::endl;

    return 0;
}
