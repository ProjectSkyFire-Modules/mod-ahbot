/*
 * Thin wrappers around SkyFire ConfigMgr. Upstream uses sConfigMgr->GetOption<T>().
 */

#ifndef AHBOT_CONFIG_H
#define AHBOT_CONFIG_H

#include "Config.h"
#include "Define.h"
#include <string>

namespace AHBotConfig
{
    inline bool GetBool(char const* name, bool def)
    {
        return sConfigMgr->GetBoolDefault(name, def);
    }

    inline bool GetBool(std::string const& name, bool def)
    {
        return sConfigMgr->GetBoolDefault(name.c_str(), def);
    }

    inline uint32 GetUInt(char const* name, uint32 def)
    {
        return uint32(sConfigMgr->GetIntDefault(name, int(def)));
    }

    inline uint32 GetUInt(std::string const& name, uint32 def)
    {
        return uint32(sConfigMgr->GetIntDefault(name.c_str(), int(def)));
    }

    inline float GetFloat(char const* name, float def)
    {
        return sConfigMgr->GetFloatDefault(name, def);
    }

    inline float GetFloat(std::string const& name, float def)
    {
        return sConfigMgr->GetFloatDefault(name.c_str(), def);
    }

    inline std::string GetString(char const* name, std::string const& def)
    {
        return sConfigMgr->GetStringDefault(name, def);
    }

    inline std::string GetString(std::string const& name, std::string const& def)
    {
        return sConfigMgr->GetStringDefault(name.c_str(), def);
    }
}

#endif
