#include "components.h"

#include <windows.h>




namespace CompStyle
{
    inline std::string KeyNameForVK(int vk)
    {
        if (vk <= 0) return {};

        switch (vk)
        {
            case VK_LBUTTON:  return "Mouse Left";
            case VK_RBUTTON:  return "Mouse Right";
            case VK_MBUTTON:  return "Mouse Middle";
            case VK_XBUTTON1: return "Mouse 4";
            case VK_XBUTTON2: return "Mouse 5";
            default: break;
        }

        UINT scanCode = MapVirtualKeyA((UINT)vk, MAPVK_VK_TO_VSC);
        switch (vk)
        {
            case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
            case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
            case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK:
                scanCode |= 0x100;
            default: break;
        }

        const LONG lp = (LONG)(scanCode << 16);
        char name[64] = {};
        if (GetKeyNameTextA(lp, name, sizeof(name)) > 0)
            return std::string(name);
        return "Unknown";
    }
}




namespace
{
    inline void DrawComponentName(const char* name, float x, float y, ImU32 col)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddText(ImGui::GetFont(),
                    ImGui::GetFontSize() * 1.05f,
                    ImVec2(x, y),
                    col,
                    name);
    }
}




OvalButton::OvalButton(std::string name, bool* value,
                       float width, float height)
    : abstract_component(component_type::toggle_button),
      mName(std::move(name)), mValue(value),
      mWidth(width), mHeight(height) {}

std::string OvalButton::serialize() const
{
    return (mValue && *mValue) ? "true" : "false";
}

void OvalButton::deserialize(const std::string& data)
{
    if (mValue) *mValue = (data == "true" || data == "1");
}

void OvalButton::render_in_settings_panel() { render_impl(true); }
void OvalButton::render_in_tile_module()    {  }

void OvalButton::render_impl(bool )
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;

    const ImU32 nameCol = *mValue ? CompStyle::NAME_ON : CompStyle::NAME_OFF;


    DrawComponentName(mName.c_str(), startX + CompStyle::TEXT_OFFSET_X, startY, nameCol);


    const float availW = ImGui::GetContentRegionAvail().x;
    const float ovalX  = startX + availW - mWidth - 10.0f;
    const float ovalY  = startY + (ImGui::GetTextLineHeight() - mHeight) * 0.5f;

    ImGui::SetCursorScreenPos(ImVec2(ovalX, ovalY));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0));
    ImGui::InvisibleButton("##oval", ImVec2(mWidth, mHeight));
    if (ImGui::IsItemClicked() && mValue)
        *mValue = !*mValue;
    ImGui::PopStyleColor(3);

    const float target = (mValue && *mValue) ? 1.0f : 0.0f;
    mAnim += (target - mAnim) * 0.18f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float radius = mHeight * 0.5f;
    const float knobR  = radius - 2.0f;
    const ImU32 accent = CompStyle::Accent();
    const ImU32 offColor = IM_COL32(40, 40, 45, 255);

    const ImU32 trackColor = ImGui::GetColorU32(
        ImLerp(ImGui::ColorConvertU32ToFloat4(offColor),
               ImGui::ColorConvertU32ToFloat4(accent), mAnim));

    dl->AddRectFilled(ImVec2(ovalX, ovalY),
                      ImVec2(ovalX + mWidth, ovalY + mHeight),
                      trackColor, radius);

    const float kxA = ovalX + radius;
    const float kxB = ovalX + mWidth - radius;
    const float kx  = kxA + (kxB - kxA) * mAnim;
    dl->AddCircleFilled(ImVec2(kx, ovalY + radius), knobR,
                        IM_COL32(255, 255, 255, 255));


    ImGui::SetCursorScreenPos(ImVec2(startX, startY + mHeight + 4.0f));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));
    ImGui::PopID();
}




template <typename T>
std::string SliderComponent<T>::serialize() const
{
    if (!mValue) return "0";
    return std::to_string(*mValue);
}

