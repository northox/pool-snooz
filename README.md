# pool-snooz
Pool pump snoozer

This project allows you to control a pool pump using an ESP8266 microcontroller, with the ability to manage the pump via MQTT and integrate with Home Assistant. The ESP8266 listens for MQTT messages to control a relay connected to the pool pump, enabling you to remotely turn the pump on or off, as well as implement snooze and compensation features through Home Assistant.

## Features

- **Remote Control via MQTT**: Turn the pool pump on or off through MQTT messages.
- **Home Assistant Integration**: Seamlessly integrates with Home Assistant for manual control, scheduling, and advanced features like snooze and compensation.
- **WiFi Connectivity**: The ESP8266 connects to your WiFi network, allowing for easy remote access.
- **Fail-Safe Mechanism**: A watchdog timer ensures that the system resets if it becomes unresponsive.

## Hardware Requirements

- ESP8266 microcontroller (e.g., ESP-12f)
- Relay module to control the pool pump - A Normally CLOSED relay is best.
- 3.3V power supply

## Wiring Diagram

1. **Relay Module**:
   - **IN** → GPIO5
   - **GND** → GND
   - **NC** (Normally Closed) → Pump power line
   - **COM** (Common) → Pump power supply

2. **ESP8266**:
   - **VCC** → 3.3V
   - **GND** → GND & Relay IN
   - **GPIO5** → Relay IN

### 3. Home Assistant Integration

To control the pool pump via Home Assistant, follow these steps:

#### 3.1 Define an Input Boolean and Input Number

Add the following to your `configuration.yaml`:

```yaml
input_boolean:
  pool_pump_snooze:
    name: "Pool Pump Snooze"
    icon: mdi:sleep

input_number:
  pool_pump_snooze_time:
    name: "Total Snooze Time"
    initial: 0
    min: 0
    max: 600  # Max 10 hours
    step: 1
    unit_of_measurement: "minutes"
```

#### 3.2 Create Automations

Define automations to handle the pump control, snooze, and compensation:

```yaml
automation:
  - alias: "Snooze Pool Pump"
    trigger:
      - platform: state
        entity_id: input_boolean.pool_pump_snooze
        to: 'on'
    action:
      - service: mqtt.publish
        data:
          topic: "pool/pump/control"
          payload: "OFF"
          qos: 1
      - service: input_number.set_value
        data_template:
          entity_id: input_number.pool_pump_snooze_time
          value: >
            {{ (states('input_number.pool_pump_snooze_time') | int) + 30 }}
      - delay: '00:30:00'  # Snooze for 30 minutes
      - service: mqtt.publish
        data:
          topic: "pool/pump/control"
          payload: "ON"
          qos: 1
      - service: input_boolean.turn_off
        entity_id: input_boolean.pool_pump_snooze

  - alias: "Compensate Pool Pump Overnight"
    trigger:
      - platform: time
        at: "22:00:00"
    action:
      - service: mqtt.publish
        data:
          topic: "pool/pump/control"
          payload: "ON"
          qos: 1
      - delay:
          minutes: "{{ states('input_number.pool_pump_snooze_time') | int }}"
      - service: mqtt.publish
        data:
          topic: "pool/pump/control"
          payload: "OFF"
          qos: 1
      - service: input_number.set_value
        data:
          entity_id: input_number.pool_pump_snooze_time
          value: 0
```

#### 3.3 Add Controls to Lovelace Dashboard

Add the input boolean and input number to your Lovelace dashboard:

```yaml
views:
  - title: Pool Control
    cards:
      - type: entities
        title: Pool Pump Control
        entities:
          - input_boolean.pool_pump_snooze
          - input_number.pool_pump_snooze_time
```

### 4. Using the System

- **Manual Control**: Use the Home Assistant interface to manually snooze or run the pump.
- **Automation**: The system will automatically compensate for any snooze time by running the pump longer overnight.

### License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
