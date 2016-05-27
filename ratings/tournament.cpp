#include "tournament.h"
#include <framework/rtl/expect.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

namespace my {
namespace ratings {

Player::Player(const string8_t& fullName)
{
	size_t it = fullName.find(" ");
	m_firstName = ConvertTo16(fullName.substr(0, it));
	m_secondName = ConvertTo16(fullName.substr(it+1));
	boost::replace_all(m_firstName, TEXT16("¸"), TEXT16("å"));
	boost::replace_all(m_secondName, TEXT16("¸"), TEXT16("å"));
	boost::replace_all(m_firstName, TEXT16(" "), TEXT16(""));
	boost::replace_all(m_secondName, TEXT16(" "), TEXT16(""));
}

Player::Player(const string8_t& firstName, const string8_t& secondName)
	: m_firstName(ConvertTo16(firstName))
	, m_secondName(ConvertTo16(secondName))
{
}

bool Player::operator<(const Player& rhv) const
{
	bool reverseLhv = (m_firstName < m_secondName);
	bool reverseRhv = (rhv.m_firstName < rhv.m_secondName);
	const string16_t& lhv1 = (reverseLhv ? m_secondName : m_firstName);
	const string16_t& lhv2 = (reverseLhv ? m_firstName : m_secondName);
	const string16_t& rhv1 = (reverseRhv ? rhv.m_secondName : rhv.m_firstName);
	const string16_t& rhv2 = (reverseRhv ? rhv.m_firstName : rhv.m_secondName);

	if (lhv1 == rhv1)
		return lhv2 < rhv2;
	return lhv1 < rhv1;
}

bool Player::operator==(const Player& rhv) const
{
	return (m_firstName == rhv.m_firstName && m_secondName == rhv.m_secondName) ||
		(m_firstName == rhv.m_secondName && m_secondName == rhv.m_firstName);
}

string8_t Player::ToString() const
{
	string8_t a = ConvertTo8(m_firstName);
	string8_t b = ConvertTo8(m_secondName);
	string8_t c = a + " " + b;
	return c;
}


Tournament ReadTournament(const string8_t& filePath)
{
	using namespace boost::property_tree;

	Tournament result;
	result.m_name = string8_t(boost::find_last(filePath, "/").begin() + 1, filePath.end() - 4);

	ptree xmlDocument;
	read_xml(filePath, xmlDocument);
	const ptree& root = xmlDocument.get_child("root");

	const ptree& header = root.get_child("header");
	result.m_date = boost::gregorian::from_string(header.get<string8_t>("date"));
	string8_t b = boost::gregorian::to_simple_string(result.m_date);
	BOOST_FOREACH(const ptree::value_type& city, header.get_child("location"))
	{
		result.m_locations.push_back(city.second.get<string8_t>(""));
	}
	EXPECT(!result.m_locations.empty());
	
	vector<Player> players;
	BOOST_FOREACH(const ptree::value_type& match, root.get_child("matches"))
	{
		Match currentMatch(match.second.get<string8_t>("player1"), match.second.get<string8_t>("player2"));
		players.push_back(currentMatch.m_player1);
		players.push_back(currentMatch.m_player2);
		
		BOOST_FOREACH(const ptree::value_type& game, match.second.get_child("games"))
		{
			uint8_t score1 = game.second.get<uint8_t>("score1");
			uint8_t score2 = game.second.get<uint8_t>("score2");
			currentMatch.m_games.push_back(Game(score1, score2));
		}
		result.m_matches.push_back(currentMatch);
	}

	boost::sort(players);
	boost::iterator_range<vector<Player>::iterator> uniquePlayers = boost::unique(players);
	result.m_players = vector<Player>(uniquePlayers.begin(), uniquePlayers.end());
	return result;
}

vector<Player> GetPlayers(const vector<Tournament>& tournaments)
{
	vector<Player> players;
	BOOST_FOREACH(const Tournament& tournament, tournaments)
	{
		BOOST_FOREACH(const Player& player, tournament.m_players)
		{
			if (boost::range::find(players, player) == players.end())
			{
				players.push_back(player);
			}
		}
	}
	return players;
}

vector<Player> GetActivePlayers(boost::gregorian::date_duration& timeout, const vector<Tournament>& tournaments)
{
	struct City
	{
		City(const string8_t& name, const boost::gregorian::date& date) : m_name(name), m_date(date) { }

		string8_t m_name;
		boost::gregorian::date m_date;
	};

	struct PlayerLocations
	{
		explicit PlayerLocations(const Player& name) : m_name(name) { }

		Player m_name;
		vector<City> m_cities;
	};

	vector<PlayerLocations> players;
	vector<City> lastTournaments;

	BOOST_FOREACH(const Tournament& tournament, tournaments)
	{
		const vector<Player>& currentPlayers = tournament.m_players;
		const boost::gregorian::date& currentDate = tournament.m_date;
		BOOST_FOREACH(const Player& currentPlayer, currentPlayers)
		{
			vector<PlayerLocations>::iterator playerIt = boost::find_if(players, boost::bind(&PlayerLocations::m_name, _1) == currentPlayer);
			if (playerIt == players.end())
			{
				players.push_back(PlayerLocations(currentPlayer));
				playerIt = players.end() - 1;
			}

			BOOST_FOREACH(const string8_t& city, tournament.m_locations)
			{
				vector<City>::iterator cityIt = boost::find_if(playerIt->m_cities, boost::bind(&City::m_name, _1) == city);
				if (cityIt != playerIt->m_cities.end())
				{
					*cityIt = City(city, currentDate);
				}
				else
				{
					playerIt->m_cities.push_back(City(city, currentDate));
				}
			}

			BOOST_FOREACH(const string8_t& city, tournament.m_locations)
			{
				vector<City>::iterator cityIt = boost::find_if(lastTournaments, boost::bind(&City::m_name, _1) == city);
				if (cityIt != lastTournaments.end())
				{
					*cityIt = City(city, currentDate);
				}
				else
				{
					lastTournaments.push_back(City(city, currentDate));
				}
			}
		}
	}

	vector<Player> result;
	BOOST_FOREACH(const PlayerLocations& player, players)
	{
		BOOST_FOREACH(const City& city, player.m_cities)
		{
			const boost::gregorian::date& lastTournament = boost::find_if(lastTournaments, boost::bind(&City::m_name, _1) == city.m_name)->m_date;
			if ((lastTournament - timeout) < city.m_date)
			{
				result.push_back(player.m_name);
				break;
			}
		}
	}

	return result;
}

} // namespace ratings
} // namespace my
