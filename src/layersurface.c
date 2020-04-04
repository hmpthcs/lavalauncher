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
#include"output.h"
#include"seat.h"
#include"draw.h"

/* Helper function to configure the bar surface. Careful: This function does not
 * commit!
 */
void configure_surface (struct Lava_data *data, struct Lava_output *output)
{
	if (! output->configured)
		return;

	uint32_t width, height;
	if ( data->orientation == ORIENTATION_HORIZONTAL )
		width = output->w * output->scale, height = data->h;
	else
		width = data->w, height = output->h * output->scale;

	if (data->verbose)
		fprintf(stderr, "Surface size: w=%d h=%d\n", width, height);

	zwlr_layer_surface_v1_set_size(output->layer_surface, width, height);

	/* Anchor the surface to the correct edge. */
	switch (data->position)
	{
		case POSITION_TOP:
			zwlr_layer_surface_v1_set_anchor(output->layer_surface,
					ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
			break;

		case POSITION_RIGHT:
			zwlr_layer_surface_v1_set_anchor(output->layer_surface,
					ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
			break;

		case POSITION_BOTTOM:
			zwlr_layer_surface_v1_set_anchor(output->layer_surface,
					ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
			break;

		case POSITION_LEFT:
			zwlr_layer_surface_v1_set_anchor(output->layer_surface,
					ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
			break;
	}

	/* Set margin.
	* Since we create a surface spanning the entire length of an outputs
	* edge, margins parallel to it would move it outside the boundaries of
	* the output, which may or may not cause issues in some compositors.
	* To work around this, we simply cheat a bit: Margins parallel to the
	* bar will be simulated in the draw code by adjusting the bar offsets.
	*
	* See: update_bar_offset()
	*
	* Here we set the margins not parallel to the edges length, which are
	* real layer shell margins.
	*/
	if ( data->orientation == ORIENTATION_HORIZONTAL )
		zwlr_layer_surface_v1_set_margin(output->layer_surface,
				data->margin_top, 0,
				data->margin_bottom, 0);
	else
		zwlr_layer_surface_v1_set_margin(output->layer_surface,
				0, data->margin_right,
				0, data->margin_left);

	/* Set exclusive zone to prevent other surfaces from obstructing ours. */
	zwlr_layer_surface_v1_set_exclusive_zone(output->layer_surface,
			data->exclusive_zone);

	/* Create a region of the visible part of the surface. */
	struct wl_region *region = wl_compositor_create_region(data->compositor);
	if ( data->mode == MODE_DEFAULT )
		wl_region_add(region, output->bar_x_offset, output->bar_y_offset,
				data->w, data->h);
	else
		wl_region_add(region, 0, 0, output->w, output->h);

	/* Set input region. This is necessary to prevent the unused parts of
	 * the surface to catch pointer and touch events.
	 */
	wl_surface_set_input_region(output->wl_surface, region);

	/* If both border and background are opaque, set opaque region. This
	 * will inform the compositor that it does not have to render anything
	 * below the surface.
	 */
	if ( data->bar_colour[3] == 1 && data->border_colour[3] == 1 )
		wl_surface_set_opaque_region(output->wl_surface, region);

	wl_region_destroy(region);
}

static void layer_surface_handle_configure (void *raw_data,
		struct zwlr_layer_surface_v1 *surface, uint32_t serial,
		uint32_t w, uint32_t h)
{
	struct Lava_output *output = (struct Lava_output *)raw_data;
	struct Lava_data   *data   = output->data;
	output->configured         = true;

	if (data->verbose)
		fprintf(stderr, "Layer surface configure request:"
				" w=%d h=%d serial=%d\n", w, h, serial);

	zwlr_layer_surface_v1_ack_configure(surface, serial);
	configure_surface(data, output);
	render_bar_frame(data, output);
}

static void layer_surface_handle_closed (void *raw_data,
		struct zwlr_layer_surface_v1 *surface)
{
	struct Lava_data *data = (struct Lava_data *)raw_data;
	fputs("Layer surface has been closed.\n", stderr);
	data->loop = false;
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_handle_configure,
	.closed    = layer_surface_handle_closed
};

bool create_bar (struct Lava_data *data, struct Lava_output *output)
{
	if (data->verbose)
		fputs("Creating bar.\n", stderr);

	output->wl_surface = wl_compositor_create_surface(data->compositor);
	if ( output->wl_surface == NULL )
	{
		fputs("ERROR: Compositor did not create wl_surface.\n", stderr);
		return false;
	}

	output->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
			data->layer_shell, output->wl_surface,
			output->wl_output, data->layer, "LavaLauncher");
	if ( output->layer_surface == NULL )
	{
		fputs("ERROR: Compositor did not create layer_surface.\n", stderr);
		return false;
	}

	configure_surface(data, output);
	zwlr_layer_surface_v1_add_listener(output->layer_surface,
			&layer_surface_listener, output);
	wl_surface_commit(output->wl_surface);
	return true;
}