template <typename T>
void SliderComponent<T>::deserialize(const std::string& data)
{
    if (!mValue || data.empty()) return;
    try
    {
        if constexpr (std::is_floating_point_v<T>)
            *mValue = static_cast<T>(std::stod(data));
        else
            *mValue = static_cast<T>(std::stoll(data));
    }
    catch (...) {}
}
template <typename T>
void SliderComponent<T>::render_in_settings_panel()
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;
    const float titleH = ImGui::GetTextLineHeight();
    const float offX   = CompStyle::TEXT_OFFSET_X;

    const ImU32 nameCol = g_current_module_enabled
        ? CompStyle::NAME_ON : CompStyle::NAME_OFF;


    DrawComponentName(mName.c_str(), startX + offX, startY, nameCol);


    char buf[64];
    if constexpr (std::is_same_v<T, int>)
        snprintf(buf, sizeof(buf), "%d", *mValue);
    else
        snprintf(buf, sizeof(buf), "%.1f", static_cast<float>(*mValue));

    const float availW = ImGui::GetContentRegionAvail().x;
    const float valW   = ImGui::CalcTextSize(buf).x;
    const float valX   = startX + availW - valW - 10.0f;

    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddText(ImVec2(valX, startY),
                    IM_COL32(150, 150, 155, 255), buf);
    }


    const float margin = 12.0f;
    const float panelW = availW - offX - 8.0f;
    const float trackW = panelW - margin * 2.0f;

    ImGui::SetCursorScreenPos(ImVec2(startX + offX, startY + titleH + 5.0f));

    const float posX   = ImGui::GetCursorScreenPos().x + margin;
    const float posY   = ImGui::GetCursorScreenPos().y;
    const float frameH = ImGui::GetFrameHeight();
    const float barH   = frameH * 0.20f;
    const float barY   = posY + (frameH - barH) * 0.5f;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0));
    ImGui::InvisibleButton("##slider", ImVec2(trackW, frameH));
    ImGui::PopStyleColor(3);


    if (ImGui::IsItemActive())
    {
        float t = ImClamp((ImGui::GetMousePos().x - posX) / trackW, 0.0f, 1.0f);
        T nv = static_cast<T>(mMin + (mMax - mMin) * t);
        if (mStep > T(0))
            nv = static_cast<T>(mMin + std::round(
                static_cast<float>(nv - mMin) / static_cast<float>(mStep)) * mStep);
        *mValue = ImClamp(nv, mMin, mMax);
    }

    const float t     = ImClamp(static_cast<float>(*mValue - mMin) /
                                static_cast<float>(mMax - mMin), 0.0f, 1.0f);
    const float knobX = posX + t * trackW;
    const ImU32 acA   = CompStyle::Accent();

    dl->AddRectFilled(ImVec2(posX, barY),
                      ImVec2(posX + trackW, barY + barH),
                      IM_COL32(40, 40, 45, 180), barH * 0.5f);

    if (knobX > posX)
        dl->AddRectFilled(ImVec2(posX, barY),
                          ImVec2(knobX, barY + barH),
                          acA, barH * 0.5f);

    dl->AddCircleFilled(ImVec2(knobX, barY + barH * 0.5f),
                        barH * 1.4f, IM_COL32(255, 255, 255, 255));


    ImGui::SetCursorScreenPos(ImVec2(startX, startY + titleH + 5.0f + frameH + 4.0f));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));
    ImGui::PopID();
}

template <typename T>
void SliderComponent<T>::render_in_tile_module() {}




template <typename T>
std::string DropdownComponent<T>::serialize() const
{
    if (!mValue) return "0";
    return std::to_string(static_cast<int>(*mValue));
}

