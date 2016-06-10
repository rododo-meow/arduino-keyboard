
/*
  KeyboardAndMouseControl

 Controls the mouse from five pushbuttons on an Arduino Leonardo, Micro or Due.

 Hardware:
 * 5 pushbuttons attached to D2, D3, D4, D5, D6

 The mouse movement is always relative. This sketch reads
 four pushbuttons, and uses them to set the movement of the mouse.

 WARNING:  When you use the Mouse.move() command, the Arduino takes
 over your mouse!  Make sure you have control before you use the mouse commands.

 created 15 Mar 2012
 modified 27 Mar 2012
 by Tom Igoe

 this code is in the public domain

 */

#include "Keyboard.h"
#include "Mouse.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_PS2
//#define DEBUG_KEYBOARD
#endif

#define PIN_PS2_CLK 22
#define PIN_PS2_DATA 21
#define PIN_PS2_RST 23
#define PS2_TIMEOUT 100
#define PS2_READ_MOVEMENT_DELAY 0 // us
int PIN_SCAN_INPUT[8] = { 8, 9, 10, 11, 12, 18, 19, 20 };
int PIN_SCAN_OUTPUT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

int ps2_timeout = 0;

void ps2_send(int data) {
  ps2_timeout = 0;
#ifdef DEBUG_PS2
  Serial.print("PS2 send: ");
  Serial.println(data, HEX);
#endif
  int i, parity = 1;
  pinMode(PIN_PS2_CLK, OUTPUT);
  digitalWrite(PIN_PS2_CLK, LOW);
  delayMicroseconds(300);
  pinMode(PIN_PS2_DATA, OUTPUT);
  digitalWrite(PIN_PS2_DATA, LOW);
  delayMicroseconds(200);
  pinMode(PIN_PS2_CLK, INPUT_PULLUP);
  for (i = 0; i < 8; i++) {
    while (digitalRead(PIN_PS2_CLK) == 1);
    digitalWrite(PIN_PS2_DATA, data & 1);
    parity ^= data & 1;
    data = data >> 1;
    while (digitalRead(PIN_PS2_CLK) == 0);
  }
  while (digitalRead(PIN_PS2_CLK) == 1);
  digitalWrite(PIN_PS2_DATA, parity);
  while (digitalRead(PIN_PS2_CLK) == 0);
  while (digitalRead(PIN_PS2_CLK) == 1);
  pinMode(PIN_PS2_DATA, INPUT_PULLUP);
  while (digitalRead(PIN_PS2_DATA) == 1);
  while (digitalRead(PIN_PS2_CLK) == 1);
  while (digitalRead(PIN_PS2_CLK) == 0);
  while (digitalRead(PIN_PS2_DATA) == 0);
#ifdef DEBUG_PS2
  Serial.println("Send done");
#endif
}

