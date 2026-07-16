#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <stdint.h>
#include <stdio.h>

// Khai báo các hàm C để C++ có thể gọi được
extern "C" {
    uint8_t Get_Joystick_Direction(void);
    
    // Định nghĩa Struct cấu hình tương ứng với C
    typedef struct {
        uint32_t highScore;
        uint8_t bgmVolume;
        uint8_t sfxVolume;
        uint8_t gameMode;
        uint8_t magic;
    } GameConfigC;

    void LoadGameConfig(GameConfigC *config);
    void SaveGameConfig(const GameConfigC *config);

    // Biến âm lượng volatile trong audio.c (extern để đồng bộ)
    extern volatile uint8_t bgm_volume;
    extern volatile uint8_t sfx_volume;
}

Model::Model() : modelListener(0)
{
    // Đọc toàn bộ cấu hình từ Flash khi khởi tạo
    GameConfigC config;
    LoadGameConfig(&config);

    highScore = config.highScore;
    bgmVolume = config.bgmVolume;
    sfxVolume = config.sfxVolume;
    gameMode  = config.gameMode;

    // Đồng bộ giá trị lên driver audio phần cứng
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
    if (direction != 0)
    {
        static const char* DIR_NAMES[] = {
            "IDLE", "LEFT", "RIGHT", "ROTATE", "FAST_DOWN"
        };
        printf("[JOY] dir=%u (%s)\r\n",
               direction,
               (direction <= 4u) ? DIR_NAMES[direction] : "???");
    }

    // 3. Luôn gửi direction cho Presenter, kể cả IDLE (0)
    if (modelListener != 0)
    {
        static bool wasMoved = false;

        if (direction != 0)
        {
            modelListener->joystickMoved(direction);
            wasMoved = true;
        }
        else if (wasMoved)
        {
            modelListener->joystickMoved(0);
            wasMoved = false;
        }
    }
}

// Helper hàm để đóng gói và lưu cấu hình hiện tại
void Model::persistCurrentSettings()
{
    GameConfigC config;
    config.highScore = (uint32_t)highScore;
    config.bgmVolume = (uint8_t)bgmVolume;
    config.sfxVolume = (uint8_t)sfxVolume;
    config.gameMode  = (uint8_t)gameMode;
    config.magic     = 0x5A; // Trùng với FLASH_CONFIG_MAGIC bên C

    SaveGameConfig(&config);
}

// Lưu điểm số cao nhất vào Flash
void Model::saveHighScore(int score)
{
    highScore = score;
    persistCurrentSettings();
}

// Cập nhật âm lượng nhạc nền (0-100) và lưu vào Flash
void Model::setBGMVolume(int vol)
{
    bgmVolume = (vol < 0) ? 0 : (vol > 100) ? 100 : vol;
    bgm_volume = (uint8_t)bgmVolume;
    persistCurrentSettings();
}

// Cập nhật âm lượng hiệu ứng (0-100) và lưu vào Flash
void Model::setSFXVolume(int vol)
{
    sfxVolume = (vol < 0) ? 0 : (vol > 100) ? 100 : vol;
    sfx_volume = (uint8_t)sfxVolume;
    persistCurrentSettings();
}

// Cập nhật chế độ chơi (0=EASY, 1=MEDIUM, 2=HARD) và lưu vào Flash
void Model::setGameMode(int mode) 
{ 
    gameMode = (mode < 0) ? 0 : (mode > 2) ? 2 : mode; 
    persistCurrentSettings();
}