#include <gui/homescreen_screen/HomeScreenView.hpp>
#include "audio.h"

HomeScreenView::HomeScreenView()
{

}

void HomeScreenView::setupScreen()
{
    HomeScreenViewBase::setupScreen();
    Audio_StartBGM();
}

void HomeScreenView::tearDownScreen()
{
    HomeScreenViewBase::tearDownScreen();
}
