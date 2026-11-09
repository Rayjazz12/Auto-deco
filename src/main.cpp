#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include "ObjectString.hpp"
#include "Themes.hpp"

using namespace geode::prelude;

namespace {

// Objects created by the last theme application, tracked by unique ID so the
// "Remove theme" button can clean them up without touching the user's own
// triggers. Only valid for the current editor session.
std::unordered_set<int> g_appliedUniqueIDs;

int densityFromSettings() {
    auto value = Mod::get()->getSettingValue<std::string>("pulse-density");
    if (value == "Off") return 0;
    if (value == "Low") return 1;
    if (value == "High") return 3;
    return 2;
}

float measureLevelLength(LevelEditorLayer* editor) {
    float maxX = 0.f;
    if (editor) {
        for (auto obj : editor->m_objectVector) {
            if (obj) {
                maxX = std::max(maxX, static_cast<float>(obj->getPositionX()));
            }
        }
    }
    return std::max(maxX + 1200.f, 1500.f);
}

void notification(bool success, std::string const& text) {
    Notification::create(text, success ? NotificationIcon::Success : NotificationIcon::Warning)->show();
}

void removeAppliedTheme() {
    auto editor = LevelEditorLayer::get();
    if (!editor) {
        notification(false, "Auto Deco: open a level in the editor first");
        return;
    }

    if (g_appliedUniqueIDs.empty()) {
        notification(false, "Auto Deco: nothing tracked to remove in this session");
        return;
    }

    std::vector<GameObject*> toRemove;
    for (auto obj : editor->m_objectVector) {
        if (obj && g_appliedUniqueIDs.count(obj->m_uniqueID) > 0) {
            toRemove.push_back(obj);
        }
    }
    for (auto obj : toRemove) {
        editor->removeObject(obj, false);
    }

    int removed = static_cast<int>(toRemove.size());
    g_appliedUniqueIDs.clear();
    log::info("Auto Deco: removed {} theme objects", removed);
    notification(true, "Auto Deco: removed " + std::to_string(removed) + " theme objects");
}

void applyTheme(EditorUI* editorUI, autodeco::Theme const& theme) {
    auto editor = LevelEditorLayer::get();
    if (!editor || !editorUI) {
        notification(false, "Auto Deco: open a level in the editor first");
        return;
    }

    autodeco::ApplyOptions opts;
    opts.fadeIn = static_cast<float>(Mod::get()->getSettingValue<double>("fade-in"));
    opts.pulseDensity = densityFromSettings();
    opts.shake = Mod::get()->getSettingValue<bool>("shake") && theme.shake;
    opts.drift = Mod::get()->getSettingValue<bool>("drift");
    opts.driftGroup = static_cast<int>(Mod::get()->getSettingValue<int64_t>("drift-group"));
    opts.driftDistance = static_cast<int>(Mod::get()->getSettingValue<int64_t>("drift-distance"));
    opts.levelLength = measureLevelLength(editor);

    // Drift only makes sense if some objects were selected: they get put into
    // the drift group so the move triggers have something to move.
    int grouped = 0;
    if (opts.drift) {
        auto selected = editorUI->getSelectedObjects();
        if (selected) {
            for (auto obj : CCArrayExt<GameObject*>(selected)) {
                if (!obj) continue;
                editor->addObjectToGroup(obj, opts.driftGroup);
                ++grouped;
            }
        }
        if (grouped == 0) {
            opts.drift = false;
        }
    }

    auto objectString = autodeco::buildTriggerString(theme, opts);
    auto created = editor->createObjectsFromString(objectString, false, false);
    if (!created) {
        notification(false, "Auto Deco: the editor rejected the theme, try a shorter level section");
        return;
    }

    // Accumulate instead of clearing: applying a second theme should still be
    // removable in one go. Only "Remove theme" clears the tracking.
    std::size_t before = g_appliedUniqueIDs.size();
    for (auto obj : CCArrayExt<GameObject*>(created)) {
        if (!obj) continue;
        g_appliedUniqueIDs.insert(obj->m_uniqueID);
    }
    int count = static_cast<int>(g_appliedUniqueIDs.size() - before);

    std::string message = "Auto Deco: " + std::string(theme.name) + " applied (" +
        std::to_string(count) + " triggers)";
    if (opts.drift) {
        message += ", grouping " + std::to_string(grouped) + " objects for drift";
    }
    log::info("{}", message);
    notification(true, message);
}

} // namespace