template <typename T>
void DropdownComponent<T>::deserialize(const std::string& data)
{
    if (!mValue || data.empty()) return;
    try { *mValue = static_cast<T>(std::stoi(data)); } catch (...) {}
}
template <typename T>
void DropdownComponent<T>::render_in_settings_panel()
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;
    const float offX   = CompStyle::TEXT_OFFSET_X;


    int currentIndex = 0;
    for (size_t i = 0; i < mOptions.size(); ++i)
        if (mOptions[i].second == *mValue) { currentIndex = (int)i; break; }


    const ImU32 nameCol = g_current_module_enabled
        ? CompStyle::NAME_ON : CompStyle::NAME_OFF;
    DrawComponentName(mName.c_str(), startX + offX, startY, nameCol);


    const float labelH = ImGui::GetTextLineHeight();
    const float rowH   = 32.0f;
    const float itemH  = 28.0f;


    const float availW = ImGui::GetContentRegionAvail().x - offX - 8.0f;
    const float width  = availW;

    const ImVec2 rowPos(startX + offX, startY + labelH + 6.0f);
    const ImVec2 rowEnd(rowPos.x + width, rowPos.y + rowH);

    const ImVec2 mouse = ImGui::GetMousePos();
    ImDrawList* dl = ImGui::GetWindowDrawList();




    ImGui::SetCursorScreenPos(rowPos);
    ImGui::InvisibleButton("##dd_header", ImVec2(width, rowH));
    const bool headerHovered = ImGui::IsItemHovered();
    const bool headerClicked = ImGui::IsItemClicked();

    constexpr ImU32 kBaseBg  = IM_COL32(25, 25, 30, 255);
    constexpr ImU32 kHoverBg = IM_COL32(32, 32, 38, 255);
    constexpr ImU32 kBorder  = IM_COL32(40, 40, 48, 255);

    const ImU32 headerBg = (headerHovered || mOpen) ? kHoverBg : kBaseBg;
    dl->AddRectFilled(rowPos, rowEnd, headerBg, 6.0f);
    dl->AddRect(rowPos, rowEnd, kBorder, 6.0f, 0, 1.0f);


    const std::string label = mOptions.empty() ? std::string("-")
                                               : mOptions[currentIndex].first;
    const ImVec2 ts = ImGui::CalcTextSize(label.c_str());
    dl->AddText(ImVec2(rowPos.x + (width - ts.x) * 0.5f,
                       rowPos.y + (rowH - ts.y) * 0.5f),
                IM_COL32(230, 230, 235, 255), label.c_str());


    {
        const float arrow_size = 5.0f;
        const float pad_right  = 12.0f;
        const float cx = rowEnd.x - pad_right - arrow_size;
        const float cy = (rowPos.y + rowEnd.y) * 0.5f;

        const float dir = mOpen ? -1.0f : 1.0f;
        const ImU32 arrowCol = headerHovered
            ? IM_COL32(255, 255, 255, 255)
            : IM_COL32(140, 140, 150, 255);

        dl->AddTriangleFilled(
            ImVec2(cx - arrow_size, cy - arrow_size * 0.3f * dir),
            ImVec2(cx + arrow_size, cy - arrow_size * 0.3f * dir),
            ImVec2(cx,              cy + arrow_size * 0.7f * dir),
            arrowCol);
    }

    if (headerClicked)
        mOpen = !mOpen;




    int clickedItem = -1;

    if (mOpen && !mOptions.empty())
    {
        const float listY = rowEnd.y + 4.0f;

        for (size_t i = 0; i < mOptions.size(); ++i)
        {
            const ImVec2 p0(rowPos.x, listY + itemH * (float)i);
            const ImVec2 p1(rowEnd.x, p0.y + itemH);

            const bool itemHovered =
                mouse.x >= p0.x && mouse.x <= p1.x &&
                mouse.y >= p0.y && mouse.y <= p1.y;

            ImGui::SetCursorScreenPos(p0);
            ImGui::PushID((int)i);
            ImGui::InvisibleButton("##dd_item", ImVec2(width, itemH));
            const bool itemClicked = ImGui::IsItemClicked();
            ImGui::PopID();

            if (itemClicked)
                clickedItem = (int)i;

            const ImU32 itemBg = itemHovered
                ? IM_COL32(36, 36, 42, 255)
                : kBaseBg;
            dl->AddRectFilled(p0, p1, itemBg, 4.0f);

            if (i > 0)
                dl->AddLine(ImVec2(p0.x + 6.0f, p0.y),
                            ImVec2(p1.x - 6.0f, p0.y),
                            IM_COL32(30, 30, 36, 255), 1.0f);

            const std::string& itemLabel = mOptions[i].first;
            const ImVec2 its = ImGui::CalcTextSize(itemLabel.c_str());

            const ImU32 textCol = itemHovered
                ? IM_COL32(255, 255, 255, 255)
                : IM_COL32(58, 58, 73, 255);

            dl->AddText(ImVec2(p0.x + 10.0f,
                               p0.y + (itemH - its.y) * 0.5f),
                        textCol, itemLabel.c_str());
        }
    }




    if (clickedItem >= 0 && clickedItem < (int)mOptions.size())
    {
        *mValue = mOptions[clickedItem].second;
        mOpen = false;
    }




    float advance = labelH + 6.0f + rowH + 6.0f;
    if (mOpen && !mOptions.empty())
        advance += itemH * (float)mOptions.size() + 4.0f;

    ImGui::SetCursorScreenPos(ImVec2(startX, startY + advance));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));

    ImGui::PopID();
}

template <typename T>
void DropdownComponent<T>::render_in_tile_module() {}

ColorPickerComponent::ColorPickerComponent(std::string name, float* color,
                                           bool alpha, float width)
    : abstract_component(component_type::color_picker),
      mName(std::move(name)), mColor(color), mAlpha(alpha), mWidth(width)
{
    float s, v;
    ImGui::ColorConvertRGBtoHSV(mColor[0], mColor[1], mColor[2], mHue, s, v);
}

std::string ColorPickerComponent::serialize() const
{
    if (!mColor) return "1.0,1.0,1.0,1.0";
    char buf[128];
    snprintf(buf, sizeof(buf), "%.3f,%.3f,%.3f,%.3f",
             mColor[0], mColor[1], mColor[2], mAlpha ? mColor[3] : 1.0f);
    return buf;
}

void ColorPickerComponent::deserialize(const std::string& data)
{
    if (!mColor || data.empty()) return;
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    if (sscanf(data.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a) >= 3)
    {
        mColor[0] = r; mColor[1] = g; mColor[2] = b;
        if (mAlpha) mColor[3] = a;
        float s = 1.0f, v = 1.0f;
        ImGui::ColorConvertRGBtoHSV(mColor[0], mColor[1], mColor[2], mHue, s, v);
    }
}

