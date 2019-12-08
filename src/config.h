/*
 * LavaLauncher - A simple launcher for Wayland
 *
 * Copyright (C) 2019 Leon Plickat <leonhenrik.plickat@stud.uni-goettingen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define _POSIX_C_SOURCE 200112L

#include"lavalauncher.h"

void sensible_defaults (struct Lava_data *data);
void config_add_button(struct Lava_data *data, char *path, char *cmd);
void config_set_layer(struct Lava_data *data, const char *arg);
void config_set_position(struct Lava_data *data, const char *arg);
void config_set_bar_size(struct Lava_data *data, const char *arg);
void config_set_border_size(struct Lava_data *data, const char *arg);
void config_set_bar_colour(struct Lava_data *data, const char *arg);
void config_set_border_colour(struct Lava_data *data, const char *arg);

#endif