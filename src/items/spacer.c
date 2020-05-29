/*
 * LavaLauncher - A simple launcher panel for Wayland
 *
 * Copyright (C) 2020 Leon Henrik Plickat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define _POSIX_C_SOURCE 200809L

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#include"lavalauncher.h"
#include"items/item.h"

bool create_spacer (struct Lava_data *data)
{
	struct Lava_item *new_spacer = calloc(1, sizeof(struct Lava_item));
	if ( new_spacer == NULL )
	{
		fprintf(stderr, "ERROR: Could not allocate.\n");
		return false;
	}

	item_nullify(new_spacer);
	new_spacer->type = TYPE_SPACER;

	data->last_item = new_spacer;
	wl_list_insert(&data->items, &new_spacer->link);
	return true;
}

static bool spacer_set_length (struct Lava_item *spacer, const char *length)
{
	int len = atoi(length);
	if ( len <= 0 )
	{
		fputs("ERROR: Spacer size must be greater than 0.\n", stderr);
		return false;
	}
	spacer->length = (unsigned int)len;
	return true;
}

struct
{
	const char *variable;
	bool (*set)(struct Lava_item*, const char*);
} spacer_configs[] = {
	{ .variable = "length", .set = spacer_set_length }
};

bool spacer_set_variable (struct Lava_data *data, const char *variable,
		const char *value, int line)
{
	for (size_t i = 0; i < (sizeof(spacer_configs) / sizeof(spacer_configs[0])); i++)
		if (! strcmp(spacer_configs[i].variable, variable))
			return spacer_configs[i].set(data->last_item, value);
	fprintf(stderr, "ERROR: Unrecognized spacer setting \"%s\": "
			"line=%d\n", variable, line);
	return false;
}
