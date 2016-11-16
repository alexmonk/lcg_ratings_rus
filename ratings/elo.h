#ifndef _8868ADD5_882C_4198_A626_9F4B5468FCE9_
#define _8868ADD5_882C_4198_A626_9F4B5468FCE9_

#include "system.h"

namespace my {
namespace ratings {

struct EloSettings
{
	EloSettings(double startRating, double changeFactor, double logisticPowerBase, double logisticRatingDenominator);

	double m_startRating;
	double m_ratingPerPoint;
	double m_logisticPowerBase;
	double m_logisticRatingDenominator;
};

EloSettings StandartEloSettings(double ratingPerPoint);

std::auto_ptr<ISystem> CreateEloSystem(const EloSettings& settings);

} // namespace ratings
} // namespace my

#endif // _8868ADD5_882C_4198_A626_9F4B5468FCE9_
