/**
 Wifi driver board using the ESP8266 on a H801 WIFI LED Controller

 Make sure you install the ESP8266 board:
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// RGB FET
// SWAPPED RED AND W1 in enschede
constexpr uint8_t led_pin_red   = 14;
constexpr uint8_t led_pin_green = 13;
constexpr uint8_t led_pin_blue  = 12;

// W FET
constexpr uint8_t led_pin_w1    = 15;
constexpr uint8_t led_pin_w2    = 4;

// onboard green LED D1
constexpr uint8_t led_pin_onboard_green = 5;
// onboard red LED D2
constexpr uint8_t leD_pin_onboard_red   = 1;

constexpr char ssid[] = "YOURSSID";
constexpr char password[] = "SECRET!";
constexpr char mqtt_server[] = "mqtthost.lan";
constexpr short mqtt_port = 1883;

constexpr char mqtt_prefix[]            = "/devices/dimmer/";
constexpr char mqtt_w1_status[]         = "/w1/status";
constexpr char mqtt_w2_status[]         = "/w2/status";
constexpr char mqtt_w1_status_set[]     = "/w1/status_set";
constexpr char mqtt_w2_status_set[]     = "/w2/status_set";
constexpr char mqtt_w1_brightness[]     = "/w1/brightness";
constexpr char mqtt_w2_brightness[]     = "/w2/brightness";
constexpr char mqtt_w1_brightness_set[] = "/w1/brightness_set";
constexpr char mqtt_w2_brightness_set[] = "/w2/brightness_set";
constexpr char mqtt_fade_time[]         = "/fadetime";
constexpr char mqtt_fade_time_set[]     = "/fadetime_set";

constexpr uint8_t led_index_r = 0;
constexpr uint8_t led_index_g = 1;
constexpr uint8_t led_index_b = 2;
constexpr uint8_t led_index_w1 = 3;
constexpr uint8_t led_index_w2 = 4;

WiFiClient espClient;
PubSubClient client(espClient);
uint8_t mac[6];
unsigned long lastLedCheck = 0;
char friendly_mac_address[18] = {0};

constexpr short led_loop_time = 20;

short led_brightness_current[5] = {0};
short led_brightness[5] = {0};
short led_step_size[5] = {0};
bool  led_status[5] = {0};
short led_fade_time = 200;
uint8_t led_pins[] = {led_pin_red, led_pin_blue, led_pin_green, led_pin_w1, led_pin_w2};

/**
Start WiFi connection
**/
void setup_wifi() {

  delay(10);
  // Connect to the local WiFi network
  Serial1.println();
  Serial1.print("Connecting to ");
  Serial1.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }

  randomSeed(micros());

  Serial1.println("");
  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());
}

/**
Update led status to new value
**/
void set_led_status(uint8_t led, bool status) {
  led_status[led] = status;
  Serial1.print("Led status ");
  Serial1.print(String(led));
  Serial1.print(" to ");
  Serial1.print(String(status));
  Serial1.println();

  const char* topic;
  std::string device_prefix(mqtt_prefix);
  device_prefix += std::string(friendly_mac_address);
  if(led == led_index_w1) {
    topic = (device_prefix + mqtt_w1_status).c_str();
  } else {
    topic = (device_prefix + mqtt_w2_status).c_str();
  }
  client.publish(topic, String(led_status[led]).c_str());
}

/**
Update led brightness to new value
**/
void set_led_brightness(uint8_t led, uint16_t status) {
  led_brightness[led] = status;
  short difference = led_brightness[led] - led_brightness_current[led];
  short number_of_steps = (led_fade_time / led_loop_time);
  led_step_size[led] = difference / number_of_steps;

  // Print a bit to inform of changing brightness
  Serial1.print("Led brightness ");
  Serial1.print(String(led));
  Serial1.print(" to ");
  Serial1.print(String(status));
  Serial1.println();

  const char* topic;
  std::string device_prefix(mqtt_prefix);
  device_prefix += std::string(friendly_mac_address);
  if(led == led_index_w1) {
    topic = (device_prefix + mqtt_w1_brightness).c_str();
  } else {
    topic = (device_prefix + mqtt_w2_brightness).c_str();
  }
  client.publish(topic, String(led_brightness[led]).c_str());
}

