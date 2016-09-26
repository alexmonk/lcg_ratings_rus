#include <framework/rtl/expect.h>
#include <framework/types/vector.h>
#include <framework/types/string.h>
#include <framework/types/types.h>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace my {

template<typename ProbabilityType, typename ObjectType>
struct ProbabilityObject
{
	ProbabilityObject() { }
	ProbabilityObject(const ProbabilityType& probability, const ObjectType& object) : m_probability(probability), m_object(object) { }

	ProbabilityType m_probability;
	ObjectType m_object;
};

const uint32_t ByeTag = -1;

struct SwissTournament
{
	struct Score
	{
		uint32_t m_name;
		uint32_t m_score;
	};
	vector<Score> m_scores;

	struct Match
	{
		uint32_t m_first;
		uint32_t m_second;

		bool IsBye() const
		{
			return m_first == ByeTag || m_second == ByeTag;
		}
	};
	vector<Match> m_pairs;
};


class MatchResolver
{
public:
	struct Player
	{
		Player(const string8_t& name, double power) : m_name(name), m_power(power) { }

		string8_t m_name;
		double m_power;
	};

public:
	MatchResolver(const vector<Player>& players)
		: m_players(players)
	{
	}

	vector<uint32_t> GetPlayers() const
	{
		vector<uint32_t> result;
		for (size_t i = 0; i < m_players.size(); ++i)
		{
			result.push_back(i);
		}

		if (result.size() % 2)
		{
			result.push_back(ByeTag);
		}

		return result;
	}

	double ProbabilityOfWin(uint32_t winner, uint32_t loser) const
	{
		EXPECT(winner != loser);
		if (ByeTag == loser)
		{
			return 1;
		}
		else if (ByeTag == loser)
		{
			return 0;
		}

		EXPECT(winner < m_players.size());
		EXPECT(loser < m_players.size());
		return CalculateProbability(m_players[winner].m_power, m_players[loser].m_power);
	}

	double GetPower(uint32_t player) const
	{
		EXPECT(ByeTag != player);
		EXPECT(player < m_players.size());
		return m_players[player].m_power;
	}

private:
	double CalculateProbability(double winner, double loser) const
	{
		return winner/(winner + loser);
	}

private:
	vector<Player> m_players;
};

struct TournamentResult
{
	bool operator==(const TournamentResult& rhv) const
	{
		return false;
	}

	double m_maxPowerDifference;
};

TournamentResult CalculateResult(const MatchResolver& resolver, const SwissTournament& tournament)
{
	TournamentResult result;

	double maxPowerDifference = 0;
	BOOST_FOREACH(const SwissTournament::Match& match, tournament.m_pairs)
	{
		if (match.IsBye())
			continue;
		double powerDifference = std::abs(resolver.GetPower(match.m_first) - resolver.GetPower(match.m_second));
		if (powerDifference > maxPowerDifference)
		{
			maxPowerDifference = powerDifference;
		}
	}
	result.m_maxPowerDifference = maxPowerDifference;

	return result;
}

bool CreatePairings(const vector<SwissTournament::Score>& players_, const vector<SwissTournament::Match>& prevPairings, vector<vector<SwissTournament::Match> >& possiblePairings)
{
	if (players_.empty())
		return true;

	vector<SwissTournament::Score> players = players_;
	SwissTournament::Score mainPlayer = players[0];
	players.erase(players.begin());
	EXPECT(!players.empty());

	vector<uint32_t> scores;
	BOOST_FOREACH(const SwissTournament::Score& player, players)
	{
		uint32_t currentScore = player.m_score;
		if (boost::find(scores, currentScore) == scores.end())
		{
			scores.push_back(currentScore);
		}
	}

	size_t playerIt = 0;
	BOOST_FOREACH(uint32_t score, scores)
	{
		bool pairingsDone = false;
		for (; playerIt < players.size(); ++playerIt)
		{
			if (players[playerIt].m_score != score)
				break;

			vector<SwissTournament::Score> currentPlayers = players;
			currentPlayers.erase(currentPlayers.begin() + playerIt);
			vector<vector<SwissTournament::Match> > currentPossiblePairings;
			if (!CreatePairings(currentPlayers, prevPairings, currentPossiblePairings))
				continue;

			pairingsDone = true;
			SwissTournament::Match currentMatch;
			currentMatch.m_first = mainPlayer.m_name;
			currentMatch.m_second = players[playerIt].m_name;
			BOOST_FOREACH(vector<SwissTournament::Match>& newPairings, currentPossiblePairings)
			{
				newPairings.push_back(currentMatch);
				possiblePairings.push_back(newPairings);
			}
		}

		if (pairingsDone)
			break;
	}

	return !possiblePairings.empty();
}

typedef ProbabilityObject<double, SwissTournament> ProbabilityOfTournament; 
vector<ProbabilityOfTournament> RunTour(const vector<ProbabilityOfTournament>& prevTours, const MatchResolver& resolver, size_t numOfTours, const vector<SwissTournament::Match>& pairings)
{
	EXPECT(numOfTours > 0);


	/*
	BOOST_FOREACH(const ProbabilityOfTournament& prevTour, prevTours)
	{
		EXPECT(CreatePairings(prevTour.m_object.m_scores, prevTour.m_object.m_pairs, ));
	}
	*/

	vector<ProbabilityOfTournament> results;
}

typedef ProbabilityObject<double, TournamentResult> ProbabilityOfResult;
vector<ProbabilityOfResult> RunTournament(const MatchResolver& resolver, size_t numOfTours, const vector<SwissTournament::Match>& startPairings)
{
	vector<ProbabilityOfTournament> startTournament;
	startTournament.push_back(ProbabilityOfTournament(1, SwissTournament()));
	vector<ProbabilityOfTournament> results = RunTour(startTournament, resolver, numOfTours, startPairings);
	vector<ProbabilityOfResult> calculatedResults;

	double normalization = 0;
	BOOST_FOREACH(const ProbabilityOfTournament& node, results)
	{
		normalization += node.m_probability;
		TournamentResult currentResult = CalculateResult(resolver, node.m_object);
		vector<ProbabilityOfResult>::iterator it = boost::find_if(calculatedResults, boost::bind(&ProbabilityOfResult::m_object, _1) == currentResult);
		if (it == calculatedResults.end())
		{
			calculatedResults.push_back(ProbabilityOfResult(node.m_probability, currentResult));
		}
		else
		{
			it->m_probability += node.m_probability;
		}
	}

	for (vector<ProbabilityOfResult>::iterator it = calculatedResults.begin(); it != calculatedResults.end(); ++it)
	{
		it->m_probability /= normalization;
	}

	return calculatedResults;
}

} // namespace my


int main(int argc, char** argv)
{
	using namespace my;

	try
	{
		vector<MatchResolver::Player> players;
		players.push_back(MatchResolver::Player("1", 1180.980));
		players.push_back(MatchResolver::Player("20", 1042.590));
		players.push_back(MatchResolver::Player("35", 1010.190));
		players.push_back(MatchResolver::Player("58", 987.512));
		players.push_back(MatchResolver::Player("70", 981.125));
		players.push_back(MatchResolver::Player("90", 971.199));
		players.push_back(MatchResolver::Player("116", 859.498));

		MatchResolver resolver(players);
		vector<ProbabilityObject<double, TournamentResult> > results = RunTournament(resolver, 3, vector<SwissTournament::Match>());
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

    return 0;
}