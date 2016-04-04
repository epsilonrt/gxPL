/*
 * sensor.c
 * >>> Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <math.h>
#include <string.h>
#include "template.h"

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* types ==================================================================== */

/* public variables ========================================================= */
// exceed the gap between current and previous value causes sending message TRIG
double dSensorGap = SENSOR_GAP;

/* private variables ======================================================== */
static gxPLMessage * currentmsg;

/* private functions ======================================================== */

//
/* -----------------------------------------------------------------------------
 * Generates a dummy triangular signal between 0 and 10 volts
 * !!!!!!!!!!!! TODO !!!!!!!!!!!!
 * replace this code with that of your sensor
 */
static double
prdReadDummySensor (void) {
  static int x;
  static int step = 1;

  double f =  x / 100.0f;

  x += step;
  if (x >= 1000) {

    step = -1;
  }
  else if (x <= 0) {

    step = 1;
  }

  return f;
}

// -----------------------------------------------------------------------------
static int
priSendCurrentValue (gxPLDevice * device, gxPLMessageType msgtype) {
  static double previous = -1.0f; // negative to force sending the first measure
  double current;

  // Reads te sensor (ie voltage...)
  current = prdReadDummySensor();

  if (msgtype == gxPLMessageTrigger) {

    /*
     * if trig message requested, check that the gap between the current and
     * previous value is sufficient.
     */
    if (fabs (current - previous) < dSensorGap) {

      // if not, do nothing
      return 0;
    }
  }

  gxPLMessageTypeSet (currentmsg, msgtype);
  gxPLMessagePairSetFormat (currentmsg, "current", "%.2f", current);
  /*
   * if the device is configurable, the instance ID can be
   * changed and should be updated.
   */
  if (gxPLDeviceIsConfigurale (device)) {

    gxPLMessageSourceInstanceIdSet (currentmsg, gxPLDeviceInstanceId (device));
  }

  if (msgtype == gxPLMessageTrigger) {

    previous = current;
  }

  // Broadcast the message
  PDEBUG ("sensor broadcast current value = %.2f", current);
  
  if (gxPLDeviceMessageSend (device, currentmsg) < 0) {

    return -1;
  }
  return 0;
}

// -----------------------------------------------------------------------------
static void
prvSensorMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) {

  if (gxPLMessagePairExist (msg, "request") == true) {
    // the request key is present in the message

    if (strcmp (gxPLMessagePairGet (msg, "request"), "current") == 0) {

      // this is a request for the current value
      if (gxPLMessagePairExist (msg, "device") == true) {

        if (strcmp (gxPLMessagePairGet (msg, "device"), SENSOR_NAME) != 0) {
          // this request doesn't match to own sensor name
          return;
        }
      }

      if (gxPLMessagePairExist (msg, "type") == true) {

        if (strcmp (gxPLMessagePairGet (msg, "type"), SENSOR_TYPE) != 0) {
          // this request doesn't match to own sensor type
          return;
        }
      }

      // we must send a message to the current value
      priSendCurrentValue (device, gxPLMessageStatus);
    }
  }
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
int
iSensorOpen (gxPLDevice * device) {
  int ret;

  // Add a responder for sensor.request schema
  ret = gxPLDeviceListenerAdd (device, prvSensorMessageListener,
                               gxPLMessageCommand, "sensor", "request", NULL);
  assert (ret == 0);

  // Create a sensor.basic message conforming to http://xplproject.org.uk/wiki/Schema_-_SENSOR.html
  currentmsg = gxPLDeviceMessageNew (device, gxPLMessageTrigger);
  assert (currentmsg);

  // Setting up the message
  gxPLMessageBroadcastSet (currentmsg, true);
  gxPLMessageSchemaSet (currentmsg, "sensor", "basic");
  gxPLMessagePairAdd (currentmsg, "device", SENSOR_NAME);
  gxPLMessagePairAdd (currentmsg, "type", SENSOR_TYPE);

  // Setting up hardware for the sensor

  return 0;
}

// -----------------------------------------------------------------------------
int
iSensorClose (gxPLDevice * device) {

  // Add here the necessary steps to close the sensor

  gxPLMessageDelete (currentmsg);
  return 0;
}

// -----------------------------------------------------------------------------
int
iSensorPoll (gxPLDevice * device) {

  return priSendCurrentValue (device, gxPLMessageTrigger);
}

/* ========================================================================== */
