# How the EPC Data Generator Works

## Purpose

`epc_data_generator.cpp` is a standalone C++17 program that creates and verifies a compact 96-bit RFID EPC value for an electronic toll collection research prototype.

The current layout is simplified for edge hardware. It stores only broad vehicle class, color, plate tail, and backend reference data. It does not store vehicle vendor or exact model in the EPC.

The generated EPC contains:

- class-only vehicle metadata
- a CRC-8 checksum for accidental read or decoding errors
- a 32-bit keyed MAC for tamper detection

## Why Class-Only

Class-only verification is easier to run on toll-lane edge devices than vendor or exact model recognition.

The AI camera only needs to classify:

- car
- truck
- bus
- van
- motorcycle
- other configured classes

This reduces:

- model size
- inference time
- training data requirements
- edge GPU or CPU load
- false alarms caused by difficult make/model recognition

The trade-off is lower fraud-detection detail. The system can flag a swapped or cloned tag when the class changes, such as car to truck, but it may not detect a copied EPC used on a vehicle of the same class and similar color. Exact vendor and model data should remain in the registered vehicle profile.

## Source File

The implementation is in:

```text
epc_data_generator.cpp
```

It does not require OpenSSL or any external cryptography library. The file includes its own SHA-256 and HMAC-SHA256 implementation so it can be compiled directly with a standard C++17 compiler.

## Build Command

