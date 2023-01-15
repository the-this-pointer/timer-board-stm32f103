#ifndef TIMER_CONF_TEMPLATE
#define TIMER_CONF_TEMPLATE

#define INVALID_PLAN				255
#define INVALID_SLOT				255
#define UNINIT_STATUS				2

#define MAX_PLANS						10
#define MAX_TIMES_PER_PLAN	6

/* Different modes:
Timer:
	Timer thread activated
	Times can be set by uart or ui
	Outputs triggered by uart or io
Setup:
	No timer thread and outputs are not triggered
	Has ui to get times from user
	Has uart to set times on a [Timer Mode Device]
*/

#define DEVICE_MODE_TIMER		1
#define DEVICE_MODE_SETUP		2
#define DEVICE_MODE					DEVICE_MODE_TIMER

// maybe later...
/*#define SET_TIMER_MODE_UI		1
#define SET_TIMER_MODE_UART	2
#define SET_TIMER_MODE			SET_TIMER_MODE_UI

#define SAVE_TIME_MODE_EE		1
#define SAVE_TIME_MODE_UART	2
#define SAVE_TIME_MODE			SAVE_TIME_MODE_EE*/

#endif