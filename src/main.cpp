
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class PhysicsToolkitPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(300.f, 300.f))
            return false;

        setTitle("GD Physics Toolkit");

        auto size = m_mainLayer->getContentSize();

        auto addPanel = [&](const char* title, ccColor3B color,
                            float y, const char* left,
                            const char* right) {
            auto label = CCLabelBMFont::create(title, "bigFont.fnt");
            label->setScale(0.55f);
            label->setColor(color);
            label->setPosition({size.width / 2.f, y});
            m_mainLayer->addChild(label);

            auto a = CCLabelBMFont::create(left, "bigFont.fnt");
            a->setScale(0.42f);
            a->setPosition({size.width * 0.32f, y - 25.f});
            m_mainLayer->addChild(a);

            auto b = CCLabelBMFont::create(right, "bigFont.fnt");
            b->setScale(0.42f);
            b->setPosition({size.width * 0.68f, y - 25.f});
            m_mainLayer->addChild(b);
        };

        addPanel("BOTTING", {120, 190, 255},
                 size.height - 65.f, "RECORD", "SAVE / PLAY");

        addPanel("UTILS", {120, 255, 190},
                 size.height - 135.f, "HITBOXES", "NOCLIP");

        addPanel("TPS", {255, 220, 100},
                 size.height - 205.f, "FPS CHANGER", "PHYSICS");

        auto speed = CCLabelBMFont::create(
            "SPEEDHACK  |  CHANGE TPS", "bigFont.fnt"
        );
        speed->setScale(0.42f);
        speed->setPosition({size.width / 2.f, 25.f});
        m_mainLayer->addChild(speed);

        return true;
    }

public:
    static PhysicsToolkitPopup* create() {
        auto ret = new PhysicsToolkitPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(ToolkitMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto menu = getChildByID("bottom-menu");
        if (menu) {
            auto sprite = ButtonSprite::create(
                "Toolkit", 80, true, "goldFont.fnt",
                "GJ_button_01.png", 30.f, 0.8f
            );

            auto button = CCMenuItemSpriteExtra::create(
                sprite, this,
                menu_selector(ToolkitMenuLayer::openToolkit)
            );

            menu->addChild(button);
            menu->updateLayout();
        }
        return true;
    }

    void openToolkit(CCObject*) {
        PhysicsToolkitPopup::create()->show();
    }
};

class $modify(ToolkitPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = CCMenu::create();
        menu->setPosition({
            CCDirector::sharedDirector()->getWinSize().width - 65.f,
            CCDirector::sharedDirector()->getWinSize().height - 85.f
        });

        auto sprite = ButtonSprite::create(
            "Toolkit", 80, true, "goldFont.fnt",
            "GJ_button_01.png", 30.f, 0.7f
        );

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(ToolkitPauseLayer::openToolkit)
        );

        menu->addChild(button);
        addChild(menu);
    }

    void openToolkit(CCObject*) {
        PhysicsToolkitPopup::create()->show();
    }
};
