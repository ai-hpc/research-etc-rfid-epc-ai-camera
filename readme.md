# Electronic Toll Collection Research Topics

This repository contains four Master's thesis research directions for RFID-based Electronic Toll Collection (ETC) systems. Each topic starts from a baseline ETC system and proposes a specific technical improvement.

## 1. Secure EPC Metadata Encoding and AI-Based Vehicle Verification

Original system baseline: RFID-only ETC lane that trusts the EPC/tag identity without checking whether the physical vehicle matches the tag.

Research focus: secure EPC metadata encoding, cryptographic validation, and AI camera-based vehicle verification for fraud detection.

### Research Title

Secure EPC Metadata Encoding and AI-Based Vehicle Verification for Real-Time Fraud Detection in RFID-Based Electronic Toll Collection Systems

### 3-Sentence Summary

RFID-only ETC systems can be abused when a tag is cloned, swapped to another vehicle, or generated without authorization. This research embeds lightweight vehicle identity metadata into the RFID EPC and verifies it against AI camera-based vehicle class, color, and plate-last-four recognition. The expected outcome is a real-time fraud detection method that flags forged, cloned, or swapped tags before the transaction is accepted as normal.

### Objectives / Key Contributions

1. Encode lightweight vehicle metadata into EPC data for fast edge-side decoding.
2. Detect cloned or swapped tags by comparing EPC metadata with AI camera recognition.
3. Secure EPC generation using keyed validation, hash integrity protection, and checksum verification.

### Most Important References

1. GS1 EPC Tag Data Standard: https://ref.gs1.org/standards/tds/
2. NIST FIPS 198-1, The Keyed-Hash Message Authentication Code: https://csrc.nist.gov/pubs/fips/198-1/final

## 2. Real-Time Autonomous Detection of Unpaid and No-Tag Vehicles

Original system baseline: RFID-based ETC lane that depends on a valid tag read and may miss vehicles with no tag, unreadable tags, or failed payment.

Research focus: multi-modal no-tag and unpaid-vehicle detection using RFID, AI camera monitoring, multi-object tracking, plate recognition, and lane/speed sensors.

### Research Title

Research on Real-Time Autonomous Detection of Unpaid and No-Tag Vehicles in RFID-Based Electronic Toll Collection Systems Using Multi-Modal Verification

### 3-Sentence Summary

RFID-only ETC systems can fail to enforce payment when a vehicle passes without a readable tag or when payment fails after a tag is detected. This research combines 24/7 AI camera monitoring, multi-object tracking, RFID sensing, plate recognition, payment validation, and lane/speed sensors to detect no-tag and unpaid passages in real time. The expected outcome is an autonomous enforcement framework that generates evidence-backed violation candidates without requiring vehicles to stop.

### Objectives / Key Contributions

1. Detect no-tag vehicles by associating continuous camera tracks with RFID absence and lane-crossing events.
2. Detect unpaid vehicles by linking RFID reads with payment status and physical vehicle passage.
3. Integrate RFID, camera, plate, vehicle-attribute, and sensor evidence into a real-time violation decision engine.

### Most Important References

1. ISO 12855:2025, Electronic fee collection information exchange: https://www.iso.org/standard/88211.html
2. ByteTrack, Multi-Object Tracking by Associating Every Detection Box: https://arxiv.org/abs/2110.06864

## 3. Predictive Maintenance for Passive RFID Readers and Antennas

Original system baseline: passive RFID ETC infrastructure monitored mainly by offline status or manual inspection after read reliability has already degraded.

Research focus: AI-based predictive maintenance using reader telemetry, antenna-port statistics, read-rate trends, RF indicators, and anomaly detection.

### Research Title

Research on AI-Based Predictive Maintenance for Passive RFID Readers and Antennas Using Telemetry, Read-Reliability Analytics, and Time-Series Anomaly Detection

### 3-Sentence Summary

Passive RFID ETC lanes can lose read reliability when readers, antennas, cables, power, network links, or RF conditions degrade before a hard failure is reported. This research collects 24/7 reader and antenna telemetry and applies anomaly detection, health scoring, and diagnostic rules to predict infrastructure faults early. The expected outcome is a proactive maintenance system that warns operators before missed reads affect toll collection reliability.

### Objectives / Key Contributions

1. Collect reader, antenna-port, RF, network, and environmental telemetry for continuous health monitoring.
2. Detect early degradation using read-rate baselines, RSSI trends, reference-tag behavior, and anomaly scoring.
3. Develop per-reader and per-antenna health scores with evidence-backed maintenance recommendations.

### Most Important References

1. RAIN RFID System Design Guidelines: https://rainrfid.org/wp-content/uploads/2023/09/RAIN-RFID_System_Design_Guidelines-V2.pdf
2. Isolation-Based Anomaly Detection: https://cs.nju.edu.cn/zhouzh/zhouzh.files/publication/tkdd11.pdf

## 4. Encrypted Passive RFID Read Access

Original system baseline: passive RFID ETC or asset-identification system where EPC values and tag memory can be read by compatible readers without strong privacy protection.

Research focus: encrypted passive RFID reads using pseudonymous EPC tokens, encrypted metadata payloads, authenticated decryption, and cryptographic tag authentication where supported.

### Research Title

Research on Secure and Encrypted Passive RFID Read Access Using Cryptographic Tag Authentication, Encrypted Metadata Payloads, and Privacy-Preserving EPC Design

### 3-Sentence Summary

Passive RFID systems can expose sensitive identity data when EPC values or tag memory contain plaintext information. This research protects readable tag data by using random EPC tokens, encrypted metadata payloads, authenticated backend decryption, and optional Gen2v2/ISO 29167 cryptographic tag authentication. The expected outcome is a secure read architecture that reduces unauthorized data exposure and improves resistance to clone or replay attacks when crypto-capable tags are available.

### Objectives / Key Contributions

1. Encrypt sensitive metadata stored on passive RFID tags so unauthorized reads reveal only ciphertext.
2. Secure tag identity by replacing plaintext EPC data with random or pseudonymous lookup tokens.
3. Integrate cryptographic tag authentication and backend replay checks for stronger clone resistance.

### Most Important References

1. ISO/IEC 29167-10, AES-128 security services for RFID air interface communications: https://www.iso.org/standard/88959.html
2. NIST SP 800-38D, Galois/Counter Mode for authenticated encryption: https://csrc.nist.gov/pubs/sp/800/38/d/final
