# MiniProject1 for IoT course
This Mini Project1, designed for the IoT course, centers around the utilization of an m3 board. The project facilitates the reading of temperature/pressure via "lps start/stop" commands and acceleration/magnetic force through "lsp start/stop" commands on the m3 node. Temperature and pressure data are collected every 30 seconds, while accelerometer and magnetometer data are gathered every 15 seconds. Subsequently, this information is transmitted to an AWS cloud server, specifically configured to receive these calls.

To get started, set up the RIOT environment following the instructions outlined at https://github.com/RIOT-OS/RIOT. Additionally, establish the border router by referring to https://www.iot-lab.info/learn/tutorials/riot/riot-public-ipv6-m3/.

Conduct an experiment in (FitIoT Lab)[https://www.iot-lab.info/] and submit the .elf file into one node. Locate the node's firmware in MiniProject/task01 by executing the command "make."
Once the firmware is flashed into the board you can activate the sensors, lps for temperature and pressure and lsm for acceleration and magnetic force, by:
```bash
lps start
lsm start
```
To stop the sensors from reading data you can also do:
```bash
lps stop
lsm stop
````


https://drive.google.com/file/d/1CCrxBQtsxZgoeXTnDD2AEPtg2aGmS_1H/view?usp=sharing

