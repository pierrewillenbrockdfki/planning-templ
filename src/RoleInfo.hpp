#ifndef TEMPL_ROLE_INFO_HPP
#define TEMPL_ROLE_INFO_HPP

#include <set>
#include "SharedPtr.hpp"
#include "Role.hpp"

namespace templ {

/**
 * Allow to collect related roles into a single object
 */
class RoleInfo
{
public:
    typedef shared_ptr<RoleInfo> Ptr;

    enum Tag { UNKNOWN,
        ASSIGNED = 1,
        REQUIRED,
        AVAILABLE
    };

    // assigned (according to current solution -- CSP step)
    // available (according to multi-commodity min cost flow step)
    // required (per mission spec)
    enum Status { UNKNOWN_STATUS,
        REQUIRED_ASSIGNED,   // GREEN
        REQUIRED_UNASSIGNED, // RED  infeasible csp / min cost flow, but might still be an overall feasible solution (due to compensation through other)
        NOTREQUIRED_ASSIGNED,   // YELLOW
        NOTREQUIRED_AVAILABLE, // BLACK
    };

    static std::map<Tag, std::string> TagTxt;

    RoleInfo();

    void addRole(const Role& role, const Tag& tag);

    void addRole(const Role& role, const std::string& tag = "");

    bool hasRole(const Role& role, const std::string& tag = "") const;

    bool hasRole(const Role& role, const Tag& tag) const;

    const std::set<Role>& getRoles(const std::string& tag ="") const;

    const std::set<Role>& getRoles(const Tag& tag) const;

    const std::set<Role>& getAllRoles() const;

    /**
     * Get the status a particular role
     */
    Status getStatus(const owlapi::model::IRI& model, uint32_t id) const;

    /**
     * Get the tags for a role
     */
    std::set<std::string> getTags(const Role& role) const;

    /**
     * Check if the role has associated tags
     */
    bool hasTag(const Role& role) const;

    /**
     * Retrieve a role if it exists, otherwise throw
     * \throw std::invalid_argument if role does not exist
     */
    const Role& getRole(const owlapi::model::IRI& model, uint32_t id) const;

    /**
     * Compute the complement C between two role sets with respect to the tag0 set,
     * defined as:
     *
     \f[
        C = TAG0 \ TAG1
     \f]
     * \return complement
     */
    Role::List getRelativeComplement(const std::string& tag0, const std::string& tag1) const;

    /**
     * Get intersection between tag0 and tag1 set
     */
    Role::List getIntersection(const std::string& tag0, const std::string& tag1) const;

    /**
     * Get the symmetric difference between tag0 and tag1 set
     */
    Role::List getSymmetricDifference(const std::string& tag0, const std::string& tag1) const;

    void clear();

    std::string toString(uint32_t indent = 0) const;

protected:
    mutable std::set<Role> mAllRoles;
    std::set<Role> mRoles;
    mutable std::map<std::string, Role::Set> mTaggedRoles;
};

}
#endif // TEMPL_ROLE_INFO_HPP
