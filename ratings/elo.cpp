#include "elo.h"
#include <framework/rtl/expect.h>
#include <framework/rtl/formatting.h>
#include <framework/system/file.h>
#include <framework/system/filesystem.h>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <math.h>

namespace my {
namespace ratings {

const uint8_t StandartPrintDigitsAfterDot = 3;

namespace {

struct PlayerHistory
{
	PlayerHistory(const Player& name, double startRating) : m_name(name), m_rating(startRating) { }

	struct PlayerMatch
	{
		PlayerMatch(double prevRating, double newRating, uint8_t score)
			: m_prevRating(prevRating)
			, m_newRating(newRating)
			, m_score(score)
		{
		}

		string8_t GetRatingText() const
		{
			double changeInRating = m_newRating - m_prevRating;
			return "(" + ToString(m_prevRating, StandartPrintDigitsAfterDot) + (changeInRating > 0 ? " +" : " ") + ToString(changeInRating, StandartPrintDigitsAfterDot) +  ")";
		}

		string8_t GetScoreText() const
		{
			return "(" + std::to_string(static_cast<_Longlong>(m_score)) + ")";
		}

		double m_prevRating;
		double m_newRating;
		uint8_t m_score;
	};

	struct Match
	{
		Match(const string8_t& tournament, const Player& opponentName, const PlayerMatch& me, const PlayerMatch& opponent)
			: m_tournament(tournament)
			, m_opponentName(opponentName)
			, m_me(me)
			, m_opponent(opponent)
		{
		}

		string8_t m_tournament;
		Player m_opponentName;
		PlayerMatch m_me;
		PlayerMatch m_opponent;
	};

	void AddMatch(const string8_t& tournament, const Player& opponentName, const PlayerMatch& myMatch, const PlayerMatch& opponentMatch)
	{
		Match match(tournament, opponentName, myMatch, opponentMatch);
		m_matches.push_back(match);
		m_rating = match.m_me.m_newRating;
	}

	Player m_name;
	vector<Match> m_matches;
	double m_rating;
};

struct Rating
{
	Rating(const Player& name, double rating)
		: m_name(name)
		, m_rating(rating)
	{
	}

