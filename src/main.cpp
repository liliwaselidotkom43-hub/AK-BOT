
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/Popup.hpp>

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

namespace Toolkit {
    struct InputEvent {
        int tick;
        int button;
        bool player2;
        bool pressed;
    };

    inline bool recording = false;
    inline bool noclip = false;
    inline bool hitboxes = false;
    inline bool speedhack = false;
    inline bool fpsEnabled = false;
    inline bool bypass = false;

    inline float speed = 1.0f;
    inline float targetFPS = 60.0f;
    inline int targetTPS = 240;
    inline int tick = 0;

    inline std::vector<InputEvent> events;

    inline std::string macroPath() {
        return (Mod::get()->getSaveDir() / "macro.csv").string();
    }

    inline void recordInput(PlayerObject* player, PlayerButton button,
                            bool pressed) {
        if (!recording || !PlayLayer::get())
            return;

        events.push_back({
            tick,
            static_cast<int>(button),
            player->m_isSecondPlayer,
            pressed
        });
    }

    inline void startRecording() {
        if (!PlayLayer::get()) {
            FLAlertLayer::create(
                "Botting",
                "Enter a level before recording.",
                "OK"
            )->show();
            return;
        }

        events.clear();
        tick = 0;
        recording = true;

        FLAlertLayer::create(
            "Botting",
            "Recording started. Play the level, then press SAVE.",
            "OK"
        )->show();
    }

    inline void saveRecording() {
        recording = false;

        std::ofstream file(macroPath());
        if (!file.is_open()) {
            FLAlertLayer::create(
                "Botting",
                "Could not create macro.csv.",
                "OK"
            )->show();
            return;
        }

        file << "tick,button,player2,pressed\n";
        for (auto const& e : events) {
            file << e.tick << ','
                 << e.button << ','
                 << e.player2 << ','
                 << e.pressed << '\n';
        }

        file.close();

        FLAlertLayer::create(
            "Botting",
            "Macro saved to the mod save folder.",
            "OK"
        )->show();
    }

    inline void playRecording() {
        std::ifstream file(macroPath());

        if (!file.is_open()) {
            FLAlertLayer::create(
                "Botting",
                "No saved macro found. Record and SAVE first.",
                "OK"
            )->show();
            return;
        }

        FLAlertLayer::create(
            "Botting",
            "The macro file exists, but automatic input playback "
            "is not implemented in this build.",
            "OK"
        )->show();
    }
}

class PhysicsToolkitPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(340.f, 390.f))
            return false;

        this->setTitle("GD Physics Toolkit");

        auto bg = CCLayerColor::create(
            ccc4(12, 18, 34, 245),
            320.f,
            345.f
        );
        bg->setPosition({10.f, 18.f});
        this->addChild(bg);

        auto addLabel = [&](char const* text, float x, float y,
                            ccColor3B color) {
            auto label = CCLabelBMFont::create(text, "bigFont.fnt");
            label->setScale(0.42f);
            label->setColor(color);
            label->setPosition({x, y});
            bg->addChild(label);
        };

        auto addButton = [&](char const* text, float x, float y,
                             int tag, ccColor3B color) {
            auto sprite = ButtonSprite::create(text, 100, true, "goldFont.fnt",
                                               "GJ_button_01.png", 24.f, 0.55f);
            sprite->setColor(color);

            auto button = CCMenuItemSpriteExtra::create(
                sprite, this,
                menu_selector(PhysicsToolkitPopup::onButton)
            );
            button->setTag(tag);
            button->setPosition({x, y});
            return button;
        };

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        bg->addChild(menu, 10);

        auto white = ccc3(235, 242, 255);
        auto blue = ccc3(100, 190, 255);
        auto green = ccc3(100, 255, 190);
        auto yellow = ccc3(255, 225, 100);

        addLabel("BOTTING", 160.f, 315.f, blue);
        menu->addChild(addButton("RECORD", 65.f, 278.f, 1, blue));
        menu->addChild(addButton("SAVE", 160.f, 278.f, 2, blue));
        menu->addChild(addButton("PLAY", 255.f, 278.f, 3, blue));

        addLabel("UTILS", 160.f, 225.f, green);
        menu->addChild(addButton("NOCLIP", 65.f, 188.f, 4, green));
        menu->addChild(addButton("HITBOXES", 160.f, 188.f, 5, green));
        menu->addChild(addButton("SPEED x2", 255.f, 188.f, 6, green));

        addLabel("TPS / FPS", 160.f, 133.f, yellow);
        menu->addChild(addButton("FPS 60", 65.f, 96.f, 7, yellow));
        menu->addChild(addButton("BYPASS", 160.f, 96.f, 8, yellow));
        menu->addChild(addButton("TPS 240", 255.f, 96.f, 9, yellow));

        addLabel("Tap buttons to toggle supported features", 160.f, 38.f, white);

        return true;
    }

    void onButton(CCObject* sender) {
        auto tag = sender->getTag();

        switch (tag) {
            case 1:
                Toolkit::startRecording();
                break;

            case 2:
                Toolkit::saveRecording();
                break;

            case 3:
                Toolkit::playRecording();
                break;

            case 4: {
                auto* layer = PlayLayer::get();
                if (!layer) {
                    FLAlertLayer::create("Utils", "Enter a level first.", "OK")->show();
                    break;
                }

                Toolkit::noclip = !Toolkit::noclip;
                layer->toggleIgnoreDamage(Toolkit::noclip);
                break;
            }

            case 5: {
                auto* layer = PlayLayer::get();
                if (!layer) {
                    FLAlertLayer::create("Utils", "Enter a level first.", "OK")->show();
                    break;
                }

                Toolkit::hitboxes = !Toolkit::hitboxes;
                layer->toggleDebugDraw();
                break;
            }

            case 6:
                Toolkit::speedhack = !Toolkit::speedhack;
                Toolkit::speed = Toolkit::speedhack ? 2.0f : 1.0f;
                FLAlertLayer::create(
                    "Speedhack",
                    Toolkit::speedhack ? "Speed multiplier: 2x" : "Speedhack disabled",
                    "OK"
                )->show();
                break;

            case 7:
                Toolkit::fpsEnabled = !Toolkit::fpsEnabled;
                Toolkit::targetFPS = Toolkit::fpsEnabled ? 120.f : 60.f;
                CCDirector::sharedDirector()->setAnimationInterval(
                    1.0 / Toolkit::targetFPS
                );
                break;

            case 8:
                FLAlertLayer::create(
                    "Physics Bypass",
                    "Custom TPS bypass is not implemented in this build.",
                    "OK"
                )->show();
                break;

            case 9:
                FLAlertLayer::create(
                    "TPS",
                    "Custom TPS input is not implemented in this build.",
                    "OK"
                )->show();
                break;
        }
    }

public:
    static PhysicsToolkitPopup* create() {
        auto ret = new PhysicsToolkitPopup();
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

        auto sprite = CCSprite::createWithSpriteFrameName(
            "GJ_optionsBtn_001.png"
        );

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(ToolkitMenuLayer::openToolkit)
        );

        button->setID("physics-toolkit-button"_spr);
        menu->addChild(button);
        menu->updateLayout();

        return true;
    }

    void openToolkit(CCObject*) {
        PhysicsToolkitPopup::create()->show();
    }
};

class $modify(ToolkitPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        Toolkit::recordInput(this, button, true);
        return PlayerObject::pushButton(button);
    }

    bool releaseButton(PlayerButton button) {
        Toolkit::recordInput(this, button, false);
        return PlayerObject::releaseButton(button);
    }
};

class $modify(ToolkitPlayLayer, PlayLayer) {
    void update(float dt) {
        if (Toolkit::speedhack)
            dt *= Toolkit::speed;

        PlayLayer::update(dt);

        if (Toolkit::recording)
            ++Toolkit::tick;
    }

    void onQuit() {
        Toolkit::recording = false;

        if (Toolkit::noclip) {
            this->toggleIgnoreDamage(false);
            Toolkit::noclip = false;
        }

        PlayLayer::onQuit();
    }
};
