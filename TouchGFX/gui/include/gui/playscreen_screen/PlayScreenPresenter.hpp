#ifndef PLAYSCREENPRESENTER_HPP
#define PLAYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class PlayScreenView;

class PlayScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    PlayScreenPresenter(PlayScreenView& v);

    /**
     * activate() được gọi tự động khi màn hình PlayScreen được kích hoạt.
     * Dùng để reset trạng thái game ban đầu.
     */
    virtual void activate();

    /**
     * deactivate() được gọi tự động khi rời khỏi màn hình PlayScreen.
     */
    virtual void deactivate();

    virtual ~PlayScreenPresenter() {}

    /**
     * @brief Override callback từ ModelListener.
     *        Được Model::tick() gọi mỗi khi Joystick lệch khỏi IDLE.
     *        Presenter chỉ làm nhiệm vụ "chuyển thư" xuống View.
     *
     * @param direction  1=LEFT | 2=RIGHT | 3=ROTATE | 4=FAST_DOWN
     */
    virtual void joystickMoved(uint8_t direction);

    /**
     * @brief Lấy điểm số cao nhất từ Model.
     *
     * @return int  Điểm số cao nhất
     */
    int getHighScore() const { return model->getHighScore(); }
    
    /**
     * @brief Lưu điểm số cao nhất vào Model.
     *
     * @param score  Điểm số cao nhất
     */
    void saveHighScore(int score) { model->saveHighScore(score); }

    /**
     * @brief Lấy chế độ chơi hiện tại từ Model.
     *        0=EASY, 1=MEDIUM, 2=HARD
     */
    int getGameMode() const { return model->getGameMode(); }
    
private:
    PlayScreenPresenter();

    PlayScreenView& view;
};

#endif // PLAYSCREENPRESENTER_HPP
