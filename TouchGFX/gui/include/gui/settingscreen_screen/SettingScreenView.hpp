#ifndef SETTINGSCREENVIEW_HPP
#define SETTINGSCREENVIEW_HPP

#include <gui_generated/settingscreen_screen/SettingScreenViewBase.hpp>
#include <gui/settingscreen_screen/SettingScreenPresenter.hpp>
#include <touchgfx/Unicode.hpp>

class SettingScreenView : public SettingScreenViewBase
{
public:
    SettingScreenView();
    virtual ~SettingScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    /**
     * @brief Nhận tín hiệu joystick từ Presenter.
     *        1=LEFT (giảm), 2=RIGHT (tăng), 4=FAST_DOWN (chuyển tùy chọn)
     */
    void onJoystickInput(uint8_t direction);

protected:
    // --------------------------------------------------------
    // Chỉ số tùy chọn đang được chọn: 0=BGM, 1=SFX, 2=Mode
    // --------------------------------------------------------
    int selectedOption; // 0, 1, 2

    // --------------------------------------------------------
    // Wildcard buffers (UTF-16) cho 3 dòng giá trị
    // --------------------------------------------------------
    touchgfx::Unicode::UnicodeChar bgmBuf[8];  // "0".."100\0"
    touchgfx::Unicode::UnicodeChar sfxBuf[8];  // "0".."100\0"
    touchgfx::Unicode::UnicodeChar modeBuf[8]; // "EASY\0", "MEDIUM\0", "HARD\0"

    // --------------------------------------------------------
    // Cập nhật toàn bộ UI (wildcard text + màu highlight)
    // --------------------------------------------------------
    void updateSettingsUI();
};

#endif // SETTINGSCREENVIEW_HPP