void ps2_send_timeout(int data) {
  ps2_timeout = 0;
#ifdef DEBUG_PS2
  Serial.print("PS2 send: ");
  Serial.println(data, HEX);
#endif
  int i, parity = 1;
  unsigned long j;
  pinMode(PIN_PS2_CLK, OUTPUT);
  digitalWrite(PIN_PS2_CLK, LOW);
  delayMicroseconds(300);
  pinMode(PIN_PS2_DATA, OUTPUT);
  digitalWrite(PIN_PS2_DATA, LOW);
  delayMicroseconds(200);
  pinMode(PIN_PS2_CLK, INPUT_PULLUP);
  j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ / 1000 > PS2_TIMEOUT) { Serial.println("PS2 send timeout at waiting device ACK"); ps2_timeout = 1; return; }
  for (i = 0; i < 8; i++) {
    j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
    digitalWrite(PIN_PS2_DATA, data & 1);
    parity ^= data & 1;
    data = data >> 1;
    j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  }
  j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  digitalWrite(PIN_PS2_DATA, parity);
  j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  pinMode(PIN_PS2_DATA, INPUT_PULLUP);
  j = 0; while (digitalRead(PIN_PS2_DATA) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
  j = 0; while (digitalRead(PIN_PS2_DATA) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 send timeout"); ps2_timeout = 1; return; }
#ifdef DEBUG_PS2
  Serial.println("Send done");
#endif
}

int ps2_recv() {
  ps2_timeout = 0;
  int i = 1, parity = 1, data = 0;
  while (i != 0) {
    while (digitalRead(PIN_PS2_CLK) == 1);
    i = digitalRead(PIN_PS2_DATA);
    while (digitalRead(PIN_PS2_CLK) == 0);
  }
  for (i = 0; i < 8; i++) {
    while (digitalRead(PIN_PS2_CLK) == 1);
    parity ^= digitalRead(PIN_PS2_DATA);
    data |= digitalRead(PIN_PS2_DATA) << i;
    while (digitalRead(PIN_PS2_CLK) == 0);
  }
  while (digitalRead(PIN_PS2_CLK) == 1);
  parity ^= digitalRead(PIN_PS2_DATA);
  while (digitalRead(PIN_PS2_CLK) == 0);
  while (digitalRead(PIN_PS2_CLK) == 1);
  i = digitalRead(PIN_PS2_DATA);
  while (digitalRead(PIN_PS2_CLK) == 0);
#ifdef DEBUG_PS2
  Serial.print("PS2 recv: ");
  Serial.println(data, HEX);
#endif
  return data;
}

int ps2_recv_timeout() {
  ps2_timeout = 0;
  int i = 1, parity = 1, data = 0, j;
  while (i != 0) {
    j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
    i = digitalRead(PIN_PS2_DATA);
    j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
  }
  for (i = 0; i < 8; i++) {
    j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
    parity ^= digitalRead(PIN_PS2_DATA);
    data |= digitalRead(PIN_PS2_DATA) << i;
    j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
  }
  j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
  parity ^= digitalRead(PIN_PS2_DATA);
  j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
  j = 0; while (digitalRead(PIN_PS2_CLK) == 1) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
  i = digitalRead(PIN_PS2_DATA);
  j = 0; while (digitalRead(PIN_PS2_CLK) == 0) if (j++ > PS2_TIMEOUT) { Serial.println("PS2 recv timeout"); ps2_timeout = 1; return 0; }
#ifdef DEBUG_PS2
  Serial.print("PS2 recv: ");
  Serial.println(data, HEX);
#endif
  return data;
}

void handle_ps2_report(int b1, int b2, int b3) {
  static int btn1 = 0, btn2 = 0, btn3 = 0;
  static int dx = 0, dy = 0;
  dx += (((((b1 << 4) & 0x100) | b2) << 7) >> 7) * 30;
  dy += (((((b1 << 3) & 0x100) | b3) << 7) >> 7) * 30;
  Mouse.move(dx / 50, -dy / 50, 0);
  dx %= 50;
  dy %= 50;
  if (b1 & 1) {
    if (!btn1)
      Mouse.press(MOUSE_LEFT);
  } else {
    if (btn1)
      Mouse.release(MOUSE_LEFT);
  }
  if (b1 & 2) {
    if (!btn2)
      Mouse.press(MOUSE_RIGHT);
  } else {
    if (btn2)
      Mouse.release(MOUSE_RIGHT);
  }
  if (b1 & 4) {
    if (!btn3)
      Mouse.press(MOUSE_MIDDLE);
  } else {
    if (btn3)
      Mouse.release(MOUSE_MIDDLE);
  }
  btn1 = b1 & 1;
  btn2 = b1 & 2;
  btn3 = b1 & 4;
}

void handle_ps2_packet(int packet) {
  static int state = 0;
  static int b1, b2, b3;
  if (packet == 0xaa)
  switch (state) {
  case 0: b1 = packet; state = 1; break;
  case 1: b2 = packet; state = 2; break;
  case 2: b3 = packet; state = 0; handle_ps2_report(b1, b2, b3); break;
  }
}

void handle_ps2_bit(int bit) {
  static int state = 0;
  static int packet = 0;
  int parity;
  
  if (state == 0) {
    if (bit == 0) {
      state = 1;
      packet = 0;
    }
  } else if (state >= 1 && state <= 8) {
    packet |= bit << (state - 1);
    state++;
  } else if (state == 9) {
    parity = packet;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    parity ^= bit;
    if (parity & 1)
      state = 10;
    else
      state = 11;
  } else if (state == 10) {
    if (bit == 1)
      handle_ps2_packet(packet);
    state = 0;
  } else if (state == 11) {
    state = 0;
  }
}

void handle_ps2() {
  static int clk = 1;
  int new_clk = digitalRead(PIN_PS2_CLK);
  if (clk == 1 && new_clk == 0)
    handle_ps2_bit(digitalRead(PIN_PS2_DATA));
  clk = new_clk;
}

int ps2_set_sample(int sample) {
  if (ps2_send_command(0xf3))
    return 1;
  if (ps2_send_command(sample))
    return 1;
  return 0;
}

int ps2_get_dev_id() {
  int res;
  if (ps2_send_command(0xf2))
    return 255;
  res = ps2_recv();
  if (ps2_timeout)
    return 255;
  return res;
}

int ps2_send_command(int cmd) {
  int res;
  ps2_send_timeout(cmd);
  if (ps2_timeout)
    return 1;
  while ((res = ps2_recv_timeout()) == 0xfe) {
    if (ps2_timeout)
      return 1;
    ps2_send_timeout(cmd);
    if (ps2_timeout)
      return 1;
  }
  if (res != 0xfa)
    return 1;
  return 0;
}

int ps2_set_resolution(int resolution) {
  int res_cmd;
  if (ps2_send_command(0xe8))
    return 1;
  switch (resolution) {
    case 1: res_cmd = 0; break;
    case 2: res_cmd = 1; break;
    case 4: res_cmd = 2; break;
    case 8: res_cmd = 3; break;
    default: return 1;
  }
  if (ps2_send_command(res_cmd))
    return 1;
  return 0;
}

void init_ps2() {
  Serial.println("Reset PS2");
  pinMode(PIN_PS2_CLK, INPUT_PULLUP);
  pinMode(PIN_PS2_DATA, INPUT_PULLUP);
  pinMode(PIN_PS2_RST, OUTPUT);

restart:
  digitalWrite(PIN_PS2_RST, 1);
  delay(100);
  digitalWrite(PIN_PS2_RST, 0);
  delay(100);

  if (ps2_send_command(0xff))
    goto restart;
  ps2_recv();
  ps2_recv();
  
  if (ps2_send_command(0xf0))
    goto restart;

  if (ps2_set_resolution(1))
    goto restart;

  if (ps2_send_command(0xe7))
    goto restart;
}

void read_ps2_movement() {
  int res, b1, b2, b3;
  ps2_send(0xeb);
  delayMicroseconds(PS2_READ_MOVEMENT_DELAY);
  while ((res = ps2_recv_timeout()) == 0xfe) {
    ps2_send(0xeb);
    delayMicroseconds(PS2_READ_MOVEMENT_DELAY);
  }
  if (ps2_timeout)
    return;
  if (res != 0xfa) {
    init_ps2();
    return;
  }

  b1 = ps2_recv_timeout();
  if (ps2_timeout)
    return;
  b2 = ps2_recv_timeout();
  if (ps2_timeout)
    return;
  b3 = ps2_recv_timeout();
  if (ps2_timeout)
    return;
  handle_ps2_report(b1, b2, b3);
}

static unsigned char keyPressedMap[8][8];
static unsigned char keyMap[8][8] = {
  { '2', '3', 'w', 'e', 's', 'd', 'x', 'c' },
  { '`', '1', KEY_TAB, 'q', KEY_CAPS_LOCK, 'a', KEY_LEFT_SHIFT, 'z' },
  { '4', '5', 'r', 't', 'f', 'g', 'v', 'b' },
  { '6', '7', 'y', 'u', 'h', 'j', 'n', 'm' },
  { '8', '9', 'i', 'o', 'k', 'l', ',', '.' },
  { KEY_RIGHT_CTRL, KEY_RIGHT_GUI, KEY_RIGHT_ALT, ' ', KEY_LEFT_ALT, KEY_RIGHT_GUI, KEY_LEFT_CTRL, KEY_LEFT_GUI },
  { '0', '-', 'p', '[', ';', '\'', '/', KEY_RIGHT_SHIFT },
  { '=', KEY_DELETE, ']', KEY_BACKSPACE, KEY_RETURN, '\\', 255, 255 }
};

#define FN_ROW 7
#define FN_COL 6
static unsigned char funcKeyMap[8][8] = {
  { KEY_F2, KEY_F3, 'w', 'e', 's', 'd', 'x', 'c' },
  { KEY_ESC, KEY_F1, KEY_TAB, 'q', KEY_CAPS_LOCK, 'a', KEY_LEFT_SHIFT, 'z' },
  { KEY_F4, KEY_F5, 'r', 't', 'f', 'g', 'v', 'b' },
  { KEY_F6, KEY_F7, 'y', 'u', 'h', 'j', 'n', 'm' },
  { KEY_F8, KEY_F9, 'i', 'o', KEY_HOME, KEY_PAGE_UP, KEY_END, KEY_PAGE_DOWN },
  { KEY_RIGHT_CTRL, KEY_RIGHT_GUI, KEY_RIGHT_ALT, ' ', KEY_LEFT_ALT, KEY_RIGHT_GUI, KEY_LEFT_CTRL, KEY_LEFT_GUI },
  { KEY_F10, KEY_F11, 'p', KEY_UP_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_SHIFT },
  { KEY_F12, KEY_DELETE, ']', KEY_BACKSPACE, KEY_RETURN, '\\', 255, 255 }
};

int key_state[8][8];

void handle_hit(int i, int j) {
#ifdef DEBUG_KEYBOARD
  Serial.print("Press ");
  Serial.print(i);
  Serial.println(j);
#endif
  if (key_state[FN_ROW][FN_COL] == 1)
    keyPressedMap[i][j] = funcKeyMap[i][j];
  else
    keyPressedMap[i][j] = keyMap[i][j];
  if (keyPressedMap[i][j] != 255)
    Keyboard.press(keyPressedMap[i][j]);
}

void handle_release(int i, int j) {
#ifdef DEBUG_KEYBOARD
  Serial.print("Release ");
  Serial.print(i);
  Serial.println(j);
#endif
  if (keyPressedMap[i][j] != 255)
    Keyboard.release(keyPressedMap[i][j]);
}

void handle_keyboard() {
  int i, j;
  for (i = 0; i < 8; i++) {
    pinMode(PIN_SCAN_OUTPUT[i], OUTPUT);
    digitalWrite(PIN_SCAN_OUTPUT[i], LOW);
    delayMicroseconds(50);
    for (j = 0; j < 8; j++) {
      if (digitalRead(PIN_SCAN_INPUT[j]) == 0) {
        if (key_state[i][j] == 0) {
          key_state[i][j] = 1;
          handle_hit(i, j);
        }
      } else {
        if (key_state[i][j] == 1) {
          key_state[i][j] = 0;
          handle_release(i, j);
        }
      }
    }
    digitalWrite(PIN_SCAN_OUTPUT[i], HIGH);
    pinMode(PIN_SCAN_OUTPUT[i], INPUT);
  }
}

void init_keyboard() {
  int i = 0;
  for (i = 0; i < 8; i++) {
    pinMode(PIN_SCAN_INPUT[i], INPUT_PULLUP);
    pinMode(PIN_SCAN_OUTPUT[i], OUTPUT);
  }
}

void setup() { // initialize the buttons' inputs:
#ifdef DEBUG
  while (true)
    if (Serial)
      break;
#endif
  Serial.begin(9600);
  init_keyboard();
  init_ps2();

  // initialize mouse control:
  Mouse.begin();
  Keyboard.begin();
}

void loop() {
  read_ps2_movement();
  handle_keyboard();
}

