#ifndef MACHOBJS_TIMERS_H
#define MACHOBJS_TIMERS_H

#include <list>
#include <functional>
#include <algorithm>

// kernel/include
#include <Windows.h>

using namespace std;
#define INVALID_TIMER_ID	0

namespace Macho 
{
struct strTimer;
/* ************************************************************************** */
/*!
* \brief Timers manager

*/
/* ************************************************************************** */
class Timers
{
public:

	enum eTimerType { OneShotTimer, RepeatTimer };

	Timers()											{ m_uiImmediateId = INVALID_TIMER_ID; }
	virtual ~Timers()									{ killTimers(); m_uiImmediateId = INVALID_TIMER_ID; }

	// run
	void run();
	// add a timer in secs
	unsigned int addTimer( unsigned long iSecs, eTimerType type = OneShotTimer );
	
	
	// kill timer
	void killTimer( unsigned int id );
	void killTimers();
	// event
	virtual void onTimerEvent( unsigned int ) = 0;

private:
	Timers( const Timers & )							{ ; }
	Timers & operator=(const Timers&rhs)				{ ; }

	static unsigned int m_id;			// id placeholder
	unsigned int m_uiImmediateId;
	std::list<strTimer*> m_timer_list;
};

}

// for backward compatibility
using namespace Macho;

#endif // MACHOBJS_TIMERS_H
