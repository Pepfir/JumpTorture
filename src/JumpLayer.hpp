#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <random>

using namespace geode::prelude;

class JumpLayer : public CCLayer {
protected:
    GJBaseGameLayer* m_gameLayer = nullptr;
    bool m_isPlayer2 = false;
    
    int m_currentStage = 1;
    int m_correctAnswer = 0;
    char m_targetChar = 'A';
    std::string m_currentInput = ""; 
    
    std::mt19937 m_rng; 
    
    CCLabelBMFont* m_questionLabel;
    geode::TextInput* m_inputNode;
    CCMenuItemSpriteExtra* m_confirmBtn;
    
    bool init(GJBaseGameLayer* gameLayer, bool isPlayer2);
    void generateStage();
    void onConfirm(CCObject* sender);
    void resetTorture();
    void finishTorture();

    virtual void registerWithTouchDispatcher() override;
    virtual void keyDown(enumKeyCodes key, double timestamp) override;
    virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    virtual void keyBackClicked() override;

public:
    static JumpLayer* create(GJBaseGameLayer* gameLayer, bool isPlayer2);
    inline static bool isTortureActive = false;
    inline static bool jumpApproved = false;
};
