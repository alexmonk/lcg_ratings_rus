#include <ratings.h>
#include <tome_format.h>
#include <framework/system/file.h>
#include <framework/system/filesystem.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <iostream>

string8_t GameToString(uint8_t score1, uint8_t score2)
{
	if (score1 == 10)
		return "<score1>2</score1><score2>0</score2>";
	if (score2 == 10)
		return "<score1>0</score1><score2>2</score2>";

	if (score1 == score2)
		return "<score1>1</score1><score2>1</score2>";

	if (score1 > score2)
		return "<score1>1</score1><score2>0</score2>";
	else
		return "<score1>0</score1><score2>1</score2>";
}

string8_t TableToString(const boost::property_tree::ptree& root)
{
	using namespace boost::property_tree;

	string8_t text;
	BOOST_FOREACH(const ptree::value_type& game, root)
	{
		if (game.first != "Game" && !boost::starts_with(game.first, "Final_"))
			continue;
		if (game.second.get<bool>("IsBYE"))
			continue;

		text += "\t<match>\n";
		text += "\t\t<player1>" + game.second.get<string8_t>("Player1Alias") + "</player1>\n";
		text += "\t\t<player2>" + game.second.get<string8_t>("Player2Alias") + "</player2>\n";

		uint8_t score1;
		uint8_t score2;
		text += "\t\t<games>\n";
		score1 = game.second.get<uint8_t>("Player1Score1");
		score2 = game.second.get<uint8_t>("Player2Score1");
		if (score1 != 0 || score2 != 0)
		{
			text += "\t\t\t<game>" + GameToString(score1, score2) + "</game>\n";
		}

		score1 = game.second.get<uint8_t>("Player1Score2");
		score2 = game.second.get<uint8_t>("Player2Score2");
		if (!game.second.get<bool>("IsSecondGameNotStarted") && (score1 != 0 || score2 != 0))
		{
			text += "\t\t\t<game>" + GameToString(score1, score2) + "</game>\n";
		}
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
	text += "\t<tags>\n";
	text += "\t\t<tag>Msk</tag>\n";
	text += "\t</tags>\n";
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

	string8_t playoff = TableToString(root.get_child("Playoffs16"));
	if (playoff.empty())
	{
		playoff = TableToString(root.get_child("Playoffs8"));
	}
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
		if (output.find(".ant") != string8_t::npos)
		{
			boost::replace_all(output, ".ant", ".xml");
			ConvertLog(input, output);
		}
		else
		{
			boost::replace_all(output, ".txt", ".xml");
			my::ratings::ConvertTomeLog(input, output);
		}

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
		my::ratings::CalculateRatings(logDir, rootDir);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

    return 0;
}
