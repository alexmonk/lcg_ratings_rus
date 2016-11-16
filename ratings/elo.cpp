#include "elo.h"
#include "history.h"
#include <framework/rtl/expect.h>
#include <framework/rtl/formatting.h>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

namespace my {
namespace ratings {
namespace {

class RatingStorage
{
public:
	explicit RatingStorage(double startRating) : m_startRating(startRating) { }

public:
	double Get(const string8_t& player)
	{
		return GetInternal(player);
	}

	void Set(const string8_t& player, double newRating)
	{
		GetInternal(player) = newRating;
	}

	vector<Rating> GetRatings()
	{
		vector<Rating> result = m_ratings;
		std::sort(result.begin(), result.end(), boost::bind(&Rating::value, _1) > boost::bind(&Rating::value, _2));
		return result;
	}

private:
	double& GetInternal(const string8_t& player)
	{
		vector<Rating>::iterator it = boost::find_if(m_ratings, boost::bind(&Rating::player, _1) == player);
		if (it != m_ratings.end())
		{
			return it->value;
		}
		else
		{
			Rating newRating;
			newRating.player = player;
			newRating.value = m_startRating;
			m_ratings.push_back(newRating);
			return m_ratings.back().value;
		}
	}

private:
	const double m_startRating;
	vector<Rating> m_ratings;
};

double ChangeOfRating(double myRating, double opponentRating, double score, double changeFactor, const EloSettings& settings)
{
	double ratingDifference = (opponentRating - myRating)/settings.m_logisticRatingDenominator;
	double scoreExpectation = 1./(1. + pow(settings.m_logisticPowerBase, ratingDifference));
	return changeFactor * (score - scoreExpectation);
}

string8_t RatingChangeToString(double prevRating, double changeInRating)
{
	return "(" + ToString(prevRating, StandartPrintDigitsAfterDot) + (changeInRating > 0 ? " +" : " ") + ToString(changeInRating, StandartPrintDigitsAfterDot) +  ")";
}

} // namespace

class EloTournament: public ITournament
{
public:
	explicit EloTournament(const EloSettings& settings, RatingStorage& ratings, std::auto_ptr<HistoryStorage::Tournament>& tournamentHistory)
		: m_settings(settings)
		, m_ratings(ratings)
		, m_tournamentHistory(tournamentHistory)
	{
	}

public:
	void AddMatch(const string8_t& playerA, const string8_t& playerB, uint32_t scoreA, uint32_t scoreB)
	{
		double ratingA = m_ratings.Get(playerA);
		double ratingB = m_ratings.Get(playerB);
		double totalScore = scoreA + scoreB;
		double changeOfRating = ChangeOfRating(ratingA, ratingB, double(scoreA)/totalScore, totalScore * m_settings.m_ratingPerPoint, m_settings);
		string8_t ratingTextA = RatingChangeToString(ratingA, changeOfRating);
		string8_t ratingTextB = RatingChangeToString(ratingB, -changeOfRating);
		m_tournamentHistory->AddRecord(playerA, ratingTextA + ", " + playerA + ", " + "(" + ToString(scoreA) + ") - (" + ToString(scoreB) + ")" + ", " + playerB + ", " + ratingTextB);
		m_tournamentHistory->AddRecord(playerB, ratingTextB + ", " + playerB + ", " + "(" + ToString(scoreB) + ") - (" + ToString(scoreA) + ")" + ", " + playerA + ", " + ratingTextA);
		m_ratings.Set(playerA, ratingA + changeOfRating);
		m_ratings.Set(playerB, ratingB - changeOfRating);
	}

	void End()
	{
		m_tournamentHistory->End(m_ratings.GetRatings());
	}

private:
	const EloSettings& m_settings;
	RatingStorage& m_ratings;
	boost::scoped_ptr<HistoryStorage::Tournament> m_tournamentHistory;
};

class EloSeason: public ISeason
{
public:
	explicit EloSeason(const EloSettings& settings)
		: m_settings(settings)
		, m_ratings(settings.m_startRating)
	{
	}

public:
	std::auto_ptr<ITournament> NewTournament(const string8_t& name)
	{
		return std::auto_ptr<ITournament>(new EloTournament(m_settings, m_ratings, std::auto_ptr<HistoryStorage::Tournament>(new HistoryStorage::Tournament(m_history, name))));
	}

	void DumpHistory(const string8_t& ratingFile, const string8_t& ratingHistoryFile, const string8_t& playersDir)
	{
		m_history.DumpHistory(ratingFile, ratingHistoryFile, playersDir);
	}

	void DumpActiveRating(const string8_t& ratingFile, const vector<string8_t>& activePlayers)
	{
		m_history.DumpActiveRating(ratingFile, activePlayers);
	}

private:
	const EloSettings m_settings;
	HistoryStorage m_history;
	RatingStorage m_ratings;
};

class EloSystem: public ISystem
{
public:
	explicit EloSystem(const EloSettings& settings) : m_settings(settings) { }

public:
	std::auto_ptr<ISeason> NewSeason()
	{
		return std::auto_ptr<ISeason>(new EloSeason(m_settings));
	}

private:
	const EloSettings m_settings;
};

std::auto_ptr<ISystem> CreateEloSystem(const EloSettings& settings)
{
	return std::auto_ptr<ISystem>(new EloSystem(settings));
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

} // namespace ratings
} // namespace my
