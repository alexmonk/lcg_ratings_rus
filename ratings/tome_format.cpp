#include <tome_format.h>
#include <framework/system/file.h>
#include <framework/types/vector.h>
#include <framework/types/types.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>

namespace my {
namespace ratings {

namespace {

struct Player
{
	Player(const string8_t& name, const string8_t& tag) : m_name(name), m_tag(tag) { }

	string8_t m_name;
	string8_t m_tag;
};

struct Match
{
	struct Player
	{
		Player(const string8_t& tag, string8_t points) : m_tag(tag), m_points(points) { }

		string8_t m_tag;
		string8_t m_points;
	};

	Match(const string8_t& matchTag, const string8_t& playerTag, string8_t points)
		: m_tag(matchTag)
	{
		m_players.push_back(Player(playerTag, points));
	}

	string8_t m_tag;
	vector<Player> m_players;
};

} // namespace 

void ConvertTomeLog(const string8_t& input, const string8_t& output)
{
	string8_t dateText = input.substr(input.find('/') + 1);
	boost::gregorian::date date = boost::gregorian::from_string(dateText);

	using namespace boost::property_tree;

	ptree jsonDocument;
	read_json(input, jsonDocument);
	const ptree& root = jsonDocument.get_child("entityGroupMap");
	const ptree& participants = root.get_child("Participant:#.entities");
	const ptree& matchParticipants = root.get_child("MatchParticipant:#.entities");

	vector<Player> players;
	BOOST_FOREACH(const ptree::value_type& participant, participants)
	{
		string8_t name = participant.second.get<string8_t>("last_name") + " " + participant.second.get<string8_t>("first_name");
		string8_t tag = participant.second.get<string8_t>("pk");
		players.push_back(Player(name, tag));
	}

	vector<Match> matches;
	BOOST_FOREACH(const ptree::value_type& matchParticipant, matchParticipants)
	{
		string8_t matchTag = matchParticipant.second.get<string8_t>("match_pk");
		string8_t playerTag = matchParticipant.second.get<string8_t>("participant_pk");
		string8_t points = matchParticipant.second.get<string8_t>("points_earned");

		vector<Match>::iterator it = boost::find_if(matches, boost::bind(&Match::m_tag, _1) == matchTag);
		if (it == matches.end())
		{
			matches.push_back(Match(matchTag, playerTag, points));
		}
		else
		{
			it->m_players.push_back(Match::Player(playerTag, points));
		}
	}
	matches.erase(boost::remove_if(matches, boost::bind(&vector<Match::Player>::size, boost::bind(&Match::m_players, _1)) < size_t(2)), matches.end());

	string8_t outputText = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	outputText += "<root>\n";

	outputText += "<header>\n";
	outputText += "\t<date>" + boost::gregorian::to_iso_extended_string(date) + "</date>\n";
	outputText += "\t<location>\n";
	outputText += "\t\t<city>Msk</city>\n";
	outputText += "\t</location>\n";
	outputText += "</header>\n";

	outputText += "<matches>\n";
	BOOST_FOREACH(const Match& match, matches)
	{
		outputText += "\t<match>\n";
		string8_t player1 = boost::find_if(players, boost::bind(&Player::m_tag, _1) == match.m_players[0].m_tag)->m_name;
		string8_t player2 = boost::find_if(players, boost::bind(&Player::m_tag, _1) == match.m_players[1].m_tag)->m_name;
		outputText += "\t\t<player1>" + player1 + "</player1>\n";
		outputText += "\t\t<player2>" + player2 + "</player2>\n";
		outputText += "\t\t<games>\n";
		outputText += "\t\t\t<game><score1>" + match.m_players[0].m_points + "</score1><score2>" + match.m_players[1].m_points + "</score2></game>\n";
		outputText += "\t\t</games>\n";
		outputText += "\t</match>\n";
	}
	outputText += "</matches>\n";
	outputText += "</root>";

	my::system::File outputFile(output, my::system::file_access_rights::Write, my::system::file_creation::CreateAlways);
	outputFile.Write(outputText);
}

} // namespace ratings
} // namespace my
