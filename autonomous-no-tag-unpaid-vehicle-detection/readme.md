# Real-Time Autonomous Detection of Unpaid and No-Tag Vehicles

## Research Title

Research on Real-Time Autonomous Detection of Unpaid and No-Tag Vehicles in RFID-Based Electronic Toll Collection Systems Using Multi-Modal Verification

## Overview

This research proposes a real-time toll enforcement framework for detecting vehicles that pass through RFID-based electronic toll collection lanes without a valid tag or without successful payment.

The system combines RFID sensing, continuous AI camera monitoring, multi-object vehicle tracking, plate recognition, vehicle attribute analysis, and lane/speed sensors. Its goal is to identify unauthorized toll passages without requiring the vehicle to stop.

## References

Standards, research papers, and implementation references are listed here:

- [References](references.md)

## Three-Sentence Summary

This research aims to automatically detect vehicles passing through ETC lanes without a valid RFID tag or without payment. The system combines RFID sensing, 24/7 AI camera monitoring, multi-object tracking, plate recognition, vehicle attribute analysis, and lane/speed sensors to identify unpaid or no-tag vehicles in real time. It also generates evidence and triggers violations autonomously without requiring vehicles to stop, enabling high-speed multi-lane toll enforcement.

## Problem Statement

RFID-based ETC systems are vulnerable to enforcement gaps when a vehicle passes through a toll lane without a readable tag, with an invalid tag, or with a failed payment transaction. If the system depends only on RFID reads, a vehicle with no tag or a missed tag read may pass without immediate accountability.

This research addresses that problem by using multi-modal sensing. Vehicle presence is detected independently from RFID, and camera evidence is matched with RFID and payment status to determine whether the passage is valid, suspicious, or a violation.

## Research Objectives

- Detect no-tag vehicles by combining RFID absence detection with continuous AI camera monitoring, multi-object tracking, and lane/speed sensors.
- Detect unpaid vehicles by checking RFID account balance, transaction validity, or failed payment status in real time.
- Generate evidence packages containing plate, vehicle image, timestamp, lane ID, class/color attributes, and transaction status.
- Support high-speed toll enforcement without requiring vehicles to stop.
- Reduce false violations by using track-based, time-windowed sensor fusion instead of relying on a single signal.

## Core Idea

The system separates vehicle presence detection from RFID payment validation. Unlike an RFID-trigger-only system, the AI camera must monitor the toll zone continuously because no-tag vehicles do not create an RFID trigger.

```text
AI camera continuously detects and tracks vehicles
        |
        v
Lane/speed sensor confirms toll-zone crossing
        |
        v
RFID/payment data is associated with the vehicle track
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

The AI camera module runs continuously and maintains vehicle tracks in the toll zone.

It extracts:

- license plate number
- plate last four digits
- vehicle class
- vehicle color
- vehicle image crop
- vehicle track ID
- lane assignment
- virtual-line crossing timestamp
- confidence values

For edge hardware, the first version should focus on plate recognition, broad vehicle class, and color. Exact make/model recognition can be avoided to reduce compute load.

### Multi-Object Tracking Module

The multi-object tracking module assigns a stable temporary ID to every detected vehicle while it moves through the toll zone.

It is responsible for:

- tracking multiple vehicles at the same time
- maintaining vehicle identity across frames
- estimating lane association
- detecting virtual-line crossing
- linking camera evidence to RFID reads and payment status
- preventing evidence from one vehicle being assigned to another vehicle

Suitable tracking approaches include SORT, Deep SORT, ByteTrack, or OC-SORT. For RK3576-class edge devices, a lightweight detector with ByteTrack or SORT-style tracking is a practical first implementation.

### Fusion and Decision Engine

The fusion engine combines RFID, payment, camera, and lane/sensor data inside a short time window.

Example association window:

```text
Track crossing time: T
RFID search window: T - 500 ms to T + 500 ms
Payment window:     T - 500 ms to T + 1500 ms
Evidence window:    track start to track exit
```

The exact window should be tuned based on lane geometry, vehicle speed, RFID reader placement, and camera position.

## Real-Time Workflow

```text
1. AI camera continuously detects vehicles in the toll zone.
2. Multi-object tracker assigns each vehicle a track ID.
3. Tracker estimates lane and detects virtual-line crossing.
4. RFID reader reports tag reads by timestamp, antenna, and lane.
5. Fusion engine associates RFID reads with active vehicle tracks.
6. Payment module validates tag/account/transaction status.
7. AI module extracts plate, class, and color evidence for each track.
8. System outputs valid, review, or violation.
```

## Runtime Operation

For this research, the AI camera and tracker should run 24/7.

Continuous components:

```text
- camera frame capture
- vehicle detection
- multi-object tracking
- lane assignment
- virtual-line crossing detection
- RFID listener
- payment event listener
```

Triggered or selective components:

```text
- high-quality plate crop saving
- OCR on selected frames
- evidence package generation
- final violation decision
```

This avoids missing no-tag vehicles while still controlling compute load. The system can run lightweight detection/tracking continuously and run expensive OCR only when a vehicle track crosses the enforcement line.

## No-Tag Detection Logic

A no-tag vehicle is detected when:

```text
vehicle_presence = true
vehicle_track_id exists
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
- the tracker maintained a stable track ID
- no RFID read exists in the allowed time window
- the camera detected a vehicle with sufficient confidence
- the sensor confirms the vehicle crossed the toll point

## Unpaid Detection Logic

An unpaid vehicle is detected when:

```text
vehicle_presence = true
vehicle_track_id exists
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
  "track_id": "L03-T000421",
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
  "tracking": {
    "track_start_utc": "2026-05-22T15:30:11.930Z",
    "line_crossing_utc": "2026-05-22T15:30:12.420Z",
    "track_end_utc": "2026-05-22T15:30:12.910Z",
    "id_switch_detected": false
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
- A 24/7 AI camera and multi-object tracking architecture for high-speed toll-zone monitoring.
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
- multi-object tracking accuracy
- ID switch rate
- virtual-line crossing accuracy
- lane association accuracy
- sensor fusion latency
- end-to-end decision time
- evidence completeness rate

## Research Questions

- How accurately can no-tag vehicles be detected by combining RFID absence with independent vehicle presence sensors?
- How quickly can unpaid RFID transactions be identified and linked to the correct vehicle?
- What time-window strategy best associates RFID reads, payment events, camera tracks, and lane/speed sensor events?
- Which multi-object tracking method is most reliable for toll-lane vehicle association under occlusion and high speed?
- How much can multi-modal verification reduce false violation decisions compared with RFID-only enforcement?
- What edge hardware configuration is sufficient for real-time multi-lane toll enforcement?

## Correct Claim

This system does not prevent a vehicle from physically passing through the toll lane. Instead, it detects and documents no-tag or unpaid passages in real time by combining independent vehicle presence sensing, RFID/payment status, and camera evidence.

The main enforcement value is autonomous evidence generation and violation flagging, not physical stopping.
