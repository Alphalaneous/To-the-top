#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class Broverlay : public CCNode {
public:
    static Broverlay* get();
    void onEnter() override;
    void removeChild(CCNode* child, bool cleanup) override;
    void keepAcrossScenes(CCNode* node);
    void forget(CCNode* node);
    std::span<Ref<CCNode> const> getPersistedNodes();
protected:
    std::vector<Ref<cocos2d::CCNode>> m_persistedNodes;
    void recursiveTouchFix(CCNode* node);
};