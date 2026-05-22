# Encrypted Passive RFID Read Access

## Research Title

Research on Secure and Encrypted Passive RFID Read Access Using Cryptographic Tag Authentication, Encrypted Metadata Payloads, and Privacy-Preserving EPC Design

## Overview

This research proposes a secure read framework for passive RFID systems where tag data should not be exposed to unauthorized readers or useful to eavesdroppers.

The system protects passive RFID reads by combining privacy-safe EPC design, encrypted tag payloads, backend decryption, cryptographic integrity validation, and optional Gen2v2/ISO 29167 tag authentication for secure passive UHF RFID tags that support cryptographic operations.

## References

Standards, security references, and implementation references are listed here:

- [References](references.md)

## Three-Sentence Summary

This research aims to protect passive RFID read operations by ensuring that readable tag data does not expose sensitive identity or business information. The proposed system stores only random or pseudonymous identifiers in the EPC field, stores sensitive metadata as encrypted payloads, and verifies payload integrity using cryptographic validation. For higher-security deployments, the system uses cryptographic-capable passive tags that support challenge-response authentication and Gen2v2/ISO 29167 security features so unauthorized readers cannot produce trusted reads.

## Problem Statement

Many passive RFID deployments treat the EPC value as a plain identifier. Any compatible reader within RF range may be able to inventory the tag and observe that identifier unless privacy controls are used.

This creates several risks:

- unauthorized readers can collect tag identifiers
- EPC values can reveal asset, vehicle, product, or account information
- eavesdroppers can record tag-reader communication
- cloned or replayed identifiers may be accepted by weak backend systems
- locked memory or access passwords may be mistaken for strong encryption

This research addresses the problem by designing RFID tag data so that a raw read is not sensitive, and by adding cryptographic verification so the backend can distinguish trusted reads from copied data.

## Important Technical Claim

Standard low-cost passive RFID tags usually cannot perform full encrypted communication like a powered computer. Many tags can store and return data, but they cannot decrypt, encrypt, or sign fresh messages unless they include a cryptographic engine.

Therefore, this research separates three security goals:

| Goal | Works With Basic Passive Tags | Requires Crypto-Capable Tags |
| --- | --- | --- |
| Hide sensitive metadata from casual readers | yes, store encrypted payload only | yes |
| Prevent EPC from revealing identity | yes, use random EPC token | yes |
| Prove tag authenticity with challenge-response | no | yes |
| Prevent replay of a previously observed read | limited backend detection | yes, if fresh challenge-response is supported |
| Restrict useful reads to authorized readers | partially, by encrypted data design | yes, with tag/reader authentication |

## Research Objectives

- Design a passive RFID data format where raw EPC reads do not expose sensitive identity.
- Store vehicle, asset, product, or account metadata as encrypted payloads instead of plaintext tag data.
- Verify encrypted payload integrity using authenticated encryption or MAC validation.
- Support secure tag authentication for Gen2v2/ISO 29167 compatible passive RFID tags.
- Detect copied EPC and replayed encrypted payloads using backend state, counters, timestamps, or challenge-response where available.
- Define a practical migration path from low-cost passive tags to cryptographic passive tags.
- Evaluate read speed, tag memory size, security strength, and deployment complexity.

## Core Idea

The tag should not be trusted just because it responds to a reader.

```text
Reader inventories tag
        |
        v
EPC is treated as a random lookup token, not sensitive identity
        |
        v
Reader reads encrypted payload from user memory or secure memory
        |
        v
Backend decrypts and validates integrity
        |
        v
If supported, cryptographic tag authentication proves the tag is genuine
        |
        v
Trusted read / suspicious read / reject
```

## Security Architecture

### Layer 1: Privacy-Safe EPC

The EPC should not contain sensitive plaintext.

Recommended EPC design:

```text
EPC = random_token_or_pseudonym
```

The backend maps the token to the real record:

```text
epc_token -> internal_tag_id -> account_or_asset_record
```

Avoid storing directly recognizable values in EPC, such as:

- full plate number
- owner ID
- account number
- product serial number with business meaning
- location or route information
- vehicle class if it is sensitive in the deployment

### Layer 2: Encrypted Payload

Sensitive metadata can be stored as encrypted data in tag user memory or backend-linked data.

Example plaintext before encryption:

```json
{
  "tag_id": "TAG-2026-000914",
  "asset_type": "vehicle",
  "vehicle_class": "truck",
  "plate_last4": "6789",
  "issuer_id": "ETC-AUTH-01",
  "policy_id": "P3",
  "version": 4
}
```

