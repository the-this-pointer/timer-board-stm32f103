#include "Timer.h"
#include <stdlib.h>
#include <string.h>

void Timer_NewTimeList(TimeList** list)
{
	*list = malloc(sizeof(TimeList));
	if ((*list) == NULL)
		return;
	
	memset(*list, 0, sizeof(TimeList));
}

uint8_t Timer_GetEmptySlot(TimeList* list)
{
	uint8_t i;
	for(i = 0; i < MAX_PLANS; i++)
	{
		if (TimePlan_IsEmpty(&list->plans[i]))
			return i;
	}
	return INVALID_SLOT;
}

TimePlan* Timer_AddPlan(TimeList* list, enum TimeMode mode)
{
	if (mode == NotAssigned)
		return NULL;
	
	uint8_t slot = Timer_GetEmptySlot(list);
	if (slot == INVALID_SLOT)
		return NULL;

	list->plans[slot].mode = mode;
	list->plans[slot].day	=	0;
	memset(list->plans[slot].items, 0, sizeof(TimerItem*) * MAX_TIMES_PER_DAY);
	
	return &(list->plans[slot]);
}

void Timer_RemovePlan(TimeList* list, uint8_t offset)
{
	if (MAX_PLANS < offset)
		return;

	int i;
	for (i = 0; i < MAX_TIMES_PER_DAY; i++)
		TimePlan_RemoveItem(&list->plans[offset], i);
	
	list->plans[offset].mode = NotAssigned;
	list->plans[offset].day	=	0;
}

TimePlan* Timer_GetNextFullSlot(TimeList* list, uint8_t start)
{
	uint8_t i;
	for(i = start; i < MAX_PLANS; i++)
	{
		if (!TimePlan_IsEmpty(&list->plans[i]))
			return &list->plans[i];
	}
	return NULL;
}

TimePlan* Timer_GetPrevFullSlot(TimeList* list, uint8_t start)
{
	int8_t i;
	for(i = start; i >= 0; i--)
	{
		if (!TimePlan_IsEmpty(&list->plans[i]))
			return &list->plans[i];
	}
	return NULL;
}

TimerItem* TimePlan_AddItem(TimePlan* plan)
{
	uint8_t slot = TimePlan_GetEmptySlot(plan);
	if (slot == INVALID_SLOT)
		return NULL;
	
	if (plan->items[slot] == NULL)
	{
		plan->items[slot] = malloc(sizeof(TimerItem));
		if (!plan->items[slot])
			return NULL;
	}
	memset(plan->items[slot], 0, sizeof(TimerItem));
	return plan->items[slot];
}

void TimePlan_RemoveItem(TimePlan* plan, uint8_t offset)
{
	if (MAX_TIMES_PER_DAY < offset || plan->items[offset] == NULL)
		return;
	
	free(plan->items[offset]);
	plan->items[offset] = NULL;
}

uint8_t TimePlan_GetEmptySlot(TimePlan* plan)
{
	uint8_t i;
	for(i = 0; i < MAX_TIMES_PER_DAY; i++)
	{
		if (TimerItem_IsEmpty(plan->items[i]))
			return i;
	}
	return INVALID_SLOT;
}

TimerItem* TimePlan_GetNextFullSlot(TimePlan* plan, uint8_t start)
{
	uint8_t i;
	for(i = start; i < MAX_TIMES_PER_DAY; i++)
	{
		if (!TimerItem_IsEmpty(plan->items[i]))
			return plan->items[i];
	}
	return NULL;
}

TimerItem* TimePlan_GetPrevFullSlot(TimePlan* plan, uint8_t start)
{
	int8_t i;
	for(i = start; i >= 0; i--)
	{
		if (!TimerItem_IsEmpty(plan->items[i]))
			return plan->items[i];
	}
	return NULL;
}

uint8_t TimePlan_IsEmpty(TimePlan* plan)
{
	return plan->mode == NotAssigned;
}

uint8_t TimerItem_IsEmpty(TimerItem* item)
{
	return item == NULL || item->time == 0;
}
