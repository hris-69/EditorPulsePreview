#include "LevelEditorLayer.hpp"
#include "RodGameObject.hpp"

void MyLevelEditorLayer::updateEditor(float dt) {
    LevelEditorLayer::updateEditor(dt);

    const auto fields = m_fields.self();

    if (!this->m_editorUI->m_isPlayingMusic && this->m_playbackMode != PlaybackMode::Playing) {
        if (fields->m_wasPulsing) {
            for (const auto& object : this->m_objects->asExt<GameObject*>()) {
                if (object->m_usesAudioScale && !object->m_hasNoAudioScale) {
                    object->setRScale(1.0f);
                }
            }

            fields->m_wasPulsing = false;
        }

        return;
    }

    fields->m_wasPulsing = true;
    if (fields->m_AEL) fields->m_AEL->audioStep(dt);

    const auto fmod = FMODAudioEngine::get();
    auto scale = 0.5f;
        
    if (fmod->m_metering) {
        scale = fmod->getMeteringValue();
    } else if (fields->m_AEL) {
        scale = fields->m_AEL->m_audioScale;
    }
        
    for (const auto& object : this->m_objects->asExt<GameObject*>()) {
        if (!object->m_unk3F8 && object->m_usesAudioScale && !object->m_hasNoAudioScale) {
            if (object->m_customAudioScale) {
                object->setRScale(object->m_minAudioScale + ((scale - 0.1f) * (object->m_maxAudioScale - object->m_minAudioScale)));
            } else {
                object->setRScale(scale);
            }
        }
    }
}

bool MyLevelEditorLayer::init(GJGameLevel* level, bool noUI) {
    g_hkCreateWithKey->enable() 
        ? log::info("Successfully enabled the GameObject::createWithKey hook") 
        : log::warn("Failed to enable the GameObject::createWithKey hook");
    m_fields->m_rodBallIndex = utils::random::generate(1, 4);

	if (!LevelEditorLayer::init(level, noUI)) return false;
    if (level->m_songID > 0 || level->m_audioTrack > 19) {
        FMODAudioEngine::sharedEngine()->enableMetering();
    } else {
        m_fields->m_AEL = AudioEffectsLayer::create(LevelTools::getAudioString(level->m_audioTrack));
        this->m_objectLayer->addChild(m_fields->m_AEL);
    }

    return true;
}

void MyLevelEditorLayer::addSpecial(GameObject* object) {
    LevelEditorLayer::addSpecial(object);

    if (object->m_objectID >= 15 && object->m_objectID <= 17) {
        auto& ball = static_cast<RodGameObject*>(object)->m_rodBall;

        // INFO: The rod ball can't be created with LevelEditorLayer::createObject
        ball = GameObject::createWithFrame(fmt::format("rod_ball_{:02}_001.png", m_fields->m_rodBallIndex).c_str());
        ball->m_objectID = 37;
        ball->firstSetup();
        ball->customSetup();

        if (object->m_activeMainColorID != 1004) ball->setMainColorMode(object->m_activeMainColorID);
        ball->saveActiveColors();

        ball->setStartPos(object->convertToWorldSpace({object->m_obRect.size.width * 0.5f, object->m_obRect.size.height + 10.0f}));
        this->addToSection(ball);

        this->m_objects->addObject(ball);
        ball->copyGroups(object);
        this->addToGroups(ball, true);
    }
}

void MyLevelEditorLayer::removeSpecial(GameObject* object) {
    if (object->m_objectID >= 15 && object->m_objectID <= 17)
        this->removeObject(static_cast<RodGameObject*>(object)->m_rodBall, true);

    LevelEditorLayer::removeSpecial(object);  
}