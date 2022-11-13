#include "Timer.h"
#include <stdlib.h>
#include <string.h>

void Timer_TimeListInit(TimeList* list)
{
	uint8_t i,j;
	for(i = 0; i < MAX_PLANS; i++)
	{
		list->plans[i].mode = NotAssigned;
		list->plans[i].day = 0;
		
		for (j = 0; j < MAX_TIMES_PER_DAY; j++)
			list->plans[i].items[j] = NULL;
	}
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
	if (offset == INVALID_SLOT || MAX_PLANS < offset)
		return;

	int i;
	for (i = 0; i < MAX_TIMES_PER_DAY; i++)
		TimePlan_RemoveItem(&list->plans[offset], i);
	
	list->plans[offset].mode = NotAssigned;
	list->plans[offset].day	=	0;
}

TimePlan* Timer_GetNextFullSlot(TimeList* list, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	uint8_t i;
	for(i = start + 1; i < MAX_PLANS + start + 2; i++)
	{
		if (!TimePlan_IsEmpty(&list->plans[i % MAX_PLANS]))
			return &list->plans[i % MAX_PLANS];
	}
	return NULL;
}

TimePlan* Timer_GetPrevFullSlot(TimeList* list, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	int8_t i;
	for(i = start - 1; i >= start - MAX_PLANS - 2; i--)
	{
		uint8_t idx = i % MAX_PLANS < 0? MAX_PLANS + (i % MAX_PLANS): i % MAX_PLANS;
		if (!TimePlan_IsEmpty(&list->plans[idx]))
			return &list->plans[idx];
	}
	return NULL;
}

uint8_t Timer_ToOffset(TimeList* list, TimePlan* plan)
{
	if (plan == NULL)
		return INVALID_SLOT;
	
	return plan - list->plans;
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
	if (offset == INVALID_SLOT || MAX_TIMES_PER_DAY < offset || plan->items[offset] == NULL)
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
	if (start == INVALID_SLOT)
		return NULL;

	uint8_t i;
	for(i = start + 1; i < MAX_TIMES_PER_DAY + start + 2; i++)
	{
		if (!TimerItem_IsEmpty(plan->items[i % MAX_TIMES_PER_DAY]))
			return plan->items[i % MAX_TIMES_PER_DAY];
	}
	return NULL;
}

TimerItem* TimePlan_GetPrevFullSlot(TimePlan* plan, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	int8_t i;
	for(i = start - 1; i >= start - MAX_TIMES_PER_DAY - 2; i--)
	{
		uint8_t idx = i % MAX_TIMES_PER_DAY < 0? MAX_TIMES_PER_DAY + (i % MAX_TIMES_PER_DAY): i % MAX_TIMES_PER_DAY;
		if (!TimerItem_IsEmpty(plan->items[idx]))
			return plan->items[idx];
	}
	return NULL;
}

uint8_t TimePlan_ToOffset(TimePlan* plan, TimerItem* item)
{
	if (item == NULL)
		return INVALID_SLOT;
	
	uint8_t offset = INVALID_SLOT;
	uint8_t i;
	for (i = 0; i < MAX_TIMES_PER_DAY; i++)
		if (item == plan->items[i])
		{
			offset = i;
			break;
		}
	
	return offset;
}

uint8_t TimePlan_IsEmpty(TimePlan* plan)
{
	return plan->mode == NotAssigned;
}

uint8_t TimerItem_IsEmpty(TimerItem* item)
{
	return item == NULL;
}
