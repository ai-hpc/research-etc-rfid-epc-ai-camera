# Real-Time Autonomous Detection of Unpaid and No-Tag Vehicles

## Research Title

Research on Real-Time Autonomous Detection of Unpaid and No-Tag Vehicles in RFID-Based Electronic Toll Collection Systems Using Multi-Modal Verification

## Overview

This research proposes a real-time toll enforcement framework for detecting vehicles that pass through RFID-based electronic toll collection lanes without a valid tag or without successful payment.

The system combines RFID sensing, AI camera-based plate recognition, vehicle attribute analysis, and lane/speed sensors. Its goal is to identify unauthorized toll passages without requiring the vehicle to stop.

## Three-Sentence Summary

This research aims to automatically detect vehicles passing through ETC lanes without a valid RFID tag or without payment. The system combines RFID sensing, AI camera-based plate recognition, vehicle attribute analysis, and lane/speed sensors to identify unpaid or no-tag vehicles in real time. It also generates evidence and triggers violations autonomously without requiring vehicles to stop, enabling high-speed multi-lane toll enforcement.

## Problem Statement

RFID-based ETC systems are vulnerable to enforcement gaps when a vehicle passes through a toll lane without a readable tag, with an invalid tag, or with a failed payment transaction. If the system depends only on RFID reads, a vehicle with no tag or a missed tag read may pass without immediate accountability.

This research addresses that problem by using multi-modal sensing. Vehicle presence is detected independently from RFID, and camera evidence is matched with RFID and payment status to determine whether the passage is valid, suspicious, or a violation.

## Research Objectives

- Detect no-tag vehicles by combining RFID absence detection with AI camera-based vehicle recognition and lane/speed sensors.
- Detect unpaid vehicles by checking RFID account balance, transaction validity, or failed payment status in real time.
- Generate evidence packages containing plate, vehicle image, timestamp, lane ID, class/color attributes, and transaction status.
- Support high-speed toll enforcement without requiring vehicles to stop.
- Reduce false violations by using time-windowed sensor fusion instead of relying on a single signal.

## Core Idea

The system separates vehicle presence detection from RFID payment validation.

```text
Lane/speed sensor confirms vehicle passage
        |
        v
RFID system checks for valid tag/payment
        |
        v
AI camera captures vehicle and plate evidence
        |
        v
Fusion engine classifies transaction
        |
        v
Valid / review / violation
```

## Main Detection Cases

| Case | RFID Status | Payment Status | Camera/Sensor Status | Result |
| --- | --- | --- | --- | --- |
| Valid passage | Valid tag read | Payment success | Vehicle observed | Valid |
| No-tag vehicle | No tag read in time window | No payment | Vehicle observed crossing lane | Violation candidate |
| Invalid tag | Tag read but not registered/authorized | No payment | Vehicle observed | Violation candidate |
| Unpaid vehicle | Valid tag read | Balance insufficient or payment failed | Vehicle observed | Violation candidate |
| Ambiguous event | Weak/missing sensor or camera data | Unknown | Low confidence | Review |

## System Components

### RFID Module

The RFID module reads tag identifiers and reports:

- tag EPC
- read timestamp
- antenna/lane ID
- read strength or confidence
- account reference
- transaction status

### Payment Validation Module

The payment validation module checks whether the RFID transaction is financially valid.

It may verify:

- account existence
- tag authorization
- balance availability
- toll transaction success
- failed transaction reason

### Lane and Speed Sensor Module

Lane/speed sensors provide independent proof that a vehicle physically crossed the toll zone.

Possible sensors:

- inductive loop
- radar
- LiDAR
- axle counter
- infrared beam
- camera-based virtual line crossing

The key purpose is to detect a vehicle even when RFID is missing.

### AI Camera Module

The AI camera module captures visual evidence and extracts:

- license plate number
- plate last four digits
- vehicle class
- vehicle color
- vehicle image crop
- confidence values

For edge hardware, the first version should focus on plate recognition, broad vehicle class, and color. Exact make/model recognition can be avoided to reduce compute load.

### Fusion and Decision Engine

The fusion engine combines RFID, payment, camera, and lane/sensor data inside a short time window.

Example time window:

