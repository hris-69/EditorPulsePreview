#include "RodGameObject.hpp"

RodGameObject* RodGameObject::create(const char* frame, int key) {
    const auto ret = new RodGameObject();
    if (ret->init(frame)) {
        ret->autorelease();
        ret->m_objectID = key;

        // NOTE: Setting m_classType to Game prevents the game 
        // from treating the object as a trigger and prevents Tinker
        // from adding a slider to the object
        ret->m_classType = GameObjectClassType::Game; 
        return ret;
    }

    delete ret;
    return nullptr;
}

void RodGameObject::setStartPos(cocos2d::CCPoint position) {
    EffectGameObject::setStartPos(position);
    if (!this->m_rodBall) return;

    const auto parent = this->m_rodBall->getParent();
    if (!parent) return;

    this->m_rodBall->setStartPos(parent->convertToNodeSpace(
        this->convertToWorldSpace({this->m_obRect.size.width * 0.5f, this->m_obRect.size.height + 10.0f})));
}

void RodGameObject::setPosition(const cocos2d::CCPoint& position) {
    EffectGameObject::setPosition(position);
    if (!this->m_rodBall) return;

    const auto parent = this->m_rodBall->getParent();
    if (!parent) return;

    this->m_rodBall->setPosition(parent->convertToNodeSpace(
        this->convertToWorldSpace({this->m_obRect.size.width * 0.5f, this->m_obRect.size.height + 10.0f})));
}

int RodGameObject::addToGroup(int id) {
    const auto ret = EffectGameObject::addToGroup(id);
    if (this->m_rodBall) this->m_rodBall->addToGroup(id);
    return ret;
}

void RodGameObject::removeFromGroup(int id) {
    EffectGameObject::removeFromGroup(id);
    if (this->m_rodBall) this->m_rodBall->removeFromGroup(id);
}