Compile the generator with:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic epc_data_generator.cpp -o epc_data_generator
```

## EPC-96 Format

The generator creates a 96-bit EPC.

That means:

- 96 bits total
- 12 bytes total
- 24 hexadecimal characters when printed

Example EPC:

```text
30C236A14100099CF3BEE5B9
```

The same EPC split into bytes:

```text
30 C2 36 A1 41 00 09 9C F3 BE E5 B9
```

## High-Level EPC Structure

The 96-bit EPC is divided into three major areas:

| Area | Size | Byte Range | Purpose |
| --- | ---: | --- | --- |
| Metadata | 56 bits | Bytes 0-6 | Encoded class-only vehicle and issuer data |
| CRC-8 | 8 bits | Byte 7 | Accidental error detection |
| MAC-32 | 32 bits | Bytes 8-11 | Keyed integrity validation |

The full layout is:

```text
+------------------+---------+----------+
| Metadata 56 bits | CRC 8   | MAC 32   |
+------------------+---------+----------+
| Bytes 0-6        | Byte 7  | Bytes 8-11
+------------------+---------+----------+
```

Removing the vendor/make field frees 8 bits. This implementation uses that space to increase the keyed MAC from 24 bits to 32 bits while keeping the EPC at 96 bits.

## Metadata Bit Layout

The first 56 bits store the compact metadata.

Bits are written from left to right, most significant bit first.

| Field | Bit Offset | Width | Range | Description |
| --- | ---: | ---: | --- | --- |
| Version | 0 | 4 bits | 0-15 | EPC format version |
| Issuer ID | 4 | 8 bits | 0-255 | Toll operator or authorized issuer |
| Vehicle class code | 12 | 4 bits | 0-15 | Broad class such as car, truck, bus, or van |
| Color code | 16 | 4 bits | 0-15 | Vehicle color code |
| Plate last 4 | 20 | 14 bits | 0-9999 | Last four plate digits |
| Tag reference | 34 | 16 bits | 0-65535 | Compact backend tag or account reference |
| Key ID | 50 | 4 bits | 0-15 | Identifies the validation key |
| Nonce | 54 | 2 bits | 0-3 | Small random or user-provided value |

The field widths add up to 56 bits:

```text
4 + 8 + 4 + 4 + 14 + 16 + 4 + 2 = 56 bits
```

## Field Meaning

### Version

The current program uses version `3` by default.

Version `3` identifies the class-only EPC layout:

```text
vehicle class + color + plate tail + tag reference + integrity protection
```

### Issuer ID

The issuer ID identifies the organization or toll operator that issued the EPC.

Example:

```text
12
```

### Vehicle Class Code

The vehicle class code is a compact numeric representation of the broad vehicle type.

Configured class codes are stored in [vehicle_class_codes.md](vehicle_class_codes.md).

Common examples:

| Code | Meaning |
| ---: | --- |
| 1 | Car |
| 2 | Truck |
| 3 | Bus |
| 4 | Van |
| 5 | Motorcycle |
| 15 | Other / Unrecognizable |

The program accepts either a numeric class code or a class name:

```bash
--class 2
--class truck
```

### Color Code

The color code stores a compact numeric representation of the registered vehicle color.

Example mapping:

| Code | Meaning |
| ---: | --- |
| 0 | Unknown |
| 1 | White |
| 2 | Black |
| 3 | Silver |
| 4 | Red |
| 5 | Blue |
| 6 | Gray |

The color mapping should match the AI camera model output and backend registration data.

### Plate Last 4

This field stores the last four digits of the plate number.

Example:

```text
6789
```

The field allows values from `0` to `9999`.

If the last four digits start with zero, the program still stores the numeric value. For example:

```text
Plate digits: 0047
Stored value: 47
Printed as:   0047
```

### Tag Reference

The tag reference links the EPC to a backend registration record.

It is not intended to store the full account number. The backend can use this value to look up:

- full registered vehicle profile
- owner account
- exact vendor and model
- RFID tag status
- issuance record
- fraud history or blacklist status

### Key ID

The key ID tells the verifier which secret key should be used to validate the EPC.

This allows key rotation:

```text
key-id 2
```

A production backend would retrieve the correct secret from a secure key store based on this ID.

### Nonce

The nonce is a 2-bit value from `0` to `3`.

If the user does not provide `--nonce`, the program generates a random value in that range.

## Generation Mode

Generation mode creates a new EPC from user-provided metadata.

Command format:

```bash
./epc_data_generator --issuer N --class CODE_OR_NAME --color N --plate N --tag N --key-id N --secret TEXT [--version N] [--nonce N]
```

Required arguments:

| Argument | Meaning |
| --- | --- |
| `--issuer` | Issuer ID |
| `--class` | Vehicle class code or name |
| `--color` | Vehicle color code |
| `--plate` | Last four plate digits |
| `--tag` | Compact tag or account reference |
| `--key-id` | Key identifier |
| `--secret` | Secret used for HMAC validation |

Optional arguments:

| Argument | Meaning |
| --- | --- |
| `--version` | EPC format version, default is 3 |
| `--nonce` | 2-bit nonce, random if omitted |

You can print configured classes with:

```bash
./epc_data_generator --list-classes
```

Example:

```bash
./epc_data_generator --issuer 12 --class truck --color 3 --plate 6789 --tag 1024 --key-id 2 --secret toll-secret --nonce 1
```

Example output:

```text
Generated EPC-96: 30C236A14100099CF3BEE5B9

