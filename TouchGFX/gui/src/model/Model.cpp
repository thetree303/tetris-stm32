#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <stdint.h>
#include <stdio.h>

// Khai báo các hàm C để C++ có thể gọi được
extern "C" {
    uint8_t Get_Joystick_Direction(void);
    uint32_t LoadScoreFromFlash(void);
    void SaveScoreToFlash(uint32_t score);

    // Biến âm lượng volatile trong audio.c (extern để đồng bộ)
    extern volatile uint8_t bgm_volume;
    extern volatile uint8_t sfx_volume;
}

Model::Model() : modelListener(0), highScore(LoadScoreFromFlash()),
                 bgmVolume(50), sfxVolume(50), gameMode(0)
{
    // Đồng bộ giá trị mặc định lên audio driver ngay khi khởi tạo
    bgm_volume = (uint8_t)bgmVolume;
    sfx_volume = (uint8_t)sfxVolume;
}


// Hàm này được TouchGFX gọi mỗi frame đồ họa (~60 lần/giây).
void Model::tick()
{
    // 1. Đọc trạng thái nút bấm từ C (GPIO)
    uint8_t direction = Get_Joystick_Direction();

    // Nếu direction = 255 => chưa hết debounce, bỏ qua không gọi callback
    if (direction == 255)
    {
        return; 
    }

    // 2. Log ra UART khi có lệnh hợp lệ
    // printf được điều hướng => USART1 TX qua __io_putchar() trong main.c.
    if (direction != 0)
    {
        static const char* DIR_NAMES[] = {
            "IDLE", "LEFT", "RIGHT", "ROTATE", "FAST_DOWN"
        };
        printf("[JOY] dir=%u (%s)\r\n",
               direction,
               (direction <= 4u) ? DIR_NAMES[direction] : "???");
    }

    // 3. Luôn gửi direction cho Presenter, kể cả IDLE (0) (để Presenter reset cờ joystickHeld)
    if (modelListener != 0)
    {
        static bool wasMoved = false;

        if (direction != 0)
        {
            // Có lệnh, forward xuống Presenter => View
            modelListener->joystickMoved(direction);
            wasMoved = true;
        }
        else if (wasMoved)
        {
            // IDLE vừa xuất hiện sau khi có lệnh, forward xuống Presenter => View
            modelListener->joystickMoved(0);
            wasMoved = false;
        }
        // Nếu IDLE liên tục, không forward xuống Presenter (để tránh spam)
    }
}

// Lưu điểm số cao nhất vào Flash
void Model::saveHighScore(int score)
{
    highScore = score;
    SaveScoreToFlash(score);
}

void Model::setBGMVolume(int vol)
{
    bgmVolume = (vol < 0) ? 0 : (vol > 100) ? 100 : vol;
    bgm_volume = (uint8_t)bgmVolume;
}

void Model::setSFXVolume(int vol)
{
    sfxVolume = (vol < 0) ? 0 : (vol > 100) ? 100 : vol;
    sfx_volume = (uint8_t)sfxVolume;
}