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

#include "farm.hpp"
#include "core/position.hpp"
#include "core/exception.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/helper.hpp"
#include "city/helper.hpp"
#include "good/goodhelper.hpp"
#include "city/city.hpp"
#include "core/utils.hpp"
#include "gfx/tilemap.hpp"
#include "core/gettext.hpp"
#include "core/logger.hpp"
#include "constants.hpp"
#include "walker/locust.hpp"
#include "core/foreach.hpp"
#include "game/gamedate.hpp"

using namespace constants;
using namespace gfx;

class FarmTile
{
public:
  FarmTile(const good::Type outGood, const TilePos& pos );
  virtual ~FarmTile();
  void computePicture(const int percent);
  Picture& getPicture();

private:
  TilePos _pos;
  Picture _picture;
  Animation _animation;
};

FarmTile::FarmTile(const good::Type outGood, const TilePos& pos )
{
  _pos = pos;

  int picIdx = 0;
  switch (outGood)
  {
  case good::wheat: picIdx = 13; break;
  case good::vegetable: picIdx = 18; break;
  case good::fruit: picIdx = 23; break;
  case good::olive: picIdx = 28; break;
  case good::grape: picIdx = 33; break;
  case good::meat: picIdx = 38; break;
  default:
    Logger::warning( "Unexpected farmType in farm" + GoodHelper::name( outGood ) );
    _CAESARIA_DEBUG_BREAK_IF( "Unexpected farmType in farm ");
  }

  _animation.load( ResourceGroup::commerce, picIdx, 5);
  computePicture(0);
}

void FarmTile::computePicture(const int percent)
{
  Pictures& pictures = _animation.frames();

  int picIdx = (percent * (pictures.size()-1)) / 100;
  _picture = pictures[picIdx];
  _picture.addOffset( util::tilepos2screen( _pos ));
}

Picture& FarmTile::getPicture() {  return _picture; }
FarmTile::~FarmTile() {}

class Farm::Impl
{
public:
  typedef std::vector<FarmTile> SubTiles;
  SubTiles subTiles;
  Picture pictureBuilding;  // we need to change its offset
};

Farm::Farm(const good::Type outGood, const Type type )
  : Factory( good::none, outGood, type, Size(3) ), _d( new Impl )
{
  _d->pictureBuilding = Picture::load( ResourceGroup::commerce, 12);  // farm building
  _d->pictureBuilding.addOffset( 30, 15);

  setPicture( _d->pictureBuilding );
  outStockRef().setCapacity( 100 );

  init();
}

bool Farm::canBuild( const CityAreaInfo& areaInfo ) const
{
  bool is_constructible = Construction::canBuild( areaInfo );
  bool on_meadow = false;

  TilesArray area = areaInfo.city->tilemap().getArea( areaInfo.pos, size() );
  foreach( tile, area )
  {
    on_meadow |= (*tile)->getFlag( Tile::tlMeadow );
  }

  const_cast< Farm* >( this )->_setError( on_meadow ? "" : _("##farm_need_farmland##") );

  return (is_constructible && on_meadow);  
}


void Farm::init()
{
  good::Type farmType = produceGoodType();
  // add subTiles in draw order
  _d->subTiles.push_back(FarmTile(farmType, TilePos( 0, 0 ) ));
  _d->subTiles.push_back(FarmTile(farmType, TilePos( 2, 2 ) ));
  _d->subTiles.push_back(FarmTile(farmType, TilePos( 1, 0 ) ));
  _d->subTiles.push_back(FarmTile(farmType, TilePos( 2, 1 ) ));
  _d->subTiles.push_back(FarmTile(farmType, TilePos( 2, 0 ) ));

  _fgPicturesRef().resize(5+1);
  computePictures();
}

void Farm::computePictures()
{
  int amount = progress();
  int percentTile;

  for (int n = 0; n<5; ++n)
  {
    if (amount >= 20)   // 20 = 100 / nbSubTiles
    {
      // this subtile is at maximum
      percentTile = 100;  // 100%
      amount -= 20;  // for next subTiles
    }
    else
    {
      // this subtile is not at maximum
      percentTile = 5 * amount;
      amount = 0;  // for next subTiles
    }
    _d->subTiles[n].computePicture(percentTile);
  }

  for (int n = 0; n<5; ++n)
  {
    _fgPicturesRef()[n] = _d->subTiles[n].getPicture();
  }
}

void Farm::timeStep(const unsigned long time)
{
  Factory::timeStep(time);

  if( game::Date::isDayChanged() && mayWork() && progress() < 100 )
  {
    computePictures();
  }
}

bool Farm::build(PlayerCityPtr city, const TilePos& pos)
{
  Factory::build( city, pos );
  computePictures();

  return true;
}

void Farm::save( VariantMap& stream ) const
{
  Factory::save( stream );
}

void Farm::load( const VariantMap& stream )
{
  Factory::load( stream );

  computePictures();
}

unsigned int Farm::getProduceQty() const
{
  return productRate() * getFinishedQty() * numberWorkers() / maximumWorkers();
}

Farm::~Farm() {}

FarmWheat::FarmWheat() : Farm(good::wheat, objects::wheatFarm)
{
}

std::string FarmWheat::troubleDesc() const
{
  city::Helper helper( _city() );

  LocustList lc = helper.find<Locust>( walker::locust, pos() );
  if( !lc.empty() )
  {
    return "##trouble_farm_was_blighted_by_locust##";
  }

  return Factory::troubleDesc();
}

bool FarmWheat::build(PlayerCityPtr pcity, const TilePos& pos)
{
  bool ret = Farm::build( pcity, pos );
  if( pcity->climate() == game::climate::central )
  {
    setProductRate( productRate() * 2 );
  }

  return ret;
}

FarmOlive::FarmOlive() : Farm(good::olive, objects::oliveFarm)
{
}

FarmGrape::FarmGrape() : Farm(good::grape, objects::grapeFarm)
{
}

FarmMeat::FarmMeat() : Farm(good::meat, objects::pigFarm)
{
}

FarmFruit::FarmFruit() : Farm(good::fruit, objects::fruitFarm)
{
}

FarmVegetable::FarmVegetable() : Farm(good::vegetable, objects::vegetableFarm)
{
}
