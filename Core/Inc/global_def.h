#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

#include <stdint.h>
#include <math.h>
#include "timer_conf.h"

enum TimeMode {
	NotAssigned,
	Daily,
	Weekly,
	Monthly
};

typedef struct timer_item {
	uint8_t hours;
	uint8_t minutes;
	uint8_t status;
} TimerItem;

typedef struct time_plan {
	enum TimeMode mode;
	uint8_t day;		// used in monthly and weekly

	TimerItem items[MAX_TIMES_PER_PLAN];
} TimePlan;

typedef struct time_list {
	TimePlan plans[MAX_PLANS];
} TimeList;

typedef struct eeprom_timelist_data {
	uint16_t maxPlans;
	uint16_t maxTimePerPlan;
	TimeList timelist;
} EEPROM_TimeList;

#endif // GLOBAL_DEF_H