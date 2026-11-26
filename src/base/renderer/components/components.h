#pragma once



#include "abstract_component.h"

#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <cstdio>
#include <algorithm>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

#include <Windows.h>
inline bool g_current_module_enabled = false;




namespace CompStyle
{
    constexpr float TEXT_OFFSET_X = 12.0f;




    constexpr ImU32 NAME_OFF  = IM_COL32( 60,  60,  76, 255);
    constexpr ImU32 NAME_ON   = IM_COL32(255, 255, 255, 255);

    inline ImU32 Accent()
    {
        const ImVec4& c = ImGui::GetStyle().Colors[ImGuiCol_Button];
        return IM_COL32((int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255), 255);
    }
}




class OvalButton : public abstract_component
{
public:
    OvalButton(std::string name, bool* value,
               float width = 48.0f, float height = 24.0f);

    std::string serialize() const override;
    void deserialize(const std::string& data) override;

    void render_in_settings_panel() override;
    void render_in_tile_module()    override;

    bool  wants_tile_render()  const override { return true;  }
    bool  wants_panel_render() const override { return true;  }
    float get_height() const override { return mHeight + 6.0f; }

private:

    void render_impl(bool enabled_tint);

    std::string mName;
    bool*       mValue;
    float       mWidth;
    float       mHeight;
    float       mAnim = 0.0f;
};




template <typename T>
class SliderComponent : public abstract_component
{
public:
    SliderComponent(std::string name, T* value, T min, T max, T step,
                    float width = 275.0f)
        : abstract_component(component_type::slider),
          mName(std::move(name)), mValue(value),
          mMin(min), mMax(max), mStep(step), mWidth(width) {}

    std::string serialize() const override;
    void deserialize(const std::string& data) override;

    void render_in_settings_panel() override;
    void render_in_tile_module()    override;

    float get_height() const override { return ImGui::GetTextLineHeight() + 40.0f; }

private:
    std::string mName;
    T* mValue;
    T  mMin, mMax, mStep;
    float mWidth;
};




template <typename T>
class DropdownComponent : public abstract_component
{
public:
    DropdownComponent(std::string name, T* value,
                      std::vector<std::pair<std::string, T>> options,
                      float width = 250.0f)
        : abstract_component(component_type::dropdown),
          mName(std::move(name)), mValue(value),
          mOptions(std::move(options)), mWidth(width) {}

    std::string serialize() const override;
    void deserialize(const std::string& data) override;

    void render_in_settings_panel() override;
    void render_in_tile_module()    override;

    float get_height() const override { return ImGui::GetTextLineHeight() + 60.0f; }

private:
    std::string mName;
    T* mValue;
    std::vector<std::pair<std::string, T>> mOptions;
    float mWidth;


    bool mOpen = false;
};




template <typename T>
class WhitelistDropdownComponent : public abstract_component
{
public:
    WhitelistDropdownComponent(
        std::string name,
        std::vector<T>* value,
        std::vector<std::pair<std::string, T>> options,
        float width = 250.0f)
        : abstract_component(component_type::dropdown),
          mName(std::move(name)),
          mValue(value),
          mOptions(std::move(options)),
          mWidth(width) {}

    std::string serialize() const override
    {
        if (!mValue) return {};
        std::string out;
        for (size_t i = 0; i < mValue->size(); ++i)
        {
            if (i) out += ",";
            out += std::to_string(static_cast<int>((*mValue)[i]));
        }
        return out;
    }

    void deserialize(const std::string& data) override
    {
        if (!mValue) return;
        mValue->clear();
        if (data.empty()) return;

        size_t start = 0;
        while (start <= data.size())
        {
            const size_t comma = data.find(',', start);
            const std::string part = data.substr(
                start, comma == std::string::npos ? std::string::npos
                                                  : comma - start);
            if (!part.empty())
            {
                try { mValue->push_back(static_cast<T>(std::stoi(part))); }
                catch (...) {}
            }
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
    }

    void render_in_settings_panel() override;
    void render_in_tile_module()    override {}

    float get_height() const override
    {
        return ImGui::GetTextLineHeight() + 60.0f;
    }

private:
    std::string mName;
    std::vector<T>* mValue;
    std::vector<std::pair<std::string, T>> mOptions;
    float mWidth;
    bool  mOpen = false;
};






class ColorPickerComponent : public abstract_component
{
public:
    ColorPickerComponent(std::string name, float* color,
                         bool alpha = false, float width = 275.0f);

