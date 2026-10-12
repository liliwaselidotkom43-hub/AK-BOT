
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class ToolkitPopup : public geode::Popup {
protected:
    bool init() {
        if (!Popup::init(340.f, 270.f))
            return false;

        this->setTitle("GD PHYSICS TOOLKIT");

        auto addLabel = [this](
            const char* text,
            float x,
            float y,
            float scale,
            cocos2d::ccColor3B color
        ) {
            auto label = cocos2d::CCLabelBMFont::create(
                text, "bigFont.fnt"
            );
            if (!label) return;

            label->setScale(scale);
            label->setColor(color);
            label->setPosition({x, y});
            m_mainLayer->addChild(label);
        };

        auto addPanel = [this](
            float x, float y, float w, float h,
            cocos2d::ccColor4B color
        ) {
            auto panel = cocos2d::CCLayerColor::create(
                color, w, h
            );
            panel->setPosition({x, y});
            m_mainLayer->addChild(panel);
        };

        auto blue  = cocos2d::ccColor3B{67, 184, 255};
        auto mint  = cocos2d::ccColor3B{84, 230, 176};
        auto yellow = cocos2d::ccColor3B{255, 211, 78};
        auto white = cocos2d::ccColor3B{245, 248, 255};

        // Panel backgrounds
        addPanel(20, 177, 300, 39, {20, 42, 62, 255});
        addPanel(20, 94, 300, 75, {19, 49, 43, 255});
        addPanel(20, 20, 300, 66, {55, 46, 24, 255});

        // BOTTING
        addLabel("BOTTING", 170, 202, 0.48f, blue);
        addLabel("RECORD     SAVE     PLAY",
                 170, 187, 0.38f, white);

        // UTILS
        addLabel("UTILS", 170, 152, 0.48f, mint);
        addLabel("SHOW HITBOXES", 170, 134, 0.38f, white);
        addLabel("NOCLIP     SPEEDHACK",
                 170, 112, 0.38f, white);

        // TPS
        addLabel("TPS", 170, 72, 0.48f, yellow);
        addLabel("FPS CHANGER", 100, 53, 0.36f, white);
        addLabel("PHYSICS BYPASS", 235, 53, 0.32f, white);
        addLabel("60 FPS", 100, 35, 0.36f, yellow);
        addLabel("240 TPS", 235, 35, 0.36f, yellow);

        addLabel("UI PREVIEW", 170, 9, 0.25f, white);

        return true;
    }

public:
    static ToolkitPopup* create() {
        auto ret = new ToolkitPopup();

        if (ret->init()) {
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

        auto menu = this->getChildByID("bottom-menu");
        if (!menu)
            return true;

        auto label = cocos2d::CCLabelBMFont::create(
            "PT", "bigFont.fnt"
        );

        if (!label)
            return true;

        auto button = CCMenuItemLabel::create(
            label,
            this,
            menu_selector(ToolkitMenuLayer::onOpenToolkit)
        );

        if (!button)
            return true;

        button->setScale(0.65f);
        button->setID("open-physics-toolkit"_spr);
        menu->addChild(button);
        menu->updateLayout();

        return true;
    }

    void onOpenToolkit(cocos2d::CCObject*) {
        auto popup = ToolkitPopup::create();
        if (popup)
            popup->show();
    }
};
