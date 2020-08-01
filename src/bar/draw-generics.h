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

#ifndef LAVALAUNCHER_BAR_DRAW_GENERICS_H
#define LAVALAUNCHER_BAR_DRAW_GENERICS_H

#include<stdint.h>
#include<cairo/cairo.h>

struct Lava_data;
struct Lava_bar;

void circle (cairo_t *cairo, uint32_t x, uint32_t y, uint32_t size);
void rounded_rectangle (cairo_t *cairo, uint32_t x, uint32_t y,
		uint32_t width, uint32_t height,
		double tl_r, double tr_r, double bl_r, double br_r);
void draw_bar_background (cairo_t *cairo,
		uint32_t x, uint32_t y, uint32_t w, uint32_t h,
		uint32_t border_top, uint32_t border_right,
		uint32_t border_bottom, uint32_t border_left,
		uint32_t top_left_radius, uint32_t top_right_radius,
		uint32_t bottom_left_radius, uint32_t bottom_right_radius,
		uint32_t scale, float center_colour[4], float border_colour[4]);
void clear_buffer (cairo_t *cairo);

#endif
