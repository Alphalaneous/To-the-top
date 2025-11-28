#include <Geode/Geode.hpp>
#include <Geode/modify/CCNode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "Broverlay.hpp"

using namespace geode::prelude;

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_IOS)
#include <Geode/modify/CCEGLView.hpp>
class $modify(MyCCEGLView, CCEGLView) {
    void swapBuffers() {
        Broverlay::get()->visit();
        CCEGLView::swapBuffers();
    }
};
#else
#include <Geode/modify/CCDirector.hpp>
class $modify(MyCCDirector, CCDirector) {
    void drawScene() {
        CCDirector::drawScene();
        Broverlay::get()->visit();
    }
};
#endif

class $modify(MyCCNode, CCNode) {

    struct Fields {
        bool m_grabbedType;
        bool m_isCCScene;
    };

    inline bool isCCScene() {
        auto fields = m_fields.self();
        if (!fields->m_grabbedType) {
            fields->m_isCCScene = typeinfo_cast<CCScene*>(this) && !typeinfo_cast<CCTransitionScene*>(this);
            fields->m_grabbedType = true;
        }
        return fields->m_isCCScene;
    }

    /*unsigned int getChildrenCount() const {
        auto self = const_cast<MyCCNode*>(this);
        if (self->isCCScene()) [[unlikely]] {
            return CCNode::getChildrenCount() + Broverlay::get()->getChildrenCount();
        }
        return CCNode::getChildrenCount();
    }*/

    CCArray* getChildren() {
        if (isCCScene()) [[unlikely]] {
            auto children = CCNode::getChildren();

            if (children) children = children->shallowCopy();
            else children = CCArray::create();

            children->addObjectsFromArray(Broverlay::get()->getChildren());
            return children;
        }
        return CCNode::getChildren();
    }

    void onEnter() {
        CCNode::onEnter();
        if (isCCScene()) [[unlikely]] {
            Broverlay::get()->onEnter();
        }
    }
};

void keepAcrossScenes_H(SceneManager* self, cocos2d::CCNode* node) {
    Broverlay::get()->keepAcrossScenes(node);
}

void forget_H(SceneManager* self, cocos2d::CCNode* node) {
    Broverlay::get()->forget(node);
}

std::span<Ref<CCNode> const> getPersistedNodes_H(SceneManager* self) {
    return Broverlay::get()->getPersistedNodes();
}

CCNode* getChildByIDRecursive_H(CCNode* self, std::string_view id) {
    if (auto child = self->getChildByID(id)) {
        return child;
    }
    for (auto child : self->getChildrenExt()) {
        if ((child = child->getChildByIDRecursive(id))) {
            return child;
        }
    }
    return nullptr;
}

$on_mod(Loaded) {
    (void) Mod::get()->hook(
        reinterpret_cast<void*>(addresser::getNonVirtual(&SceneManager::keepAcrossScenes)),
        &keepAcrossScenes_H,
        "SceneManager::keepAcrossScenes"
    );

    (void) Mod::get()->hook(
        reinterpret_cast<void*>(addresser::getNonVirtual(&SceneManager::forget)),
        &forget_H,
        "SceneManager::forget"
    );

    (void) Mod::get()->hook(
        reinterpret_cast<void*>(addresser::getNonVirtual(&SceneManager::getPersistedNodes)),
        &getPersistedNodes_H,
        "SceneManager::getPersistedNodes"
    );

    (void) Mod::get()->hook(
        reinterpret_cast<void*>(addresser::getNonVirtual(&CCNode::getChildByIDRecursive)),
        &getChildByIDRecursive_H,
        "CCNode::getChildByIDRecursive"
    );
}
