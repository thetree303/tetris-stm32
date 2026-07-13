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

    void tick();
protected:
    ModelListener* modelListener;

    // Biến lưu điểm số cao nhất
    int highScore;
};

#endif // MODEL_HPP
