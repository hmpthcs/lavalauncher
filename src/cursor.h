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

#ifndef LAVALAUNCHER_CURSOR_H
#define LAVALAUNCHER_CURSOR_H

#include<stdbool.h>
#include<stdint.h>
#include<wayland-client.h>

struct Lava_data;

struct Lava_cursor
{
	char                   *name;
	struct wl_cursor_theme *theme;
	struct wl_cursor_image *image;
	struct wl_surface      *surface;
};

bool init_cursor (struct Lava_data *data);
void finish_cursor (struct Lava_data *data);
void attach_cursor (struct Lava_data *data, struct wl_pointer *wl_pointer,
		uint32_t serial);

#endif

