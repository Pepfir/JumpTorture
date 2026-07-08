#include "JumpLayer.hpp"

JumpLayer* JumpLayer::create(GJBaseGameLayer* gameLayer, bool isPlayer2) {
    auto ret = new JumpLayer();
    if (ret && ret->init(gameLayer, isPlayer2)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool JumpLayer::init(GJBaseGameLayer* gameLayer, bool isPlayer2) {
    if (!CCLayer::init()) return false;
    
    m_gameLayer = gameLayer;
    m_isPlayer2 = isPlayer2;
    
    // Ставим игру на паузу при создании слоя
    if (m_gameLayer) m_gameLayer->setPaused(true);
    
    std::random_device rd;
    m_rng.seed(rd());
    
    this->setKeyboardEnabled(true);
    this->setTouchEnabled(true);
    this->setTouchMode(kCCTouchesOneByOne);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto bg = CCLayerColor::create({0, 0, 0, 200});
    this->addChild(bg);

    m_questionLabel = CCLabelBMFont::create("Loading...", "bigFont.fnt");
    m_questionLabel->setPosition(winSize.width / 2, winSize.height / 2 + 50);
    m_questionLabel->setScale(0.6f);
    this->addChild(m_questionLabel);

    m_inputNode = geode::TextInput::create(150.f, "Ответ", "bigFont.fnt");
    m_inputNode->setPosition(winSize.width / 2, winSize.height / 2 - 10);
    m_inputNode->setFilter("0123456789-");
    
    m_inputNode->setCallback([this](const std::string& text) {
        m_currentInput = text;
    });
    this->addChild(m_inputNode);

    auto menu = CCMenu::create();
    menu->setPosition(0, 0);
    // Приоритет выше, чем у игры, чтобы кнопка нажималась
    menu->setTouchPriority(-501); 
    
    auto btnSprite = ButtonSprite::create("ВВОД");
    m_confirmBtn = CCMenuItemSpriteExtra::create(
        btnSprite, this, menu_selector(JumpLayer::onConfirm)
    );
    m_confirmBtn->setPosition(winSize.width / 2, winSize.height / 2 - 70);
    menu->addChild(m_confirmBtn);
    this->addChild(menu);

    generateStage();
    return true;
}

void JumpLayer::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);
}

bool JumpLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    return true;
}

void JumpLayer::keyBackClicked() {}

void JumpLayer::generateStage() {
    m_inputNode->setString(""); 
    m_currentInput = "";

    if (m_currentStage == 1) {
        std::uniform_int_distribution<> charDist(0, 25);
        m_targetChar = 'A' + charDist(m_rng);
        m_questionLabel->setString(fmt::format("ЭТАП 1: Нажми кнопку '{}'", m_targetChar).c_str());
        m_inputNode->setVisible(false);
        m_confirmBtn->setVisible(false);
    } 
    else {
        m_inputNode->setVisible(true);
        m_confirmBtn->setVisible(true);
        
        // ... (логика этапов прежняя, сократил для краткости)
        if (m_currentStage == 2) {
             int a = 10, b = 10; m_correctAnswer = 20;
             m_questionLabel->setString("ЭТАП 2: 10 + 10 = ?");
        }
        // [Остальной код логики этапов оставь как был раньше]
    }
}

void JumpLayer::keyDown(enumKeyCodes key, double timestamp) {
    if (m_currentStage == 1) {
        if (key >= KEY_A && key <= KEY_Z) {
            char pressed = 'A' + (key - KEY_A);
            if (pressed == m_targetChar) {
                m_currentStage++;
                generateStage();
            } else {
                resetTorture();
            }
        }
    }
    CCLayer::keyDown(key, timestamp); 
}

void JumpLayer::onConfirm(CCObject* sender) {
    if (m_currentStage == 1) return;
    if (m_currentInput.empty()) return;

    int answer = std::stoi(m_currentInput);
    if (answer == m_correctAnswer) {
        if (m_currentStage < 10) {
            m_currentStage++;
            generateStage();
        } else {
            finishTorture();
        }
    } else {
        resetTorture();
    }
}

void JumpLayer::resetTorture() {
    m_currentStage = 1;
    generateStage();
}

void JumpLayer::finishTorture() {
    JumpLayer::jumpApproved = true;
    JumpLayer::isTortureActive = false;
    
    // Снимаем паузу при завершении
    if (m_gameLayer) {
        m_gameLayer->setPaused(false);
        bool isPlayer1 = !m_isPlayer2; 
        m_gameLayer->handleButton(true, 1, isPlayer1);  
        m_gameLayer->handleButton(false, 1, isPlayer1); 
    }
    
    JumpLayer::jumpApproved = false;
    this->removeFromParent();
}
