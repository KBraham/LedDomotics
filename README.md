# LedDomotics
Configuration to connect ESP modules to Domotics


## Installation
To install LedDomotics the following components are required:

- A MQTT broker (http://mosquitto.org)
- Node-RED wiring tools (http://nodered.org)
- NodeMCU (https://github.com/nodemcu/nodemcu-firmware)

### MQTT
Follow the instructions at http://mosquitto.org/download/


### Setting up Node-RED
Execute the following commands to install Node-RED
    sudo npm install -g node-red
    node-red

Go to node-red_addr:1883 and start creating the links between all nodes. The following set-up is an example of a working Node-RED architecture for Domoticz to LedDomotics message delivery.

![Node-Red architecture](/../screenshots/node-red.png?raw=true)

The following function converts the Domoticz output (light level 0-31) to LedDomotics

```javascript
var position = msg.payload.svalue1;
var level = 0;
if(position === 0){
    
} else if(msg.payload.nvalue === 0) {
    
} else {
  // position will be between 0 and 31
  var minp = 0;
  var maxp = 31;

  // The result should be between 0 and 1023
  var minv = 0;
  var maxv = Math.log(1023);

  // calculate adjustment factor
  var scale = (maxv-minv) / (maxp-minp);

  level = Math.exp(minv + scale*(position-minp));
}

level = Math.floor(level);
return {"payload": {"led":2, "l": level, "ft": 1000}};
```

In this message `led` refers to the output pin of the ESP, `l` is the PWM level ranging from 0 to 1023 and `ft` is the fadetime, the effective time to use for a transition between levels.

### Running NodeMCU

NodeMCU is a platform to run lua scripts on the ESP8266.
Make sure to modify the settings in init2 for the correct wifi credentials and mq.lua for the correct MQTT-broker location.
To run the scripts simply flash the NodeMCU firmware and upload the three base files to the flash memory.

Each device will identify using its MAC address by default. By looking at the messages send via MQTT (tip: use the websocket interface of mosquitto) you will find a new topic with the correct identifier for each node.


## Connecting to Domoticz
The connection between nodes and Domoticz runs via MQTT. To enable this connection activate the MQTT plugin of Domoticz. Doing so makes Domoticz to send all updates via MQTT to the Domoticz/out topic. As Node-RED listens to this topic, it transforms the message to a shorter message processable by the ESP node.