// ============================================================================
//  Общий попап (используется и в панели, и в карточке)
// ============================================================================
void ColorPickerComponent::render_popup(const char* popup_id)
{
    if (!ImGui::BeginPopup(popup_id))
        return;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float pop_w = ImGui::GetContentRegionAvail().x;

    // ---- Hue bar ----
    const float hue_h = 14.0f;
    const float hue_r = hue_h * 0.5f;

    ImVec2 hue_pos = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton("##hue", ImVec2(pop_w, hue_h));
    if (ImGui::IsItemActive() || ImGui::IsItemClicked())
    {
        mHue = ImClamp((ImGui::GetMousePos().x - hue_pos.x) / pop_w, 0.0f, 1.0f);
        ImGui::ColorConvertHSVtoRGB(mHue, mSat, mVal,
                                    mColor[0], mColor[1], mColor[2]);
    }

    static const ImU32 hueColors[7] = {
        IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255),
        IM_COL32(0, 255, 0, 255), IM_COL32(0, 255, 255, 255),
        IM_COL32(0, 0, 255, 255), IM_COL32(255, 0, 255, 255),
        IM_COL32(255, 0, 0, 255),
    };

    auto hueColorAt = [&](float t) -> ImU32
    {
        t = ImClamp(t, 0.0f, 1.0f);
        const float f = t * 6.0f;
        const int   i = (std::min)((int)f, 5);
        const float lt = f - (float)i;
        ImVec4 a = ImGui::ColorConvertU32ToFloat4(hueColors[i]);
        ImVec4 b = ImGui::ColorConvertU32ToFloat4(hueColors[i + 1]);
        ImVec4 c = ImLerp(a, b, lt);
        return ImGui::ColorConvertFloat4ToU32(c);
    };

    const float cy_hue = hue_pos.y + hue_h * 0.5f;

    for (float dx = 0.0f; dx < pop_w; dx += 1.0f)
    {
        const float x = hue_pos.x + dx;
        const float t = dx / pop_w;

        float halfH = hue_h * 0.5f;
        const float distLeft  = dx;
        const float distRight = pop_w - 1.0f - dx;

        if (distLeft < hue_r)
        {
            const float k = (hue_r - distLeft) / hue_r;
            halfH = hue_h * 0.5f * ImSqrt(ImMax(0.0f, 1.0f - k * k));
        }
        else if (distRight < hue_r)
        {
            const float k = (hue_r - distRight) / hue_r;
            halfH = hue_h * 0.5f * ImSqrt(ImMax(0.0f, 1.0f - k * k));
        }

        dl->AddRectFilled(
            ImVec2(x,     cy_hue - halfH),
            ImVec2(x + 1.0f, cy_hue + halfH),
            hueColorAt(t));
    }

    const float hueKnobX = hue_pos.x + mHue * pop_w;
    dl->AddCircleFilled(ImVec2(hueKnobX, cy_hue), hue_h * 0.65f,
                        IM_COL32(255, 255, 255, 255), 32);
    dl->AddCircle(ImVec2(hueKnobX, cy_hue), hue_h * 0.65f,
                  IM_COL32(0, 0, 0, 120), 32, 1.5f);

    ImGui::Dummy(ImVec2(0, 4));

    // ---- Preview + hex ----
    const float prev_h = 22.0f;
    const float prev_w = 34.0f;

    ImVec2 row_pos = ImGui::GetCursorScreenPos();

    const ImU32 c = IM_COL32(
        (int)(mColor[0] * 255),
        (int)(mColor[1] * 255),
        (int)(mColor[2] * 255),
        mAlpha ? (int)(mColor[3] * 255) : 255);

    dl->AddRectFilled(row_pos,
                      ImVec2(row_pos.x + prev_w, row_pos.y + prev_h),
                      c, 4.0f, ImDrawFlags_RoundCornersAll);
    dl->AddRect(row_pos,
                ImVec2(row_pos.x + prev_w, row_pos.y + prev_h),
                IM_COL32(40, 40, 48, 255), 4.0f,
                ImDrawFlags_RoundCornersAll, 1.0f);

    snprintf(mHexBuf, sizeof(mHexBuf), "#%02X%02X%02X",
             (int)(mColor[0] * 255),
             (int)(mColor[1] * 255),
             (int)(mColor[2] * 255));

    const float hex_x = row_pos.x + prev_w + 6.0f;
    const float hex_w = pop_w - prev_w - 6.0f;

    ImGui::SetCursorScreenPos(ImVec2(hex_x, row_pos.y));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        IM_COL32(25, 25, 30, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(30, 30, 36, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  IM_COL32(35, 35, 42, 255));
    ImGui::PushStyleColor(ImGuiCol_Border,         IM_COL32(48, 48, 56, 255));
    ImGui::PushStyleColor(ImGuiCol_Text,           IM_COL32(220, 220, 225, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     ImVec2(8, 3));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  1.0f);
    ImGui::SetNextItemWidth(hex_w);

    if (ImGui::InputText("##hex", mHexBuf, sizeof(mHexBuf),
                         ImGuiInputTextFlags_CharsHexadecimal |
                         ImGuiInputTextFlags_CharsUppercase |
                         ImGuiInputTextFlags_EnterReturnsTrue))
    {
        unsigned int r, g, b;
        if (sscanf(mHexBuf, "#%02X%02X%02X", &r, &g, &b) == 3)
        {
            mColor[0] = r / 255.0f;
            mColor[1] = g / 255.0f;
            mColor[2] = b / 255.0f;
            float s, v;
            ImGui::ColorConvertRGBtoHSV(mColor[0], mColor[1], mColor[2], mHue, s, v);
            mSat = s;
            mVal = v;
        }
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);

    ImGui::SetCursorScreenPos(ImVec2(row_pos.x, row_pos.y + prev_h + 8.0f));

    // ---- R/G/B/A sliders ----
    const char* labels[4]  = { "R", "G", "B", "A" };
    float*      values[4]  = { &mColor[0], &mColor[1], &mColor[2], &mColor[3] };
    const bool  enabled[4] = { true, true, true, mAlpha };

    for (int i = 0; i < 4; ++i)
    {
        if (!enabled[i]) continue;

        ImGui::PushID(i);

        const float line_h = ImGui::GetTextLineHeight();
        ImVec2 line_pos = ImGui::GetCursorScreenPos();

        char buf[8];
        snprintf(buf, sizeof(buf), "%d",
                 (int)(ImClamp(*values[i], 0.0f, 1.0f) * 255));

        const ImVec2 tsv = ImGui::CalcTextSize(buf);

        dl->AddText(ImVec2(line_pos.x, line_pos.y),
                    IM_COL32(150, 150, 155, 255), labels[i]);
        dl->AddText(ImVec2(line_pos.x + pop_w - tsv.x, line_pos.y),
                    IM_COL32(255, 255, 255, 255), buf);

        ImGui::Dummy(ImVec2(0, line_h));

        const float sl_h   = 4.0f;
        const float knob_r = 8.0f;
        const float sl_rounding = sl_h * 0.5f;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float track_y = pos.y + knob_r - sl_h * 0.5f;

        const float hit_h = knob_r * 2.0f;
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
        ImGui::InvisibleButton("##col_sl", ImVec2(pop_w, hit_h));

        if (ImGui::IsItemActive())
        {
            float t = ImClamp((ImGui::GetMousePos().x - pos.x) / pop_w, 0.0f, 1.0f);
            *values[i] = t;

            float s, v;
            ImGui::ColorConvertRGBtoHSV(mColor[0], mColor[1], mColor[2], mHue, s, v);
            mSat = s;
            mVal = v;
        }

        const float v = ImClamp(*values[i], 0.0f, 1.0f);
        const float knob_x = pos.x + pop_w * v;

        dl->AddRectFilled(
            ImVec2(pos.x, track_y),
            ImVec2(pos.x + pop_w, track_y + sl_h),
            IM_COL32(45, 45, 50, 255),
            sl_rounding, ImDrawFlags_RoundCornersAll);

        if (v > 0.0f)
            dl->AddRectFilled(
                ImVec2(pos.x, track_y),
                ImVec2(knob_x, track_y + sl_h),
                IM_COL32(200, 200, 205, 255),
                sl_rounding, ImDrawFlags_RoundCornersAll);

        dl->AddCircleFilled(ImVec2(knob_x, track_y + sl_h * 0.5f),
                            knob_r, IM_COL32(255, 255, 255, 255), 32);
        dl->AddCircle(ImVec2(knob_x, track_y + sl_h * 0.5f),
                      knob_r, IM_COL32(0, 0, 0, 60), 32, 1.2f);

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + hit_h));
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PopID();
    }

    ImGui::EndPopup();
}

