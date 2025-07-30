# Development of a UWB DS-TWR-Based Tunnel Vehicle Over-Speed Detection System

> A low-cost, high-accuracy tunnel speed enforcement system using UWB and Kalman Filter.

---

## 1. Introduction

Conventional tunnel speed enforcement systems use ANPR cameras, which are expensive and sensitive to environmental changes.

This project proposes a UWB (Ultra-Wideband)-based system using **DS-TWR (Double-Sided Two-Way Ranging)** and **Kalman Filter** for accurate, real-time speed detection.

---

## 2. Environment & Setup

<!-- UWB Module -->
<div style="display: flex; align-items: center; margin-bottom: 20px;">
  <img src="images/UWB.png" alt="UWB Module" width="250" style="margin-right: 20px;" />
  <div>
    <ul>
      <li><b>UWB Module:</b> Decawave DWM1000</li>
    </ul>
  </div>
</div>

<!-- Tunnel Setup -->
<div style="display: flex; align-items: center; margin-bottom: 20px;">
  <img src="images/Tunnel.png" alt="Tunnel Setup" width="300" style="margin-right: 20px;" />
  <div>
    <ul>
      <li><b>Tunnel Length:</b> 5 meters</li>
      <li><b>Tunnel Height:</b> 1 meter</li>
      <li><b>Anchor placement:</b> Entrance & Exit</li>
    </ul>
  </div>
</div>

<!-- Vehicle Tag Position -->
<div style="display: flex; align-items: center;">
  <img src="images/Car.png" alt="UWB tag on vehicle" width="320" style="margin-right: 20px;" />
  <div>
    <ul>
      <li><b>Tag placement:</b> Front of the vehicle</li>
    </ul>
  </div>
</div>


---

## 3. Process Overview

<img src="images/process_overview.png" alt="Process Overview" width="450" style="margin-right: 30px;" />
  <!-- 오른쪽: 평균 속도 수식과 설명 -->
  <div>
    <h3> Average Speed Formula</h3>
    <img src="images/Average_speed.png" alt="Average Speed Formula" width="200" style="margin-bottom: 10px;" />
    <ul>
      <li><b>T1:</b> Entry timestamp</li>
      <li><b>T2:</b> Exit timestamp</li>
    </ul>
  </div>
</div>

---

## 4. Methods


### DS-TWR (Double-Sided Two-Way Ranging)

<img src="images/TWR.png" alt="DS-TWR" width="500"/>

DS-TWR uses three message exchanges to calculate **Time of Flight (ToF)** and estimate distance precisely:


---

### Kalman Filter

Used to reduce noise from UWB measurements and improve real-time estimation:

<img src="images/Kalman_filter.png" alt="Kalman Filter" width="600"/>


Where:
- x̂k: Estimated distance at time step k  
- zk: Measured value (DS-TWR)  
- Pk: Estimation error  
- R: Measurement noise  
- Kk: Kalman gain  

---

## 5. Results

### Accuracy Table

| Trial | Real Speed (m/s) | Measured Speed (m/s) | Error Rate | Over-speed |
|-------|------------------|----------------------|------------|------------|
| 1     | 1.51             | 1.55                 | 1.65%      | Yes     |
| 2     | 1.84             | 1.78                 | 3.26%      | Yes     |
| 3     | 1.85             | 1.78                 | 3.78%      | Yes     |
| 4     | 1.58             | 1.65                 | 4.43%      | Yes     |
| 5     | 0.77             | 0.75                 | 2.67%      | No      |
| 6     | 0.66             | 0.68                 | 3.33%      | No      |
| 7     | 0.83             | 0.85                 | 2.40%      | No      |

### Kalman Filter vs Raw Measurements

<img src="images/speed.png" alt="Speed Graph" width="400"/>

---


## Conclusion & Effects

- **Low-cost** and **privacy-respecting** alternative to ANPR
- **Accurate even in dark / non-visible environments**
- Can be extended to predict routes or detect abnormal driving

---

## Project Info

- **Team**: LEETAEBOK
- **Advisor**: Prof. Jang Byeong-Jun  
- **University**: Kookmin University – Dept. of Electronic Engineering

---

## Keywords

`UWB` `DS-TWR` `Kalman Filter` `Tunnel` `Over-Speed Detection` `Embedded Systems`
