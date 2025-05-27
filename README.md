# ESP32VirtualPet_devmodex
epaper virtual pet HUGE update


ESP32 Virtual Pet
<img alt="ESP32 Virtual Pet" src="https://via.placeholder.com/800x400?text=ESP32+Virtual+Pet">
Overview
This is my ESP32-based virtual pet project that combines hardware tinkering with nostalgic digital pet gameplay. I created this project to explore e-paper display technology while building something fun and interactive.

The virtual pet lives on a 1.54" Waveshare e-paper display and features animations, score tracking, battery management, and time display. Your pet reacts to button presses and can be interacted with through a custom menu system.

Features
⚡ Smooth animations powered by an ESP32 microcontroller
🕒 Real-time clock synchronized via WiFi/NTP
🔋 Virtual battery simulation
📊 Score tracking system
📱 Interactive menu system
💤 Power-efficient e-paper display
🔄 Double-click to access menu
📈 Score increases when you interact with your pet
Hardware Requirements
To build your own, you'll need:

ESP32 development board
Waveshare 1.54" e-Paper display module
2× Push buttons
Jumper wires
Micro USB cable
3D printed case (optional)
Software Dependencies
The project uses the following libraries:

Arduino core for ESP32
Waveshare e-Paper ESP32 driver library
ESP32 WiFi libraries
SNTP (Simple Network Time Protocol)
How It Works
I designed the program with several key components:

Animation System
The pet has different animation states handled by a simple frame-based animation system. The main loop cycles through animation frames at a configurable rate (currently 200ms per frame). After completing several animation cycles, the pet "falls asleep" until you interact with it again.

Time Management
The ESP32 connects to WiFi and synchronizes time with NTP servers. I added a custom 12-hour time format display since the Waveshare library only supports 24-hour format by default. The time is shown at the bottom of the screen and updates every second.

Button Interaction
Two buttons control the interface:

The main button wakes up the pet, increases your score, and selects menu items
A double-click on the main button opens the settings menu
The scroll button navigates through menu options
Menu System
I implemented a simple menu system that lets you:

Reset your score
Reset the battery level
Exit the menu
The menu is designed to be expandable for future features.

Battery Simulation
A virtual battery indicator shows in the top status bar. The battery gradually depletes over time to simulate a real virtual pet. You can recharge it through the menu.

Installation
Clone this repository or download the source code
Install the required libraries through Arduino Library Manager
Connect your ESP32 to the e-paper display according to the pin definitions
Update the WiFi credentials in the code
Compile and upload to your ESP32
Configuration
Adjust these constants to customize behavior:

Usage
Power on the device
Wait for WiFi connection and time synchronization
Press the main button to interact with your pet
Double-click to enter the menu
Use the scroll button to navigate menu options
Press the main button to select a menu item
Enjoy watching your virtual pet animate!
Technical Notes
The program uses the ESP32's SPI interface to communicate with the e-paper display. I implemented partial refresh mode to improve animation performance, though e-paper displays are still relatively slow compared to traditional LCDs.

The power consumption is very low due to the e-paper display's ability to maintain an image without power. This makes it perfect for battery-powered applications.

Future Improvements
I'm planning to add:

More pet animations and interactions
Mini-games to earn points
Custom pet naming
Battery-saving sleep mode
Weather display integration
License
This project is open source and available under the MIT License.

Built with ❤️ by me
