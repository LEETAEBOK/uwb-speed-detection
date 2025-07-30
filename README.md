# UWB-Based Tunnel Vehicle Over-Speed Detection System

> A low-cost, high-precision speed monitoring system using UWB DS-TWR and Kalman Filter for tunnel environments.

---

## 📌 Introduction

Conventional speed enforcement systems in tunnels rely on ANPR (Automatic Number Plate Recognition) cameras, which are costly and vulnerable to environmental conditions.

This project proposes an alternative system using **UWB (Ultra-Wideband)** with **DS-TWR (Double-Sided Two-Way Ranging)** to calculate a vehicle's average speed in a tunnel and detect over-speed in real-time.

---

## ⚙️ System Overview

- **UWB Module**: Decawave DWM1000
- **Tunnel Length**: 5 meters
- **Placement**:
  - Anchors: entrance & exit of tunnel
  - Tag: attached to front of vehicle
- **Over-speed Threshold**: 1.5 m/s

### Speed Calculation Formula:


Where:
- T1 = Timestamp at tunnel entry
- T2 = Timestamp at tunnel exit

---

## 🧠 Key Methods

### 🛰️ DS-TWR (Double-Sided Two-Way Ranging)

- Measures round-trip time (ToF) for accurate distance
- Corrects propagation delay and calculates position

### 🔧 Kalman Filter

- Reduces noise in UWB distance measurements
- Combines prediction and observation in real time

---

## 🚘 Experiment Results

| Trial | Real Speed [m/s] | Measured Speed [m/s] | Error Rate | Over-speed |
|-------|------------------|----------------------|------------|------------|
| 1     | 1.51             | 1.55                 | 1.65%      | ✅ Yes     |
| 2     | 1.84             | 1.78                 | 3.26%      | ✅ Yes     |
| 3     | 1.85             | 1.78                 | 3.78%      | ✅ Yes     |
| 4     | 1.58             | 1.65                 | 4.43%      | ✅ Yes     |
| 5     | 0.77             | 0.75                 | 2.67%      | ❌ No      |
| 6     | 0.66             | 0.68                 | 3.33%      | ❌ No      |
| 7     | 0.83             | 0.85                 | 2.40%      | ❌ No      |

---

## ✅ Advantages

| Factor                  | Traditional Camera System | UWB-Based System           |
|-------------------------|---------------------------|----------------------------|
| Installation Cost       | High                      | Low                        |
| Environmental Sensitivity | High                    | Low                        |
| Accuracy                | Moderate                  | High (with Kalman Filter) |
| Privacy                 | Low (image-based)         | High (no image capture)    |

- **Advisor**: Prof. 장병준  
- **Affiliation**: Kookmin University – Dept. of Electronic Engineering

---

## 📎 Keywords

`UWB` `DS-TWR` `Kalman Filter` `Over-Speed Detection` `Tunnel Monitoring` `Embedded Systems`
