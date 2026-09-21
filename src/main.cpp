#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        // 좌측 상단 프레임 윈도우 라벨 목록
        CCLabelBMFont* m_lbl9_12 = nullptr;
        CCLabelBMFont* m_lbl7_8  = nullptr;
        CCLabelBMFont* m_lbl5_6  = nullptr;
        CCLabelBMFont* m_lbl4    = nullptr; // 60Hz 1FP 판정 (4틱)
        CCLabelBMFont* m_lbl3    = nullptr;
        CCLabelBMFont* m_lbl2    = nullptr;
        CCLabelBMFont* m_lbl1    = nullptr;

        // 각 구간별 성공 카운트
        int m_c9_12 = 0;
        int m_c7_8  = 0;
        int m_c5_6  = 0;
        int m_c4    = 0;
        int m_c3    = 0;
        int m_c2    = 0;
        int m_c1    = 0;

        int m_groundTicks = 0;
        bool m_wasOnGround = false;
    };

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

        // NaN GD 영상 색상 구성
        m_fields->m_lbl9_12 = makeRow("9-12", {70, 110, 255}, spacing * 0);
        m_fields->m_lbl7_8  = makeRow("7-8",  {70, 180, 255}, spacing * 1);
        m_fields->m_lbl5_6  = makeRow("5-6",  {90, 255, 120}, spacing * 2);
        m_fields->m_lbl4    = makeRow("4",    {255, 255, 255}, spacing * 3); // 60fps 판정 강조
        m_fields->m_lbl3    = makeRow("3",    {255, 245, 110}, spacing * 4);
        m_fields->m_lbl2    = makeRow("2",    {255, 140, 50},  spacing * 5);
        m_fields->m_lbl1    = makeRow("1",    {255, 70, 70},   spacing * 6);

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_player1) return;

        if (m_player1->m_isOnGround) {
            m_fields->m_groundTicks++;
            m_fields->m_wasOnGround = true;
        } else {
            m_fields->m_groundTicks = 0;
            m_fields->m_wasOnGround = false;
        }
    }

    // 판정 적중 시 프레임 난이도별 히트사운드 재생 (FMOD)
    void playHitSound(float pitch) {
        auto fmod = FMODAudioEngine::sharedEngine();
        if (fmod) {
            // 게임 기본 클릭/비프 사운드 재생 (피치 및 볼륨 조절)
            // 틱이 낮을수록 높은 톤의 날카로운 비프음 발생
            fmod->playEffect("hit01.ogg", pitch, 0.0f, 0.7f);
        }
    }

    void pushButton(int playerButton, bool isPlayer2) {
        PlayLayer::pushButton(playerButton, isPlayer2);

        if (isPlayer2 || !m_player1) return;

        if (m_fields->m_wasOnGround && m_fields->m_groundTicks > 0) {
            int ticks = m_fields->m_groundTicks;

            if (ticks == 1) {
                m_fields->m_c1++;
                playHitSound(1.6f); // 240Hz 1FP: 가장 높은 톤
            } else if (ticks == 2) {
                m_fields->m_c2++;
                playHitSound(1.4f);
            } else if (ticks == 3) {
                m_fields->m_c3++;
                playHitSound(1.25f);
            } else if (ticks == 4) {
                // 60Hz 1FP 판정 (4틱)
                m_fields->m_c4++;
                playHitSound(1.1f); // 60Hz 정타 사운드
            } else if (ticks >= 5 && ticks <= 6) {
                m_fields->m_c5_6++;
                playHitSound(0.95f);
            } else if (ticks >= 7 && ticks <= 8) {
                m_fields->m_c7_8++;
                playHitSound(0.85f);
            } else if (ticks >= 9 && ticks <= 12) {
                m_fields->m_c9_12++;
                playHitSound(0.75f);
            }

            renderCounts();
        }
    }

    void renderCounts() {
        auto updateText = [](CCLabelBMFont* lbl, const char* title, int count) {
            if (lbl) {
                lbl->setString(fmt::format("{}: {}", title, count).c_str());
            }
        };

        updateText(m_fields->m_lbl9_12, "9-12", m_fields->m_c9_12);
        updateText(m_fields->m_lbl7_8,  "7-8",  m_fields->m_c7_8);
        updateText(m_fields->m_lbl5_6,  "5-6",  m_fields->m_c5_6);
        updateText(m_fields->m_lbl4,    "4",    m_fields->m_c4);
        updateText(m_fields->m_lbl3,    "3",    m_fields->m_c3);
        updateText(m_fields->m_lbl2,    "2",    m_fields->m_c2);
        updateText(m_fields->m_lbl1,    "1",    m_fields->m_c1);
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        m_fields->m_c9_12 = 0;
        m_fields->m_c7_8  = 0;
        m_fields->m_c5_6  = 0;
        m_fields->m_c4    = 0;
        m_fields->m_c3    = 0;
        m_fields->m_c2    = 0;
        m_fields->m_c1    = 0;

        m_fields->m_groundTicks = 0;
        m_fields->m_wasOnGround = false;

        renderCounts();
    }
};
