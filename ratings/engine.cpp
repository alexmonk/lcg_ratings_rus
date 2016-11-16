#include "engine.h"
#include <framework/rtl/expect.h>
#include <framework/rtl/formatting.h>
#include <boost/foreach.hpp>

namespace my {
namespace ratings {
namespace {

void GetScore(const vector<Game>& games, uint32_t& scoreA, uint32_t& scoreB)
{
	scoreA = 0;
	scoreB = 0;
	BOOST_FOREACH(const Game& game, games)
	{
		scoreA += game.m_score1;
		scoreB += game.m_score2;
	}
}

} // namespace 

Engine::Engine(const string8_t& name, std::auto_ptr<ISystem>& system)
	: m_name(name)
	, m_system(system)
{
	m_overallSeason.reset(m_system->NewSeason().release());
	m_seasons.push_back(m_system->NewSeason().release());
}

void Engine::ProcessTournament(const Tournament& tournament)
{
	boost::scoped_ptr<ITournament> overallTournament(m_overallSeason->NewTournament(tournament.m_name));
	boost::scoped_ptr<ITournament> seasonTournament(m_seasons.back().NewTournament(tournament.m_name));

	BOOST_FOREACH(const Match& match, tournament.m_matches)
	{
		string8_t playerA = match.m_player1.ToString();
		string8_t playerB = match.m_player2.ToString();
		uint32_t scoreA = 0;
		uint32_t scoreB = 0;
		GetScore(match.m_games, scoreA, scoreB);
		overallTournament->AddMatch(playerA, playerB, scoreA, scoreB);
		seasonTournament->AddMatch(playerA, playerB, scoreA, scoreB);
	}

	overallTournament->End();
	seasonTournament->End();

	if (tournament.m_endOfSeason)
	{
		m_seasons.push_back(m_system->NewSeason().release());
	}
}

void Engine::End(const vector<string8_t>& activePlayers)
{
	string8_t oveallDir = "./ratings/" + m_name + "/overall";
	m_overallSeason->DumpActiveRating(oveallDir + "/rating_active.csv", activePlayers);
	m_overallSeason->DumpHistory(oveallDir + "/rating.csv", oveallDir + "/history.csv", oveallDir + "/players");

	if (m_seasons.size() == 1)
		return;

	for (uint32_t i = 0; i < m_seasons.size(); ++i)
	{
		string8_t seasonDir = "./ratings/" + m_name + "/season" + ToString(i + 1);
		m_seasons.at(i).DumpHistory(seasonDir + "/rating.csv", seasonDir + "/history.csv", seasonDir + "/players");
	}
}

} // namespace ratings
} // namespace my
