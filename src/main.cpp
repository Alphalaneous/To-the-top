#include <Geode/Geode.hpp>
#include <Geode/modify/CCScene.hpp>

using namespace geode::prelude;

class $modify(CCScene) {

    int getHighestChildZ() {
		auto sceneManager = SceneManager::get();

		std::unordered_map<CCNode*, int> zOrders;

		for (auto node : sceneManager->getPersistedNodes()) {
			zOrders[node] = node->getZOrder();
			node->setZOrder(0);
		}

		auto ret = CCScene::getHighestChildZ();

		for (auto node : sceneManager->getPersistedNodes()) {
			node->setZOrder(zOrders[node]);
		}

		return ret;
	}
};