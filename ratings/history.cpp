#include "history.h"
#include <framework/system/file.h>
#include <framework/rtl/expect.h>
#include <framework/rtl/formatting.h>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

namespace my {
namespace ratings {
namespace {

string8_t GetRatingsText(const vector<Rating>& ratings)
{
	string8_t text;
	for (size_t i = 0; i < ratings.size(); ++i)
	{
		const Rating& rating = ratings[i];
		text += ToString(i+1) + ", " + rating.player + ", " + ToString(rating.value, StandartPrintDigitsAfterDot) + "\n";
	}
	return text;
}

} // namespace 

HistoryStorage::Tournament::Tournament(HistoryStorage& storage, const string8_t& name)
	: m_storage(storage)
	, m_name(name)
{
}

void HistoryStorage::Tournament::AddRecord(const string8_t& player, const string8_t& text)
{
	AddPlayerHistory(player, text, m_playerHistory);
}

void HistoryStorage::Tournament::End(const vector<Rating>& ratings)
{
	m_storage.m_tournamentNames += ", " + m_name;
	m_storage.m_ratingsHistory.push_back(ratings);
	BOOST_FOREACH(const PlayerHistory& item, m_playerHistory)
	{
		AddPlayerHistory(item.player, m_name + ",,,,\n" + item.text, m_storage.m_playersHistory);
	}
}


void HistoryStorage::DumpHistory(const string8_t& ratingFile, const string8_t& ratingHistoryFile, const string8_t& playersDir)
{
	if (m_ratingsHistory.empty())
		return;

	system::SaveToFile(ratingFile, GetRatingsText(m_ratingsHistory.back()));

	uint32_t numTournaments = m_ratingsHistory.size();
	uint32_t numPlayers = m_ratingsHistory.back().size();
	string8_t ratingHistoryText = m_tournamentNames + "\r\n";
	for (size_t row = 0; row < numPlayers; ++row)
	{
		string8_t rowText = ToString(row + 1);
		for (size_t column = 0; column < numTournaments; ++column)
		{
			rowText += ", ";
			if (row < m_ratingsHistory[column].size())
			{
				const Rating& rating = m_ratingsHistory[column][row];
				rowText += rating.player + ":" + ToString(rating.value, StandartPrintDigitsAfterDot);
			}
		}
		ratingHistoryText += rowText + "\r\n";
	}
	system::SaveToFile(ratingHistoryFile, ratingHistoryText);

	BOOST_FOREACH(const PlayerHistory& item, m_playersHistory)
	{
		system::SaveToFile(playersDir + "/" + item.player + ".csv", "(ratingA +/- deltaA), nameA, (scoreA) - (scoreB), nameB, (ratingB +/- deltaB)\n" + item.text);
	}
}

void HistoryStorage::DumpActiveRating(const string8_t& ratingFile, const vector<string8_t>& activePlayers)
{
	if (m_ratingsHistory.empty())
		return;

	vector<Rating> activeRating;
	BOOST_FOREACH(const Rating& rating, m_ratingsHistory.back())
	{
		if (boost::find(activePlayers, rating.player) != activePlayers.end())
		{
			activeRating.push_back(rating);
		}
	}
	system::SaveToFile(ratingFile, GetRatingsText(activeRating));
}

void HistoryStorage::AddPlayerHistory(const string8_t& player, const string8_t& text, vector<HistoryStorage::PlayerHistory>& history)
{
	vector<PlayerHistory>::iterator it = boost::find_if(history, boost::bind(&PlayerHistory::player, _1) == player);
	if (it != history.end())
	{
		it->text += "\n" + text;
	}
	else
	{
		PlayerHistory playerHistory(player);
		playerHistory.text += text;
		history.push_back(playerHistory);
	}
}

} // namespace ratings
} // namespace my


