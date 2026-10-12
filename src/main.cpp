
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/ui/Popup.hpp>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace Toolkit {

struct InputEvent {
    int tick;
    int button;
    bool player2;
    bool pressed;
};

static std::vector<InputEvent> macro;

static bool recording = false;
static bool playing = false;
static bool noclip = false;
static bool hitboxes = false;
static bool bypass = false;
static bool speedhack = false;

static int tick = 0;
static size_t playIndex = 0;

static int tpsValues[] = {120, 240, 360, 480, 720};
static int tpsIndex = 1;

static float speedValues[] = {0.5f, 1.0f, 1.5f, 2.0f, 3.0f};
static int speedIndex = 1;

static double accumulator = 0.0;

static std::filesystem::path macroFile() {
    return Mod::get()->getSaveDir() / "macro.csv";
}

static void alert(const char* title, const std::string& message) {
    FLAlertLayer::create(title, message, "OK")->show();
}

static float gameSpeed() {
    return speedValues[speedIndex];
}

static int targetTPS() {
    return tpsValues[tpsIndex];
}

static bool saveMacro() {
    std::error_code ec;
    auto path = macroFile();

    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream out(path);
    if (!out)
        return false;

    out << "tick,button,player2,pressed\n";

    for (const auto& e : macro) {
        out << e.tick << ','
            << e.button << ','
            << e.player2 << ','
            << e.pressed << '\n';
    }

    return out.good();
}

static bool loadMacro() {
    std::ifstream in(macroFile());
    if (!in)
        return false;

    std::vector<InputEvent> loaded;
    std::string line;

    std::getline(in, line);

    while (std::getline(in, line)) {
        InputEvent e{};
        int p2 = 0;
        int pressed = 0;

        if (std::sscanf(
            line.c_str(),
            "%d,%d,%d,%d",
            &e.tick,
            &e.button,
            &p2,
            &pressed
        ) != 4) {
            continue;
        }

        e.player2 = p2 != 0;
        e.pressed = pressed != 0;
        loaded.push_back(e);
    }

    if (loaded.empty())
        return false;

    std::stable_sort(
        loaded.begin(),
        loaded.end(),
        [](const InputEvent& a, const InputEvent& b) {
            return a.tick < b.tick;
        }
    );

    macro = std::move(loaded);
    return true;
}

static void record() {
    auto pl = PlayLayer::get();

    if (!pl) {
        alert(
            "RECORD",
            "Open a level first, then open Toolkit."
        );
        return;
    }

    if (recording) {
        recording = false;
        alert("RECORD", "Recording stopped.");
        return;
    }

    macro.clear();
    tick = 0;
    playIndex = 0;
    accumulator = 0.0;
    playing = false;

    pl->togglePracticeMode(true);
    recording = true;

    alert(
        "RECORD",
        "Recording started. Resume the level and play."
    );
}

static void save() {
    if (recording) {
        recording = false;
    }

    if (macro.empty()) {
        alert("SAVE", "There are no recorded inputs.");
        return;
    }

    if (saveMacro()) {
        alert("SAVE", "Macro saved successfully.");
    } else {
        alert("SAVE FAILED", "Could not save macro.csv.");
    }
}

static void playMacro() {
    auto pl = PlayLayer::get();

    if (!pl) {
        alert("PLAY", "Open a level first.");
        return;
    }

    recording = false;

    if (macro.empty() && !loadMacro()) {
        alert("PLAY", "No saved macro found.");
        return;
    }

    pl->togglePracticeMode(true);

    tick = 0;
    playIndex = 0;
    accumulator = 0.0;
    playing = true;

    pl->resetLevel();

    alert(
        "PLAY",
        "Macro playback started. Resume the level."
    );
}

static void toggleNoclip() {
    auto pl = PlayLayer::get();

    if (!pl) {
        alert("NOCLIP", "Open a level first.");
        return;
    }

    noclip = !noclip;
    pl->toggleIgnoreDamage(noclip);

    alert(
        "NOCLIP",
        noclip ? "No-damage mode enabled."
               : "No-damage mode disabled."
    );
}

static void toggleHitboxes() {
    auto pl = PlayLayer::get();

    if (!pl) {
        alert("SHOW HITBOXES", "Open a level first.");
        return;
    }

    hitboxes = !hitboxes;
    pl->toggleDebugDraw();

    alert(
        "SHOW HITBOXES",
        hitboxes ? "Debug drawing toggled ON."
                 : "Debug drawing toggled OFF."
    );
}

static void changeSpeed() {
    speedIndex =
        (speedIndex + 1) %
        (sizeof(speedValues) / sizeof(speedValues[0]));

    speedhack = speedIndex != 1;

    alert(
        "SPEEDHACK",
        fmt::format("Speed multiplier: {}x", gameSpeed())
    );
}

static void changeFPS() {
    static int fpsValues[] = {60, 120, 144, 240, 360};
    static int fpsIndex = 0;

    fpsIndex = (fpsIndex + 1) %
        (sizeof(fpsValues) / sizeof(fpsValues[0]));

    CCDirector::sharedDirector()->setAnimationInterval(
        1.0 / fpsValues[fpsIndex]
    );

    alert(
        "FPS CHANGER",
        fmt::format("Requested FPS: {}", fpsValues[fpsIndex])
    );
}

static void changeTPS() {
    tpsIndex = (tpsIndex + 1) %
        (sizeof(tpsValues) / sizeof(tpsValues[0]));

    accumulator = 0.0;

    alert(
        "TPS",
        fmt::format("Selected target: {} TPS", targetTPS())
    );
}

static void toggleBypass() {
    bypass = !bypass;
    accumulator = 0.0;

    alert(
        "PHYSICS BYPASS",
        bypass
            ? fmt::format("Enabled. Target: {} TPS", targetTPS())
            : "Disabled. Using normal update timing."
    );
}

static void recordInput(
    PlayerObject* player,
    PlayerButton button,
    bool pressed
) {
    if (!recording)
        return;

    auto pl = PlayLayer::get();
    if (!pl)
        return;

    bool p2 = player == pl->m_player2;

    if (player != pl->m_player1 && !p2)
        return;

    macro.push_back({
        tick,
        static_cast<int>(button),
        p2,
        pressed
    });
}

static void playbackInputs(PlayLayer* pl) {
    if (!playing || !pl)
        return;

    while (playIndex < macro.size() &&
           macro[playIndex].tick <= tick) {
        const auto& e = macro[playIndex];

        PlayerObject* player =
            e.player2 ? pl->m_player2 : pl->m_player1;

        if (player) {
            auto button = static_cast<PlayerButton>(e.button);

            if (e.pressed)
                player->pushButton(button);
            else
                player->releaseButton(button);
        }

        ++playIndex;
    }

    if (playIndex >= macro.size()) {
        playing = false;
        alert("PLAY", "Macro playback finished.");
    }
}

} // namespace Toolkit


