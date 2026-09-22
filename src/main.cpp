#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

// 전역 또는 인스턴스 공유를 위한 카운터 상태 관리
namespace CounterState {
    inline int groundTicks = 0;
    inline bool wasOnGround = false;

    inline int c9_12 = 0;
    inline int c7_8  = 0;
    inline int c5_6  = 0;
    inline int c4    = 0;
    inline int c3    = 0;
    inline int c2    = 0;
    inline int c1    = 0;

    inline CCLabelBMFont* lbl9_12 = nullptr;
    inline CCLabelBMFont* lbl7_8  = nullptr;
    inline CCLabelBMFont* lbl5_6  = nullptr;
    inline CCLabelBMFont* lbl4    = nullptr;
    inline CCLabelBMFont* lbl3    = nullptr;
    inline CCLabelBMFont* lbl2    = nullptr;
    inline CCLabelBMFont* lbl1    = nullptr;

    inline void renderCounts() {
        auto updateText = [](CCLabelBMFont* lbl, const char* title, int count) {
            if (lbl) {
                lbl->setString(fmt::format("{}: {}", title, count).c_str());
            }
        };

        updateText(lbl9_12, "9-12", c9_12);
        updateText(lbl7_8,  "7-8",  c7_8);
        updateText(lbl5_6,  "5-6",  c5_6);
        updateText(lbl4,    "4",    c4);
        updateText(lbl3,    "3",    c3);
        updateText(lbl2,    "2",    c2);
        updateText(lbl1,    "1",    c1);
    }

    inline void playHitSound(float pitch) {
        auto fmod = FMODAudioEngine::sharedEngine();
        if (fmod) {
            fmod->playEffect("hit01.ogg", pitch, 0.0f, 0.7f);
        }
    }
}

// 1. 화면 라벨 생성 및 매 프레임 착지 틱 업데이트 (PlayLayer)
class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float startX = 14.0f;
        float startY = winSize.height - 18.0f;
        float spacing = 18.0f;

        auto makeRow = [&](const char* title, ccColor3B color, float yOffset) {
            auto label = CCLabelBMFont::create(fmt::format("{}: 0", title).c_str(), "bigFont.fnt");
            label->setScale(0.42f);
            label->setAnchorPoint({0.0f, 0.5f});
            label->setColor(color);
            label->setPosition({startX, startY - yOffset});
            label->setZOrder(999);
            this->addChild(label);
            return label;
        };

        // NaN GD 스타일 카운터 라벨 배치
        CounterState::lbl9_12 = makeRow("9-12", {70, 110, 255}, spacing * 0);
        CounterState::lbl7_8  = makeRow("7-8",  {70, 180, 255}, spacing * 1);
        CounterState::lbl5_6  = makeRow("5-6",  {90, 255, 120}, spacing * 2);
        CounterState::lbl4    = makeRow("4",    {255, 255, 255}, spacing * 3);
        CounterState::lbl3    = makeRow("3",    {255, 245, 110}, spacing * 4);
        CounterState::lbl2    = makeRow("2",    {255, 140, 50},  spacing * 5);
        CounterState::lbl1    = makeRow("1",    {255, 70, 70},   spacing * 6);

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_player1) return;

        if (m_player1->m_isOnGround) {
            CounterState::groundTicks++;
            CounterState::wasOnGround = true;
        } else {
            CounterState::groundTicks = 0;
            CounterState::wasOnGround = false;
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        CounterState::c9_12 = 0;
        CounterState::c7_8  = 0;
        CounterState::c5_6  = 0;
        CounterState::c4    = 0;
        CounterState::c3    = 0;
        CounterState::c2    = 0;
        CounterState::c1    = 0;
        CounterState::groundTicks = 0;
        CounterState::wasOnGround = false;

        CounterState::renderCounts();
    }
};

// 2. 점프 입력 후킹 및 틱 판정 (GJBaseGameLayer)
class $modify(MyBaseGameLayer, GJBaseGameLayer) {
    void pushButton(PlayerButton playerButton, bool isPlayer2) {
        GJBaseGameLayer::pushButton(playerButton, isPlayer2);

        // 점프 버튼이 아니거나 플레이어 2 입력이면 무시
        if (playerButton != PlayerButton::Jump || isPlayer2) return;

        if (CounterState::wasOnGround && CounterState::groundTicks > 0) {
            int ticks = CounterState::groundTicks;

            if (ticks == 1) {
                CounterState::c1++;
                CounterState::playHitSound(1.6f);
            } else if (ticks == 2) {
                CounterState::c2++;
                CounterState::playHitSound(1.4f);
            } else if (ticks == 3) {
                CounterState::c3++;
                CounterState::playHitSound(1.25f);
            } else if (ticks == 4) {
                CounterState::c4++;
                CounterState::playHitSound(1.1f);
            } else if (ticks >= 5 && ticks <= 6) {
                CounterState::c5_6++;
                CounterState::playHitSound(0.95f);
            } else if (ticks >= 7 && ticks <= 8) {
                CounterState::c7_8++;
                CounterState::playHitSound(0.85f);
            } else if (ticks >= 9 && ticks <= 12) {
                CounterState::c9_12++;
                CounterState::playHitSound(0.75f);
            }

            CounterState::renderCounts();
        }
    }
};