	Player m_name;
	double m_rating;
};

vector<Rating> GetFinalRatings(const vector<PlayerHistory>& history)
{
	vector<Rating> result;
	BOOST_FOREACH(const PlayerHistory& playerHistory, history)
	{
		if (!playerHistory.m_matches.empty())
		{
			result.push_back(Rating(playerHistory.m_name, playerHistory.m_rating));
		}
	}
	boost::sort(result, boost::bind(&Rating::m_rating, _1) > boost::bind(&Rating::m_rating, _2));
	return result;
}

double ChangeOfRating(double myRating, double opponentRating, double score, double changeFactor, const EloSettings& settings)
{
	double ratingDifference = (opponentRating - myRating)/settings.m_logisticRatingDenominator;
	double scoreExpectation = 1./(1. + pow(settings.m_logisticPowerBase, ratingDifference));
	return changeFactor * (score - scoreExpectation);
}

std::pair<uint8_t, uint8_t> CalculateScore(const Match& match)
{
	uint8_t score1 = 0;
	uint8_t score2 = 0;
	BOOST_FOREACH(const Game& game, match.m_games)
	{
		score1 += game.m_score1;
		score2 += game.m_score2;
	}
	return std::pair<uint8_t, uint8_t>(score1, score2);
}

void PrintRatings(const vector<Rating>& ratings, const vector<Player>& players, const string8_t& filePath)
{
	string8_t text;
	size_t position = 1;
	BOOST_FOREACH(const Rating& rating, ratings)
	{
		if (boost::find(players, rating.m_name) == players.end())
			continue;

		text += ToString(position) + ", " + rating.m_name.ToString() + ", " + ToString(rating.m_rating, StandartPrintDigitsAfterDot) + "\r\n";
		++position;
	}

	system::File file(filePath, system::file_access_rights::Write, system::file_creation::CreateAlways);
	file.Write(text);
}

} // namespace

FileSettings::FileSettings(const string8_t& history, const string8_t& activeRatings, const string8_t& allRatings, const string8_t& playersHistoryDir)
	: m_history(history)
	, m_activeRatings(activeRatings)
	, m_allRatings(allRatings)
	, m_playersHistoryDir(playersHistoryDir)
{
}

FileSettings StandartFileSettings(const string8_t& rootDir, const string8_t& name)
{
	FileSettings settings(
		rootDir + "/ratings/" + name + "_history",
		rootDir + "/ratings/" + name + "_rating_active",
		rootDir + "/ratings/" + name + "_rating",
		rootDir + "/playerHistory/" + name + "/");

	system::CreateDirectory(rootDir + "/ratings");
	system::CreateDirectory(rootDir + "/playerHistory");
	system::CreateDirectory(rootDir + "/playerHistory/" + name);
	return settings;
}

EloSettings::EloSettings(double startRating, double ratingPerPoint, double logisticPowerBase, double logisticRatingDenominator)
	: m_startRating(startRating)
	, m_ratingPerPoint(ratingPerPoint)
	, m_logisticPowerBase(logisticPowerBase)
	, m_logisticRatingDenominator(logisticRatingDenominator)
{
	EXPECT(m_startRating > 0);
	EXPECT(m_ratingPerPoint > 0);
	EXPECT(m_logisticPowerBase > 0);
	EXPECT(m_logisticRatingDenominator > 0);
}

EloSettings StandartEloSettings(double ratingPerPoint)
{
	return EloSettings(1000, ratingPerPoint, 10, 400);
}

void CalculateElo(const vector<Tournament>& tournaments, const vector<Player>& activePlayers, const EloSettings& settings, const FileSettings& fileSettings)
{
	vector<PlayerHistory> history;
	vector<Player> playerNames = GetPlayers(tournaments);
	BOOST_FOREACH(const Player& playerName, playerNames)
	{
		history.push_back(PlayerHistory(playerName, settings.m_startRating));
	}

	vector<std::pair<string8_t, vector<Rating> > > historyOfRatings;
	BOOST_FOREACH(const Tournament& tournament, tournaments)
	{
		string8_t tournamentName = tournament.m_name;
		BOOST_FOREACH(const Match& match, tournament.m_matches)
		{
			Player player1 = match.m_player1;
			Player player2 = match.m_player2;
			PlayerHistory& history1 = *boost::find_if(history, boost::bind(&PlayerHistory::m_name, _1) == player1);
			PlayerHistory& history2 = *boost::find_if(history, boost::bind(&PlayerHistory::m_name, _1) == player2);
			double oldRating1 = history1.m_rating;
			double oldRating2 = history2.m_rating;
			std::pair<uint8_t, uint8_t> score = CalculateScore(match);
			double totalScore = score.first + score.second;
			double changeOfRating = ChangeOfRating(oldRating1, oldRating2, double(score.first)/totalScore, totalScore * settings.m_ratingPerPoint, settings);

			PlayerHistory::PlayerMatch match1(oldRating1, oldRating1 + changeOfRating, score.first);
			PlayerHistory::PlayerMatch match2(oldRating2, oldRating2 - changeOfRating, score.second);
			history1.AddMatch(tournamentName, player2, match1, match2);
			history2.AddMatch(tournamentName, player1, match2, match1);
		}

		historyOfRatings.push_back(std::pair<string8_t, vector<Rating> >(tournamentName, GetFinalRatings(history)));
	}

	string8_t playerHistoryHeader = "(ratingA +/- deltaA), nameA, (scoreA) - (scoreB), nameB, (ratingB +/- deltaB)\r\n";
	BOOST_FOREACH(const PlayerHistory& playerHistory, history)
	{
		string8_t text = playerHistoryHeader;
		string8_t tournamentName = "";
		BOOST_FOREACH(const PlayerHistory::Match& match, playerHistory.m_matches)
		{
			if (tournamentName != match.m_tournament)
			{
				tournamentName = match.m_tournament;
				text += tournamentName + ",,,,\r\n";
			}

			text += match.m_me.GetRatingText() + ", "
				+ playerHistory.m_name.ToString()
				+ ", " + match.m_me.GetScoreText() + " - " + match.m_opponent.GetScoreText() + ", "
				+ match.m_opponentName.ToString() + ", "
				+ match.m_opponent.GetRatingText() + "\r\n";
		}

		system::File file(fileSettings.m_playersHistoryDir + playerHistory.m_name.ToString() + ".csv", system::file_access_rights::Write, system::file_creation::CreateAlways);
		file.Write(text);
	}

	vector<Rating> ratings = GetFinalRatings(history);
	PrintRatings(ratings, playerNames, fileSettings.m_allRatings + ".csv");
	PrintRatings(ratings, activePlayers, fileSettings.m_activeRatings + ".csv");

	{
		size_t numTournaments = historyOfRatings.size();
		size_t numPlayers = historyOfRatings[numTournaments-1].second.size();

		string8_t header;
		for (size_t column = 0; column < numTournaments; ++column)
		{
			header += ", " + historyOfRatings[numTournaments-1].first;
		}

		string8_t text = header + "\r\n";
		for (size_t row = 0; row < numPlayers; ++row)
		{
			string8_t rowText = ToString(row + 1);
			for (size_t column = 0; column < numTournaments; ++column)
			{
				rowText += ", ";
				if (row < historyOfRatings[column].second.size())
				{
					const Rating& rating = historyOfRatings[column].second[row];
					rowText += rating.m_name.ToString() + ":" + ToString(rating.m_rating, StandartPrintDigitsAfterDot);
				}
			}
			text += rowText + "\r\n";
		}
		system::File file(fileSettings.m_history + ".csv", system::file_access_rights::Write, system::file_creation::CreateAlways);
		file.Write(text);
	}
}

} // namespace ratings
} // namespace my