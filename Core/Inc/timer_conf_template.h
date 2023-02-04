#ifndef TIMER_CONF_TEMPLATE
#define TIMER_CONF_TEMPLATE

#define INVALID_PLAN				255
#define INVALID_SLOT				255
#define UNINIT_STATUS				2

#define MAX_PLANS						10
#define MAX_TIMES_PER_PLAN	6

/*
Timer:
	Timer thread activated, so outputs get triggered
	Times can be set by uart or ui
*/

#define DEVICE_MODE_TIMER

#endif