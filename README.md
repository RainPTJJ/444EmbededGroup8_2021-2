# 2102444 (2021/2) Introduction to Embedded Systems
# Group 8 -> Project name : Medical Logistics (การขนส่งยาและเวชภัณฑ์)

# THIS IS JUST PROTOTYPE!!!

SET UP of this project:
1. DHT sensor pin (Humidity & Temperature)
2. LEDs pin
3. RFID pin
4. RTC pin (Real time clock)
5. 4x4 Matrix keypad pin
6. Button pin
7. Water level sensor pin
8. LCD pin
9. Buzzer pin

WHAT sensor which warning & display (INPUT):
1. Humidity
2. Temperature
3. Water level
4. LDR valuse (light)
5. Deadline Time

Process of this project:
1. Checking valid RFID to start Waring & Showing value system
2. Choosing Type of Medical Supply
3. Warning if Humidity, Temperature, Water level and Light ARE NOT in range : BUZZER & LED
    LED : RED-Humidity, Temperature, YELLOW-Light and GREEN-Water level
4. Pushing keypad to display command
    1-Humidity (%), 2-Temperature (Celcius), 3-Water level, 4-LDR value, 5-Deadline Time
5. Pushing blue button to open door (interrupt) 
6. Pushing 16 keypad to stop program

Variable tha you can reconfigure:
1. UID of valid RFID card
2. buzzer frequency
3. Add new type of Medical Supply (In based-program : (5 Types) 1-Organs 2-Blood 3-Morphine 4-Light-sensitive vaccines 5-Liquid Oxygen)
4. Configure the range of Humidity, Temperature, Water level and Light and Choose what sensor to use
