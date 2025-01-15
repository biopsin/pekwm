//
// Daytime.hh for pekwm
// Copyright (C) 2025 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_DAYTIME_HH_
#define _PEKWM_DAYTIME_HH_

#include <time.h>

enum TimeOfDay {
	TIME_OF_DAY_NIGHT,
	TIME_OF_DAY_DAWN,
	TIME_OF_DAY_DAY,
	TIME_OF_DAY_DUSK
};

const char *time_of_day_to_string(enum TimeOfDay tod);

/**
 * Class used to calculate sun rise and set times based on the given
 * coordinates and elevation.
 *
 * Based on the sun rise equation on Wikipedia.
 */
class Daytime {
public:
	Daytime(time_t now, double latitude, double longitude,
		double elevation=0.0);
	~Daytime();

	time_t getSunRise() const { return _sun_rise; }
	time_t getSunSet() const { return _sun_set; }
	int getDayLengthS() const { return _day_length_s; }

	enum TimeOfDay getTimeOfDay(time_t ts);

private:
	time_t _sun_rise;
	time_t _sun_set;
	int _day_length_s;
};

#endif // _PEKWM_DAYTIME_HH_
