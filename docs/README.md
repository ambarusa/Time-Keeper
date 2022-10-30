# Time Keeper

## How to intergrate in Home Assistant:
#### Setting up a Mosquitto MQTT Broker:
- Install mosquitto:
```
sudo apt-get install mosquitto
sudo apt-get install mosquitto-clients
```

- Set up a user
```
sudo systemctl stop mosquitto
sudo mosquitto_passwd -c /etc/mosquitto/passwd <user_name>
sudo nano /etc/mosquitto/mosquitto.conf
```
In mosquitto.conf add the following lines at the bottom:
```
password_file /etc/mosquitto/passwd
allow_anonymous false
```
```
sudo systemctl start mosquitto
```

#### Configuring Home Assistant:
Add the following section to configuration.yaml to integrate the MQTT Broker. 
```
mqtt:
  broker: localhost
  client_id: homeassistant
  username: <user_name>
  password: <password>
  discovery: true
  discovery_prefix: homeassistant
```  

Next, integrate the Fleurie, as a light entity, by adding the following section to configuration.yaml
```  
light:
  - platform: mqtt
    schema: json
    name: "Fleurie"
    state_topic: "fleurie"
    command_topic: "fleurie/set"
    brightness: true
    brightness_scale: 97
    qos: 1
    retain: true
```
or make a lights.yaml file, then add the following section.

```
- platform: mqtt
  schema: json
  name: "Fleurie"
  state_topic: "fleurie"
  command_topic: "fleurie/set"
  brightness: true
  brightness_scale: 97
  qos: 1
  retain: true
...
```

Then add the following line to configuration.yaml:
```
light: !include lights.yaml
```
- Restart Home Assistant
- Fleurie should be configured, make a light card for it.
