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

#include "groupbox.hpp"
#include "gfx/decorator.hpp"
#include "gfx/picture.hpp"
#include "gfx/engine.hpp"

using namespace gfx;

namespace gui
{

class GroupBox::Impl
{
public:
	PictureRef texture;
  Picture backgroundImage;
	bool scaleImage;
  GroupBox::Style style;
  bool needUpdateTexture;
};

//! constructor
GroupBox::GroupBox( Widget* parent, const Rect& rectangle, int id, Style style)
: Widget( parent, id, rectangle ), _d( new Impl )
{
	#ifdef _DEBUG
    	setDebugName("GroupBox");
	#endif

  _d->scaleImage = true;	
  _d->needUpdateTexture = true;
  _d->style = style;
}

//! destructor
GroupBox::~GroupBox() {}

//! draws the element and its children
void GroupBox::draw(gfx::Engine& painter )
{
  if (!visible())
      return;

  if( !_d->texture.isNull() )
  {
    painter.draw( *_d->texture, screenLeft(), screenTop(), &absoluteClippingRectRef() );
  }

  Widget::draw( painter );
}

//! Returns true if the image is scaled to fit, false if not
bool GroupBox::isBackgroundImageScaled() const {	return _d->scaleImage; }

void GroupBox::setBackgroundImage( const Picture& image )
{
  _d->backgroundImage = image;
  _d->needUpdateTexture = true;
}

const Picture& GroupBox::getBackgroundImage() const {  return _d->backgroundImage; }
void GroupBox::setScaleBackgroundImage( bool scale ) { _d->scaleImage = scale; }

void GroupBox::setStyle( Style style )
{
  _d->style = style;
  _d->needUpdateTexture = true;
}

void GroupBox::beforeDraw(gfx::Engine& painter )
{
  if( _d->needUpdateTexture )
  {
    _d->needUpdateTexture = false;
    Size mySize = size();

    if( !_d->texture.isNull() && _d->texture->size() != mySize )
      _d->texture.reset();

    if( _d->texture.isNull() )
      _d->texture.reset( Picture::create( mySize ) );

    if( _d->backgroundImage.isValid() )
    {
      if( _d->scaleImage )
      {
        _d->texture->draw( _d->backgroundImage, Rect( Point( 0, 0 ), _d->backgroundImage.size()), Rect( Point(0, 0), mySize) );
      }
      else
      {
        _d->texture->draw( _d->backgroundImage, Point(0, 0) );
      }
    }
    else
    {
      Decorator::draw( *_d->texture, Rect( Point( 0, 0 ), size() ), 
                              _d->style == whiteFrame ? Decorator::whiteFrame : Decorator::blackFrame,
                              true, true );
    }
  }

  Widget::beforeDraw( painter );
}

}//end namespace gui
