/*
 * port_delay.c
 *
 *  Created on: Aug 13, 2024
 *      Author: guirespi
 */
#include <stdlib.h>
#include "api_delay.h"
#include "port_delay.h"

port_delay_hdle port_delay_init(uint32_t ms)
{
	delay_t * delay = malloc(sizeof(*delay));
	delay_init(delay, ms);
	return delay;
}

bool port_delay_read(port_delay_hdle delay_hdle)
{
	if(delay_hdle == NULL) return false;
	return delay_read((delay_t *) delay_hdle);
}

void port_delay_deinit(port_delay_hdle * delay_hdle)
{
	if(delay_hdle == NULL) return;
	if(*delay_hdle)
	{
		free(*delay_hdle);
		*delay_hdle = NULL;
	}
}
