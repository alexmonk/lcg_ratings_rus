#ifndef _1579D6DD_5CF7_4D4B_ADD7_F60AC8917092_
#define _1579D6DD_5CF7_4D4B_ADD7_F60AC8917092_

#include <framework/types/string.h>
#include <framework/types/vector.h>
#include <framework/types/types.h>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace my {
namespace ratings {

class Player
{
public:
	explicit Player(const string8_t& fullName);
	Player(const string8_t& firstName, const string8_t& secondName);

public:
	bool operator<(const Player& rhv) const;
	bool operator==(const Player& rhv) const;

public:
	string8_t ToString() const;

private:
	string16_t m_firstName;
	string16_t m_secondName;
};

struct Game
{
	Game(uint8_t score1, uint8_t score2)
		: m_score1(score1)
		, m_score2(score2)
	{
	}

	uint8_t m_score1;
	uint8_t m_score2;
};

struct Match
{
	Match(const string8_t& player1, const string8_t& player2)
		: m_player1(player1)
		, m_player2(player2)
	{
	}

	Player m_player1;
	Player m_player2;
	vector<Game> m_games;
};

struct Tournament
{
	string8_t m_name;
	boost::gregorian::date m_date;
	vector<string8_t> m_tags;
	vector<Match> m_matches;
	vector<Player> m_players;
	bool m_endOfSeason;
};

Tournament ReadTournament(const string8_t& filePath);

vector<Player> GetPlayers(const vector<Tournament>& tournaments);
vector<string8_t> GetActivePlayers(boost::gregorian::date_duration& timeout, const vector<Tournament>& tournaments);

} // namespace ratings
} // namespace my

#endif // _1579D6DD_5CF7_4D4B_ADD7_F60AC8917092_
