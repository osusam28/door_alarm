/*
 * Project Firmware_v1
 * Description:
 * Author:
 * Date:
 */

//How long to check for whether the button is debounced
const int DEBOUNCE_WAIT = 100;
//How often to check the button status
const int POLL_TIME = 10;

//Pin definitions
int buzzer = D0;
int door = D1;
int keypad = D2;
int led = D7;

//PIN fields
int pin_value[4] = {1,1,1,1};
int pin_buffer[4] = {0,0,0,0};
int pos = 0;

//State fields
int state = 0;

//Timer routines
Timer door_timer(POLL_TIME, doorOpened);

int lastDoorValue = 0;
bool doorOpened = false;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin();
  Serial.println("starting program ...");

  door_timer.start();

  pinMode(buzzer, OUTPUT);
  pinMode(door, INPUT);
  pinMode(keypad, INPUT);
  pinMode(led, OUTPUT);

  digitalWrite(buzzer, LOW);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code http://forum.arduino.cc/index.php?topic=41954.0will likely live here.

}

void doorOpened() {
  static int count = 0;

  int doorValue = digitalRead(door);

  if(count > 0) {
    if(lastDoorValue == doorValue) {
      count--;
    }
    else {
      count = DEBOUNCE_WAIT / POLL_TIME;
    }

    if(count == 0) {
      Serial.print("door opened ... ");
      Serial.println("");

      doorOpened = true;
    }
  }
  else if(doorValue > lastDoorValue) {
    count = DEBOUNCE_WAIT / POLL_TIME;
  }

  lastDoorValue = doorValue;
}
