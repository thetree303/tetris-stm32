#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    int getHighScore() const { return highScore; }
    void saveHighScore(int score);

    // --------------------------------------------------------
    // SETTINGS - BGM/SFX volume (0-100), gameMode (0=EASY,1=MEDIUM,2=HARD)
    // --------------------------------------------------------
    int getBGMVolume() const { return bgmVolume; }
    int getSFXVolume() const { return sfxVolume; }
    int getGameMode()  const { return gameMode; }

    void setBGMVolume(int vol);
    void setSFXVolume(int vol);
    void setGameMode(int mode) { gameMode  = (mode < 0) ? 0 : (mode > 2)  ? 2  : mode; }

    void tick();
protected:
    ModelListener* modelListener;

    // Biến lưu điểm số cao nhất
    int highScore;

    // Biến lưu cài đặt âm lượng và chế độ chơi
    int bgmVolume; // 0-100
    int sfxVolume; // 0-100
    int gameMode;  // 0=EASY, 1=MEDIUM, 2=HARD
};

#endif // MODEL_HPP
