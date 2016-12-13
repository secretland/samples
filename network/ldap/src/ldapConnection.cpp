#include "ldapConnection.h"

// #include "log/DebugLog.h"
#define LOG_DEBUG(info)
// #define LOG_DEBUG

#if defined(__linux__)
    #include <fstream>
#endif /* defined(__linux__) */

namespace util
{
namespace ldap
{
    LDAP* connectionSetup(Address const& address)
    {
        LOG_DEBUG(SRC_POINT_EX(DUMP_VAR_EX("Ldap server address", address)))
        LDAP* ldap = nullptr;
        int returnCode = LDAP_SUCCESS;
#if defined(WIN32)
        ULONG version = LDAP_VERSION3;
        ldap = ldap_init(static_cast<PCHAR>(const_cast<char*>(address.c_str())), 0);
        returnCode = ldap_connect(ldap, 0);
        if(returnCode != LDAP_SUCCESS)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_connect", TO_HEX(returnCode), ldap_err2string(returnCode))))
            ldap = nullptr;
        }
#else
        int version = LDAP_VERSION3;
        returnCode = ldap_initialize(&ldap, address.c_str());
        if(LDAP_SUCCESS != returnCode)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_initialize", TO_HEX(returnCode), ldap_err2string(returnCode))))
            ldap = nullptr;
        }
#endif /* defined(WIN32) */
        returnCode = ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, (void*)&version);
        if(LDAP_SUCCESS != returnCode)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_set_option", TO_HEX(returnCode), ldap_err2string(returnCode))))
#if defined(__linux__)
            ldap_unbind_ext(ldap, NULL, NULL);
#else
            ldap_unbind(ldap);
#endif /* defined(__linux__) */
            ldap = nullptr;
        }
        LOG_DEBUG(SRC_POINT_EX(OBJECT_POINTER_VALUE("ldap", ldap)))
        return ldap;
    }

    bool bind(LDAP* ldap, BindDN const& bindDN, Password const& password, Mechanism const& mechanism)
    {
        LOG_DEBUG(SRC_POINT_EX(DUMP_VAR_EX("bindDN", bindDN)))
        LOG_DEBUG(SRC_POINT_EX(DUMP_VAR_EX("mechanism", mechanism)))
        if(nullptr == ldap)
        {
            LOG_DEBUG(SRC_POINT_EX(OBJECT_POINTER_VALUE("ldap", ldap)))
            return false;
        }
#if defined(WIN32)
        ULONG returnCode = ldap_simple_bind_s(ldap, static_cast<PCHAR>(const_cast<char*>(bindDN.c_str())), 
                                                    static_cast<PCHAR>(const_cast<char*>(password.c_str())));
#else
        struct berval cred;
        cred.bv_val = const_cast<char*>(password.c_str());
        cred.bv_len = password.length();
        int msgId = 0;
        /*int returnCode = ldap_sasl_bind_s(ldap, bindDN.c_str(), mechanism.c_str(), &cred, NULL, NULL, &servcred);*/
        int returnCode = ldap_sasl_bind(ldap, bindDN.c_str(), mechanism.c_str(), &cred, NULL, NULL, &msgId);
#endif /* defined(WIN32) */
        if(returnCode != LDAP_SUCCESS)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_simple(sasl)_bind_s", TO_HEX(returnCode), ldap_err2string(returnCode))))
            return false;
        }
        return true;
    }

    bool unbind(LDAP* ldap)
    {
        if(nullptr == ldap)
        {
            LOG_DEBUG(SRC_POINT_EX(OBJECT_POINTER_VALUE("ldap", ldap)))
            return false;
        }
#if defined(__linux__)
        int returnCode = ldap_unbind_ext(ldap, NULL, NULL);
#else
        int returnCode = ldap_unbind(ldap);
#endif /* defined(__linux__) */
        if(returnCode != LDAP_SUCCESS)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_bind", TO_HEX(returnCode), ldap_err2string(returnCode))))
            return false;
        }
        return true;
    }

