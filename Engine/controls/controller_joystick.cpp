/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2017 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "controller_joystick.h"


JoystickController::JoystickController() :
    Controller(),
    m_joystickController(0)
{
    //    qDebug() << "Num of joysticks: " << SDL_NumJoysticks();
    //    if(SDL_NumJoysticks() > 0){
    //        //TODO: Select which controller you want to use.
    //        joystickController =  SDL_JoystickOpen(0);
    //        qDebug() << "Opened Controller \"" << SDL_JoystickName(joystickController) << "\"";
    //    }
    kmap.jump.val       = 1;
    kmap.jump_alt.val   = 3;
    kmap.run.val        = 0;
    kmap.run_alt.val    = 2;
    kmap.drop.val       = 8;
    kmap.start.val      = 9;
    kmap.left.val       = 10;
    kmap.right.val      = 11;
    kmap.up.val         = 12;
    kmap.down.val       = 13;
}

JoystickController::~JoystickController()
{}

void JoystickController::setJoystickDevice(SDL_Joystick *jctrl)
{
    m_joystickController = jctrl;
}

SDL_Joystick *JoystickController::getJoystickDevice() const
{
    return m_joystickController;
}

void JoystickController::updateKey(bool &key, KM_Key &mkey)
{
    Sint32 val = 0, dx = 0, dy = 0;
    Sint16 val_initial = 0;

    switch(mkey.type)
    {
    case KeyMapJoyCtrls::JoyAxis:
        //Note: SDL_JoystickGetAxisInitialState is a new API function added into dev version
        //      and doesn't available in already released assemblies
        if(SDL_JoystickGetAxisInitialState(m_joystickController, mkey.id, &val_initial) == SDL_FALSE)
        {
            key = false;
            break;
        }
        val = SDL_JoystickGetAxis(m_joystickController, mkey.id);

        if(mkey.val > val_initial)
            key = (val > val_initial);
        else if(mkey.val < val_initial)
            key = (val < val_initial);
        else key = false;

        break;

    case KeyMapJoyCtrls::JoyBallX:
        SDL_JoystickGetBall(m_joystickController, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key = (dx > 0);
        else if(mkey.id < 0)
            key = (dx < 0);
        else key = false;

        break;

    case KeyMapJoyCtrls::JoyBallY:
        SDL_JoystickGetBall(m_joystickController, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key = (dy > 0);
        else if(mkey.id < 0)
            key = (dy < 0);
        else key = false;

        break;

    case KeyMapJoyCtrls::JoyHat:
        val = (Sint32)SDL_JoystickGetHat(m_joystickController, mkey.id);
        key = (val == mkey.val);
        break;

    case KeyMapJoyCtrls::JoyButton:
        key = (Sint32)SDL_JoystickGetButton(m_joystickController, mkey.id);
        break;

    default:
        key = false;
        break;
    }
}

void JoystickController::update()
{
    if(!m_joystickController)
        return;

    //SDL_PumpEvents();
    SDL_JoystickUpdate();
    updateKey(keys.jump, kmap.jump);
    updateKey(keys.alt_jump, kmap.jump_alt);
    updateKey(keys.run, kmap.run);
    updateKey(keys.alt_run, kmap.run_alt);
    updateKey(keys.right, kmap.right);
    updateKey(keys.left, kmap.left);
    updateKey(keys.up, kmap.up);
    updateKey(keys.down, kmap.down);
    updateKey(keys.drop, kmap.drop);
    updateKey(keys.start, kmap.start);
}



bool JoystickController::bindJoystickKey(SDL_Joystick *joy, KM_Key &k)
{
    Sint32 val = 0;
    Sint16 val_initial = 0;
    int dx = 0, dy = 0;
    //SDL_PumpEvents();
    SDL_JoystickUpdate();
    int balls = SDL_JoystickNumBalls(joy);
    int hats = SDL_JoystickNumHats(joy);
    int buttons = SDL_JoystickNumButtons(joy);
    int axes = SDL_JoystickNumAxes(joy);

    for(int i = 0; i < balls; i++)
    {
        dx = 0;
        dy = 0;
        SDL_JoystickGetBall(joy, i, &dx, &dy);

        if(dx != 0)
        {
            k.val = dx;
            k.id = i;
            k.type = (int)KeyMapJoyCtrls::JoyBallX;
            return true;
        }
        else if(dy != 0)
        {
            k.val = dy;
            k.id = i;
            k.type = (int)KeyMapJoyCtrls::JoyBallY;
            return true;
        }
    }

    for(int i = 0; i < hats; i++)
    {
        val = 0;
        val = SDL_JoystickGetHat(joy, i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KeyMapJoyCtrls::JoyHat;
            return true;
        }
    }

    for(int i = 0; i < buttons; i++)
    {
        val = 0;
        val = SDL_JoystickGetButton(joy, i);

        if(val == 1)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KeyMapJoyCtrls::JoyButton;
            return true;
        }
    }

    for(int i = 0; i < axes; i++)
    {
        val = 0;
        //Note: SDL_JoystickGetAxisInitialState is a new API function added into dev version
        //      and doesn't available in already released assemblies
        if(SDL_JoystickGetAxisInitialState(joy, i, &val_initial) == SDL_FALSE)
            break;

        val = SDL_JoystickGetAxis(joy, i);

        if(val != (Sint32)val_initial)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KeyMapJoyCtrls::JoyAxis;
            return true;
        }
    }

    k.val = 0;
    k.id = 0;
    k.type = (int)KeyMapJoyCtrls::NoControl;
    return false;
}
