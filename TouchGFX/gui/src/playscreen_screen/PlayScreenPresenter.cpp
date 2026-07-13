#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>

PlayScreenPresenter::PlayScreenPresenter(PlayScreenView& v)
    : view(v)
{
}

void PlayScreenPresenter::activate()
{
    // Khi màn hình được kích hoạt, bảo View khởi động game mới
    view.startGame();
}

void PlayScreenPresenter::deactivate()
{
    // Không cần dọn dẹp đặc biệt
}

void PlayScreenPresenter::joystickMoved(uint8_t direction)
{
    // Presenter chỉ chuyển tiếp tín hiệu, không xử lý logic
    view.onJoystickInput(direction);
}