// ============================================================================
//  Settings panel
// ============================================================================
void ColorPickerComponent::render_in_settings_panel()
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;
    const float offX   = CompStyle::TEXT_OFFSET_X;

    const ImU32 nameCol = g_current_module_enabled
        ? CompStyle::NAME_ON : CompStyle::NAME_OFF;

    DrawComponentName(mName.c_str(), startX + offX, startY, nameCol);

    constexpr float btn_w = 56.0f;
    constexpr float btn_h = 22.0f;

    const float availW = ImGui::GetContentRegionAvail().x;
    const float btn_x  = startX + availW - btn_w - 10.0f;
    const float btn_y  = startY + (ImGui::GetTextLineHeight() - btn_h) * 0.5f;

    ImGui::SetCursorScreenPos(ImVec2(btn_x, btn_y));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0,0,0,0));
    ImGui::InvisibleButton("##cp_preview", ImVec2(btn_w, btn_h));
    const bool btnClicked = ImGui::IsItemClicked();
    ImGui::PopStyleColor(3);

    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        constexpr float rounding = 5.0f;

        const ImU32 fill = IM_COL32(
            (int)(mColor[0] * 255),
            (int)(mColor[1] * 255),
            (int)(mColor[2] * 255),
            mAlpha ? (int)(mColor[3] * 255) : 255);

        dl->AddRectFilled(ImVec2(btn_x, btn_y),
                          ImVec2(btn_x + btn_w, btn_y + btn_h),
                          fill, rounding, ImDrawFlags_RoundCornersAll);

        dl->AddRect(ImVec2(btn_x, btn_y),
                    ImVec2(btn_x + btn_w, btn_y + btn_h),
                    IM_COL32(55, 55, 64, 255), rounding,
                    ImDrawFlags_RoundCornersAll, 1.0f);
    }

    if (btnClicked)
        ImGui::OpenPopup("##cp_popup");

    ImGui::SetNextWindowSize(ImVec2(210, 0), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,   ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(6, 6));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(20, 19, 24, 250));
    ImGui::PushStyleColor(ImGuiCol_Border,  IM_COL32(48, 48, 56, 255));

    render_popup("##cp_popup");

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);

    ImGui::SetCursorScreenPos(ImVec2(startX,
                                     startY + ImGui::GetTextLineHeight() + 4.0f));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));

    ImGui::PopID();
}

