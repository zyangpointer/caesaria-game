// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "changespeed.hpp"
#include "dispatcher.hpp"
#include "core/utils.hpp"
#include "game/game.hpp"
#include "city/city.hpp"
#include "gfx/tilemap.hpp"
#include "warningmessage.hpp"
#include "core/gettext.hpp"
#include "game/resourcegroup.hpp"
#include "gui/environment.hpp"
#include "gui/label.hpp"

using namespace gfx;

namespace events
{
static const Size wdgSize( 450, 50 );

static const int windowGamePausedId = Hash( "gamepause" );

GameEventPtr Pause::create( Mode mode )
{
  Pause* e = new Pause();
  e->_mode = mode;

  GameEventPtr ret( e );
  ret->drop();
  return ret;
}

bool Pause::_mayExec(Game& game, unsigned int) const{  return true;}

Pause::Pause() : _mode( unknown ) {}

void Pause::_exec(Game& game, unsigned int)
{
  gui::Label* wdg = safety_cast< gui::Label* >( game.gui()->findWidget( windowGamePausedId ) );

  switch( _mode )
  {
  case toggle:
  case pause:
  case play:
    game.setPaused( _mode == toggle ? !game.isPaused() : (_mode == pause) );

    if( game.isPaused()  )
    {
      if( !wdg )
      {
        Size scrSize = game.gui()->vsize();
        wdg = &game.gui()->add<gui::Label>( Rect( Point( (scrSize.width() - wdgSize.width())/2, 40 ), wdgSize ),
                                            _("##game_is_paused##"), false, gui::Label::bgWhiteFrame, windowGamePausedId );
        wdg->setTextAlignment( align::center, align::center );
      }
    }
    else
    {
      if( wdg )
      {
        wdg->deleteLater();
      }
    }
  break;

  case hidepause:
  case hideplay:
    game.setPaused( _mode == hidepause );
  break;

  default: break;
  }
}


GameEventPtr Step::create(unsigned int count)
{
  Step* e = new Step(count);
  GameEventPtr ret( e );
  ret->drop();
  return ret;
}

void Step::_exec(Game &game, unsigned int)
{
  game.step(_count);
}

bool Step::_mayExec(Game &game, unsigned int) const
{
  return game.isPaused();
}

Step::Step(unsigned int count):_count(count)
{
}

} //end namespace events