    std::string serialize() const override;
    void deserialize(const std::string& data) override;

    void render_in_settings_panel() override;
    void render_in_tile_module()    override;

    float get_height() const override { return ImGui::GetTextLineHeight() + 40.0f; }

private:
    void render_popup(const char* popup_id);   
    std::string mName;
    float*      mColor;
    bool        mAlpha;
    float       mWidth;
    float       mHue = 0.0f;

    bool  mPopupOpen   = false;
    char  mHexBuf[10]  = {};

    float mSat = 1.0f;
    float mVal = 1.0f;

};




class KeybindComponent : public abstract_component
{
public:
    KeybindComponent(std::string name, int* value,
                     std::function<void()> onPressed = nullptr,
                     float width = 275.0f);

    ~KeybindComponent() override;

    std::string serialize() const override;
    void deserialize(const std::string& data) override;

    void render_in_settings_panel() override;
    void render_in_tile_module()    override;

    bool  wants_tile_render()  const override { return true;  }
    bool  wants_panel_render() const override { return false; }

    float get_height() const override { return 32.0f + 6.0f; }

    static void PollAllHotkeys();

private:
    static std::vector<KeybindComponent*>& Registry();

    void armCapture();
    void pollCapture();
    void pollHotkey();

    std::string mName;
    int*        mValue;
    std::function<void()> mOnPressed;
    float       mWidth;

    bool mCapturing = false;
    bool mPrevKeyDown[256] = {};
    bool mHotkeyWasDown    = false;
    float mAnim = 0.0f;
};




class ButtonComponent : public abstract_component
{
public:
    ButtonComponent(std::string name, std::function<void()> callback,
                    float width = 275.0f, float height = 30.0f);

    void render_in_settings_panel() override;
    void render_in_tile_module()    override;

    float get_height() const override { return mHeight + 6.0f; }

private:
    std::string mName;
    std::function<void()> mCallback;
    float mWidth, mHeight;
};






template <typename T>
class ChooseButtonsComponent : public abstract_component
{
public:
    ChooseButtonsComponent(std::string name, T* value,
                           std::vector<std::pair<std::string, T>> options,
                           float buttonWidth = 60.0f)
        : abstract_component(component_type::choose_buttons),
          mName(std::move(name)), mValue(value),
          mOptions(std::move(options)), mButtonWidth(buttonWidth) {}

    std::string serialize() const override
    {
        if (!mValue) return {};
        for (auto& o : mOptions)
            if (o.second == *mValue) return o.first;
        return {};
    }

    void deserialize(const std::string& data) override
    {
        if (!mValue || data.empty()) return;
        for (auto& o : mOptions)
            if (o.first == data) { *mValue = o.second; return; }
    }

    void render_in_settings_panel() override { render_impl(); }
    void render_in_tile_module()    override {  }

    float get_height() const override { return ImGui::GetTextLineHeight() + 8.0f; }

private:
      void render_impl()
    {
        ImGui::PushID(this);

        const float startX = ImGui::GetCursorScreenPos().x;
        const float startY = ImGui::GetCursorScreenPos().y;
        const float rowH   = 26.0f;


        ImDrawList* dl = ImGui::GetWindowDrawList();

        const bool modOn = g_current_module_enabled;
        const ImU32 nameCol = modOn ? CompStyle::NAME_ON : CompStyle::NAME_OFF;

        dl->AddText(ImGui::GetFont(),
                    ImGui::GetFontSize() * 1.05f,
                    ImVec2(startX + CompStyle::TEXT_OFFSET_X, startY),
                    nameCol,
                    mName.c_str());


        const float buttonsTotalW =
            mOptions.size() * mButtonWidth + (mOptions.size() - 1) * 4.0f;

        const float availW = ImGui::GetContentRegionAvail().x;
        float bx = startX + availW - buttonsTotalW - 10.0f;
        const float by = startY;

        const ImU32 accent = CompStyle::Accent();


        constexpr ImU32 kBaseBg   = IM_COL32(25, 25, 30, 255);
        constexpr ImU32 kHoverBg  = IM_COL32(40, 40, 48, 255);

        const float dt = ImGui::GetIO().DeltaTime;
        const float k_anim = 1.0f - std::exp(-16.0f * dt);


        if (mButtonAnim.size() != mOptions.size())
            mButtonAnim.assign(mOptions.size(), 0.0f);

        for (size_t i = 0; i < mOptions.size(); ++i)
        {
            const bool selected = (*mValue == mOptions[i].second);

            const ImVec2 p0(bx, by);
            const ImVec2 p1(bx + mButtonWidth, by + rowH);

            ImGui::SetCursorScreenPos(p0);
            ImGui::PushID((int)i);
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0));
            ImGui::InvisibleButton("##cb", ImVec2(mButtonWidth, rowH));
            const bool hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked())
                *mValue = mOptions[i].second;
            ImGui::PopStyleColor(3);
            ImGui::PopID();




