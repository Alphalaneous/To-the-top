#include <Geode/Geode.hpp>
#include <Geode/modify/CCScene.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/VMTHookManager.hpp>
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

class $modify(MyCCScene, CCScene) {
    
    bool init() {
        if (!CCScene::init()) return false;
        if (!typeinfo_cast<CCTransitionScene*>(this)) {
            (void) VMTHookManager::get().addHook<ResolveC<MyCCScene>::func(&MyCCScene::getChildren)>(this, "cocos2d::CCScene::getChildren");
            (void) VMTHookManager::get().addHook<ResolveC<MyCCScene>::func(&MyCCScene::getChildrenCount_nc)>(this, "cocos2d::CCScene::getChildrenCount");
            (void) VMTHookManager::get().addHook<ResolveC<MyCCScene>::func(&MyCCScene::onEnter)>(this, "cocos2d::CCScene::onEnter");
        }
        return true;
    }

    CCArray* getChildren() {
        // this can return nullptr :(
        auto children = this->getChildren();
        // I don't wanna actually add them to the children array
        if (children) children = children->shallowCopy();
        else children = CCArray::create();
        children->addObjectsFromArray(Broverlay::get()->getChildren());
        return children;
    }

    unsigned int getChildrenCount_nc() { 
        return static_cast<const MyCCScene*>(this)->getChildrenCount(); 
    }

    unsigned int getChildrenCount() const {
        return this->getChildrenCount() + Broverlay::get()->getChildrenCount();
    }

    void onEnter() {
        this->onEnter();
        Broverlay::get()->onEnter();
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
    for (auto child : CCArrayExt<CCNode*>(self->getChildren())) {
        if ((child = child->getChildByIDRecursive(id))) {
            return child;
        }
    }
    return nullptr;
}

/*struct TestNode : public CCNode {

    static inline TestNode* s_self = nullptr;

    static TestNode* create() {
        auto ret = new TestNode();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() {
        scheduleUpdate();
        s_self = this;
        return true;
    }

    static TestNode* get() {
        if (!s_self) s_self = TestNode::create();
        return s_self;
    }

    ~TestNode() {
        s_self = nullptr;
    }

    void update(float dt) {
        log::info("updating");
    }
};

class $modify(MyMenuLayer, MenuLayer) {

    bool init() {
        if (!MenuLayer::init()) return false;

        if (auto menu = getChildByID("bottom-menu")) {
            auto btnSpr = CCSprite::createWithSpriteFrameName("GJ_achBtn_001.png");
            btnSpr->setColor(ccRED);
            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(MyMenuLayer::removeTestNode));

            menu->addChild(btn);

            auto btnSpr2 = CCSprite::createWithSpriteFrameName("GJ_achBtn_001.png");
            btnSpr2->setColor(ccGREEN);
            auto btn2 = CCMenuItemSpriteExtra::create(btnSpr2, this, menu_selector(MyMenuLayer::addTestNode));

            menu->addChild(btn2);

            menu->updateLayout();
        }

        return true;
    }

    void removeTestNode(CCObject* sender) {
        TestNode::get()->removeFromParent();
    }

    void addTestNode(CCObject* sender) {
        SceneManager::get()->keepAcrossScenes(TestNode::get());
    }
};*/


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
