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

#include "resourceloader.hpp"
#include "core/logger.hpp"
#include "vfs/filesystem.hpp"
#include "vfs/directory.hpp"
#include "core/saveadapter.hpp"
#include "game/settings.hpp"

using namespace vfs;

class ResourceLoader::Impl
{
public:

public oc3_signals:
  Signal1<std::string> onStartLoadingSignal;
};


ResourceLoader::ResourceLoader() : _d( new Impl )
{
}

ResourceLoader::~ResourceLoader(){  }

void ResourceLoader::loadFromModel( Path path2model, Directory dir )
{
  VariantMap archives = SaveAdapter::load( path2model );
  foreach( a, archives )
  {
    Path absArchivePath = GameSettings::rcpath( a->second.toString() );

    if( !absArchivePath.exist() )
    {
      Path rpath = a->second.toString();
      absArchivePath = dir/rpath;
    }
    Logger::warning( "Game: try mount archive " + absArchivePath.toString() );

    Directory dir( absArchivePath.directory() );
    absArchivePath = dir.find( absArchivePath.baseName(), Path::ignoreCase );

    ArchivePtr archive = FileSystem::instance().mountArchive( absArchivePath );
    if( archive.isValid() )
    {
      oc3_emit _d->onStartLoadingSignal( a->first );
    }
    else
    {
      Logger::warning( "Game: cannot load archive " + absArchivePath.toString() );
    }
  }
}

Signal1<std::string> &ResourceLoader::onStartLoading() { return _d->onStartLoadingSignal;}