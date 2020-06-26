#ifdef MST
#include <Arduino.h>
#include "panel.h"
#include "Wire.h"

// Init peripherals.
/* NhdDisplay display(DISPLAY_PIN); */
/* Encoder enc(ENC_DT_PIN, ENC_CLK_PIN); */
/* ButtonManager encoder_button(ENC_BUTTON_PIN, true); */
/* ButtonManager stop_button(STOP_BUTTON_PIN, false); */
// TODO: Change the normally closed state tot he correct one.
/* ButtonManager limit_right(LIMIT_RIGHT_PIN, true); */
/* ButtonManager limit_left(LIMIT_LEFT_PIN, true); */

// Initialize ventilator state object.
VentIO vio;

// Default settings.
VentSettings vs = {'X', 500, 12, 1, 3, 0, 00, 20, 0, 0, 0, false, false}; 

// Default limits.
VentLimits vl;

// Calibration for TV on Ambu bag SPUR II.
int cal[9] = {950, 1062, 1155, 1238, 1320, 1393, 1465, 1538, 1610};

// String params for splash screens.
String* splash_text = new String[4];
String* warning_text = new String[4];

// Init panel pointers.
Panel* splash_ptr;
Panel* warning_ptr;
Panel* start_ptr;
Panel* apply_ptr;
Panel* run_ptr;
Panel* pause_ptr;

// Set static pointers for panels.
NhdDisplay* Panel::_disp_ptr = &vio.display;
Encoder* Panel::_encoder_ptr = &vio.enc;
ButtonManager* Panel::_em_button_ptr = &vio.encoder_button;
ButtonManager* Panel::_stop_button_ptr = &vio.stop_button;
VentSettings* Panel::_vs_ptr = &vs;
VentLimits* Panel::_vl_ptr = &vl;
VentIO* Panel::_vio_ptr = &vio;
/* VentAlarm* Panel::_va_ptr = &va; */

// Init display panel pointer.
Panel* cur_panel;

void transmit() {

  // Transmit to the slave device.
  Wire.beginTransmission(SLAVE_ADDR);

  // Write the mode first.
  Wire.write(vs.mode);

  // Send settings to controller unit, if we're loading.
  if (vs.mode == 'L') {
    // Calculate setpoint based on calibration.
    int setpoint = cal[(vs.tidal_volume - vl.min_tidal_volume)/vl.delta_tidal_volume];

    // Send the ventilation parameters to slave.
    Wire.write(byte(setpoint >> 8));
    Wire.write(byte(setpoint & 0x00FF));
    Wire.write(byte(vs.respiration_rate));
    Wire.write(byte(vs.inhale));
    Wire.write(byte(vs.exhale));
    Wire.write(byte(vs.hold_seconds));
    Wire.write(byte(vs.hold_decimals));
    Wire.write(byte(vs.delta_time));

    // Set the mode to on as device will start.
    vs.mode = 'O';
  }

  // Send i2c message.
  Wire.endTransmission();

  // Sent settings so disable send.
  vs.send = false;
}

/* void checkAlarm() */
/* { */

/*   int near = digitalRead(NEAR_PIN); */


/* } */

void setup()
{

  // Join i2c bus.
  Wire.begin();

  // Start serial debug connection.
  Serial.begin(9600);

  // Set digital read pin for when arms are close to 0.
  pinMode(NEAR_PIN, INPUT);

  // Start display.
  vio.display.begin(9600);
  vio.display.clearDisplay();

  // Init slash text.
  splash_text[0] = "";
  splash_text[1] = "     ApolloBVM";
  splash_text[2] = "";
  splash_text[3] = "";

  //Init warning text.
  warning_text[0] = "      WARNING: ";
  warning_text[1] = "   USE ADULT SIZED";
  warning_text[2] = "   BAG VALVE MASK";
  warning_text[3] = "";

  // Init panels.
  start_ptr = new EditPanel("Confirm & Run?", &run_ptr, 0);
  warning_ptr = new SplashPanel(warning_text, 2000, &start_ptr);
  splash_ptr = new SplashPanel( splash_text, 2000, &warning_ptr);
  apply_ptr = new EditPanel("Apply Changes?", &run_ptr, &pause_ptr);
  run_ptr = new RunningPanel(&apply_ptr, &pause_ptr);
  pause_ptr = new PausePanel(&start_ptr, &run_ptr);

  // Delay just cause.
  delay(100);

  // Set current panel to splash.
  cur_panel = splash_ptr;
  cur_panel->start();

}

void loop()
{

  // Poll button status.
  vio.encoder_button.poll();
  vio.stop_button.poll();

  // Update current panel.
  Panel* new_panel = cur_panel->update();

  // If we get a new panel, start and switch to it.
  if (new_panel != 0) {
    cur_panel = new_panel;
    cur_panel->start();
  }

  // Transmit to the device if necessary.
  if (vs.send)
    transmit();
}


#endif
