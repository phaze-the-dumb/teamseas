#include "main.hpp"
#include "ModConfig.hpp"

#include "config-utils/shared/config-utils.hpp"

#include "GlobalNamespace/GameEnergyUIPanel.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector2.hpp"

#include "HMUI/ImageView.hpp"
#include "HMUI/Touchable.hpp"

#include "TMPro/TextMeshProUGUI.hpp"

#include "Utils/WebUtils.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup
DEFINE_CONFIG(ModConfig);

TMPro::TextMeshProUGUI* text;

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

MAKE_HOOK_MATCH(MainMenuViewController_DidActivate, &MainMenuViewController::DidActivate, void,
    MainMenuViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    MainMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    getLogger().info("Menu Opened");

    if(firstActivation){
        text = BeatSaberUI::CreateText(self->get_transform(), "Loading...");
        auto json = WebUtils::GetJSON("https://api.phazed.xyz/api/v3/teamseas/total");

        text->get_transform()->set_localPosition({ 0.0f, 50.0f, 0.0f });

        if(json->IsObject()){
            auto jsonObject = json->GetObject();
            
            text->set_text(il2cpp_utils::createcsstr(std::string(jsonObject["total"].GetString()) + " Pounds removed from the ocean!"));
        }
    } else{
        text->set_text(il2cpp_utils::createcsstr("Loading..."));
        auto json = WebUtils::GetJSON("https://api.phazed.xyz/api/v3/teamseas/total");

        if(json->IsObject()){
            auto jsonObject = json->GetObject();

            text->set_text(il2cpp_utils::createcsstr(std::string(jsonObject["total"].GetString()) + " Pounds removed from the ocean!"));
        }
    }
};

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    if(firstActivation){
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* settings = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        
    } else{
        
    };
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    getLogger().info("Successfully installed Settings UI!");

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, MainMenuViewController_DidActivate);
    getLogger().info("Installed all hooks!");
}