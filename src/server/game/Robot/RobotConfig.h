#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#define ROBOT_CONFIG_FILE_NAME  "robot.conf"

#include <string>
#include <list>
#include <vector>
#include <ace/Singleton.h>
#include <ace/Configuration_Import_Export.h>
#include <ace/Thread_Mutex.h>
#include <AutoPtr.h>

typedef Trinity::AutoPtr<ACE_Configuration_Heap, ACE_Null_Mutex> Config;

class RobotConfig
{
    friend class ACE_Singleton<RobotConfig, ACE_Null_Mutex>;
    friend class ConfigLoader;

    RobotConfig() { }
    ~RobotConfig() { }

public:
    /// Method used only for loading main configuration files (authserver.conf and worldserver.conf)
    bool LoadInitial(char const* file);

    /**
     * This method loads additional configuration files
     * It is recommended to use this method in WorldScript::OnConfigLoad hooks
     *
     * @return true if loading was successful
     */
    bool LoadMore(char const* file);

    bool Reload();

    std::string GetStringDefault(const char* name, const std::string& def, bool logUnused = true);
    bool GetBoolDefault(const char* name, bool def, bool logUnused = true);
    int GetIntDefault(const char* name, int def, bool logUnused = true);
    float GetFloatDefault(const char* name, float def, bool logUnused = true);

    std::list<std::string> GetKeysByString(std::string const& name);

    bool isDryRun() { return this->dryRun; }
    void setDryRun(bool mode) { this->dryRun = mode; }

    bool StartRobotSystem();
    uint8 enable;
    uint8 reset;
    std::string robotAccountNamePrefix;

private:
    bool dryRun = false;

    bool GetValueHelper(const char* name, ACE_TString& result);
    bool LoadData(char const* file);

    typedef ACE_Thread_Mutex LockType;
    typedef ACE_Guard<LockType> GuardType;

    std::vector<std::string> _confFiles;
    Config _config;
    LockType _configLock;

    RobotConfig(RobotConfig const&);
    RobotConfig& operator=(RobotConfig const&);
};

#define sRobotConfig ACE_Singleton<RobotConfig, ACE_Null_Mutex>::instance()

#endif