```text
Vehicle trigger time: T
RFID search window:  T - 500 ms to T + 500 ms
Camera frame window: T - 300 ms to T + 300 ms
```

The exact window should be tuned based on lane geometry, vehicle speed, RFID reader placement, and camera position.

## Real-Time Workflow

```text
1. Lane sensor detects vehicle entering toll zone.
2. Camera captures vehicle frame and plate image.
3. RFID reader searches for tag reads near the same timestamp and lane.
4. Payment module validates tag/account/transaction status.
5. AI module extracts plate, class, and color.
6. Fusion engine links sensor event, camera evidence, and RFID/payment result.
7. System outputs valid, review, or violation.
```

## No-Tag Detection Logic

A no-tag vehicle is detected when:

```text
vehicle_presence = true
rfid_tag_read = false
camera_vehicle_detected = true
vehicle_crossed_enforcement_line = true
```

Result:

```text
no_tag_violation_candidate
```

To reduce false positives, the system should confirm:

- the vehicle is inside the correct lane
- no RFID read exists in the allowed time window
- the camera detected a vehicle with sufficient confidence
- the sensor confirms the vehicle crossed the toll point

## Unpaid Detection Logic

An unpaid vehicle is detected when:

```text
vehicle_presence = true
rfid_tag_read = true
payment_status != success
vehicle_crossed_enforcement_line = true
```

Possible unpaid reasons:

- insufficient balance
- account suspended
- tag not authorized
- transaction timeout
- payment gateway failure
- tag/account mismatch

Result:

```text
unpaid_violation_candidate
```

## Evidence Package

Each violation candidate should generate an evidence record.

Example:

```json
{
  "event_id": "ETC-L03-20260522-153012-00421",
  "timestamp_utc": "2026-05-22T15:30:12.420Z",
  "lane_id": "L03",
  "violation_type": "no_tag",
  "rfid": {
    "tag_read": false,
    "epc": null,
    "payment_status": "none"
  },
  "vehicle": {
    "plate": "ABC6789",
    "plate_last4": "6789",
    "class": "truck",
    "color": "silver",
    "class_confidence": 0.94,
    "plate_confidence": 0.91
  },
  "sensor": {
    "speed_kmh": 72.4,
    "crossed_enforcement_line": true
  },
  "evidence": {
    "overview_image": "evidence/L03/event_00421_overview.jpg",
    "plate_image": "evidence/L03/event_00421_plate.jpg"
  },
  "decision": "violation_candidate"
}
```

## Decision States

| State | Meaning |
| --- | --- |
| `valid` | RFID and payment are valid, and vehicle passage is linked correctly |
| `review` | Evidence is incomplete or confidence is too low |
| `violation_candidate` | No-tag or unpaid passage is detected with sufficient evidence |

For operational or legal deployment, `violation_candidate` may still require human review depending on local enforcement rules.

## Expected Contributions

- A multi-modal no-tag detection method for RFID-based ETC lanes.
- A real-time unpaid transaction detection method linked to physical vehicle passage.
- A sensor fusion framework that connects RFID, payment status, AI camera evidence, and lane/speed sensors.
- A structured evidence generation model for autonomous toll enforcement.
- An evaluation method for real-time detection accuracy, latency, and false violation rate.

## Evaluation Metrics

The research can evaluate:

- no-tag detection rate
- unpaid vehicle detection rate
- false violation rate
- missed violation rate
- plate recognition accuracy
- vehicle class recognition accuracy
- lane association accuracy
- sensor fusion latency
- end-to-end decision time
- evidence completeness rate

## Research Questions

- How accurately can no-tag vehicles be detected by combining RFID absence with independent vehicle presence sensors?
- How quickly can unpaid RFID transactions be identified and linked to the correct vehicle?
- What time-window strategy best associates RFID reads, camera frames, and lane/speed sensor events?
- How much can multi-modal verification reduce false violation decisions compared with RFID-only enforcement?
- What edge hardware configuration is sufficient for real-time multi-lane toll enforcement?

## Correct Claim

This system does not prevent a vehicle from physically passing through the toll lane. Instead, it detects and documents no-tag or unpaid passages in real time by combining independent vehicle presence sensing, RFID/payment status, and camera evidence.

The main enforcement value is autonomous evidence generation and violation flagging, not physical stopping.
