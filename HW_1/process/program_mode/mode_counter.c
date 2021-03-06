//
// Created by taeguk on 2017-04-05.
//

#include <stdlib.h>
#include "mode_counter.h"
#include "../../message/output_message.h"

enum notation
{
  NOTATION_DECIMAL = 0, /* must be started from 0 */
  NOTATION_OCTAL,
  NOTATION_QUATERNION,
  NOTATION_BINARY,

  _NOTATION_COUNT  /* The number of notations */
};

static const int notation_to_base[_NOTATION_COUNT] =
    {
        [NOTATION_DECIMAL] = 10,
        [NOTATION_OCTAL] = 8,
        [NOTATION_QUATERNION] = 4,
        [NOTATION_BINARY] = 2
    };

static union led_data notation_to_led_data[_NOTATION_COUNT];

struct mode_counter_status
{
  int output_pipe_fd;
  int number;
  int notation;
};

static fnd_data_t make_fnd_data (int number, enum notation notation)
{
  int base = notation_to_base[notation];
  int digit[4];
  for (int i = 0; i < 4; ++i)
    {
      digit[i] = number % base;
      number /= base;
    }
  return 1000 * digit[3] + 100 * digit[2] + 10 * digit[1] + digit[0];
}

struct mode_counter_status *mode_counter_construct (int output_pipe_fd)
{
  struct mode_counter_status *status;
  status = malloc (sizeof (*status));

  // Initializing
  {
    status->output_pipe_fd = output_pipe_fd;
    status->number = 0;
    status->notation = NOTATION_DECIMAL;

    notation_to_led_data[NOTATION_BINARY].val = 0;
    notation_to_led_data[NOTATION_BINARY].bit_fields.e1 = 1;
    notation_to_led_data[NOTATION_DECIMAL].val = 0;
    notation_to_led_data[NOTATION_DECIMAL].bit_fields.e2 = 1;
    notation_to_led_data[NOTATION_OCTAL].val = 0;
    notation_to_led_data[NOTATION_OCTAL].bit_fields.e3 = 1;
    notation_to_led_data[NOTATION_QUATERNION].val = 0;
    notation_to_led_data[NOTATION_QUATERNION].bit_fields.e4 = 1;

    output_message_fnd_send (status->output_pipe_fd, make_fnd_data (status->number, status->notation));
    output_message_led_send (status->output_pipe_fd, notation_to_led_data[status->notation]);
  }

  return status;
}

void mode_counter_destroy (struct mode_counter_status *status)
{
  free (status);
}

int mode_counter_switch (struct mode_counter_status *status, union switch_data data)
{
  int base = notation_to_base[status->notation];

  if (data.bit_fields.s1)
    {
      status->notation = (status->notation + 1) % _NOTATION_COUNT;
      output_message_fnd_send (status->output_pipe_fd, make_fnd_data (status->number, status->notation));
      output_message_led_send (status->output_pipe_fd, notation_to_led_data[status->notation]);
    }

  if (data.bit_fields.s2)
    {
      status->number += base * base;
      output_message_fnd_send (status->output_pipe_fd, make_fnd_data (status->number, status->notation));
    }

  if (data.bit_fields.s3)
    {
      status->number += base;
      output_message_fnd_send (status->output_pipe_fd, make_fnd_data (status->number, status->notation));
    }

  if (data.bit_fields.s4)
    {
      status->number += 1;
      output_message_fnd_send (status->output_pipe_fd, make_fnd_data (status->number, status->notation));
    }

  return 0;
}
