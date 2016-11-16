#ifndef _0256A281_BA44_4084_8456_86F59B8A38BF_
#define _0256A281_BA44_4084_8456_86F59B8A38BF_

#include "basic.h"
#include <framework/types/string.h>
#include <framework/types/vector.h>

namespace my {
namespace ratings {

class HistoryStorage
{
private:
	struct PlayerHistory
	{
		explicit PlayerHistory(const string8_t& player_) : player(player_) { }

		string8_t player;
		string8_t text;
	};

public:
	class Tournament
	{
	public:
		explicit Tournament(HistoryStorage& storage, const string8_t& name);

	public:
		void AddRecord(const string8_t& player, const string8_t& text);
		void End(const vector<Rating>& ratings);

	private:
		HistoryStorage& m_storage;
		const string8_t m_name;
		vector<PlayerHistory> m_playerHistory;
	};

public:
	void DumpHistory(const string8_t& ratingFile, const string8_t& ratingHistoryFile, const string8_t& playersDir);
	void DumpActiveRating(const string8_t& ratingFile, const vector<string8_t>& activePlayers);

private:
	static
	void AddPlayerHistory(const string8_t& player, const string8_t& text, vector<HistoryStorage::PlayerHistory>& history);

private:
	vector<vector<Rating> > m_ratingsHistory;
	vector<PlayerHistory> m_playersHistory;
	string8_t m_tournamentNames;
};

} // namespace ratings
} // namespace my

#endif // _0256A281_BA44_4084_8456_86F59B8A38BF_
