#ifndef _1CA23567_62D7_402E_9287_33FBB773C2ED_
#define _1CA23567_62D7_402E_9287_33FBB773C2ED_

#include "tournament.h"

namespace my {
namespace ratings {

struct FileSettings
{
	FileSettings(const string8_t& history, const string8_t& activeRatings, const string8_t& allRatings, const string8_t& playersHistoryDir);

	string8_t m_history;
	string8_t m_activeRatings;
	string8_t m_allRatings;
	string8_t m_playersHistoryDir;
};

FileSettings StandartFileSettings(const string8_t& rootDir, const string8_t& name);

struct EloSettings
{
	EloSettings(double startRating, double changeFactor, double logisticPowerBase, double logisticRatingDenominator);

	double m_startRating;
	double m_changeFactor;
	double m_logisticPowerBase;
	double m_logisticRatingDenominator;
};

EloSettings StandartEloSettings();

void CalculateElo(const vector<Tournament>& tournaments, const vector<Player>& activePlayers, const EloSettings& settings, const FileSettings& fileSettings);

} // namespace ratings
} // namespace my

#endif // _1CA23567_62D7_402E_9287_33FBB773C2ED_
