#include "porting.h"
#include <SDL2/SDL.h>

uint32_t HAL_GetTick(void)
{
    return SDL_GetTicks();
}

void wdog_refresh(void)
{

}
