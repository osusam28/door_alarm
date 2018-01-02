/*
 * Project Firmware_v1
 * Description:
 * Author:
 * Date:
 */

//How long to check for whether the button is debounced
const int DEBOUNCE_WAIT = 50;
//How often to check the button status
const int POLL_TIME = 10;
//Time to pulse before alarm sounds
const int ALARM_POLL_TIME = 1000;
//Time before alarm sounds
const int ALARM_DELAY = 6000;

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
enum states {
  NORMAL,
  ALARM_SET,
  ALARM_TRIGGERED,
  ALARM_ON
} currentState;

enum events {
  KEY_PRESSED,
  DOOR_OPENED,
  ALARM
};

//Timer routines
Timer door_timer(POLL_TIME, door_opened);
Timer keypad_timer(POLL_TIME, key_pressed);
Timer alarm_timer(ALARM_POLL_TIME, alarm_ctrl);

int lastDoorValue = 0;
bool doorOpened = false;
int lastKeyValue = 0;
bool keyPressed = false;

int alarm_count = ALARM_DELAY / ALARM_POLL_TIME;
bool alarmTimeUp = false;

void change_state(events e, int data);

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin();
  Serial.println("starting program ...");

  door_timer.start();
  keypad_timer.start();

  pinMode(buzzer, OUTPUT);
  pinMode(door, INPUT);
  pinMode(keypad, INPUT);
  pinMode(led, OUTPUT);

  digitalWrite(buzzer, LOW);
  digitalWrite(led, LOW);

  currentState = NORMAL;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code http://forum.arduino.cc/index.php?topic=41954.0will likely live here.
  if(keyPressed) {
    change_state(KEY_PRESSED, 1);

    keyPressed = false;
  }
  if(doorOpened) {
    change_state(DOOR_OPENED, 0);

    doorOpened = false;
  }
  if(alarmTimeUp) {
    change_state(ALARM, 0);

    alarmTimeUp = false;
  }
}

void change_state(events e, int data) {
  switch(currentState) {
    case NORMAL:
      if(e == KEY_PRESSED) {
        enter_key(data);
        Serial.print("key ");
        Serial.print(data);
        Serial.println(" entered ...");

        if(is_pin_entered()) {
          currentState = ALARM_SET;
          Serial.println("alarm is set ...");
        }
      }
      break;
    case ALARM_SET:
      if(e == KEY_PRESSED) {
        enter_key(data);
        Serial.print("key ");
        Serial.print(data);
        Serial.println(" entered ...");

        if(is_pin_entered()) {
          currentState = NORMAL;
          Serial.println("back to normal ...");
        }
      }
      if(e == DOOR_OPENED) {
        Serial.println("door opened ...");

        alarm_timer.start();
        currentState = ALARM_TRIGGERED;
        Serial.println("alarm has been triggered ...");
      }
      break;
    case ALARM_TRIGGERED:
      if(e == KEY_PRESSED) {
        enter_key(data);
        Serial.print("key ");
        Serial.print(data);
        Serial.println(" entered ...");

        if(is_pin_entered()) {
          digitalWrite(led, LOW);
          digitalWrite(buzzer, LOW);
          alarm_timer.stop();
          currentState = NORMAL;
          Serial.println("back to normal ...");
        }
      }
      if(e == ALARM) {
        Serial.println("alarm sounding ...");

        digitalWrite(led, HIGH);
        digitalWrite(buzzer, HIGH);
        send_text();
        alarm_timer.stop();
        alarm_count = ALARM_DELAY / ALARM_POLL_TIME;
        currentState = ALARM_ON;
      }
      break;
    case ALARM_ON:
      if(e == KEY_PRESSED) {
        enter_key(data);
        Serial.print("key ");
        Serial.print(data);
        Serial.println(" entered ...");

        if(is_pin_entered()) {
          digitalWrite(led, LOW);
          digitalWrite(buzzer, LOW);
          currentState = NORMAL;
          Serial.println("back to normal ...");
        }
      }
      break;
    default:
      break;
  }
}

void send_text() {
  Serial.println("sending alert text ...");
  String data = String(10);
  Particle.publish("alarm_text", data, PRIVATE);
}

void enter_key(int value) {
  pin_buffer[pos] = value;
  pos++;

  if(pos == 4) {
    pos = 0;
  }
}

bool is_pin_entered() {
  for(int i=0;i<4;i++) {
    if(pin_buffer[i] != pin_value[i]) {
      return false;
    }
  }

  for(int i=0;i<4;i++) {
    pin_buffer[i] = 0;
  }
  return true;
}

void key_pressed() {
  static int count = 0;

  int keyValue = digitalRead(keypad);

  if(count > 0) {
    if(lastKeyValue == keyValue) {
      count--;
    }
    else {
      count = DEBOUNCE_WAIT / POLL_TIME;
    }

    if(count == 0) {
      keyPressed = true;
    }
  }
  else if(keyValue > lastKeyValue) {
    count = DEBOUNCE_WAIT / POLL_TIME;
  }

  lastKeyValue = keyValue;
}

void door_opened() {
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
      doorOpened = true;
    }
  }
  else if(doorValue > lastDoorValue) {
    count = DEBOUNCE_WAIT / POLL_TIME;
  }

  lastDoorValue = doorValue;
}

void alarm_ctrl() {
  digitalWrite(led, !digitalRead(led));
  digitalWrite(buzzer, !digitalRead(buzzer));

  if(alarm_count == 0) {
    alarmTimeUp = true;
  }
  if(alarm_count > 0) {
    Serial.println(alarm_count);
    alarm_count--;
  }
}
