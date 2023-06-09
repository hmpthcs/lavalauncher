/*
 * LavaLauncher - A simple launcher panel for Wayland
 *
 * Copyright (C) 2020 - 2021 Leon Henrik Plickat
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
#include<unistd.h>
#include<stdbool.h>
#include<string.h>
#include<getopt.h>

#include"bar.h"
#include"config.h"
#include"event-loop.h"
#include"lavalauncher.h"
#include"str.h"
#include"wayland-connection.h"
#include"misc-event-sources.h"

/* The context is used basically everywhere. So instead of passing pointers
 * around, just have it global.
 */
struct Lava_context context = {0};

static bool handle_command_flags (int argc, char *argv[])
{
	const char usage[] =
		"Usage: lavalauncher [options...]\n"
		"  -c <path>, --config <path> Path to config file.\n"
		"  -h,        --help          Print this help text.\n"
		"  -v,        --verbose       Enable verbose output.\n"
		"  -V,        --version       Show version.\n"
		"\n"
		"The configuration syntax is documented in the man page.\n";

	static struct option opts[] = {
		{"config",  required_argument, NULL, 'c'},
		{"help",    no_argument,       NULL, 'h'},
		{"verbose", no_argument,       NULL, 'v'},
		{"version", no_argument,       NULL, 'V'},
		{0,         0,                 0,    0  }
	};

	extern int optind;
	optind = 0;
	extern char *optarg;
	for (int c; (c = getopt_long(argc, argv, "c:hvV", opts, &optind)) != -1 ;) switch (c)
	{
		case 'c':
			set_string(&context.config_path, optarg);
			break;

		case 'h':
			fputs(usage, stderr);
			context.ret = EXIT_SUCCESS;
			return false;

		case 'v':
			context.verbosity++;
			break;

		case 'V':
			fputs("LavaLauncher version " LAVALAUNCHER_VERSION"\n", stderr);
			context.ret = EXIT_SUCCESS;
			return false;

		default:
			return false;
	}

	return true;
}

static bool get_default_config_path (void)
{
	struct
	{
		const char *fmt;
		const char *env;
	} paths[] = {
		{ .fmt = "./lavalauncher.conf",                           .env = NULL                      },
		{ .fmt = "%s/lavalauncher/lavalauncher.conf",             .env = getenv("XDG_CONFIG_HOME") },
		{ .fmt = "%s/.config/lavalauncher/lavalauncher.conf",     .env = getenv("HOME")            },
		{ .fmt = "/usr/local/etc/lavalauncher/lavalauncher.conf", .env = NULL                      },
		{ .fmt = "/etc/lavalauncher/lavalauncher.conf",           .env = NULL                      }
	};

	FOR_ARRAY(paths, i)
	{
		context.config_path = get_formatted_buffer(paths[i].fmt, paths[i].env);
		if (! access(context.config_path, F_OK | R_OK))
		{
			log_message(1, "[main] Using default configuration file path: %s\n", context.config_path);
			return true;
		}
		free_if_set(context.config_path);
	}

	log_message(0, "ERROR: Can not find configuration file.\n"
			"INFO: You can provide a path manually with '-c'.\n");
	return false;
}

static void init_context (void)
{
	context.ret         = EXIT_FAILURE;
	context.loop        = true;
	context.reload      = false;
	context.verbosity   = 0;
	context.config_path = NULL;

#if WATCH_CONFIG
	context.watch = false;
#endif

	context.display            = NULL;
	context.registry           = NULL;

	context.compositor         = NULL;
	context.subcompositor      = NULL;
	context.shm                = NULL;
	context.layer_shell        = NULL;
	context.xdg_output_manager = NULL;

	context.river_status_manager = NULL;
	context.need_river_status    = false;

	context.need_keyboard = false;
	context.need_pointer  = false;
	context.need_touch    = false;

	wl_list_init(&context.bars);
	context.last_bar = NULL;

	wl_list_init(&context.outputs);
	wl_list_init(&context.seats);
}

int main (int argc, char *argv[])
{
reload:
	init_context();

	if (! handle_command_flags(argc, argv))
		return context.ret;

	log_message(1, "[main] LavaLauncher: version=%s\n", LAVALAUNCHER_VERSION);

	/* If the user did not provide the path to a configuration file, try
	 * the default location.
	 */
	if ( context.config_path == NULL )
		if (! get_default_config_path())
			return EXIT_FAILURE;

	/* Try to parse the configuration file. If this fails, there might
	 * already be heap objects, so some cleanup is needed.
	 */
	if (! parse_config_file())
		goto exit;

	context.ret = EXIT_SUCCESS;

	/* Set up the event loop and attach all event sources. */
	struct Lava_event_loop loop;
	event_loop_init(&loop);
	event_loop_add_event_source(&loop, &wayland_source);
#if WATCH_CONFIG
	if (context.watch)
		event_loop_add_event_source(&loop, &inotify_source);
#endif
#if HANDLE_SIGNALS
	event_loop_add_event_source(&loop, &signal_source);
#endif

	/* Run the event loop. */
	if (! event_loop_run(&loop))
		context.ret = EXIT_FAILURE;

exit:
	free(context.config_path);

	/* Clean up objects created when parsing the configuration file. */
	destroy_all_bars();

	if (context.reload)
		goto reload;
	return context.ret;
}

