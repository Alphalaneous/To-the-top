#include "Broverlay.hpp"

Broverlay* Broverlay::get() {
    static Broverlay* instance = nullptr;
    if (!instance) {
        instance = new Broverlay();
    }
    return instance;
}

void Broverlay::recursiveTouchFix(CCNode* node) {	
    if (auto delegate = typeinfo_cast<CCTouchDelegate*>(node)) {
        if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
            auto oldTouchPrio = node->getUserObject("old-touch-prio"_spr);
            if (!oldTouchPrio) {
                oldTouchPrio = CCInteger::create(handler->getPriority());
                node->setUserObject("old-touch-prio"_spr, oldTouchPrio);
            }
            CCTouchDispatcher::get()->setPriority(static_cast<CCInteger*>(oldTouchPrio)->getValue() - 10000, handler->getDelegate());
        }
    }
    
    for (auto it = CCArrayExt<CCNode*>(node->getChildren()).rbegin();
      it != CCArrayExt<CCNode*>(node->getChildren()).rend(); ++it) {
        recursiveTouchFix(*it);
    }
}

void Broverlay::onEnter() {
    CCNode::onEnter();
	recursiveTouchFix(this);
}

void Broverlay::keepAcrossScenes(CCNode* node) {
    if (ranges::contains(m_persistedNodes, node)) {
        return;
    }
    m_persistedNodes.push_back(node);
    addChild(node);
}

void Broverlay::forget(CCNode* node) {
    std::erase(m_persistedNodes, node);
    CCNode::removeChild(node);
}

void Broverlay::removeChild(CCNode* child, bool cleanup) {
    CCNode::removeChild(child, cleanup);
    if (cleanup) {
        std::erase(m_persistedNodes, child);
    }
}

std::span<Ref<CCNode> const> Broverlay::getPersistedNodes() {
    return m_persistedNodes;
}