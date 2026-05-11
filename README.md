# 3D Spatial Mapping System



An embedded 3D spatial mapping system that performs full 360° distance scanning using a VL53L1X Time-of-Flight sensor mounted on a stepper motor assembly. Distance data is transmitted over UART to a PC, where MATLAB reconstructs the environment into a 3D point cloud and wireframe visualization.

The system was designed for indoor environment mapping applications such as hallways, rooms, and enclosed spaces.

---

## Features

- Full 360° planar scanning using a rotating ToF sensor
- Real-time distance acquisition over I2C
- Stepper motor controlled angular positioning
- UART serial communication to PC
- MATLAB-based 3D reconstruction and visualization
- Multi-sweep spatial mapping across the x-axis
- Real-time LED system status indicators
- Embedded firmware written using register-level programming

---

## System Overview

The system performs spatial mapping by rotating a VL53L1X Time-of-Flight sensor through a complete 360° sweep. At each angular position, the microcontroller triggers a ranging measurement and stores the returned distance value.

After completing a sweep, the collected data is transmitted to a MATLAB visualization script running on a PC. MATLAB converts the polar measurements into Cartesian coordinates and renders a 3D point cloud representation of the scanned environment.

Multiple sweeps can be combined by manually translating the system along the x-axis to create a complete 3D reconstruction.

---

## Hardware Components

- **Microcontroller:** TI MSP432E401Y
- **Distance Sensor:** VL53L1X Time-of-Flight Sensor
- **Motor:** 28BYJ-48 Stepper Motor
- **Motor Driver:** ULN2003 Driver Board
- **Communication Interfaces:**
  - I2C (Sensor Communication)
  - UART (PC Communication)
- **Status Indicators:** GPIO-controlled LEDs
- **Input:** Push button trigger

---

## System Architecture

```text
VL53L1X ToF Sensor
        │
        │ I2C
        ▼
TI MSP432E401Y MCU
        │
        ├── Stepper Motor Control
        ├── LED Status Indicators
        └── UART Serial Output
                    │
                    ▼
             MATLAB Visualization
                    │
                    ▼
           3D Point Cloud Model
```

---

## Scanning Process

1. User initiates a scan using the push button
2. Stepper motor rotates sensor in 11.25° increments
3. Distance measurements are captured at each position
4. Distance data is transmitted over UART
5. MATLAB converts polar data into Cartesian coordinates
6. A 3D visualization is rendered from multiple sweeps

---

## Coordinate Conversion

Measured distance values are converted from polar coordinates into Cartesian space using:

\[
y = r \cos(\theta)
\]

\[
z = r \sin(\theta)
\]

where:

- `r` = measured distance
- `θ` = angular position of the sensor

The x-axis position is incremented between sweeps to build the final 3D spatial model.

---

## Technical Specifications

| Parameter | Value |
|---|---|
| System Clock | 36 MHz |
| Angular Resolution | 11.25° |
| Steps per Revolution | 32 |
| UART Baud Rate | 115200 bps |
| I2C Clock Speed | 30 kHz |
| Distance Resolution | 1 mm |
| Power Supply | USB 5V |

---

## Example Output

The MATLAB visualization reconstructs the scanned environment into:

- 3D point cloud maps
- Wireframe hallway models
- Cross-sectional sweep rings
- Spatial depth representations

Example outputs include hallway and indoor environment reconstructions generated from successive 360° scans.

---

## Engineering Highlights

- Register-level peripheral configuration
- Embedded real-time sensor acquisition
- Stepper motor sequencing logic
- UART protocol implementation
- Sensor polling and synchronization
- 3D geometric reconstruction
- MATLAB data visualization pipeline

---

## Limitations

- Scan speed is limited primarily by ToF sensor acquisition timing
- Manual x-axis translation between sweeps
- Indoor-only operation
- Reflective surfaces can affect ranging accuracy

---

## Future Improvements

- Automated x-axis motion control
- Real-time live visualization
- SLAM-based localization
- Higher resolution scanning
- Wireless data transmission
- Autonomous navigation integration

---

## Technologies Used

- Embedded C
- MATLAB
- UART Communication
- I2C Communication
- Real-Time Embedded Systems
- Sensor Integration
- 3D Spatial Reconstruction

---

## Author

Adam Syed

```
