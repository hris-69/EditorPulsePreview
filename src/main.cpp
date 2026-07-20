#include <Geode/Geode.hpp>
#include <Geode/modify/AudioEffectsLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelSettingsLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

#define USES_METERING level->m_songID > 0 || level->m_audioTrack > 19

// audioStep has to be hooked because Robtop didn't add
// a null check for PlayLayer, causing the game to crash
// when AEL is added elsewhere. Thanks Robtop
class $modify(AudioEffectsLayer) {
	void audioStep(float dt) {
		this->m_timeElapsed += dt;

        if (!this->m_unk1c0 || !this->m_unk1c0->count())
            return;

        if (static_cast<CCString*>(this->m_unk1c0->objectAtIndex(0))->floatValue() >= this->m_timeElapsed)
            return;

        if (auto playLayer = PlayLayer::get()) {
            if (playLayer->m_isPracticeMode) {
                return;
            }
        }
        
        auto effect = static_cast<CCString*>(this->m_unk1c0->objectAtIndex(1))->floatValue();
        this->m_unk1c0->removeObjectAtIndex(0, true);
        this->m_unk1c0->removeObjectAtIndex(0, true);
        this->triggerEffect(effect);
	}
};

class $modify(MyLevelEditorLayer, LevelEditorLayer) {
	struct Fields {
		AudioEffectsLayer* m_AEL = nullptr;

        // Only used for optimization
        bool m_wasPulsing = false;
	};

    void updateEditor(float dt) {
        LevelEditorLayer::updateEditor(dt);

        if (!this->m_editorUI->m_isPlayingMusic && this->m_playbackMode != PlaybackMode::Playing) {
            if (m_fields->m_wasPulsing) {
                for (auto& object : m_activeObjects) {
                    if (object->m_usesAudioScale && !object->m_hasNoAudioScale) {
                        object->setRScale(1.0f);
                    }
                }

                m_fields->m_wasPulsing = false;
            }

            return;
        }

        m_fields->m_wasPulsing = true;
        if (m_fields->m_AEL) m_fields->m_AEL->audioStep(dt);

        auto fmod = FMODAudioEngine::sharedEngine();
        auto scale = 0.5f;
        
        if (fmod->m_metering) {
            scale = fmod->getMeteringValue();
        } else if (m_fields->m_AEL) {
            scale = m_fields->m_AEL->m_audioScale;
        }
        
        for (auto& object : m_activeObjects) {
            if (!object->m_unk3F8 && object->m_usesAudioScale && !object->m_hasNoAudioScale) {
                if (object->m_customAudioScale) {
                    object->setRScale(object->m_minAudioScale + ((scale - 0.1f) * (object->m_maxAudioScale - object->m_minAudioScale)));
                } else {
                    object->setRScale(scale);
                }
            }
        }
    }

	bool init(GJGameLevel* level, bool noUI) {
		if (!level || !LevelEditorLayer::init(level, noUI))
            return false;

        if (USES_METERING) {
            FMODAudioEngine::sharedEngine()->enableMetering();
        } else {
            m_fields->m_AEL = AudioEffectsLayer::create(LevelTools::getAudioString(level->m_audioTrack));
            if (this->m_objectLayer && m_fields->m_AEL) this->m_objectLayer->addChild(m_fields->m_AEL);
        }

        return true;
	}
};

class $modify(EditorUI) {
    void onPlayback(CCObject* sender) {
        EditorUI::onPlayback(sender);

        auto AEL = static_cast<MyLevelEditorLayer*>(this->m_editorLayer)->m_fields->m_AEL;
        if (!AEL) return;

        AEL->resetAudioVars();
        AEL->m_timeElapsed = this->m_playbackStartTime;

        // Pulsing using AEL will be off synced if audio playback
        // isn't started from the beginning, so I must remove the 
        // passed audio vars manually. Same for playtesting
        while (AEL->m_unk1c0->count() >= 2) {
            float t = static_cast<CCString*>(AEL->m_unk1c0->objectAtIndex(0))->floatValue();
            if (t >= this->m_playbackStartTime) break;

            AEL->m_unk1c0->removeObjectAtIndex(0, true);
            AEL->m_unk1c0->removeObjectAtIndex(0, true);
        }
    }

    void onPlaytest(CCObject* sender) {
        EditorUI::onPlaytest(sender);

        auto AEL = static_cast<MyLevelEditorLayer*>(this->m_editorLayer)->m_fields->m_AEL;
        if (!AEL) return;

        auto songOffset = 0.0f;
        if (this->m_editorLayer->m_startPosObject)
            songOffset = std::max(this->m_editorLayer->m_startPosObject->getStartPos().x / 311.58f, 0.0f);

        AEL->resetAudioVars();
        AEL->m_timeElapsed = songOffset;

        while (AEL->m_unk1c0->count() >= 2) {
            float t = static_cast<CCString*>(AEL->m_unk1c0->objectAtIndex(0))->floatValue();
            if (t >= songOffset) break;

            AEL->m_unk1c0->removeObjectAtIndex(0, true);
            AEL->m_unk1c0->removeObjectAtIndex(0, true);
        }
    }
};

class $modify(LevelSettingsLayer) {
    void onClose(CCObject* sender) {
        LevelSettingsLayer::onClose(sender);
        
        auto lel = this->m_editorLayer;
        auto myLEL = static_cast<MyLevelEditorLayer*>(lel);
        auto level = lel->m_level;
        
        if (USES_METERING) {
            FMODAudioEngine::sharedEngine()->enableMetering();
            if (myLEL->m_fields->m_AEL) {
                myLEL->m_fields->m_AEL->removeFromParent();
                myLEL->m_fields->m_AEL = nullptr;
            }
        } else {
            myLEL->m_fields->m_AEL = AudioEffectsLayer::create(LevelTools::getAudioString(level->m_audioTrack));
            if (lel->m_objectLayer && myLEL->m_fields->m_AEL)
                lel->m_objectLayer->addChild(myLEL->m_fields->m_AEL);

            FMODAudioEngine::sharedEngine()->disableMetering();
        }
    }
};

#undef USES_METERING