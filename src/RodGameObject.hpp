#pragma once

class RodGameObject : public EffectGameObject {
public:
    GameObject* m_rodBall = nullptr;

    static RodGameObject* create(const char* frame, int key);

    void setPosition(const cocos2d::CCPoint& position) override;
    void setStartPos(cocos2d::CCPoint position) override;

    int addToGroup(int id) override;
    void removeFromGroup(int id) override;
};