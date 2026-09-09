#include <Geode/modify/AudioEffectsLayer.hpp>
#include <Geode/modify/LevelSettingsLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/SetGroupIDLayer.hpp>
#include <Geode/modify/CustomizeObjectLayer.hpp>
#include "LevelEditorLayer.hpp"
#include "RodGameObject.hpp"

using namespace geode::prelude;

// NOTE: AudioEffectsLayer::audioStep has to be hooked because 
// Robtop didn't add a null check for PlayLayer, causing the game 
// to crash when AEL is added elsewhere. Thanks, Robtop
class $modify(AudioEffectsLayer) {
    static void onModify(auto& self) {
        self.setHookPriorityPre("AudioEffectsLayer::audioStep", Priority::Replace)
            ? log::info("Successfully set hook priority for AudioEffectsLayer::audioStep")
            : log::error("Failed to set hook priority for AudioEffectsLayer::audioStep");
    }

	void audioStep(float dt) {
		this->m_timeElapsed += dt;

        if (!this->m_unk1c0 || this->m_unk1c0->count() == 0)
            return;

        if (static_cast<CCString*>(this->m_unk1c0->objectAtIndex(0))->floatValue() >= this->m_timeElapsed)
            return;

        if (const auto playLayer = PlayLayer::get())
            if (playLayer->m_isPracticeMode) return;
        
        const auto effect = static_cast<CCString*>(this->m_unk1c0->objectAtIndex(1))->floatValue();
        this->m_unk1c0->removeObjectAtIndex(0, true);
        this->m_unk1c0->removeObjectAtIndex(0, true);
        this->triggerEffect(effect);
	}
};

class $modify(LevelSettingsLayer) {
    void onClose(CCObject* sender) {
        if (!this->m_songSelectNode) return LevelSettingsLayer::onClose(sender);
        
        const auto fields = modify_cast<MyLevelEditorLayer*>(this->m_editorLayer)->m_fields.self();
        
        if (this->m_songSelectNode->m_isCustomSong) {
            FMODAudioEngine::sharedEngine()->enableMetering();
            if (fields->m_AEL) {
                fields->m_AEL->removeFromParent();
                fields->m_AEL = nullptr;
            }
        } else {
            fields->m_AEL = AudioEffectsLayer::create(LevelTools::getAudioString(this->m_editorLayer->m_level->m_audioTrack));
            this->m_editorLayer->m_objectLayer->addChild(fields->m_AEL);
            FMODAudioEngine::sharedEngine()->disableMetering();
        }

        LevelSettingsLayer::onClose(sender);
    }
};

// XXX: I was too lazy to find where teleport portals
// had their editor layers synced so I hooked this
class $modify(SetGroupIDLayer) {
    void onClose(CCObject* sender) {
        if (this->m_targetObject && this->m_targetObject->m_objectID >= 15 && this->m_targetObject->m_objectID <= 17) {
            const auto& ball = static_cast<RodGameObject*>(this->m_targetObject)->m_rodBall;
            ball->m_editorLayer = this->m_editorLayerValue;
            ball->m_editorLayer2 = this->m_editorLayer2Value;
        } else {
            for (const auto& object : this->m_targetObjects->asExt<GameObject*>()) {
                if (object->m_objectID >= 15 && object->m_objectID <= 17) {
                    const auto& ball = static_cast<RodGameObject*>(this->m_targetObject)->m_rodBall;
                    ball->m_editorLayer = this->m_editorLayerValue;
                    ball->m_editorLayer2 = this->m_editorLayer2Value;
                }
            }
        }

        SetGroupIDLayer::onClose(sender);
    }
};

// XXX: There's probably a more proper way of doing this
class $modify(CustomizeObjectLayer) {
    void updateSelected(int id) {
        CustomizeObjectLayer::updateSelected(id);

        if (this->m_targetObject && this->m_targetObject->m_objectID >= 15 && this->m_targetObject->m_objectID <= 17) {
            const auto& ball = static_cast<RodGameObject*>(this->m_targetObject)->m_rodBall;
            ball->m_updateParents = true;
            ball->setMainColorMode(this->m_customColorChannel);
        } else {
            for (const auto& object : this->m_targetObjects->asExt<GameObject*>()) {
                if (object->m_objectID >= 15 && object->m_objectID <= 17) {
                    const auto& ball = static_cast<RodGameObject*>(this->m_targetObject)->m_rodBall;
                    ball->m_updateParents = true;
                    ball->setMainColorMode(this->m_customColorChannel);
                }
            }
        }
    }
};
 
class $modify(GameObject) {
    static void onModify(auto& self) {
        g_hkCreateWithKey = self.getHook("GameObject::createWithKey").unwrap();
        g_hkCreateWithKey->disable() 
            ? log::info("Disabled GameObject::createWithKey hook in onModify") 
            : log::warn("Failed to disable the GameObject::createWithKey in onModify");
    }

    static GameObject* createWithKey(int key) {
        return key >= 15 && key <= 17 
            ? RodGameObject::create(ObjectToolbox::sharedState()->intKeyToFrame(key), key) 
            : GameObject::createWithKey(key);
    }
};