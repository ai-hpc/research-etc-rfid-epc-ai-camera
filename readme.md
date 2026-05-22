# Secure EPC Metadata Encoding and AI-Based Vehicle Verification

## Research Title

Secure EPC Metadata Encoding and AI-Based Vehicle Verification for Real-Time Fraud Detection in RFID-Based Electronic Toll Collection Systems

## Overview

This research proposes a secure fraud detection framework for RFID-based electronic toll collection systems. The system embeds lightweight vehicle identity metadata into an RFID Electronic Product Code (EPC) and verifies the encoded information using AI camera-based vehicle recognition.

The goal is to detect forged or unauthorized RFID identities and flag cloned or swapped RFID tags when the vehicle observed at the toll lane does not match the EPC metadata.

## Problem Statement

RFID-based toll collection systems commonly depend on the RFID tag identity as the main authentication factor. This creates security risks because an RFID tag may be cloned, swapped to another vehicle, or generated without authorization.

If the system only verifies that the EPC exists in the database, a fraudulent vehicle may still pass the toll gate. This research addresses that weakness by combining secure EPC metadata encoding with AI-based visual verification.

## Research Objectives

- Encode vehicle identity metadata into RFID EPC for efficient real-time decoding.
- Include lightweight vehicle attributes such as vehicle class, vehicle color, and the last four digits of the plate number.
- Flag cloned or swapped RFID tags when EPC metadata does not match AI camera-based vehicle recognition.
- Secure EPC metadata generation using security-key-based encoding, public-key verification, hash integrity protection, and checksum validation.
- Evaluate the proposed system in terms of fraud detection accuracy, processing latency, false alarms, and resistance to unauthorized EPC generation.

## Proposed Solution

The proposed system combines four major components:

1. RFID EPC metadata encoding
2. Secure EPC generation and validation
3. AI-based vehicle class and color recognition
4. Real-time mismatch detection and fraud flagging

At the toll lane, the RFID reader captures the EPC while a camera captures the vehicle image. The system decodes and validates the EPC, then compares the encoded metadata with the AI recognition result. If the physical vehicle does not match the EPC metadata, the transaction is flagged as suspicious.

## Edge-Hardware Simplification

The current EPC design intentionally uses vehicle class instead of vehicle vendor or exact model. This reduces the complexity of the AI camera module because the edge device only needs to recognize broad classes such as car, truck, bus, van, and motorcycle.

Removing vendor/model metadata from the EPC also frees space for stronger integrity protection. The current EPC-96 generator uses 56 bits of metadata, 8 bits of CRC, and a 32-bit MAC.

## EPC Generator Documentation

The C++ EPC data generator is documented in detail here:

- [How the EPC Data Generator Works](epc_generator_working.md)
- [Vehicle Class Code Table](vehicle_class_codes.md)
- [RK3576 AI Camera Software](rk3576_ai_camera_software.md)

## RK3576 Edge Software

The RK3576 verifier is implemented in:

- [rk3576_ai_camera.py](rk3576_ai_camera.py)

It validates the EPC, reads camera or image input, performs vehicle class and color verification, and returns a JSON decision:

```text
valid / review / suspicious
```

## Runtime Operation

The system is designed to run continuously, but full AI inference should not run continuously.

Recommended operation:

```text
24/7 lightweight services:
- RFID listener
- camera ready or preview service
- EPC decoder and verifier
- event controller

Triggered only after RFID tag read:
- capture one to three camera frames
- run vehicle class inference
- run vehicle color detection
- optionally run plate-last-4 OCR
- compare camera result with EPC metadata
```

This event-driven approach reduces CPU/NPU load, heat, and power usage on RK3576 edge hardware while still supporting real-time toll-lane verification.

Target processing time after RFID read:

| Step | Target Time |
| --- | ---: |
| Camera capture | 50-150 ms |
| Vehicle class inference | 20-100 ms |
| Color detection | 5-20 ms |
| EPC comparison | <1 ms |
| Total edge decision | <300 ms |

## EPC Metadata Design

The EPC may contain compact fields such as:

| Field | Purpose |
| --- | --- |
| Version | Identifies the EPC encoding format |
| Issuer ID | Identifies the authorized tag issuer or toll operator |
| Vehicle class code | Encodes broad vehicle class such as car, truck, bus, or van |
| Vehicle color code | Encodes registered vehicle color |
| Plate last 4 digits | Adds lightweight plate identity verification |
| Account or tag reference | Links the tag to a backend registration record |
| Key ID | Identifies the cryptographic key used for validation |
| Hash or MAC | Protects metadata integrity |
| Checksum | Detects accidental transmission or decoding errors |