Example encrypted payload stored on tag:

```text
payload_version || key_id || nonce || ciphertext || auth_tag
```

The ciphertext can be read by any compatible reader, but it is not useful without the backend key.

### Layer 3: Integrity Protection

Encrypted data should be authenticated, not only encrypted.

Recommended cryptographic approach:

```text
ciphertext, auth_tag = AES-GCM(key, nonce, plaintext, associated_data)
```

Associated data can include:

```text
epc_token
tag_memory_bank
schema_version
issuer_id
key_id
```

If AES-GCM is not available in the backend environment, another standard authenticated encryption method can be used. Do not use home-made encryption.

### Layer 4: Cryptographic Tag Authentication

For higher-security deployments, use passive tags that support cryptographic challenge-response.

Example flow:

```text
1. Reader inventories tag.
2. Reader sends random challenge.
3. Tag computes cryptographic response using a secret key stored inside the tag IC.
4. Backend verifies response.
5. Backend accepts the read only if authentication succeeds.
```

This is the layer that helps prevent cloned identifiers and replayed reads. It requires tags and readers that support the relevant security commands.

### Layer 5: Optional Reader Authentication

Some deployments may require the tag to respond with protected information only after reader authorization.

Possible approaches:

- Gen2v2 authentication features where supported
- ISO/IEC 29167 crypto suites where supported
- reader certificate or secure module validated by backend
- access policy enforced by backend before releasing decrypted metadata

The exact implementation depends on tag IC capability and reader SDK support.

## Deployment Modes

### Mode A: Basic Passive Tags

This mode works with low-cost passive tags that do not have cryptographic engines.

What it can do:

- keep sensitive metadata encrypted
- keep EPC pseudonymous
- verify payload integrity after backend decryption
- detect duplicate EPC observations using backend analytics

What it cannot fully do:

- stop unauthorized readers from reading the EPC
- prove that a tag is genuine using cryptographic challenge-response
- prevent replay if an attacker copies all readable memory to another compatible tag

This mode is still useful because unauthorized reads expose only ciphertext and random tokens.

### Mode B: Cryptographic Passive Tags

This mode uses passive RFID tags with built-in cryptographic support.

What it can do:

- authenticate the tag
- support challenge-response freshness
- reduce clone and replay risk
- optionally hide or limit portions of tag data
- provide stronger proof that the read came from a genuine tag IC

Tradeoff:

- higher tag cost
- reader compatibility requirements
- key provisioning requirements
- more complex backend verification

## Proposed Tag Data Layout

Example compact binary layout:

| Field | Size | Purpose |
| --- | --- | --- |
| magic | 8 bits | identifies encrypted payload format |
| version | 4 bits | schema version |
| algorithm_id | 4 bits | cryptographic algorithm |
| key_id | 16 bits | backend key lookup |
| nonce | 96 bits | AES-GCM nonce |
| ciphertext | variable | encrypted metadata |
| auth_tag | 64-128 bits | authentication tag |
| crc | 16 bits | transport/storage error detection |

Example readable representation:

```text
EPC:  3048A13F91C00451B77E20AA
USER: E1 04 002A 8F35C7B86F8A2011D204B9EF A713... 2B04C81E9C33C217
```

The CRC is only for accidental error detection. It is not a security control.

## Backend Key Model

Recommended key design:

```text
master_key
    |
    +-- issuer_key
            |
            +-- per_tag_payload_key
            |
            +-- per_tag_auth_key, if crypto tag supports authentication
```

Rules:

- never store master keys on RFID tags
- never store plaintext keys in reader configuration files
- use hardware security module or secure key vault where possible
- rotate issuer keys by `key_id`
- support revocation for compromised tag batches
- keep tag provisioning logs auditable

## Read Verification Workflow

```text
1. Reader reads EPC token.
2. Reader reads encrypted payload memory.
3. Edge or backend checks payload format and CRC.
4. Backend looks up candidate key using key_id and issuer_id.
5. Backend performs authenticated decryption.
6. Backend verifies EPC token is bound to the decrypted tag_id.
7. If supported, backend verifies tag challenge-response.
8. Backend checks replay, counter, or duplicate-read rules.
9. System accepts, rejects, or flags the read for review.
```

## Replay and Clone Detection

For basic passive tags, replay prevention is limited because the tag may only return static memory.

Backend controls can still help:

- reject impossible duplicate reads from distant locations
- detect simultaneous reads of the same EPC token
- track unusual read frequency
- bind encrypted payload to EPC token and TID where allowed
- use periodically rotated pseudonymous EPC tokens
- use tamper-evident issuance and audit logs