class $modify(ToolkitPlayer, PlayerObject) {
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
        if (!Toolkit::bypass) {
            Toolkit::playbackInputs(this);

            PlayLayer::update(
                dt * Toolkit::gameSpeed()
            );

            ++Toolkit::tick;
            return;
        }

        constexpr double maxFrameTime = 0.25;
        constexpr int maxSteps = 24;

        double safeDt = std::clamp(
            static_cast<double>(dt),
            0.0,
            maxFrameTime
        );

        Toolkit::accumulator += safeDt;

        const double step =
            1.0 / static_cast<double>(Toolkit::targetTPS());

        int steps = 0;

        while (Toolkit::accumulator >= step &&
               steps < maxSteps) {
            Toolkit::playbackInputs(this);

            PlayLayer::update(
                static_cast<float>(
                    step * Toolkit::gameSpeed()
                )
            );

            Toolkit::accumulator -= step;
            ++Toolkit::tick;
            ++steps;
        }

        if (steps == maxSteps)
            Toolkit::accumulator = 0.0;
    }

    void onQuit() {
        Toolkit::recording = false;
        Toolkit::playing = false;

        if (Toolkit::noclip) {
            toggleIgnoreDamage(false);
            Toolkit::noclip = false;
        }

        PlayLayer::onQuit();
    }
};


// ============================================================
// PHYSICS TOOLKIT POPUP
// ============================================================

class PhysicsToolkitPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(360.f, 390.f))
            return false;

        setTitle("GD Physics Toolkit");

        auto size = m_mainLayer->getContentSize();
        float cx = size.width / 2.f;

        auto createPanel = [&](const char* title,
                               ccColor3B accent,
                               float y,
                               float h) {
            auto panel = CCLayerColor::create(
                {12, 18, 30, 245},
                size.width - 24.f,
                h
            );

            panel->setAnchorPoint({0.5f, 0.5f});
            panel->setPosition({cx, y});
            m_mainLayer->addChild(panel);

            auto stripe = CCLayerColor::create(
                {accent.r, accent.g, accent.b, 255},
                3.f,
                h - 12.f
            );

            stripe->setAnchorPoint({0.f, 0.5f});
            stripe->setPosition({8.f, h / 2.f});
            panel->addChild(stripe);

            auto heading = CCLabelBMFont::create(
                title,
                "goldFont.fnt"
            );

            heading->setScale(0.48f);
            heading->setColor(accent);
            heading->setAnchorPoint({0.f, 0.5f});
            heading->setPosition({18.f, h - 17.f});
            panel->addChild(heading);

            return panel;
        };

        auto addButton = [&](CCNode* parent,
                             const char* label,
                             int tag,
                             float x,
                             float y,
                             float width) {
            auto sprite = ButtonSprite::create(
                label,
                width,
                true,
                "goldFont.fnt",
                "GJ_button_01.png",
                25.f,
                0.55f
            );

            auto button = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(PhysicsToolkitPopup::onAction)
            );

            button->setTag(tag);

            auto menu = CCMenu::create();
            menu->setPosition({x, y});
            menu->addChild(button);
            parent->addChild(menu);

            return button;
        };

        // BOTTING — LIGHT BLUE
        auto botting = createPanel(
            "BOTTING",
            {120, 190, 255},
            size.height - 65.f,
            68.f
        );

        addButton(botting, "RECORD", 0, 57.f, 23.f, 76.f);
        addButton(botting, "SAVE", 1, 158.f, 23.f, 65.f);
        addButton(botting, "PLAY", 2, 257.f, 23.f, 65.f);

        // UTILS — MINT GREEN
        auto utils = createPanel(
            "UTILS",
            {120, 255, 190},
            size.height - 161.f,
            104.f
        );

        addButton(utils, "HITBOXES", 3, 75.f, 36.f, 100.f);
        addButton(utils, "NOCLIP", 4, 255.f, 36.f, 82.f);
        addButton(utils, "SPEEDHACK", 5, 165.f, 2.f, 100.f);

        // TPS — YELLOW
        auto tps = createPanel(
            "TPS",
            {255, 220, 100},
            52.f,
            96.f
        );

        addButton(tps, "FPS CHANGER", 6, 82.f, 37.f, 110.f);
        addButton(tps, "PHYSICS BYPASS", 7, 245.f, 37.f, 125.f);
        addButton(tps, "CHANGE TPS", 8, 165.f, 2.f, 105.f);

        return true;
    }

    void onAction(CCObject* sender) {
        auto node = static_cast<CCNode*>(sender);

        switch (node->getTag()) {
            case 0:
                Toolkit::record();
                break;

            case 1:
                Toolkit::save();
                break;

            case 2:
                Toolkit::playMacro();
                break;

            case 3:
                Toolkit::toggleHitboxes();
                break;

            case 4:
                Toolkit::toggleNoclip();
                break;

            case 5:
                Toolkit::changeSpeed();
                break;

            case 6:
                Toolkit::changeFPS();
                break;

            case 7:
                Toolkit::toggleBypass();
                break;

            case 8:
                Toolkit::changeTPS();
                break;
        }
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


// ============================================================
// MAIN MENU BUTTON
// ============================================================

class $modify(ToolkitMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto menu = getChildByID("bottom-menu");

        if (menu) {
            auto sprite = ButtonSprite::create(
                "Toolkit",
                80,
                true,
                "goldFont.fnt",
                "GJ_button_01.png",
                30.f,
                0.8f
            );

            auto button = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(ToolkitMenuLayer::openToolkit)
            );

            button->setID("physics-toolkit-button"_spr);

            menu->addChild(button);
            menu->updateLayout();
        }

        return true;
    }

    void openToolkit(CCObject*) {
        auto popup = PhysicsToolkitPopup::create();

        if (popup)
            popup->show();
    }
};


// ============================================================
// PAUSE MENU BUTTON
// ============================================================

class $modify(ToolkitPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto size =
            CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();

        menu->setPosition({
            size.width - 60.f,
            size.height - 90.f
        });

        auto sprite = ButtonSprite::create(
            "Toolkit",
            85,
            true,
            "goldFont.fnt",
            "GJ_button_01.png",
            30.f,
            0.7f
        );

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(ToolkitPauseLayer::openToolkit)
        );

        menu->addChild(button);
        addChild(menu);
    }

    void openToolkit(CCObject*) {
        auto popup = PhysicsToolkitPopup::create();

        if (popup)
            popup->show();
    }
};
