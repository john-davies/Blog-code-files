/* blinkpanel - a Blinkenlights client to drive an LED panel
 *              uses the rpi-rgb-led-marix library
 *
 * Copyright (c) 2018 John Davies
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

 // blinkpanel.c - a Blinkenlights client to drive an LED panel
 //                uses the rpi-rgb-led-marix library
 // Copyright (C) 2018 John Davies
 //
 // Usage: blinkpanel [ rpi-rgb-led-marix options ]
 //

 // This program is free software: you can redistribute it and/or modify
 // it under the terms of the GNU General Public License as published by
 // the Free Software Foundation, either version 3 of the License, or
 // (at your option) any later version.
 //
 // This program is distributed in the hope that it will be useful,
 // but WITHOUT ANY WARRANTY; without even the implied warranty of
 // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 // GNU General Public License for more details.
 //
 // You should have received a copy of the GNU General Public License
 // along with this program.  If not, see <http://www.gnu.org/licenses/>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <blib/blib.h>
#include "led-matrix-c.h"


struct RGBLedMatrixOptions options;
struct RGBLedMatrix *matrix;
struct LedCanvas *canvas;

int packets = 0;
int *dummy_data;

static GMainLoop *loop = NULL;

// ------------------------------------------------------------------------
// Interrupt handler

static void InterruptHandler( int signo )
{
  // Kill the GTK processing loop
  if( loop )
  {
    g_main_loop_quit (loop);
  }
}

// ------------------------------------------------------------------------
// Blinkenlight packet processor

static gboolean frame_callback( BReceiver *receiver, BPacket *packet, gpointer data )
{
  int row, column, maxVal, r, g, b;
  gboolean rgb = FALSE;

  // Check for colour frame
  if( packet->header.mcu_frame_h.channels == 3 )
  {
    rgb = TRUE;
  }

  // Panel setup
  led_canvas_clear( canvas );

  // Check data sizes
  if( ( packet->header.mcu_frame_h.height <= options.rows ) &&
      ( packet->header.mcu_frame_h.width <= options.cols ) )
  {
    // Packet will fit on display so copy data
    maxVal = packet->header.mcu_frame_h.maxval;

    for( row=0; row<packet->header.mcu_frame_h.height; row++ )
    {
      for( column=0; column<packet->header.mcu_frame_h.width; column++ )
      {
        if( rgb == FALSE )
        {
          // Monochrome
          r = ( ( packet->data[ ( row * packet->header.mcu_frame_h.width ) + column ] ) * 255 ) / maxVal;
          g = r;
          b = r;
        }
        else
        {
          // RGB
          r = ( ( packet->data[ ( row * packet->header.mcu_frame_h.width * packet->header.mcu_frame_h.channels ) + ( column * packet->header.mcu_frame_h.channels ) ] ) * 255 ) / maxVal;
          g = ( ( packet->data[ ( row * packet->header.mcu_frame_h.width * packet->header.mcu_frame_h.channels ) + ( column * packet->header.mcu_frame_h.channels ) + 1 ] ) * 255 ) / maxVal;
          b = ( ( packet->data[ ( row * packet->header.mcu_frame_h.width * packet->header.mcu_frame_h.channels ) + ( column * packet->header.mcu_frame_h.channels ) + 2 ] ) * 255 ) / maxVal;
        }
        led_canvas_set_pixel( canvas, column, row, r, g, b );
      }
    }
  }

  //led_canvas_set_pixel( canvas, packets % 64, packets / 64, 128, 128, 128 );
  // Write to panel
  canvas = led_matrix_swap_on_vsync( matrix, canvas );

  packets++;
  return TRUE;
}

// ------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
  BReceiver *receiver;
  gint bml_port = MCU_LISTENER_PORT;

  // Set up blinkenlights receiver
  b_init();
  receiver = b_receiver_new(frame_callback, dummy_data);
  b_receiver_listen(receiver, bml_port);

  // Set up LED panel
  memset( &options, 0, sizeof(options) );
  // Set some defaults
  options.rows = 32;
  options.cols = 64;
  options.chain_length = 1;
  // Allow override from command line
  matrix = led_matrix_create_from_options( &options, &argc, &argv );
  if( matrix == NULL )
  {
    printf( "ERROR - could not create matrix\n");
    return EXIT_FAILURE;
  }
  canvas = led_matrix_create_offscreen_canvas( matrix );

  signal( SIGTERM, InterruptHandler );
  signal( SIGINT, InterruptHandler );

  // Start gtk processing loop
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  // Print some stats and clear panel on exit
  printf( "\nClearing panel ...\n" );
  led_canvas_clear( canvas );
  led_matrix_delete( matrix );
  printf( "Packets: %d\n", packets );
  return EXIT_SUCCESS;
}
