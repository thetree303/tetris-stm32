#ifndef HOMESCREENPRESENTER_HPP
#define HOMESCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class HomeScreenView;

class HomeScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    HomeScreenPresenter(HomeScreenView& v);

    virtual void activate();

    virtual void deactivate();

    virtual ~HomeScreenPresenter() {}

private:
    HomeScreenPresenter();

    HomeScreenView& view;
};

#endif // HOMESCREENPRESENTER_HPP