Version:        3
Issuer ID:      12
Vehicle class:  2
Class name:     Truck
Color code:     3
Plate last 4:   6789
Tag reference:  1024
Key ID:         2
Nonce:          1
CRC valid:       yes
MAC valid:       yes
```

## Step-by-Step Generation Process

### Step 1: Read CLI Arguments

The example command provides:

```text
issuer = 12
class = truck
class code = 2
color = 3
plate = 6789
tag = 1024
key-id = 2
secret = toll-secret
nonce = 1
```

### Step 2: Validate Field Ranges

Each value is checked before it is packed into the EPC.

| Field | Maximum |
| --- | ---: |
| Version | 15 |
| Issuer ID | 255 |
| Vehicle class code | 15 |
| Color code | 15 |
| Plate last 4 | 9999 |
| Tag reference | 65535 |
| Key ID | 15 |
| Nonce | 3 |

### Step 3: Pack Metadata Bits

The program creates an empty 12-byte EPC buffer:

```text
00 00 00 00 00 00 00 00 00 00 00 00
```

It writes the metadata into the first 56 bits.

For the example command, the metadata portion is:

```text
30 C2 36 A1 41 00 09
```

### Step 4: Calculate CRC-8

The program calculates CRC-8 over the first 7 metadata bytes:

```text
30 C2 36 A1 41 00 09
```

It uses:

```text
Polynomial: 0x07
Initial value: 0x00
```

The CRC-8 result is stored in byte 7.

For the example:

```text
CRC-8 = 9C
```

Now the EPC buffer is:

```text
30 C2 36 A1 41 00 09 9C 00 00 00 00
```

### Step 5: Calculate HMAC-SHA256

The program calculates HMAC-SHA256 using:

```text
secret = toll-secret
message = first 8 EPC bytes
```

The first 8 EPC bytes are metadata plus CRC:

```text
30 C2 36 A1 41 00 09 9C
```

The HMAC-SHA256 output is 32 bytes long.

Because a 96-bit EPC has limited space, the program stores only the first 4 bytes of the HMAC result.

This creates a 32-bit MAC:

```text
MAC-32 = F3 BE E5 B9
```

### Step 6: Produce Final EPC

The final EPC is:

```text
30 C2 36 A1 41 00 09 9C F3 BE E5 B9
```

Printed as a single EPC-96 hex string:

```text
30C236A14100099CF3BEE5B9
```

## Decode Mode

Decode mode reads an EPC hex string and verifies it using the same secret.

Command format:

```bash
./epc_data_generator --decode EPC_HEX --secret TEXT
```

Example:

```bash
./epc_data_generator --decode 30C236A14100099CF3BEE5B9 --secret toll-secret
```

Example output:

```text
EPC-96:         30C236A14100099CF3BEE5B9
Version:        3
Issuer ID:      12
Vehicle class:  2
Class name:     Truck
Color code:     3
Plate last 4:   6789
Tag reference:  1024
Key ID:         2
Nonce:          1
Stored CRC-8:   0x9C
Expected CRC-8: 0x9C
Stored MAC-32:  F3BEE5B9
Expected MAC-32: F3BEE5B9
CRC valid:      yes
MAC valid:      yes
Overall valid:  yes
```

## Step-by-Step Decode Process

### Step 1: Parse Hex EPC

The program accepts a 24-character hex value.

It also accepts values with a `0x` prefix.

These are equivalent:

```text
30C236A14100099CF3BEE5B9
0x30C236A14100099CF3BEE5B9
```

### Step 2: Read Metadata Fields

The program reads the first 56 bits using `readBits()`.

It extracts:

```text
version
issuer ID
vehicle class code
color code
plate last 4
tag reference
key ID
nonce
```

### Step 3: Recalculate CRC-8

The program recalculates CRC-8 over bytes 0-6.

It compares:

```text
stored CRC-8   = byte 7 from EPC
expected CRC-8 = recalculated CRC-8
```

### Step 4: Recalculate MAC-32

The program recalculates HMAC-SHA256 over bytes 0-7 using the supplied secret.

It compares:

```text
stored MAC-32   = bytes 8-11 from EPC
expected MAC-32 = first 4 bytes of recalculated HMAC-SHA256
```

The EPC is valid only when both CRC and MAC checks pass.

## Tamper Detection Example

Original valid EPC:

```text
30C236A14100099CF3BEE5B9
```

Tampered EPC:

```text
30C236A14100099CF3BEE5B8
```

Only the last hex digit changed.

Decode command:

```bash
./epc_data_generator --decode 30C236A14100099CF3BEE5B8 --secret toll-secret
```

Expected result:

```text
CRC valid:      yes
MAC valid:      no
Overall valid:  no
```

The CRC may still pass because the metadata bytes did not change. The MAC fails because the keyed integrity value was modified.

## Why CRC and MAC Are Both Used

CRC-8 and MAC-32 serve different purposes.

| Mechanism | Purpose | Uses Secret Key | Detects Fraud |
| --- | --- | --- | --- |
| CRC-8 | Accidental error detection | No | No |
| MAC-32 | Keyed integrity check | Yes | Yes, within limited 32-bit strength |

CRC-8 is useful for quickly detecting accidental corruption, bad reads, or simple decoding errors.

MAC-32 is used to detect unauthorized EPC modification or generation. A valid MAC requires knowledge of the secret.

## Security Notes

This program is suitable as a research prototype and demonstration of compact EPC metadata generation.

Important limitations:

- MAC-32 is still a truncated MAC and is not as strong as a full HMAC.
- The program uses shared-secret HMAC validation, not public-key signatures.
- A full public-key signature cannot realistically fit inside this 96-bit EPC layout.
- Production systems should verify a signed backend issuance record in addition to checking the EPC.
- Secrets should not be hardcoded or passed casually on shared command lines in a real deployment.

A stronger real-world architecture would use:

- compact class-only EPC metadata on the RFID tag
- registration lookup by tag reference
- HMAC or authenticated issuer record
- public-key verification of the issuer certificate or issuance record
- AI edge verification for class and color

## Correct Security Claim

The MAC-32 field prevents unauthorized EPC generation and metadata modification when the secret key is protected. AI camera verification can flag cloned or swapped tags when the observed vehicle class, color, or plate tail does not match the EPC metadata.

This design does not encrypt the EPC, prevent RFID reading, or prevent exact static EPC cloning. A copied EPC can still validate cryptographically because the copied metadata and MAC are unchanged.

## How This Supports RFID Toll Fraud Detection

At the toll lane, the EPC generator and verifier support this workflow:

```text
RFID reader captures EPC
        |
        v
