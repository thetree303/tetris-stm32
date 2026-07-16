#include <gui/settingscreen_screen/SettingScreenView.hpp>
#include <gui/settingscreen_screen/SettingScreenPresenter.hpp>

SettingScreenPresenter::SettingScreenPresenter(SettingScreenView& v)
    : view(v)
{

}

void SettingScreenPresenter::activate()
{

}

void SettingScreenPresenter::deactivate()
{

}

void SettingScreenPresenter::joystickMoved(uint8_t direction)
{
    // Chuyển tiếp tín hiệu joystick xuống View để xử lý điều hướng Settings
    view.onJoystickInput(direction);
}
