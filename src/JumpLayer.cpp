#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "JumpLayer.hpp"

using namespace geode::prelude;

class $modify(MyGameLayer, GJBaseGameLayer) {
    void handleButton(bool push, int button, bool isPlayer1) {
        if (button != 1 || !push) { 
            GJBaseGameLayer::handleButton(push, button, isPlayer1);
            return;
        }

        if (JumpLayer::jumpApproved) {
            GJBaseGameLayer::handleButton(push, button, isPlayer1);
            return;
        }

        if (!JumpLayer::isTortureActive) {
            JumpLayer::isTortureActive = true;
            
            bool isPlayer2 = !isPlayer1;
            auto tortureOverlay = JumpLayer::create(this, isPlayer2);
            tortureOverlay->setID("jump-torture-layer");
            this->addChild(tortureOverlay, 9999);
            
            return;
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        
        JumpLayer::isTortureActive = false;
        JumpLayer::jumpApproved = false;
    }
    
    void destroyPlayer(PlayerObject* player, GameObject* hazard) {
        PlayLayer::destroyPlayer(player, hazard);
        
        if (auto layer = this->getChildByID("jump-torture-layer")) {
            layer->removeFromParent();
            JumpLayer::isTortureActive = false;
            JumpLayer::jumpApproved = false;
        }
    }

    void onQuit() {
        JumpLayer::isTortureActive = false;
        JumpLayer::jumpApproved = false;
        
        PlayLayer::onQuit();
    }
};
