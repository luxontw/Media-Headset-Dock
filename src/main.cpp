#include <Arduino.h>
#include <TFT_eSPI.h>
#include <BluetoothA2DPSink.h>
#include <BleKeyboard.h>
#include <Song.h>
#include <font_12.h>
// #define debug
#define light_sensor 36
#define main_light_sensor 39
#define power_light 2
#define connect_light 15
#define fill_light 13

struct Button
{
  const uint8_t PIN;
  bool pressed;
};

struct Encoder
{
  const uint8_t CLK_PIN;
  const uint8_t DT_PIN;
  const uint8_t SW_PIN;
  bool pressed;
  bool clockwise;
  bool counterclockwise;
  unsigned long t;
};

TFT_eSPI tft = TFT_eSPI();
BluetoothA2DPSink a2dp_sink;
BleKeyboard bleKeyboard("Bluetooth Keyboard", "Espressif", 100);
Song song;
Song temp;
Button button_up = {35, false};
Button button_down = {0, false};
Encoder encoder = {38, 37, 32, false, false, false, 0};

String audio_state;
String connection_state;
int sub_light_value = 0;
int main_light_value = 0;
bool is_play = false;
int volume = 50;

void IRAM_ATTR button_up_isr()
{
  button_up.pressed = true;
}

void IRAM_ATTR button_down_isr()
{
  button_down.pressed = true;
}

void IRAM_ATTR clk_isr()
{
  unsigned long temp = millis();
  if (temp - encoder.t < 200)
    return;
  encoder.t = temp;

  if (digitalRead(encoder.DT_PIN))
    encoder.clockwise = true;
  else
    encoder.counterclockwise = true;
}

void IRAM_ATTR sw_isr()
{
  encoder.pressed = true;
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
  String text_str((char *)text);
  if (id == 1 && text_str != " ")
  {
    temp.set_title(text_str);
  }
  if (id == 2 && text_str != " ")
  {
    temp.set_album(text_str);
  }
  if (id == 4 && text_str != " ")
  {
    temp.set_artist(text_str);
  }
  if (id == 8 && text_str != "0")
  {
    temp.set_song_id(text_str);
  }
  if (id == 16 && text_str != "0")
  {
    temp.set_album_id(text_str);
  }
  if (id == 32)
  {
    temp.set_change(true);
    song = temp;
    tft.fillScreen(TFT_BLACK);
  }
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
#ifdef debug
  Serial.println(a2dp_sink.to_str(state));
#endif
  connection_state = a2dp_sink.to_str(state);
  if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
  {
    digitalWrite(connect_light, LOW);
  }
  else if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    digitalWrite(connect_light, HIGH);
  }
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
#ifdef debug
  Serial.println(a2dp_sink.to_str(state));
#endif
  audio_state = a2dp_sink.to_str(state);
  if (state == ESP_A2D_AUDIO_STATE_STARTED)
  {
    is_play = true;
  }
  else
  {
    is_play = false;
  }
}

void setup()
{
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_on_audio_state_changed(audio_state_changed);
  a2dp_sink.set_volume(volume);
  a2dp_sink.start("MyMusic");

  pinMode(light_sensor, INPUT);
  pinMode(main_light_sensor, INPUT);
  pinMode(button_up.PIN, INPUT_PULLUP);
  pinMode(button_down.PIN, INPUT_PULLUP);
  pinMode(encoder.CLK_PIN, INPUT_PULLUP);
  pinMode(encoder.DT_PIN, INPUT_PULLUP);
  pinMode(encoder.SW_PIN, INPUT_PULLUP);

  pinMode(power_light, OUTPUT);
  pinMode(connect_light, OUTPUT);
  pinMode(fill_light, OUTPUT);

  digitalWrite(power_light, HIGH);
  digitalWrite(connect_light, LOW);
  digitalWrite(fill_light, LOW);

  attachInterrupt(button_up.PIN, button_up_isr, FALLING);
  attachInterrupt(button_down.PIN, button_down_isr, FALLING);
  attachInterrupt(encoder.CLK_PIN, clk_isr, FALLING);
  attachInterrupt(encoder.SW_PIN, sw_isr, FALLING);
#ifdef debug
  Serial.begin(115200);
#endif
}

void loop()
{
#ifdef debug
  Serial.println("-------------------------------- Start --------------------------------");
#endif
  main_light_value = analogRead(main_light_sensor);
  int current_sub_light_value = analogRead(light_sensor);
  if (main_light_value < 500)
  {
    digitalWrite(fill_light, HIGH);
  }
  else
  {
    digitalWrite(fill_light, LOW);
  }
#ifdef debug
  Serial.println(main_light_value);
  Serial.println(current_sub_light_value);
  Serial.println(sub_light_value);
#endif
  if (song.IsChange())
  {
    tft.loadFont(font_12);
    tft.drawString(song.title(), 0, 0);
    tft.drawString(song.album(), 0, 57.5);
    tft.drawString(song.artist(), 0, 77.5);
    tft.unloadFont();
    song.set_change(false);
#ifdef debug
    Serial.println(song.title());
#endif
  }
  if (current_sub_light_value >= 100 && ((current_sub_light_value - sub_light_value) > 500) && !is_play)
  {
    a2dp_sink.play();
    is_play = true;
  }
  if (current_sub_light_value < 100 && ((sub_light_value - current_sub_light_value) > 500) && is_play)
  {
    a2dp_sink.pause();
    is_play = false;
  }
  if (button_up.pressed && is_play)
  {
    a2dp_sink.previous();
    button_up.pressed = false;
  }
  if (button_down.pressed && is_play)
  {
    a2dp_sink.next();
    button_down.pressed = false;
  }
  if (encoder.clockwise)
  {
    if ((volume + 5) <= 100)
      volume += 5;
    encoder.clockwise = false;
    a2dp_sink.set_volume(volume);
  }
  if (encoder.counterclockwise)
  {
    if ((volume - 5) >= 0)
      volume -= 5;
    encoder.counterclockwise = false;
    a2dp_sink.set_volume(volume);
  }
  if (encoder.pressed)
  {
    if (!is_play)
    {
      a2dp_sink.play();
      is_play = true;
    }
    else
    {
      a2dp_sink.pause();
      is_play = false;
    }
    encoder.pressed = false;
  }
  delay(1000);
  sub_light_value = current_sub_light_value;
#ifdef debug
  Serial.println("-------------------------------- End --------------------------------");
#endif
}