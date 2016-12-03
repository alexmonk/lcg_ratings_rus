#include <ratings.h>
#include "tournament.h"
#include "engine.h"
#include "elo.h"
#include <framework/system/filesystem.h>
#include <boost/range/algorithm/sort.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

namespace my {
namespace ratings {

void CalculateRatings(const string8_t& logDir, const string8_t& rootDir)
{
	vector<string8_t> tournamentFiles = system::ListFiles(logDir);
	vector<Tournament> tournaments;
	BOOST_FOREACH(const string8_t& fileName, tournamentFiles)
	{
		tournaments.push_back(ReadTournament(fileName));
	}
	boost::sort(tournaments, boost::bind(&Tournament::m_date, _1) < boost::bind(&Tournament::m_date, _2));
	vector<string8_t> activePlayers = my::ratings::GetActivePlayers(boost::gregorian::date_duration(183), tournaments);

	Engine elo("elo", CreateEloSystem(StandartEloSettings()));

	BOOST_FOREACH(const Tournament& tournament, tournaments)
	{
		elo.ProcessTournament(tournament);
	}

	elo.End(activePlayers);
}

} // namespace ratings
} // namespace my