Because standard EPC memory is limited, especially in 96-bit EPC formats, the EPC should store only compact metadata and short validation fields. Full public-key signatures are usually better stored and verified through backend-issued records, certificates, or signed registration data.

## System Workflow

```text
Vehicle enters toll lane
        |
        v
RFID reader captures EPC
        |
        v
Decode EPC metadata
        |
        v
Validate checksum and cryptographic integrity
        |
        v
Camera captures vehicle image
        |
        v
AI model predicts vehicle class and color
        |
        v
Compare EPC metadata with AI result
        |
        v
Classify transaction as valid, suspicious, or fraudulent
```

## Security Design

The EPC generation process may follow this structure:

```text
Vehicle metadata + issuer ID + account reference + nonce
        |
        v
Canonical metadata encoding
        |
        v
Hash or MAC generation using issuer security key
        |
        v
Checksum generation
        |
        v
Final EPC value
```

The verification process checks:

- Whether the EPC format is valid.
- Whether the checksum is correct.
- Whether the hash or MAC matches the decoded metadata.
- Whether the issuer or key ID is authorized.
- Whether the backend registration record confirms the EPC assignment.
- Whether the AI-recognized vehicle attributes match the EPC metadata.

## Fraud Detection Scenarios

| Fraud Scenario | Detection Method |
| --- | --- |
| Cloned RFID tag with visible mismatch | Copied EPC appears on a vehicle with different class, color, or plate tail |
| Swapped RFID tag with visible mismatch | EPC metadata does not match the observed vehicle |
| Forged EPC | Cryptographic validation fails |
| Modified EPC metadata | Hash, MAC, or checksum validation fails |
| Unauthorized EPC generation | Public-key or backend issuer verification fails |
| Suspicious plate mismatch | Encoded plate digits do not match the vehicle record or plate recognition result |

## AI-Based Vehicle Verification

The AI camera module is responsible for recognizing vehicle attributes from the toll lane image. The main recognition targets are:

- Vehicle class such as car, truck, bus, or van
- Vehicle color
- Optional plate recognition for validating the last four plate digits

Exact vendor and model information such as Toyota Hilux or Honda Civic should be stored in the backend vehicle profile and linked through the EPC tag reference. This keeps edge verification simple while still allowing detailed backend checks.

The AI result is compared with the decoded EPC metadata. A confidence threshold can be used to reduce false alarms when image quality is poor or the vehicle is partially occluded.

## Correct Security Claim

The EPC MAC prevents unauthorized EPC generation and metadata modification when the secret key is protected. AI camera verification can flag cloned or swapped tags when the copied tag is used on a vehicle whose class, color, or plate tail does not match the EPC metadata.

The class-only EPC does not encrypt metadata, stop EPC reading, or prevent exact static EPC cloning. A cloned tag used on a matching vehicle may not be detected by edge class/color verification alone.

## Evaluation Metrics

The research can evaluate the proposed system using:

- EPC decoding time
- Cryptographic validation time
- AI vehicle recognition accuracy
- Fraud detection rate
- False positive rate
- False negative rate
- End-to-end toll lane processing latency
- Resistance to forged and modified EPC data
- Detection rate for cloned or swapped tags with class, color, or plate-tail mismatch

## Research Questions

- How can vehicle identity metadata be efficiently encoded into RFID EPC memory?
- How much cryptographic protection can be included within practical EPC size constraints?
- How accurately can AI-based vehicle recognition verify EPC-encoded vehicle attributes in real time?
- How effective is EPC-to-camera comparison for flagging cloned or swapped RFID tags with observable class, color, or plate-tail mismatch?
- What trade-off exists between security strength, EPC size, and toll lane processing speed?

## Expected Contribution

This research contributes a hybrid RFID and AI camera-based fraud detection framework for electronic toll collection systems. The main contribution is a secure EPC metadata encoding and validation approach that makes RFID identity verifiable against the physical vehicle in real time.

The proposed system improves toll collection security by preventing forged or modified EPC metadata and by flagging cloned or swapped RFID tag use when the observed vehicle metadata does not match the EPC.