class ThemePicker : public CCLayerColor {
protected:
    EditorUI* m_editorUI = nullptr;

    bool init(EditorUI* editorUI) {
        if (!CCLayerColor::initWithColor({0, 0, 0, 170})) return false;

        m_editorUI = editorUI;
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        this->setID("auto-deco-theme-picker"_spr);

        auto winSize = CCDirector::get()->getWinSize();
        auto center = ccp(winSize.width / 2.f, winSize.height / 2.f);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({360.f, 380.f});
        panel->setPosition(center);
        this->addChild(panel);

        auto title = CCLabelBMFont::create("Auto Deco", "goldFont.fnt");
        title->setPosition({center.x, center.y + 162.f});
        this->addChild(title);

        auto subtitle = CCLabelBMFont::create("Apply a theme to the whole level", "bigFont.fnt");
        subtitle->setScale(0.35f);
        subtitle->setPosition({center.x, center.y + 138.f});
        this->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        this->addChild(menu);

        auto const& themes = autodeco::allThemes();
        for (std::size_t i = 0; i < themes.size(); ++i) {
            auto sprite = ButtonSprite::create(themes[i].name, "bigFont.fnt", "GJ_button_01.png", 0.55f);
            auto button = CCMenuItemSpriteExtra::create(
                sprite, this, menu_selector(ThemePicker::onTheme)
            );
            button->setTag(static_cast<int>(i));

            int const column = static_cast<int>(i) % 2;
            int const row = static_cast<int>(i) / 2;
            button->setPosition({
                center.x + (column == 0 ? -88.f : 88.f),
                center.y + 104.f - static_cast<float>(row) * 52.f,
            });
            menu->addChild(button);
        }

        auto removeSprite = ButtonSprite::create("Remove theme", "bigFont.fnt", "GJ_button_04.png", 0.5f);
        auto removeButton = CCMenuItemSpriteExtra::create(
            removeSprite, this, menu_selector(ThemePicker::onRemove)
        );
        removeButton->setPosition({center.x, center.y - 116.f});
        menu->addChild(removeButton);

        auto closeSprite = ButtonSprite::create("Close", "bigFont.fnt", "GJ_button_05.png", 0.5f);
        auto closeButton = CCMenuItemSpriteExtra::create(
            closeSprite, this, menu_selector(ThemePicker::onClose)
        );
        closeButton->setPosition({center.x, center.y - 158.f});
        menu->addChild(closeButton);

        return true;
    }

    void onTheme(CCObject* sender) {
        auto const& themes = autodeco::allThemes();
        int const index = sender ? sender->getTag() : -1;
        if (index < 0 || static_cast<std::size_t>(index) >= themes.size()) return;
        if (m_editorUI) {
            applyTheme(m_editorUI, themes[index]);
        }
        this->removeFromParent();
    }

    void onRemove(CCObject*) {
        removeAppliedTheme();
        this->removeFromParent();
    }

    void onClose(CCObject*) {
        this->removeFromParent();
    }

    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override {
        return true; // swallow clicks so the editor underneath is not used
    }

    void keyBackClicked() override {
        this->removeFromParent();
    }

public:
    static ThemePicker* create(EditorUI* editorUI) {
        auto picker = new ThemePicker();
        if (picker && picker->init(editorUI)) {
            picker->autorelease();
            return picker;
        }
        CC_SAFE_DELETE(picker);
        return nullptr;
    }
};

class $modify(AutoDecoEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        auto winSize = CCDirector::get()->getWinSize();

        auto sprite = ButtonSprite::create("Auto Deco", "bigFont.fnt", "GJ_button_01.png", 0.5f);
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(AutoDecoEditorUI::onOpenPicker)
        );
        button->setID("auto-deco-button"_spr);

        auto menu = CCMenu::create();
        menu->setID("auto-deco-menu"_spr);
        menu->addChild(button);
        menu->setPosition({44.f, winSize.height * 0.5f});
        this->addChild(menu, 500);

        return true;
    }

    void onOpenPicker(CCObject*) {
        if (this->getChildByID("auto-deco-theme-picker"_spr)) {
            return;
        }
        if (auto picker = ThemePicker::create(this)) {
            this->addChild(picker, 1000);
        }
    }
};
