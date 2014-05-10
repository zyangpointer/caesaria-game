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

#ifndef __CAESARIA_LOADMAPWINDOW_H_INCLUDED__
#define __CAESARIA_LOADMAPWINDOW_H_INCLUDED__

#include "widget.hpp"
#include "core/scopedptr.hpp"
#include "vfs/path.hpp"
#include "core/signals.hpp"

namespace gui
{

class LoadMapWindow : public Widget
{
public:
  LoadMapWindow( Widget* parent, const Rect& rect, 
    const vfs::Directory& dir, const std::string& ext,
    int id );
  
  virtual ~LoadMapWindow();

  virtual void draw( gfx::Engine& engine );  // draw on screen

  virtual bool onEvent( const NEvent& event);

  virtual bool isPointInside(const Point& point) const;

  void setTitle( const std::string& title );

oc3_signals public:
  Signal1<std::string>& onSelectFile();

protected:
  //void _resizeEvent();

  class Impl;
  ScopedPtr< Impl > _d;
};

}//end namespace gui
#endif //__CAESARIA_LOADMAPWINDOW_H_INCLUDED__