// ============================================================================
//  Tile (карточка модуля)
// ============================================================================
void ColorPickerComponent::render_in_tile_module()
{
    ImGui::PushID(this);

    constexpr float btn_w = 56.0f;
    constexpr float btn_h = 22.0f;

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;

    ImGui::SetCursorScreenPos(ImVec2(startX, startY));
    ImGui::InvisibleButton("##cp_preview_tile", ImVec2(btn_w, btn_h));
    const bool clicked = ImGui::IsItemClicked();

    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 fill = IM_COL32(
            (int)(mColor[0] * 255),
            (int)(mColor[1] * 255),
            (int)(mColor[2] * 255),
            mAlpha ? (int)(mColor[3] * 255) : 255);

        dl->AddRectFilled(ImVec2(startX, startY),
                          ImVec2(startX + btn_w, startY + btn_h),
                          fill, 5.0f);
        dl->AddRect(ImVec2(startX, startY),
                    ImVec2(startX + btn_w, startY + btn_h),
                    IM_COL32(55, 55, 64, 255), 5.0f, 0, 1.0f);
    }

    if (clicked)
        ImGui::OpenPopup("##cp_popup_tile");

    ImGui::SetNextWindowSize(ImVec2(210, 0), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,   ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(6, 6));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(20, 19, 24, 250));
    ImGui::PushStyleColor(ImGuiCol_Border,  IM_COL32(48, 48, 56, 255));

    render_popup("##cp_popup_tile");

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);

    ImGui::SetCursorScreenPos(ImVec2(startX, startY + btn_h + 4.0f));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));

    ImGui::PopID();
}


KeybindComponent::KeybindComponent(std::string name, int* value,
                                   std::function<void()> onPressed, float width)
    : abstract_component(component_type::key_bind),
      mName(std::move(name)), mValue(value),
      mOnPressed(std::move(onPressed)), mWidth(width)
{
    Registry().push_back(this);
}

KeybindComponent::~KeybindComponent()
{
    auto& reg = Registry();
    reg.erase(std::remove(reg.begin(), reg.end(), this), reg.end());
}

std::string KeybindComponent::serialize() const
{
    return mValue ? std::to_string(*mValue) : "-1";
}

void KeybindComponent::deserialize(const std::string& data)
{
    if (!mValue || data.empty()) return;
    try { *mValue = std::stoi(data); } catch (...) {}
}

std::vector<KeybindComponent*>& KeybindComponent::Registry()
{
    static std::vector<KeybindComponent*> instances;
    return instances;
}

void KeybindComponent::PollAllHotkeys()
{
    for (KeybindComponent* comp : Registry())
        if (!comp->mCapturing)
            comp->pollHotkey();
}

void KeybindComponent::armCapture()
{
    mCapturing = true;
    for (int vk = 1; vk < 256; ++vk)
        mPrevKeyDown[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
}
void KeybindComponent::pollCapture()
{
    for (int vk = 1; vk < 256; ++vk)
    {
        const bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
        const bool pressedEdge = down && !mPrevKeyDown[vk];
        mPrevKeyDown[vk] = down;

        if (!pressedEdge) continue;

        if (vk == VK_ESCAPE)
        {
            *mValue = -1;
            mHotkeyWasDown = false;
        }
        else
        {
            *mValue = vk;
            mHotkeyWasDown = (GetAsyncKeyState(*mValue) & 0x8000) != 0;
        }

        mCapturing = false;
        break;
    }
}

void KeybindComponent::pollHotkey()
{
    if (!mValue || *mValue <= 0) { mHotkeyWasDown = false; return; }

    const bool down = (GetAsyncKeyState(*mValue) & 0x8000) != 0;
    if (down && !mHotkeyWasDown && mOnPressed)
        mOnPressed();
    mHotkeyWasDown = down;
}

void KeybindComponent::render_in_settings_panel() {}void KeybindComponent::render_in_tile_module()
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;




    const bool hasKey = mValue && *mValue > 0;

    std::string label;
    if (mCapturing)
        label = "choose a key";
    else if (hasKey)
        label = CompStyle::KeyNameForVK(*mValue);
    else
        label = "none";




    constexpr float padding_x = 14.0f;
    constexpr float boxH      = 38.0f;

    const ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
    const float  boxW      = text_size.x + padding_x * 2.0f;

    const float boxX = startX;
    const float boxY = startY - 7.0f;




    ImGui::SetCursorScreenPos(ImVec2(boxX, boxY));
    ImGui::InvisibleButton("##keybind_box", ImVec2(boxW, boxH));
    const bool clickedBox = ImGui::IsItemClicked();

    if (clickedBox && !mCapturing)
        armCapture();




    const ImU32 bg     = IM_COL32(22, 22, 26, 255);
    const ImU32 border = IM_COL32(65, 65, 70, 255);

    constexpr float rounding = 14.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(ImVec2(boxX, boxY),
                      ImVec2(boxX + boxW, boxY + boxH),
                      bg, rounding);
    dl->AddRect(ImVec2(boxX, boxY),
                ImVec2(boxX + boxW, boxY + boxH),
                border, rounding, 0, 1.0f);




    ImU32 textColor;
    if (mCapturing)
        textColor = IM_COL32(255, 255, 255, 255);
    else if (hasKey)
        textColor = CompStyle::NAME_ON;
    else
        textColor = CompStyle::NAME_OFF;

    dl->AddText(ImVec2(boxX + (boxW - text_size.x) * 0.5f,
                       boxY + (boxH - text_size.y) * 0.5f),
                textColor, label.c_str());




    if (mCapturing)
        pollCapture();

    ImGui::PopID();
}



