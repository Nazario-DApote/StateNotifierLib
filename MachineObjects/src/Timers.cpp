#include "Timers.h"
#include <ctime>
using namespace Macho;

struct Macho::strTimer
{ 
	strTimer( unsigned int id, Timers::eTimerType t, ULONG tmo, ULONG now )
	{
		tId = id;
		type = t;
		uiSecsTmo = tmo * 1000;
		dwStartTimeout = now;
	}

	unsigned int tId;
	Timers::eTimerType type;
	ULONG uiSecsTmo;
	ULONG dwStartTimeout;
};


unsigned int Timers::m_id = INVALID_TIMER_ID;

/* ************************************************************************** */
/*!
* \brief add a new timer
*
* \param iSecs - seconds
* \param Timers::eTimerType - one shot or continuosly
* \return - new timer id
*/
/* ************************************************************************** */
unsigned int Timers::addTimer( unsigned long iSecs, eTimerType type )
{
	if( ++m_id  == INVALID_TIMER_ID )
		++m_id;

	if( iSecs == 0 && (m_uiImmediateId == 0) )
	{
		m_uiImmediateId = m_id;
	}
	else
	{
		ULONG now = GetTickCount();
		m_timer_list.push_back( new strTimer( m_id, type, iSecs, now )  ); 
	}

	return m_id;
}


void Timers::killTimer( unsigned int id )
{
	if( id == 0 )
		return;

	std::list<strTimer*>::iterator it = m_timer_list.begin();
	std::list<strTimer*>::iterator itEnd = m_timer_list.end();
	for( ; it != itEnd; ++it )
	{
		strTimer * timer_event = *it;
		if( timer_event->tId == id )
		{
			m_timer_list.erase( it );
			delete( timer_event );
			break;
		}
	}
}

void Timers::killTimers()
{
	std::list<strTimer*>::iterator it = m_timer_list.begin();
	std::list<strTimer*>::iterator itEnd = m_timer_list.end();
	for( ; it != itEnd; ++it )
		delete (*it);
	m_timer_list.clear();
}

/* ************************************************************************** */
/*!
* \brief Run - check timers
*/
/* ************************************************************************** */
void Timers::run()
{
	if( m_uiImmediateId )
	{
		onTimerEvent( m_uiImmediateId );
		m_uiImmediateId = 0;
		return;
	}

	if( m_timer_list.empty() )
		return;

	ULONG now = GetTickCount();
	std::list<strTimer*>::iterator it = m_timer_list.begin();
	std::list<strTimer*>::iterator itEnd = m_timer_list.end();
	for( ; it != itEnd; ++it )
	{
		strTimer * timer_event = *it;
		if( (now - timer_event->dwStartTimeout ) >= timer_event->uiSecsTmo )
		{
			// if one shot
			if( timer_event->type == OneShotTimer )
			{
				unsigned int id = timer_event->tId;
				m_timer_list.erase( it );
				delete( timer_event );
				onTimerEvent( id );
			}
			// if repeat
			else
			{
				// update time
				timer_event->dwStartTimeout = now;
				onTimerEvent( timer_event->tId );
			}
			break;
		}
	}
}