#if defined(__linux__)
    typedef struct _tagLDAPAuth
    {
        char const* dn;
        char const* saslmech;
        char const* authuser;
        char const* user;
        char const* realm;
        BerValue    cred;
    } LDAPAuth;

    int do_interact(LDAP* ldap, unsigned flags, void* defaults, void* sin)
    {
        return LDAP_SUCCESS;
    }
    LDAP* saslBinded(Address const& address)
    {
        LDAP* ldap = NULL;
        int protocol = LDAP_VERSION3;

        LOG_DEBUG(SRC_POINT_EX(DUMP_VAR(address)))
        if(address.empty())
            return NULL;

        int retCode = ldap_initialize(&ldap, address.c_str());
        if(retCode != LDAP_SUCCESS)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_initialize", TO_HEX(retCode), ldap_err2string(retCode))))
            return NULL;
        }
        if((retCode = ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &protocol)) != LDAP_SUCCESS)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_set_option(LDAP_OPT_PROTOCOL_VERSION)", TO_HEX(retCode), ldap_err2string(retCode))))
            ldap_destroy(ldap);
            return NULL;
        }
        else
        {
            unsigned sasl_flags = LDAP_SASL_AUTOMATIC;
            char const* sasl_mech = "GSSAPI";
            LDAPAuth auth;
            if((retCode = ldap_sasl_interactive_bind_s(ldap, NULL, sasl_mech, NULL, NULL,
                                                       sasl_flags, do_interact, &auth)) != LDAP_SUCCESS)
            {
                LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_sasl_interactive_bind_s", TO_HEX(retCode), ldap_err2string(retCode))))
                ldap_destroy(ldap);
                return NULL;
            }
        }
        return ldap;
    }
    BindDN whoami(LDAP* ldap)
    {
        int id = 0;
        int retCode = ldap_whoami(ldap, NULL, NULL, &id);
        BindDN bindDN;
        if(retCode != LDAP_SUCCESS)
        {
            LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_whoami", TO_HEX(retCode), ldap_err2string(retCode))))
        }
        else
        {
            LDAPMessage* msg = NULL;
            if((retCode = ldap_result(ldap, id, 0, NULL, &msg)) != LDAP_SUCCESS)
            {
                LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_result", TO_HEX(retCode), ldap_err2string(retCode))))
            //}
            //else
            //{
                struct berval* authzid = NULL;
                if((retCode = ldap_parse_whoami(ldap, msg, &authzid)) != LDAP_SUCCESS)
                {
                    LOG_DEBUG(SRC_POINT_EX(FUNC_ERR_CODE_EX("ldap_parse_whoami", TO_HEX(retCode), ldap_err2string(retCode))))
                }
                else
                {
                    bindDN = &authzid->bv_val[3];
                    LOG_DEBUG(SRC_POINT_EX(DUMP_VAR(bindDN)))
                    ldap_memfree(authzid);
                }
                ldap_msgfree(msg);
            }
        }
        return bindDN;
    }
    Address readLdapAddress()
    {
        Address address;
        std::ifstream config("/etc/ldap/ldap.conf");
        /*std::ifstream config("/etc/nss-ldapd.conf");*/
        if(config.is_open())
        {
            std::string line;
            while(config.good())
            {
                std::getline(config, line);
                if(line.find("URI") != std::string::npos)
                {
                    std::size_t pos = line.find("ldap:");
                    if(pos != std::string::npos)
                    {
                        address = line.substr(pos);
                        break;
                    }
                }
            }
        }
        return address;
    }
    BindDN readBaseDN()
    {
        BindDN bindDN;
        std::ifstream config("/etc/ldap/ldap.conf");
        if(config.is_open())
        {
            std::string line;
            while(config.good())
            {
                std::getline(config, line);
                if(line.find("BASE") != std::string::npos)
                {
                    std::size_t pos = line.find("dc=");
                    if(pos != std::string::npos)
                    {
                        bindDN = line.substr(pos);
                        break;
                    }
                }
            }
        }
        return bindDN;
    }
#endif /* defined(__linux__) */
} /* namespace ldap */
} /* namespace util */
