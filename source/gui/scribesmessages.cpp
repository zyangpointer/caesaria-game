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

#include "scribesmessages.hpp"
#include "gameautopause.hpp"
#include "game/settings.hpp"
#include "listbox.hpp"
#include "game/resourcegroup.hpp"
#include "game/datetimehelper.hpp"
#include "core/stringhelper.hpp"
#include "widgetescapecloser.hpp"
#include "city/cityservice_info.hpp"
#include "city/city.hpp"
#include "core/logger.hpp"
#include "core/event.hpp"
#include "widgetescapecloser.hpp"
#include "popup_messagebox.hpp"
#include "texturedbutton.hpp"
#include "core/color.hpp"
#include "event_messagebox.hpp"

using namespace constants;
using namespace gfx;

namespace gui
{

namespace {
CAESARIA_LITERALCONST(opened)
CAESARIA_LITERALCONST(critical)
CAESARIA_LITERALCONST(ext)
CAESARIA_LITERALCONST(date)
}

class ScribesListBox : public ListBox
{
public:
  ScribesListBox( Widget* p, const Rect& rect ) : ListBox( p, rect )
  {
    setFlag( selectOnMove, true );
  }

  virtual ListBoxItem& addItem( const std::string& text, Font font=Font(), const int color=0 )
  {
    ListBoxItem& item = ListBox::addItem( text, font, color );

    ListBoxItem::OverrideColor& itemfc = item.OverrideColors[ ListBoxItem::simple ];
    item.OverrideColors[ ListBoxItem::hovered ].font = itemfc.font;
    item.OverrideColors[ ListBoxItem::hovered ].Use = true;
    item.OverrideColors[ ListBoxItem::hovered ].color = 0xffff0000;

    item.setIcon( Picture::load( ResourceGroup::panelBackground, 111 ));

    return item;
  }

public oc3_signals:
  Signal1<int> onShowMessage;
  Signal1<int> onRemoveMessage;

protected:
  virtual void _drawItemIcon(gfx::Picture &texture, ListBoxItem &item, const Point &pos)
  {
    VariantMap options = item.data().toMap();
    bool opened = options.get( lc_opened, false );
    bool critical = options.get( lc_critical, false );
    int imgIndex = (critical ? 113 : 111) + (opened ? 1 : 0);
    texture.draw( Picture::load( ResourceGroup::panelBackground, imgIndex ), pos + Point( 2, 2) );
  }

  virtual void _drawItemText(gfx::Picture &texture, Font font, ListBoxItem &item, const Point &pos)
  {
    VariantMap options = item.data().toMap();
    DateTime time = options[ lc_date ].toDateTime();

    font.draw( texture, DateTimeHelper::toStr( time ), pos + Point( 30, 0 ), false );
    font.draw( texture, item.text(), Point( width() / 2, pos.y() ), false );
  }

  virtual bool onEvent(const NEvent &event)
  {
    bool ret = ListBox::onEvent( event );

    if( event.EventType == sEventMouse )
    {
      switch(event.mouse.type)
      {
      case mouseLbtnRelease: oc3_emit onShowMessage( selected() ); break;
      case mouseRbtnRelease: oc3_emit onRemoveMessage( selected() ); break;
      default: break;
      }
    }

    return ret;
  }
};

class ScribesMessagestWindow::Impl
{
public:
  GameAutoPause locker;
  ScribesListBox* lbxMessages;
  PlayerCityPtr city;
};

ScribesMessagestWindow::~ScribesMessagestWindow() {}

ScribesMessagestWindow::ScribesMessagestWindow( Widget* p, PlayerCityPtr city )
  : Widget( p, -1, Rect( 0, 0, 480, 320 ) ), _d( new Impl )
{
  _d->city = city;
  _d->locker.activate();

  setupUI( GameSettings::rcpath( "/gui/scribesmessages.gui" ) );
  setCenter( p->center() );

  WidgetEscapeCloser::insertTo( this );

  WidgetEscapeCloser::insertTo( this );
  _d->lbxMessages = new ScribesListBox( this, Rect( 16, 60, width() - 16, height() - 50 ) );

  TexturedButton* btnExit = findChildA<TexturedButton*>( "btnExit", true, this );
  TexturedButton* btnHelp = findChildA<TexturedButton*>( "btnHelp", true, this );

  _fillMessages();

  CONNECT( _d->lbxMessages, onShowMessage, this, ScribesMessagestWindow::_showMessage );
  CONNECT( _d->lbxMessages, onRemoveMessage, this, ScribesMessagestWindow::_removeMessage );
  CONNECT( btnExit, onClicked(), this, ScribesMessagestWindow::deleteLater );
}

void ScribesMessagestWindow::draw(gfx::Engine& painter )
{
  if( !visible() )
    return;

  Widget::draw( painter );
}

void ScribesMessagestWindow::_fillMessages()
{
  _d->lbxMessages->clear();

  SmartPtr<city::Info> srvc = ptr_cast<city::Info>( _d->city->findService( city::Info::defaultName() ) );
  if( srvc.isValid() )
  {
    const city::Info::Messages& messages = srvc->messages();
    foreach( it, messages )
    {
      const city::Info::ScribeMessage& mt = *it;
      ListBoxItem& item = _d->lbxMessages->addItem( mt.title );
      VariantMap options;
      options[ lc_opened ] = mt.opened;
      options[ lc_date   ] = mt.date;
      options[ lc_ext    ] = mt.ext;

      item.setData( options );
    }
  }
}

void ScribesMessagestWindow::_showMessage(int index)
{
  SmartPtr<city::Info> srvc = ptr_cast<city::Info>( _d->city->findService( city::Info::defaultName() ) );
  if( srvc.isValid() )
  {
    city::Info::ScribeMessage mt = srvc->getMessage( index );
    mt.opened = true;
    srvc->changeMessage( index, mt );
    EventMessageBox* mbox = new EventMessageBox( parent(), mt.title, mt.text, mt.date, mt.gtype );
    mbox->show();
  }

  _fillMessages();
}

void ScribesMessagestWindow::_removeMessage(int index)
{
  SmartPtr<city::Info> srvc = ptr_cast<city::Info>( _d->city->findService( city::Info::defaultName() ) );
  if( srvc.isValid() )
  {
    srvc->removeMessage( index );
  }

  _fillMessages();
}

}//end namespace gui