            float& anim = mButtonAnim[i];
            const float target = selected ? 1.0f : 0.0f;
            anim += (target - anim) * k_anim;
            if (std::fabs(target - anim) < 0.001f)
                anim = target;


            ImVec4 bg_f = ImGui::ColorConvertU32ToFloat4(kBaseBg);
            if (hovered && !selected)
                bg_f = ImGui::ColorConvertU32ToFloat4(kHoverBg);
            else if (hovered && selected)
                bg_f = ImGui::ColorConvertU32ToFloat4(kHoverBg);

            const ImVec4 accent_f = ImGui::ColorConvertU32ToFloat4(accent);

            const ImVec4 mixed = ImLerp(bg_f, accent_f, anim);

            const ImU32 bg = ImGui::ColorConvertFloat4ToU32(mixed);

            dl->AddRectFilled(p0, p1, bg, 5.0f);


            const ImU32 borderCol = IM_COL32(50, 50, 58, 255);
            dl->AddRect(p0, p1, borderCol, 5.0f, 0, 1.0f);




            const std::string& label = mOptions[i].first;
            const ImVec2 ts = ImGui::CalcTextSize(label.c_str());


            const ImVec4 t_off_f = ImGui::ColorConvertU32ToFloat4(
                IM_COL32(160, 160, 165, 255));
            const ImVec4 t_on_f  = ImGui::ColorConvertU32ToFloat4(
                IM_COL32(255, 255, 255, 255));

            const ImVec4 t_mix = ImLerp(t_off_f, t_on_f, anim);
            const ImU32 tcol = ImGui::ColorConvertFloat4ToU32(t_mix);

            dl->AddText(ImVec2(p0.x + (mButtonWidth - ts.x) * 0.5f,
                               p0.y + (rowH - ts.y) * 0.5f),
                        tcol, label.c_str());

            bx += mButtonWidth + 4.0f;
        }

        ImGui::SetCursorScreenPos(ImVec2(startX, startY));
        ImGui::Dummy(ImVec2(1.0f, rowH + 6.0f));

        ImGui::PopID();
    }

    std::vector<float> mButtonAnim;

    std::string mName;
    T* mValue;
    std::vector<std::pair<std::string, T>> mOptions;
    float mButtonWidth;
};

class ThemeSelectorComponent : public abstract_component
{
public:
    struct Entry {
        ImU32       color;
        const char* name;
    };

    ThemeSelectorComponent(std::string name, int* selected,
                           const Entry* entries, int count,
                           float width = 275.0f);

    void render_in_settings_panel() override;
    void render_in_tile_module()    override {}

    float get_height() const override;

private:
    std::string       mName;
    int*              mSelected;
    const Entry*      mEntries;
    int               mCount;
    float             mWidth;
};
class ConditionalComponent : public abstract_component
{
public:
    ConditionalComponent(bool* condition, abstract_component* child)
        : abstract_component(component_type::toggle_button),
          mCondition(condition), mChild(child) {}

    ~ConditionalComponent() override { delete mChild; }

    void render_in_settings_panel() override {
        if (mCondition && *mCondition && mChild)
            mChild->render_in_settings_panel();
    }

    void render_in_tile_module() override {}

    float get_height() const override {
        return (mCondition && *mCondition && mChild) ? mChild->get_height() : 0.0f;
    }

    bool wants_panel_render() const override {
        return mCondition && *mCondition && mChild && mChild->wants_panel_render();
    }

private:
    bool*               mCondition;
    abstract_component* mChild;
};

template class SliderComponent<double>;