System decodes EPC metadata
        |
        v
System validates CRC and MAC
        |
        v
Camera captures vehicle image
        |
        v
AI predicts vehicle class and color
        |
        v
System compares EPC metadata with AI result
        |
        v
Transaction is accepted, flagged, or rejected
```

Example comparison:

| Source | Class | Color | Plate Last 4 |
| --- | --- | --- | --- |
| EPC metadata | Truck | 3 | 6789 |
| AI camera result | Truck | 3 | Optional OCR result |
| Backend record | Truck | 3 | 6789 |

If the class and color match, the transaction is likely valid.

If the EPC says class `1` for car but the camera detects class `2` for truck, the transaction can be flagged as suspicious.

## Exit Codes

The program returns:

| Exit Code | Meaning |
| ---: | --- |
| 0 | Success or valid decoded EPC |
| 1 | Invalid input or command usage error |
| 2 | Decode completed, but EPC validation failed |

Example:

```bash
./epc_data_generator --decode 30C236A14100099CF3BEE5B9 --secret toll-secret
echo $?
```

If the EPC is valid, the result is:

```text
0
```

## Practical Mapping Recommendation

For a class-only edge design, keep separate mapping tables for vehicle class and color codes.

Example class mapping:

| Code | Vehicle Class |
| ---: | --- |
| 1 | Car |
| 2 | Truck |
| 3 | Bus |
| 4 | Van |
| 5 | Motorcycle |
| 15 | Other / Unrecognizable |

Example color mapping:

| Code | Color |
| ---: | --- |
| 1 | White |
| 2 | Black |
| 3 | Silver |
| 4 | Red |
| 5 | Blue |

The EPC should store only numeric codes. The backend and AI system should use the same mapping table when comparing RFID metadata to camera predictions.

## Summary

The class-only EPC generator packs compact toll verification metadata into the first 56 bits of a 96-bit EPC, adds CRC-8, and then adds a 32-bit HMAC-derived integrity value.

The result is a short EPC value that can be decoded quickly at a toll lane and checked against lightweight AI vehicle class and color recognition for real-time fraud detection.
