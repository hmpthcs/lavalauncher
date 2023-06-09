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

#ifndef LAVALAUNCHER_STR_H
#define LAVALAUNCHER_STR_H

#include<stdbool.h>

void log_message (int level, const char *fmt, ...);
void free_if_set (void *ptr);
void set_string (char **ptr, char *arg);
char *get_formatted_buffer (const char *fmt, ...);
const char *str_orelse (const char *str, const char *orelse);
void setenvf (const char *name, const char *fmt, ...);
bool string_starts_with(const char *str, const char *prefix);

#endif

