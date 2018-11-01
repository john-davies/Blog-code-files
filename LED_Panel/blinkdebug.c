// blinkdebug.c - a Blinkenlights client to assist in debugging a
//                Blinkenlights setup with no graphical display
// Copyright (C) 2018 John Davies
//
// Usage:  blinkdebug
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


int packets = 0;
int *dummy_data;

static  GMainLoop    *loop      = NULL;

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
  packets++;
  g_print( "." );

  // Extra debug can be added here ...

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

  // Initialise the interrupt handlers
  signal( SIGTERM, InterruptHandler );
  signal( SIGINT, InterruptHandler );

  // Start gtk processing loop
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  // On exit print some stats
  printf( "\nPackets: %d\n", packets );

  return EXIT_SUCCESS;
}
