#ifndef _4E236028_7119_4D20_B2CA_EE054744F7B1_
#define _4E236028_7119_4D20_B2CA_EE054744F7B1_

#include "system.h"
#include "tournament.h"
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace my {
namespace ratings {

class Engine
{
public:
	explicit Engine(const string8_t& name, std::auto_ptr<ISystem>& system);

public:
	void ProcessTournament(const Tournament& tournament);
	void End(const vector<string8_t>& activePlayers);

private:
	const string8_t m_name;
	boost::scoped_ptr<ISystem> m_system;
	boost::scoped_ptr<ISeason> m_overallSeason;
	boost::ptr_vector<ISeason> m_seasons;
};

} // namespace ratings
} // namespace my

#endif // _4E236028_7119_4D20_B2CA_EE054744F7B1_
