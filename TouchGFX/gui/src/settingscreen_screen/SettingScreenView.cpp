#include <gui/settingscreen_screen/SettingScreenView.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/Color.hpp>

// Tên các chế độ (viết hoa)
static const char* const MODE_NAMES[3] = { "EASY", "MEDIUM", "HARD" };

SettingScreenView::SettingScreenView()
    : selectedOption(0)
{
    // Khởi tạo buffers rỗng
    bgmBuf[0]  = 0;
    sfxBuf[0]  = 0;
    modeBuf[0] = 0;
}

void SettingScreenView::setupScreen()
{
    SettingScreenViewBase::setupScreen();

    // Gán wildcard buffer cho 3 TextAreaWithOneWildcard (khai báo trong ViewBase)
    music.setWildcard(bgmBuf);
    sound.setWildcard(sfxBuf);
    mode.setWildcard(modeBuf);

    // Đọc giá trị hiện tại từ Model (qua Presenter)
    // và cập nhật UI lần đầu
    updateSettingsUI();
}

void SettingScreenView::tearDownScreen()
{
    SettingScreenViewBase::tearDownScreen();
}

// --------------------------------------------------------
// Nhận lệnh joystick từ Presenter
//   direction: 1=LEFT (giảm), 2=RIGHT (tăng), 4=FAST_DOWN (chuyển option), 0=IDLE
// --------------------------------------------------------
void SettingScreenView::onJoystickInput(uint8_t direction)
{
    if (direction == 0) return; // IDLE - bỏ qua

    if (direction == 4) // DOWN => chuyển sang tùy chọn tiếp theo (vòng tròn)
    {
        selectedOption = (selectedOption + 1) % 3;
        updateSettingsUI();
        return;
    }

    // Xác định delta thay đổi: LEFT=-1 (giảm), RIGHT=+1 (tăng)
    int delta = 0;
    if (direction == 1) delta = -1;  // LEFT
    if (direction == 2) delta = +1;  // RIGHT

    if (delta == 0) return;

    switch (selectedOption)
    {
        case 0: // BGM volume
        {
            int vol = presenter->getBGMVolume() + delta * 5;
            if (vol < 0)   vol = 0;
            if (vol > 100) vol = 100;
            presenter->setBGMVolume(vol);
            break;
        }
        case 1: // SFX volume
        {
            int vol = presenter->getSFXVolume() + delta * 5;
            if (vol < 0)   vol = 0;
            if (vol > 100) vol = 100;
            presenter->setSFXVolume(vol);
            break;
        }
        case 2: // Game mode: 0=EASY, 1=MEDIUM, 2=HARD
        {
            int m = presenter->getGameMode() + delta;
            if (m < 0) m = 0;
            if (m > 2) m = 2;
            presenter->setGameMode(m);
            break;
        }
        default:
            break;
    }

    updateSettingsUI();
}

// --------------------------------------------------------
// Cập nhật text wildcard và màu highlight cho từng dòng
// --------------------------------------------------------
void SettingScreenView::updateSettingsUI()
{
    // Yêu cầu TouchGFX xóa vùng hiển thị cũ của các text (trước khi thay đổi nội dung/kích thước).
    // Điều này ngăn hiện tượng đè chữ khi độ dài chữ mới ngắn hơn chữ cũ (ví dụ: HARDIUM, 000).
    music.invalidate();
    sound.invalidate();
    mode.invalidate();

    // --- Cập nhật giá trị BGM ---
    int bgmVal = presenter->getBGMVolume();
    touchgfx::Unicode::snprintf(bgmBuf, 8, "%d", bgmVal);

    // --- Cập nhật giá trị SFX ---
    int sfxVal = presenter->getSFXVolume();
    touchgfx::Unicode::snprintf(sfxBuf, 8, "%d", sfxVal);

    // --- Cập nhật tên Mode ---
    // Unicode::fromUTF8 chuyển đổi ASCII/UTF-8 sang Unicode::UnicodeChar (UTF-16)
    // Đây là cách đúng để copy const char* sang wildcard buffer
    int modeIdx = presenter->getGameMode();
    touchgfx::Unicode::fromUTF8(
        reinterpret_cast<const uint8_t*>(MODE_NAMES[modeIdx]),
        modeBuf,
        8
    );

    // --- Highlight dòng đang được chọn (màu vàng), các dòng còn lại màu TRẮNG ---
    // Màu highlight: vàng sáng (255, 220, 0)
    // Màu bình thường: TRẮNG (255, 255, 255) - vì nền ảnh settings màu tối
    touchgfx::colortype normalColor    = touchgfx::Color::getColorFromRGB(255, 255, 255);
    touchgfx::colortype highlightColor = touchgfx::Color::getColorFromRGB(255, 220,   0);

    music.setColor(selectedOption == 0 ? highlightColor : normalColor);
    sound.setColor(selectedOption == 1 ? highlightColor : normalColor);
    mode.setColor(selectedOption == 2  ? highlightColor : normalColor);

    // Cũng highlight label tương ứng
    touchgfx::colortype labelNormal    = touchgfx::Color::getColorFromRGB(255, 255, 255);
    touchgfx::colortype labelHighlight = touchgfx::Color::getColorFromRGB(255, 220,   0);

    text_music.setColor(selectedOption == 0 ? labelHighlight : labelNormal);
    text_sound.setColor(selectedOption == 1 ? labelHighlight : labelNormal);
    text_mode.setColor(selectedOption == 2  ? labelHighlight : labelNormal);

    // Cập nhật lại kích thước widget dựa trên chuỗi thực tế mới trong buffer
    music.resizeToCurrentText();
    sound.resizeToCurrentText();
    mode.resizeToCurrentText();

    // Yêu cầu vẽ lại vùng hiển thị mới
    music.invalidate();
    sound.invalidate();
    mode.invalidate();
    text_music.invalidate();
    text_sound.invalidate();
    text_mode.invalidate();
}
