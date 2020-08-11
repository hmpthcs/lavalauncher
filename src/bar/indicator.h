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

#ifndef LAVALAUNCHER_BAR_INDICATOR_H
#define LAVALAUNCHER_BAR_INDICATOR_H

#include"types/buffer.h"

struct Lava_seat;
struct Lava_bar;
struct Lava_item;
struct Lava_colour;

struct Lava_item_indicator
{
	struct wl_list link;

	struct Lava_seat *seat;
	struct Lava_bar  *bar;

	struct wl_surface    *indicator_surface;
	struct wl_subsurface *indicator_subsurface;
	struct Lava_buffer    indicator_buffers[2];
	struct Lava_buffer   *current_indicator_buffer;
};

void destroy_indicator (struct Lava_item_indicator *indicator);
struct Lava_item_indicator *create_indicator (struct Lava_bar *bar);
void move_indicator (struct Lava_item_indicator *indicator, struct Lava_item *item);
void indicator_set_colour (struct Lava_item_indicator *indicator, struct Lava_colour *colour);
void indicator_commit (struct Lava_item_indicator *indicator);

#endif