ButtonComponent::ButtonComponent(std::string name, std::function<void()> callback,
                                 float width, float height)
    : abstract_component(component_type::toggle_button),
      mName(std::move(name)), mCallback(std::move(callback)),
      mWidth(width), mHeight(height) {}

void ButtonComponent::render_in_settings_panel()
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;
    const float offX   = CompStyle::TEXT_OFFSET_X;

    const float availW = ImGui::GetContentRegionAvail().x - offX - 8.0f;
    const float x      = startX + offX;
    const float width  = (mWidth > 0.0f && mWidth < availW) ? mWidth : availW;
    const float height = mHeight;

    ImGui::SetCursorScreenPos(ImVec2(x, startY));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0,0,0,0));
    ImGui::InvisibleButton("##button", ImVec2(width, height));
    const bool hovered = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked() && mCallback) mCallback();
    ImGui::PopStyleColor(3);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImU32 accent = CompStyle::Accent();
    const ImU32 bg = hovered ? accent : IM_COL32(28, 28, 32, 255);

    dl->AddRectFilled(ImVec2(x, startY),
                      ImVec2(x + width, startY + height), bg, 5.0f);
    dl->AddRect(ImVec2(x, startY),
                ImVec2(x + width, startY + height),
                IM_COL32(60,60,65,200), 5.0f, 0, 1.0f);

    const ImVec2 ts = ImGui::CalcTextSize(mName.c_str());
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                ImVec2(x + (width - ts.x) * 0.5f,
                       startY + (height - ts.y) * 0.5f),
                CompStyle::NAME_ON, mName.c_str());

    ImGui::SetCursorScreenPos(ImVec2(startX, startY + height + 4.0f));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));
    ImGui::PopID();
}




