#pragma once

#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

inline Hook* g_hkCreateWithKey = nullptr;

class $modify(MyLevelEditorLayer, LevelEditorLayer) {
    struct Fields {        
		AudioEffectsLayer* m_AEL = nullptr;
        uint8_t m_rodBallIndex = 1;

        // NOTE: Only used for optimization
        bool m_wasPulsing = false;

        ~Fields() {
            log::info("Fields destructor called");

            g_hkCreateWithKey->disable() 
                ? log::info("Successfully disabled the GameObject::createWithKey hook") 
                : log::error("Failed to disable the GameObject::createWithKey hook");
        }
	};

    void updateEditor(float dt);
    bool init(GJGameLevel* level, bool noUI);
    void addSpecial(GameObject* object);
    void removeSpecial(GameObject* object);
};