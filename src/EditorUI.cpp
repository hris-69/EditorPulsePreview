#include <Geode/modify/EditorUI.hpp>
#include "LevelEditorLayer.hpp"

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
    void onPlayback(CCObject* sender) {
        EditorUI::onPlayback(sender);
        this->updateAudioEffects();
    }

    void onPlaytest(CCObject* sender) {
        EditorUI::onPlaytest(sender);
        this->updateAudioEffects();
    }

    void selectObjects(CCArray* objects, bool ignoreFilter) {
        // NOTE: A separate array has to be created because
        // removing objects from the `objects` parameter is unreliable
        auto arr = CCArray::create();
        for (const auto& object : objects->asExt<GameObject*>()) {
            if (object->m_objectID != 37) arr->addObject(object);
        }

        EditorUI::selectObjects(arr, ignoreFilter);
    }

    bool canSelectObject(GameObject* object) {
        return object->m_objectID == 37 ? false : EditorUI::canSelectObject(object);
    }

    bool shouldDeleteObject(GameObject* object) {
        return object->m_objectID == 37 ? false : EditorUI::shouldDeleteObject(object);
    }

    void updateAudioEffects() {
        const auto fields = modify_cast<MyLevelEditorLayer*>(this->m_editorLayer)->m_fields.self();
        if (!fields->m_AEL || !fields->m_AEL->m_unk1c0) return;

        auto songOffset = 0.0f;
        if (this->m_editorLayer->m_startPosObject && this->m_editorLayer->m_playbackMode == PlaybackMode::Playing) {
            songOffset = std::max(this->m_editorLayer->m_startPosObject->getStartPos().x / 311.58f, 0.0f);
        } else if (this->m_isPlayingMusic) {
            songOffset = this->m_playbackStartTime;
        }

        fields->m_AEL->resetAudioVars();
        fields->m_AEL->m_timeElapsed = songOffset;

        // NOTE: Pulsing using AEL will be out of sync if playback
        // isn't started from the beginning, so the 
        // skipped audio vars must be removed manually
        while (fields->m_AEL->m_unk1c0->count() >= 2) {
            const auto t = static_cast<CCString*>(fields->m_AEL->m_unk1c0->objectAtIndex(0))->floatValue();
            if (t >= songOffset) break;

            fields->m_AEL->m_unk1c0->removeObjectAtIndex(0, true);
            fields->m_AEL->m_unk1c0->removeObjectAtIndex(0, true);
        }
    }
};