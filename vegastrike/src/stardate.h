#ifndef __STARDATE_H
#define __STARDATE_H

#include <string>

#define HOURS_DIV 8

/*
 * Stardate class
 * Full date format is days.hours.minutes:seconds
 * Short date format is days.hours.minutes
 * Compact date format is days.hours
 */

class StarDate
{
	private:
		double *		initial_star_time;
		double			initial_time;


		// TREK Date stuff
		std::string	ConvertTrekDate( double date);
		std::string	ConvertFullTrekDate( double date);
		double		ConvertTrekDate( const std::string & date);
		float		GetFloatFromTrekDate( int faction=0);
	public:
		StarDate();
		StarDate( double time);
		void	Init( double time);
		double	GetCurrentStarTime( int faction=0);

		// TREK Date stuff
		void		InitTrek( const std::string & date);
		std::string	GetTrekDate( int faction=0);
		std::string	GetFullTrekDate( int faction=0);

		// DAN.A StarDate
		void		InitSDate( const std::string & date);
		std::string	GetSDate( int faction=0);
		std::string	GetFullSDate( int faction=0);

		// Between date format conversion
		std::string	SDateFromTrekDate( const std::string & trekdate);
		std::string	TrekDateFromSDate( const std::string & sdate);
};

#endif

