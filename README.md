# 🤖 Altronic Line Tracking RFID Robot
Autonomus line tracking robot for inventory management

## 📌 Project Overview
Altronic is a line tracking mobile robot designed to follow a predefined path and scan RFID tags placed on objects. 
The robot collects product information using RFID technology and can transmit data wirelessly.
This project is developed using the Arduino platform.
This project presents an autonomous warehouse inventory robot designed to improve stock management efficiency and accuracy. 
The system combines line-following navigation with RFID-based identification to automate item scanning and database synchronization in large storage environments.

The robot uses a predefined path navigation strategy, allowing it to move reliably through warehouse aisles without complex mapping algorithms.
As it travels, it scans inventory tags and updates digital records in real time.

## 🧠 Technology Used
- Microcontroller: Esp 32  
- Sensors: IR sensor for path tracing and navigation
- Ultrasonic: for obstacle avoidence
- buzzer: scan feedback alarm  
- Identification: RFID reader module  
- Motor Control: L298N motor driver
-  rfid tag: stores data of items

## ⚙️ Components
- esp32 board  
- DC motors 100 rpm  
- RF tag   
- RFID module  
- IR line sensors  
- Power supply system
- buck coverter
- motor drive
- servo motors mg90s 

## 🔄 Working Principle
The robot follows a marked line path inside the warehouse using IR sensors. When it reaches storage locations, 
the onboard RFID reader scans product tags attached to boxes or items. 
The collected data is transmitted wirelessly to update inventory records, 
minimizing human intervention and reducing operational errors.

## 📂 Project Structure
altronic-robot-project/
├── code/
├── images/
├── README.md

## 🚀 How to Use
1. Upload Arduino.ide code to microcontroller board.  
2. Power the robot system.  
3. Place robot on tracking line.  
4. Robot will automatically follow path and scan RFID tags.


## 👨‍💻 Author
md kaif mulla

## ⭐ Future Improvements
- Improve navigation accuracy  
- Add AI-based path optimization  
- Enhance wireless data transmission