void set_fade_time(uint16_t fadetime) {
  led_fade_time = fadetime;
  Serial1.print("Led fadetime ");
  Serial1.print(String(fadetime));
  Serial1.println();

  const char* topic;
  std::string device_prefix(mqtt_prefix);
  device_prefix += std::string(friendly_mac_address);
  topic = (device_prefix + mqtt_fade_time).c_str();
  client.publish(topic, String(led_fade_time).c_str());
}

void check_leds() {
  for (int i = 0; i < sizeof(led_status); ++i) {
    short step = led_step_size[i];
    short target = led_brightness[i];
    short current = led_brightness_current[i];

    if(!led_status[i]) {
      analogWrite(led_pins[i], 0);
    } else if(current != target) {

      if((step > 0 && (current + step) < target) || (step < 0 && (current + step) > target)) {
        led_brightness_current[i] = current + step;
      } else {
        led_brightness_current[i] = target;
      }
      analogWrite(led_pins[i], led_brightness_current[i]);
    }
  }
}

/**
Called on each message from broker to device
**/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial1.print("Message arrived [");
  Serial1.print(topic);
  Serial1.print("] ");
  Serial1.println();

  topic += sizeof(mqtt_prefix) + sizeof(friendly_mac_address) - 2;

  Serial1.print(topic);
  Serial1.println();

  
  if(strncmp(topic, mqtt_w1_status_set, sizeof(mqtt_w1_status_set)) == 0) {
    bool status = ((char)payload[0] == '1');
    set_led_status(led_index_w1, status);

  } else if(strncmp(topic, mqtt_w2_status_set, sizeof(mqtt_w2_status_set)) == 0) {
    bool status = ((char)payload[0] == '1');
    set_led_status(led_index_w2, status);

  } else if(strncmp(topic, mqtt_w1_brightness_set, sizeof(mqtt_w1_brightness_set)) == 0) {
    payload[length] = 0;
    int level = atoi((char*)payload);
    set_led_brightness(led_index_w1, level);
  } else if(strncmp(topic, mqtt_w2_brightness_set, sizeof(mqtt_w2_brightness_set)) == 0) {
    payload[length] = 0;
    int level = atoi((char*)payload);
    set_led_brightness(led_index_w2, level);
  } else if(strncmp(topic, mqtt_fade_time_set, sizeof(mqtt_fade_time_set)) == 0) {
    payload[length] = 0;
    int fadetime = atoi((char*)payload);
    set_fade_time(fadetime);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial1.println("connected to mqtt");

      // Once connected, publish an announcement...
      std::string device_prefix(mqtt_prefix);
      device_prefix += std::string(friendly_mac_address);
      client.publish((device_prefix + mqtt_w1_status).c_str(), String(led_status[led_index_w1]).c_str());
      client.publish((device_prefix + mqtt_w2_status).c_str(), String(led_status[led_index_w2]).c_str());
      client.publish((device_prefix + mqtt_w1_brightness).c_str(), String(led_brightness[led_index_w1]).c_str());
      client.publish((device_prefix + mqtt_w2_brightness).c_str(), String(led_brightness[led_index_w2]).c_str());
      client.publish((device_prefix + mqtt_fade_time).c_str(), String(led_fade_time).c_str());
      
      client.subscribe((device_prefix + mqtt_w1_status_set).c_str());
      client.subscribe((device_prefix + mqtt_w2_status_set).c_str());
      client.subscribe((device_prefix + mqtt_w1_brightness_set).c_str());
      client.subscribe((device_prefix + mqtt_w2_brightness_set).c_str());
      client.subscribe((device_prefix + mqtt_fade_time_set).c_str());
      client.subscribe((device_prefix + mqtt_fade_time_set).c_str());
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  for(int i = 0; i < sizeof(led_pins); ++i) {
    analogWrite(i, 0);
  }

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial1.begin(115200);
  setup_wifi();
  WiFi.macAddress(mac);
  for (int i = 0; i < sizeof(mac); ++i){
    snprintf(friendly_mac_address + i*3, sizeof(friendly_mac_address) - i*3, "%02x:", mac[i]);
  }
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if(now - lastLedCheck >= led_loop_time) {
    check_leds();
    lastLedCheck = now;
  }
}