template <typename T>
void WhitelistDropdownComponent<T>::render_in_settings_panel()
{
    ImGui::PushID(this);

    const float startX = ImGui::GetCursorScreenPos().x;
    const float startY = ImGui::GetCursorScreenPos().y;
    const float offX   = CompStyle::TEXT_OFFSET_X;


    const ImU32 nameCol = g_current_module_enabled
        ? CompStyle::NAME_ON : CompStyle::NAME_OFF;
    DrawComponentName(mName.c_str(), startX + offX, startY, nameCol);


    const float labelH = ImGui::GetTextLineHeight();
    const float rowH   = 32.0f;
    const float itemH  = 28.0f;

    const float availW = ImGui::GetContentRegionAvail().x - offX - 8.0f;
    const float width  = availW;

    const ImVec2 rowPos(startX + offX, startY + labelH + 6.0f);
    const ImVec2 rowEnd(rowPos.x + width, rowPos.y + rowH);

    const ImVec2 mouse = ImGui::GetMousePos();
    ImDrawList* dl = ImGui::GetWindowDrawList();




    std::string header_label;
    if (!mValue || mValue->empty())
    {
        header_label = "none";
    }
    else
    {
        for (size_t i = 0; i < mValue->size(); ++i)
        {

            bool found = false;
            for (auto& op : mOptions)
            {
                if (op.second == (*mValue)[i])
                {
                    if (!header_label.empty()) header_label += ", ";
                    header_label += op.first;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (!header_label.empty()) header_label += ", ";
                header_label += std::to_string(static_cast<int>((*mValue)[i]));
            }
        }
    }




    ImGui::SetCursorScreenPos(rowPos);
    ImGui::InvisibleButton("##wl_header", ImVec2(width, rowH));
    const bool headerHovered = ImGui::IsItemHovered();
    const bool headerClicked = ImGui::IsItemClicked();

    constexpr ImU32 kBaseBg  = IM_COL32(25, 25, 30, 255);
    constexpr ImU32 kHoverBg = IM_COL32(32, 32, 38, 255);
    constexpr ImU32 kBorder  = IM_COL32(40, 40, 48, 255);

    const ImU32 headerBg = (headerHovered || mOpen) ? kHoverBg : kBaseBg;
    dl->AddRectFilled(rowPos, rowEnd, headerBg, 6.0f);
    dl->AddRect(rowPos, rowEnd, kBorder, 6.0f, 0, 1.0f);


    {

        const float maxW = width - 24.0f;
        std::string shown = header_label;
        ImVec2 ts = ImGui::CalcTextSize(shown.c_str());
        while (ts.x > maxW && shown.size() > 3)
        {
            shown.pop_back();
            shown.back() = '.';
            shown += "..";
            if (shown.size() > 64) break;

            shown = header_label.substr(0, shown.size() - 2) + "..";
            ts = ImGui::CalcTextSize(shown.c_str());
            if (shown.size() > 6) break;
        }

        const ImU32 hcol = (mValue && !mValue->empty())
            ? IM_COL32(230, 230, 235, 255)
            : IM_COL32(58, 58, 73, 255);

        dl->AddText(ImVec2(rowPos.x + (width - ts.x) * 0.5f,
                           rowPos.y + (rowH - ts.y) * 0.5f),
                    hcol, shown.c_str());
    }


    {
        const float arrow_size = 5.0f;
        const float pad_right  = 12.0f;
        const float cx = rowEnd.x - pad_right - arrow_size;
        const float cy = (rowPos.y + rowEnd.y) * 0.5f;

        const float dir = mOpen ? -1.0f : 1.0f;
        const ImU32 arrowCol = headerHovered
            ? IM_COL32(255, 255, 255, 255)
            : IM_COL32(140, 140, 150, 255);

        dl->AddTriangleFilled(
            ImVec2(cx - arrow_size, cy - arrow_size * 0.3f * dir),
            ImVec2(cx + arrow_size, cy - arrow_size * 0.3f * dir),
            ImVec2(cx,              cy + arrow_size * 0.7f * dir),
            arrowCol);
    }

    if (headerClicked)
        mOpen = !mOpen;




    bool changed = false;

    if (mOpen && !mOptions.empty())
    {
        const float listY = rowEnd.y + 4.0f;

        for (size_t i = 0; i < mOptions.size(); ++i)
        {
            const ImVec2 p0(rowPos.x, listY + itemH * (float)i);
            const ImVec2 p1(rowEnd.x, p0.y + itemH);

            const bool itemHovered =
                mouse.x >= p0.x && mouse.x <= p1.x &&
                mouse.y >= p0.y && mouse.y <= p1.y;

            ImGui::SetCursorScreenPos(p0);
            ImGui::PushID((int)i);
            ImGui::InvisibleButton("##wl_item", ImVec2(width, itemH));
            const bool itemClicked = ImGui::IsItemClicked();
            ImGui::PopID();

            if (itemClicked && mValue)
            {

                const T val = mOptions[i].second;
                auto it = std::find(mValue->begin(), mValue->end(), val);
                if (it == mValue->end())
                    mValue->push_back(val);
                else
                    mValue->erase(it);
                changed = true;
            }


            const bool selected = mValue &&
                std::find(mValue->begin(), mValue->end(), mOptions[i].second)
                    != mValue->end();


            const ImU32 itemBg = itemHovered
                ? IM_COL32(36, 36, 42, 255)
                : kBaseBg;
            dl->AddRectFilled(p0, p1, itemBg, 4.0f);

            if (i > 0)
                dl->AddLine(ImVec2(p0.x + 6.0f, p0.y),
                            ImVec2(p1.x - 6.0f, p0.y),
                            IM_COL32(30, 30, 36, 255), 1.0f);


            const std::string& itemLabel = mOptions[i].first;
            const ImVec2 its = ImGui::CalcTextSize(itemLabel.c_str());

            const ImU32 textCol = itemHovered
                ? IM_COL32(255, 255, 255, 255)
                : (selected
                    ? IM_COL32(220, 220, 225, 255)
                    : IM_COL32(58, 58, 73, 255));

            dl->AddText(ImVec2(p0.x + 10.0f,
                               p0.y + (itemH - its.y) * 0.5f),
                        textCol, itemLabel.c_str());


            if (selected)
            {
                const float pad = 12.0f;
                const float cx  = p1.x - pad - 4.0f;
                const float cy  = (p0.y + p1.y) * 0.5f;

                dl->AddCircleFilled(ImVec2(cx, cy), 3.5f,
                                    IM_COL32(120, 180, 255, 255));
            }
        }
    }




    float advance = labelH + 6.0f + rowH + 6.0f;
    if (mOpen && !mOptions.empty())
        advance += itemH * (float)mOptions.size() + 4.0f;

    ImGui::SetCursorScreenPos(ImVec2(startX, startY + advance));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));

    ImGui::PopID();
}

void ButtonComponent::render_in_tile_module() {}




template class SliderComponent<int>;
template class SliderComponent<float>;

template class DropdownComponent<int>;
template class DropdownComponent<float>;

template class ChooseButtonsComponent<int>;
template class ChooseButtonsComponent<float>;

template class WhitelistDropdownComponent<int>;
template class WhitelistDropdownComponent<float>;