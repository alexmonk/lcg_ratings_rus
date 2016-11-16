#ifndef _ED54FAC2_CBA4_4029_B28D_63F45D1D1013_
#define _ED54FAC2_CBA4_4029_B28D_63F45D1D1013_

#include <framework/types/string.h>
#include <framework/types/types.h>
#include <memory>

namespace my {
namespace ratings {

struct ITournament
{
	virtual void AddMatch(const string8_t& playerA, const string8_t& playerB, uint32_t scoreA, uint32_t scoreB) = 0;
	virtual void End() = 0;

	virtual ~ITournament() { }
};

struct ISeason
{
	virtual std::auto_ptr<ITournament> NewTournament(const string8_t& name) = 0;
	virtual void DumpHistory(const string8_t& ratingFile, const string8_t& ratingHistoryFile, const string8_t& playersDir) = 0;
	virtual void DumpActiveRating(const string8_t& ratingFile, const vector<string8_t>& activePlayers) = 0;

	virtual ~ISeason() { }
};

struct ISystem
{
	virtual std::auto_ptr<ISeason> NewSeason() = 0;

	virtual ~ISystem() { }
};

} // namespace ratings
} // namespace my

#endif // _ED54FAC2_CBA4_4029_B28D_63F45D1D1013_