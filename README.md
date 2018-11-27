# Battlebots-Robot
Remote controlled robot for a battle bots competition based on ESP8266 and Teensy MCU

Collaborators: Delaney Kaufman, Placid Unegbu, Lizzie Tu

## Overview

The robot was designed to compete in an arena and use its weapons to attack other robots. It aims to drive to a goal location and press a button for long enough to damage the other team. It has capabilties of healing based on recognizing a certain frequency of light from a beacon and various visual and acoustic indicators to signal that the touch receptors have registered a 'hit' on another robot. 

### Side View
![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/side%20view.png)

### Top View
![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/top%20view.png)

### Controller

![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/controller.png)


## Microcontrollers 

We used a total of three microcontrollers in our car and controller, and thus created three different programs. As seen in the architecture above, the robot has one ESP8266 and one Teensy MCU. The ESP8266 on the robot is responsible for WiFi communication and control of the melee servos and drive motors on board. The onboard ESP86266 also controls the health bar. The on-board Teensy is primarily responsible for sensor reading and control of the status indicator LEDs and team number display on the robot. 
	
The controller has one ESP866, which is responsible for reading four joysticks and sending the values over WiFi to the robot. 

## Electrical Design

![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/robot%20overview.png)
![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/controller%20overview.png)

Robot
	The robot has two onboard microcontrollers, one ESP8266 and one Teensy. Both the Teensy and ESP8266 were powered by one LiPo 2S (7.4V). Through the LM7805 voltage regulator, a 5V power line was established for the Teensy. Due to high current draw from both servos, one LM7805 5V regulator powered the ESP8266 and one servo, while an additional LM7805 powered the other servo.
The Teensy was used for sensor inputs and non-Wifi related display. The robot used three ADC pins to read values for two force sensitive resistors to detect melee impact and one phototransistor to detect healing. A seven segment LED screen with a driver backpack used I2C communication to display team color and number. An RGB WS2812B LED strip was wired to a GPIO to flash red upon a melee hit and white upon healing. An SPST switch was wired to 1 GPIO to toggle between team colors. A buzzer was wired to 1 GPIO to indicate a melee hit.
	All components requiring information from Wifi packets were wired to the ESP8266. All 10 non-flash GPIOs were used. The LED health bar used one SN74LS164N shift register and 2 GPIO pins. Two servos receiving position commands from the controller were wired to one GPIO pin each through one level shifter to shift the signal from 3.3V to 5V. The SN754410 H-bridge required two PWM pins and 4 directional GPIO pins, which drove two motors. 
	In order to implement the killswitch, the ESP8266 must send a signal to the Teensy to shut off all display lights. This was achieved by linking an ESP8266 pin (D3) set as output and a Teensy pin set as an input, with common ground. 

Controller
	The controller consisted of one ESP8266 and four two-axis joysticks. The four joystick inputs were multiplexed into the single ADC pin on the ESP8266 by sending power from one pin and using diodes. The MCU sequentially powered each sensor and read the corresponding ADC value at the time. 
	
![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/robot%20ESPP%20diagram.png)
![alt text](https://github.com/OmkarSavant/Battlebots-Robot/blob/master/controller%20diagram.png)


  

  
  
