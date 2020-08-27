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

#include<wayland-server.h>
#include<wayland-client.h>
#include<wayland-client-protocol.h>

#include"wlr-layer-shell-unstable-v1-protocol.h"
#include"xdg-output-unstable-v1-protocol.h"
#include"xdg-shell-protocol.h"

#include"lavalauncher.h"
#include"log.h"
#include"output.h"
#include"seat.h"
#include"bar/bar-pattern.h"
#include"bar/bar.h"

uint32_t get_anchor (struct Lava_bar_pattern *pattern)
{
	struct {
		uint32_t anchor_triplet;
		uint32_t mode_simple_center_align;
		uint32_t mode_simple_start_align;
		uint32_t mode_simple_end_align;
	} edges[4] = {
		[POSITION_TOP] = {
			.anchor_triplet  = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
			.mode_simple_center_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
			.mode_simple_start_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
			.mode_simple_end_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
		},
		[POSITION_RIGHT] = {
			.anchor_triplet  = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
			.mode_simple_center_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
			.mode_simple_start_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
			.mode_simple_end_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
		},
		[POSITION_BOTTOM] = {
			.anchor_triplet  = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
			.mode_simple_center_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
			.mode_simple_start_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
			.mode_simple_end_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
		},
		[POSITION_LEFT] = {
			.anchor_triplet  = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
			.mode_simple_center_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
			.mode_simple_start_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
			.mode_simple_end_align = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
				| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
		},
	};

	if ( pattern->mode == MODE_SIMPLE ) switch (pattern->alignment)
	{
		case ALIGNMENT_CENTER: return edges[pattern->position].mode_simple_center_align;
		case ALIGNMENT_START:  return edges[pattern->position].mode_simple_start_align;
		case ALIGNMENT_END:    return edges[pattern->position].mode_simple_end_align;
	}
	return edges[pattern->position].anchor_triplet;
}

void configure_layer_surface (struct Lava_bar *bar)
{
	struct Lava_data        *data    = bar->data;
	struct Lava_bar_pattern *pattern = bar->pattern;

	log_message(bar->data, 1, "[bar] Configuring bar: global_name=%d\n",
			bar->output->global_name);

	zwlr_layer_surface_v1_set_size(bar->layer_surface,
			(uint32_t)bar->buffer_width, (uint32_t)bar->buffer_height);

	/* Anchor the surface to the correct edge. */
	zwlr_layer_surface_v1_set_anchor(bar->layer_surface, get_anchor(pattern));

	if ( pattern->mode == MODE_SIMPLE )
		zwlr_layer_surface_v1_set_margin(bar->layer_surface,
				(int32_t)pattern->margin_top, (int32_t)pattern->margin_right,
				(int32_t)pattern->margin_bottom, (int32_t)pattern->margin_left);
	else if ( pattern->orientation == ORIENTATION_HORIZONTAL )
		zwlr_layer_surface_v1_set_margin(bar->layer_surface,
				(int32_t)pattern->margin_top, 0,
				(int32_t)pattern->margin_bottom, 0);
	else
		zwlr_layer_surface_v1_set_margin(bar->layer_surface,
				0, (int32_t)pattern->margin_right,
				0, (int32_t)pattern->margin_left);

	/* Set exclusive zone to prevent other surfaces from obstructing ours. */
	int32_t exclusive_zone;
	if ( pattern->exclusive_zone == 1 )
	{
		if ( pattern->orientation == ORIENTATION_HORIZONTAL )
			exclusive_zone = (int32_t)bar->buffer_height;
		else
			exclusive_zone = (int32_t)bar->buffer_width;
	}
	else
		exclusive_zone = pattern->exclusive_zone;
	zwlr_layer_surface_v1_set_exclusive_zone(bar->layer_surface,
			exclusive_zone);

	/* Create a region of the visible part of the surface.
	 * Behold: In MODE_DEFAULT, the actual surface is larger than the visible
	 * bar.
	 */
	struct wl_region *region = wl_compositor_create_region(data->compositor);
	wl_region_add(region, (int32_t)bar->bar_x, (int32_t)bar->bar_y,
			(int32_t)bar->bar_width, (int32_t)bar->bar_height);

	/* Set input region. This is necessary to prevent the unused parts of
	 * the surface to catch pointer and touch events.
	 */
	wl_surface_set_input_region(bar->bar_surface, region);

	wl_region_destroy(region);
}


static void layer_surface_handle_configure (void *raw_data,
		struct zwlr_layer_surface_v1 *surface, uint32_t serial,
		uint32_t w, uint32_t h)
{
	struct Lava_bar  *bar  = (struct Lava_bar *)raw_data;
	struct Lava_data *data = bar->data;

	log_message(data, 1, "[bar] Layer surface configure request: global_name=%d w=%d h=%d serial=%d\n",
			bar->output->global_name, w, h, serial);

	bar->configured = true;
	zwlr_layer_surface_v1_ack_configure(surface, serial);
	update_bar(bar);
}

static void layer_surface_handle_closed (void *data,
		struct zwlr_layer_surface_v1 *surface)
{
	struct Lava_bar *bar = (struct Lava_bar *)data;
	log_message(bar->data, 1, "[bar] Layer surface has been closed: global_name=%d\n",
				bar->output->global_name);
	destroy_bar(bar);
}

const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_handle_configure,
	.closed    = layer_surface_handle_closed
};

void configure_subsurface (struct Lava_bar *bar)
{
	struct Lava_data        *data    = bar->data;

	log_message(bar->data, 1, "[bar] Configuring icons: global_name=%d\n",
			bar->output->global_name);

	wl_subsurface_set_position(bar->subsurface, (int32_t)bar->item_area_x, (int32_t)bar->item_area_y);

	// TODO it probably makes more sense to use the input events of the subsurface
	struct wl_region *region = wl_compositor_create_region(data->compositor);
	wl_surface_set_input_region(bar->icon_surface, region);
	wl_region_destroy(region);
}

