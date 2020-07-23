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

#ifndef LAVALAUNCHER_LAVALAUNCHER_H
#define LAVALAUNCHER_LAVALAUNCHER_H

#include<stdbool.h>
#include<stdint.h>
#include<wayland-server.h>

struct Lava_item;
struct Lava_bar_pattern;

struct Lava_data
{
	struct wl_display             *display;
	struct wl_registry            *registry;
	struct wl_compositor          *compositor;
	struct wl_subcompositor       *subcompositor;
	struct wl_shm                 *shm;
	struct zwlr_layer_shell_v1    *layer_shell;
	struct zxdg_output_manager_v1 *xdg_output_manager;

	struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
	struct wl_list toplevels;

	char config_path[1024];

	struct wl_list patterns;
	struct wl_list outputs;
	struct wl_list seats;

	struct Lava_bar_pattern *last_pattern;

	bool loop;
	bool reload;
	int  verbosity;
	int  ret;

#ifdef WATCH_CONFIG
	bool watch;
#endif
};

#endif
