#include <ratings.h>
#include <framework/system/file.h>
#include <framework/system/filesystem.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <iostream>

string8_t GameToString(const string8_t& score)
{
	const string8_t Prefix = "Score_";
	string8_t score1 = score.substr(Prefix.size(), 1);
	string8_t score2 = score.substr(Prefix.size() + 2, 1);
	return "<score1>" + score1 + "</score1><score2>" + score2 + "</score2>";
}

string8_t TableToString(const boost::property_tree::ptree& root)
{
	using namespace boost::property_tree;

	string8_t text;
	BOOST_FOREACH(const ptree::value_type& game, root)
	{
		if (!boost::starts_with(game.first, "Game") && !boost::starts_with(game.first, "Final"))
			continue;

		string8_t score = game.second.get<string8_t>("Score");
		if (score == "Score_BYE")
			continue;

		text += "\t<match>\n";
		text += "\t\t<player1>" + game.second.get<string8_t>("Player1Alias") + "</player1>\n";
		text += "\t\t<player2>" + game.second.get<string8_t>("Player2Alias") + "</player2>\n";

		text += "\t\t<games>\n";
		text += "\t\t\t<game>" + GameToString(score) + "</game>\n";
		text += "\t\t</games>\n";
		text += "\t</match>\n";
	}

	return text;
}

void ConvertLog(const string8_t& input, const string8_t& output)
{
	using namespace boost::property_tree;

	ptree xmlDocument;
	read_xml(input, xmlDocument);
	const ptree& root = xmlDocument.get_child("Tournament");
	string8_t date = root.get<string8_t>("Date");
	date = date.substr(0, date.find("T"));

	string8_t text = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	text += "<root>\n";

	text += "<header>\n";
	text += "\t<date>" + date + "</date>\n";
	text += "\t<location>\n";
	text += "\t\t<city>Msk</city>\n";
	text += "\t</location>\n";
	text += "</header>\n";

	text += "<matches>\n";
	BOOST_FOREACH(const ptree::value_type& round, root.get_child("Rounds"))
	{
		string8_t roundText = TableToString(round.second.get_child("Games"));
		if (!roundText.empty())
		{
			text += roundText;
		}
	}

	string8_t playoff = TableToString(root.get_child("Playoffs"));
	if (!playoff.empty())
	{
		text += "\n" + playoff;
	}

	text += "</matches>\n";
	text += "</root>";

	my::system::File file(output, my::system::file_access_rights::Write, my::system::file_creation::CreateAlways);
	file.Write(text);
}

void ConvertLogs(const string8_t& rawLogDir, const string8_t& logDir, const string8_t& rawLogBackuoDir)
{
	vector<string8_t> inputFiles = my::system::ListFiles(rawLogDir);
	BOOST_FOREACH(const string8_t& input, inputFiles)
	{
		string8_t output = input;
		boost::replace_all(output, rawLogDir, logDir);
		boost::replace_all(output, ".wht", ".xml");
		ConvertLog(input, output);

		output = input;
		boost::replace_all(output, rawLogDir, rawLogBackuoDir);
		my::system::CopyFile(input, output);
	}
}

int main()
{
	try
	{
		string8_t rawLogDir = "raw_logs";
		string8_t rawLogBackupDir = "raw_logs_backup";
		string8_t logDir = "logs";
		string8_t rootDir = ".";

		ConvertLogs(rawLogDir, logDir, rawLogBackupDir);
		my::ratings::CalculateRatings(logDir, rootDir, 2);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

    return 0;
}
