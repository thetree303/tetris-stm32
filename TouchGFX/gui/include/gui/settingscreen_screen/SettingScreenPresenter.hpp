#ifndef SETTINGSCREENPRESENTER_HPP
#define SETTINGSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class SettingScreenView;

class SettingScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    SettingScreenPresenter(SettingScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~SettingScreenPresenter() {}

    /**
     * @brief Override callback từ ModelListener.
     *        Được Model::tick() gọi mỗi khi Joystick lệch khỏi IDLE.
     *        Presenter chuyển tiếp xuống View.
     *
     * @param direction  1=LEFT | 2=RIGHT | 3=ROTATE | 4=FAST_DOWN
     */
    virtual void joystickMoved(uint8_t direction);

    // Getters / Setters giao tiếp với Model
    int  getBGMVolume() const { return model->getBGMVolume(); }
    int  getSFXVolume() const { return model->getSFXVolume(); }
    int  getGameMode()  const { return model->getGameMode();  }

    void setBGMVolume(int vol)  { model->setBGMVolume(vol); }
    void setSFXVolume(int vol)  { model->setSFXVolume(vol); }
    void setGameMode(int mode)  { model->setGameMode(mode); }

private:
    SettingScreenPresenter();

    SettingScreenView& view;
};

#endif // SETTINGSCREENPRESENTER_HPP
