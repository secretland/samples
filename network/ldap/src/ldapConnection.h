#if !defined(__ldapConnection_h_)
#define __ldapConnection_h_

#include <string>

#if defined(WIN32)
    #include <Windows.h>
    #include <Winldap.h>
#elif defined(__linux__)
    #include <ldap.h>
#endif //#if defined(_WIN32)

namespace util
{
namespace ldap
{
    typedef std::string Address;
    typedef std::string BindDN;
    typedef std::string Password;
    typedef std::string Mechanism;

    LDAP* connectionSetup(Address const&);

    bool bind(LDAP*, BindDN const&, Password const&, Mechanism const& = Mechanism());
    bool unbind(LDAP*);

#if defined(__linux__)
    LDAP* saslBinded(Address const&);
    BindDN whoami(LDAP*);
    Address readLdapAddress();
    BindDN readBaseDN();
#endif /* defined(__linux__) */

} /* namespace ldap */
} /* namespace util */

#endif /* !defined(__ldapConnection_h_) */