For crypto-capable tags, stronger controls are possible:

- reader challenge nonce
- tag cryptographic response
- monotonic counter if supported
- authenticated session state
- backend freshness validation

## Threat Model

| Threat | Basic Encrypted Payload | Crypto-Capable Tag |
| --- | --- | --- |
| Casual unauthorized read | ciphertext only | ciphertext or blocked/limited data |
| EPC tracking | reduced by pseudonymous EPC | reduced further by privacy features |
| Payload modification | detected by authenticated encryption | detected |
| EPC cloning | detected only by backend analytics | challenge-response can reject clone |
| Replay of old read | limited detection | freshness challenge can reject replay |
| Eavesdropping | sensitive data protected if encrypted | stronger if authenticated session is used |
| Lost reader credentials | rotate keys and revoke reader | rotate keys, revoke reader, audit |

## Edge System Role

The edge reader software should not expose decrypted metadata unless needed locally.

Recommended edge behavior:

- read EPC and encrypted payload
- forward encrypted record to backend when possible
- perform local decryption only on hardened devices
- store keys in secure storage if local decryption is required
- cache only minimum data required for offline operation
- log failed decryption and failed authentication attempts
- rate-limit repeated suspicious reads

## Example Secure Read Event

```json
{
  "event_id": "RFID-SEC-20260522-000391",
  "timestamp_utc": "2026-05-22T17:02:40.120Z",
  "reader_id": "RDR-GATE-02",
  "antenna_port": 1,
  "epc_token": "3048A13F91C00451B77E20AA",
  "payload": {
    "format_version": 1,
    "key_id": "002A",
    "nonce": "8F35C7B86F8A2011D204B9EF",
    "ciphertext_len": 64,
    "auth_tag_len": 16,
    "crc_valid": true
  },
  "verification": {
    "payload_decryption": "success",
    "auth_tag_valid": true,
    "epc_bound_to_payload": true,
    "tag_challenge_response": "success",
    "replay_check": "pass"
  },
  "decision": "trusted_read"
}
```

## Real-Time Workflow

```text
1. Reader inventories nearby passive RFID tags.
2. Reader filters target EPC tokens by lane, zone, or session.
3. Reader reads encrypted payload memory.
4. If supported, reader performs tag authentication challenge.
5. Backend decrypts payload and verifies integrity.
6. Backend checks clone/replay indicators.
7. Application receives only verified metadata.
```

## Runtime Operation

This system should run during every RFID read operation.

Continuous components:

```text
- reader inventory
- encrypted payload read
- payload format validation
- backend key lookup
- authenticated decryption
- read event logging
- replay and duplicate detection
```

Conditional components:

```text
- cryptographic tag authentication
- reader authentication
- key rotation
- tag revocation
- forensic alert generation
```

## Security Requirements

- EPC values must not contain sensitive plaintext.
- Encrypted payloads must use authenticated encryption.
- Nonces must not repeat for the same encryption key when AES-GCM is used.
- Keys must be generated and stored outside the tag unless the tag has a secure cryptographic key store.
- Reader credentials must be revocable.
- Backend must bind EPC token, encrypted payload, issuer, and tag record.
- Access passwords must not be treated as strong encryption.
- Debug logs must not print decrypted sensitive data.
- Key provisioning must be auditable.

## Evaluation Metrics

The research can evaluate:

- secure read success rate
- encrypted payload read latency
- authentication latency
- false reject rate
- clone detection rate
- replay detection rate
- unauthorized-read data exposure
- payload memory overhead
- maximum read distance impact
- reader compatibility rate
- key rotation success rate
- backend verification throughput

## Research Questions

- How much sensitive metadata can be protected using encrypted payloads on ordinary passive tags?
- What is the latency cost of adding authenticated decryption to every RFID read?
- Which passive UHF tag ICs can support cryptographic tag authentication at operational read ranges?
- How should EPC pseudonyms rotate without breaking inventory and operational workflows?
- Can backend analytics reliably detect cloned basic tags when challenge-response is unavailable?
- What key provisioning model is practical for large-scale passive RFID deployments?
- How should secure reads behave during offline edge operation?

## Correct Claim

This research does not claim that every passive RFID tag can perform full encrypted over-air communication. Basic passive tags can protect sensitive data by storing only encrypted payloads and pseudonymous identifiers, but they cannot fully prevent unauthorized inventory reads or cryptographically prove authenticity.

Strong protection against cloning, replay, and unauthorized trusted reads requires cryptographic-capable passive RFID tags, compatible readers, secure key provisioning, and backend verification.
