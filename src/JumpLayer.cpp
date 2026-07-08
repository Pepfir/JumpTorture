#include "JumpLayer.hpp"
#include <fmod/FMODAudioEngine.h>

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
    
    // Глобальная пауза игры
    CCDirector::sharedDirector()->pause();
    // Пауза музыки через FMOD
    FMODAudioEngine::sharedEngine()->pauseAll();
    
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
        
        // Логика этапов
        if (m_currentStage == 2) {
            std::uniform_int_distribution<> dist(10, 50);
            int a = dist(m_rng), b = dist(m_rng);
            m_correctAnswer = a + b;
            m_questionLabel->setString(fmt::format("ЭТАП 2: {} + {} = ?", a, b).c_str());
        }
        else if (m_currentStage == 3) {
            std::uniform_int_distribution<> dist(20, 99);
            int a = dist(m_rng), b = dist(m_rng);
            m_correctAnswer = a - b;
            m_questionLabel->setString(fmt::format("ЭТАП 3: {} - {} = ?", a, b).c_str());
        }
        else if (m_currentStage == 4) {
            std::uniform_int_distribution<> dist(12, 35);
            int a = dist(m_rng), b = dist(m_rng);
            m_correctAnswer = a * b;
            m_questionLabel->setString(fmt::format("ЭТАП 4: {} * {} = ?", a, b).c_str());
        }
        else if (m_currentStage == 5) {
            std::uniform_int_distribution<> dist(5, 15);
            int b = dist(m_rng), ans = dist(m_rng);
            int a = b * ans;
            m_correctAnswer = ans;
            m_questionLabel->setString(fmt::format("ЭТАП 5: {} / {} = ?", a, b).c_str());
        }
        else if (m_currentStage == 6) {
            std::uniform_int_distribution<> dist(2, 9);
            int x = dist(m_rng), a = dist(m_rng), b = dist(m_rng);
            int c = a * x + b;
            m_correctAnswer = x;
            m_questionLabel->setString(fmt::format("ЭТАП 6 (Найди x):\n{}x + {} = {}", a, b, c).c_str());
        }
        else if (m_currentStage == 7) {
            std::uniform_int_distribution<> dist(2, 6);
            int x = dist(m_rng);
            m_correctAnswer = x * x * x;
            m_questionLabel->setString(fmt::format("ЭТАП 7: {} ^ 3 = ?", x).c_str());
        }
        else if (m_currentStage == 8) {
            std::uniform_int_distribution<> dist(-10, 10);
            int m[2][2] = {{dist(m_rng), dist(m_rng)}, {dist(m_rng), dist(m_rng)}};
            m_correctAnswer = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
            m_questionLabel->setString(fmt::format("ЭТАП 8 (det 2x2):\n|{} {}|\n|{} {}|", m[0][0], m[0][1], m[1][0], m[1][1]).c_str());
        }
        else if (m_currentStage == 9) {
            std::uniform_int_distribution<> dist(-2, 2);
            int m[3][3];
            for(int i=0; i<3; i++) for(int j=0; j<3; j++) m[i][j] = dist(m_rng);
            m_correctAnswer = m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1]) - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0]) + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
            m_questionLabel->setString(fmt::format("ЭТАП 9 (Лёгкая 3x3):\n|{} {} {}|\n|{} {} {}|\n|{} {} {}|", m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]).c_str());
        }
        else if (m_currentStage == 10) {
            std::uniform_int_distribution<> dist(-5, 5);
            int m[3][3];
            for(int i=0; i<3; i++) for(int j=0; j<3; j++) m[i][j] = dist(m_rng);
            m_correctAnswer = m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1]) - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0]) + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
            m_questionLabel->setString(fmt::format("ЭТАП 10 (Плотная 3x3):\n|{} {} {}|\n|{} {} {}|\n|{} {} {}|", m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]).c_str());
        }
    }
}

void JumpLayer::keyDown(enumKeyCodes key, double timestamp) {
    if (key == KEY_Enter || key == KEY_KP_Enter) {
        onConfirm(nullptr);
        return;
    }

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

    int answer = 0;
    try { answer = std::stoi(m_currentInput); } catch (...) { resetTorture(); return; }

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
    
    // Возобновляем игру и музыку
    CCDirector::sharedDirector()->resume();
    FMODAudioEngine::sharedEngine()->resumeAll();
    
    if (m_gameLayer) {
        bool isPlayer1 = !m_isPlayer2; 
        m_gameLayer->handleButton(true, 1, isPlayer1);  
        m_gameLayer->handleButton(false, 1, isPlayer1); 
    }
    
    JumpLayer::jumpApproved = false;
    this->removeFromParent();
}
