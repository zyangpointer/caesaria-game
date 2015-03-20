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

#include "cityservice_festival.hpp"
#include "game/gamedate.hpp"
#include "city.hpp"
#include "city/statistic.hpp"
#include "core/gettext.hpp"
#include "core/variant_map.hpp"
#include "religion/pantheon.hpp"
#include "events/showfeastwindow.hpp"
#include "events/updatecitysentiment.hpp"
#include "events/updatehouseservice.hpp"
#include "events/fundissue.hpp"
#include "city/funds.hpp"
#include "cityservice_factory.hpp"
#include "config.hpp"

using namespace religion;
using namespace config;
using namespace events;

namespace city
{

REGISTER_SERVICE_IN_FACTORY(Festival,festival)

namespace {
  int firstFestivalSentinment[ festival::count] = { 7, 9, 12 };
  int secondFesivalSentiment[ festival::count] = { 2, 3, 5 };

  const char* festivalTitles[ festival::count] = { "", "##small_festival##", "##middle_festival##", "##great_festival##" };
  const char* festivalDesc[ festival::count] = { "", "##small_fest_description##", "##middle_fest_description##", "##big_fest_description##" };
  const char* festivalVideo[ festival::count] = { "", "festival1_feast", "festival3_glad", "festival2_chariot" };
}

class Festival::Impl
{
public:
  festival::Info info;

  DateTime prevFestivalDate;
  DateTime lastFestivalDate;
};

SrvcPtr Festival::create( PlayerCityPtr city )
{
  SrvcPtr ret( new Festival( city ) );
  ret->drop();

  return ret;
}

std::string Festival::defaultName() {  return CAESARIA_STR_EXT(Festival); }
DateTime Festival::lastFestivalDate() const { return _d->lastFestivalDate; }
DateTime Festival::nextFestivalDate() const { return _d->festivalDate; }

void Festival::assignFestival( RomeDivinityType name, int size )
{
  _d->info.size = (festival::Type)size;
  _d->info.date= game::Date::current();
  _d->info.date.appendMonth( festival::prepareMonthsDelay + size );
  _d->info.divinity = name;

  GameEventPtr e = FundIssueEvent::create( FundIssue::sundries, -statistic::getFestivalCost( _city(), (FestivalType)size ) );
  e->dispatch();
}

Festival::Festival(PlayerCityPtr city)
: Srvc( city, defaultName() ), _d( new Impl )
{
  _d->lastFestivalDate = game::Date::current();
  _d->info.date = DateTime( -550, 0, 0 );
  _d->prevFestivalDate = DateTime( -550, 0, 0 );
}

void Festival::timeStep(const unsigned int time )
{
  if( !game::Date::isWeekChanged() )
    return;

  const DateTime currentDate = game::Date::current();
  if( _d->info.date.year() == currentDate.year()
      && _d->info.date.month() == currentDate.month() )
  {
    int sentimentValue = 0;

    if( _d->prevFestivalDate.monthsTo( currentDate ) >= DateTime::monthsInYear )
    {
      int* sentimentValues = (_d->lastFestivalDate.monthsTo( game::Date::current() ) < DateTime::monthsInYear )
                                  ? secondFesivalSentiment
                                  : firstFestivalSentinment;

      sentimentValue = sentimentValues[ _d->info.size ];
    }

    _d->prevFestivalDate = _d->lastFestivalDate;
    _d->lastFestivalDate = currentDate;
    _d->info.date = DateTime( -550, 1, 1 );

    rome::Pantheon::doFestival( _d->divinity, _d->info.size );

    int id = math::clamp<int>( _d->info.size, festival::none, festival::big );
    GameEventPtr e = ShowFeastival::create( _(festivalDesc[ id ]), _(festivalTitles[ id ]),
                                            _city()->mayor()->name(), festivalVideo[ id ] );
    e->dispatch();

    e = UpdateCitySentiment::create( sentimentValue );
    e->dispatch();

    e = UpdateHouseService::create( Service::crime, -firstFestivalSentinment[ _d->info.size ] );
    e->dispatch();
  }
}

VariantMap Festival::save() const
{
  VariantMap ret;
  VARIANT_SAVE_ANY_D( ret, _d, lastFestivalDate )
  VARIANT_SAVE_ANY_D( ret, _d, prevFestivalDate )
  VARIANT_SAVE_ANY_D( ret, _d, festivalDate )
  VARIANT_SAVE_ENUM_D( ret, _d, divinity )
  VARIANT_SAVE_ENUM_D( ret, _d, festivalType )

  return ret;
}

void Festival::load( const VariantMap& stream)
{
  VARIANT_LOAD_TIME_D( _d, lastFestivalDate, stream )
  VARIANT_LOAD_TIME_D( _d, prevFestivalDate, stream )
  VARIANT_LOAD_TIME_D( _d, festivalDate, stream )
  VARIANT_LOAD_ENUM_D( _d, divinity, stream )
  VARIANT_LOAD_ENUM_D( _d, festivalType, stream )
}

}//end namespace city
