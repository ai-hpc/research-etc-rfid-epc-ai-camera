# RK3576 AI Camera Software

## Purpose

`rk3576_ai_camera.py` is the edge-side AI verification program for the class-only EPC design.

It performs:

- EPC-96 decoding
- CRC-8 validation
- MAC-32 validation
- vehicle class verification
- vehicle color verification
- optional plate-last-4 comparison
- final `valid`, `review`, or `suspicious` decision

The software is designed for RK3576 boards using Rockchip RKNN acceleration, but the comparison logic can also run on a normal PC for testing.

## Files

| File | Purpose |
| --- | --- |
| `rk3576_ai_camera.py` | Edge AI camera verifier |
| `requirements-rk3576.txt` | Basic Python dependencies |
| `vehicle_class_codes.md` | Vehicle class code mapping |
| `epc_data_generator.cpp` | EPC generator and decoder reference |

## Hardware Target

The intended hardware target is an RK3576 device with:

- Linux or Android vendor SDK
- camera input through USB or MIPI CSI
- Rockchip RKNN runtime
- RK3576 NPU for vehicle class classification

The current design avoids vendor/model recognition so the edge device only needs to recognize broad vehicle class and color.

## Processing Pipeline

```text
RFID reader gets EPC
        |
        v
AI camera captures frame
        |
        v
Optional ROI crop
        |
        v
Vehicle class inference
        |
        v
OpenCV color estimation
        |
        v
Optional plate-last-4 input
        |
        v
Compare camera result with EPC metadata
        |
        v
Return valid / review / suspicious
```

## Install Dependencies

On the RK3576 board:

```bash
python3 -m pip install -r requirements-rk3576.txt
```

Install Rockchip RKNN runtime from the board vendor SDK. The script expects the RKNN Python runtime to expose:

```python
from rknnlite.api import RKNNLite
```

If RKNN is not installed, you can still test the software using manual `--ai-class` and `--ai-color` arguments.

## Basic Manual Test

Use the sample EPC from the generator documentation:

```bash
python3 rk3576_ai_camera.py \
  --epc 30C236A14100099CF3BEE5B9 \
  --secret toll-secret \
  --ai-class truck \
  --ai-color silver \
  --ai-plate-last4 6789 \
  --pretty
```

Expected result:

```json
{
  "status": "valid",
  "reasons": []
}
```

Mismatch example:

```bash
python3 rk3576_ai_camera.py \
  --epc 30C236A14100099CF3BEE5B9 \
  --secret toll-secret \
  --ai-class car \
  --ai-color silver \
  --pretty
```

Expected result:

```json
{
  "status": "suspicious",
  "reasons": ["class_mismatch"]
}
```

## Image Mode Without RKNN

You can test color estimation from an image while manually providing the class:

```bash
python3 rk3576_ai_camera.py \
  --epc 30C236A14100099CF3BEE5B9 \
  --secret toll-secret \
  --image vehicle.jpg \
  --roi 0.1,0.1,0.8,0.8 \
  --ai-class truck \
  --pretty
```

The ROI accepts either pixel values:

```text
x,y,width,height
```

or normalized values from `0` to `1`:

```text
0.1,0.1,0.8,0.8
```

## Camera Mode Without RKNN

Use a camera device and manually provide the class:

```bash
python3 rk3576_ai_camera.py \
  --epc 30C236A14100099CF3BEE5B9 \
  --secret toll-secret \
  --camera 0 \
  --roi 0.1,0.1,0.8,0.8 \
  --ai-class truck \
  --pretty
```

This captures a frame, estimates color, and compares the result with the EPC.

## RKNN Vehicle Class Mode

When you have a class classifier converted to `.rknn`, run:

```bash
python3 rk3576_ai_camera.py \
  --epc 30C236A14100099CF3BEE5B9 \
  --secret toll-secret \
  --camera 0 \
  --roi 0.1,0.1,0.8,0.8 \
  --rknn-model vehicle_class_rk3576.rknn \
  --input-size 224x224 \
  --class-labels 0,1,2,3,4,5,6,7,8,15 \
  --pretty
```

The RKNN model is expected to be a classifier whose output order matches `--class-labels`.

Default class label order:

| Output Index | Class Code | Meaning |
| ---: | ---: | --- |
| 0 | 0 | Unknown / Unset |
| 1 | 1 | Car |
| 2 | 2 | Truck |
| 3 | 3 | Bus |
| 4 | 4 | Van |
| 5 | 5 | Motorcycle |
| 6 | 6 | Trailer |
| 7 | 7 | Emergency |
| 8 | 8 | Heavy Equipment |
| 9 | 15 | Other / Unrecognizable |

## Model Conversion Recommendation

Recommended class model:

```text
MobileNetV2 / MobileNetV3 / YOLO backbone classifier
input: 224x224 or 320x320
output: class logits
quantization: INT8
format: ONNX -> RKNN
```

Use RKNN Toolkit2 on a development PC to convert the trained ONNX model:

```text
ONNX model -> RKNN Toolkit2 -> vehicle_class_rk3576.rknn
```

Deploy the `.rknn` file to the RK3576 board and run inference through RKNN Lite.

## Output Format

The script prints JSON.

Example valid output:

```json
{
  "status": "valid",
  "reasons": [],
  "epc": {
    "valid": true,
    "vehicle_class": 2,
    "vehicle_class_name": "Truck",
    "color_code": 3,
    "color_name": "Silver",
    "plate_last4": "6789"
  },
  "camera": {
    "vehicle_class": 2,
    "vehicle_class_name": "Truck",
    "color_code": 3,
    "color_name": "Silver"
  }
}
```

Possible statuses:

| Status | Meaning |
| --- | --- |
| `valid` | EPC is valid and camera metadata matches |
| `review` | EPC is valid but camera confidence is too low |
| `suspicious` | EPC integrity failed or camera metadata mismatched |
| `error` | Command failed before verification |

Possible reasons:

| Reason | Meaning |
| --- | --- |
| `epc_integrity_failed` | CRC or MAC validation failed |
| `class_mismatch` | Camera class does not match EPC class |
| `color_mismatch` | Camera color does not match EPC color |
| `plate_last4_mismatch` | OCR plate tail does not match EPC plate tail |
| `class_low_confidence` | Class confidence is below threshold |
| `color_low_confidence` | Color confidence is below threshold |

## Exit Codes

| Exit Code | Meaning |
| ---: | --- |
| 0 | Valid |
| 1 | Runtime or input error |
| 2 | Review or suspicious result |

## Recommended RK3576 Deployment

Use event-driven processing instead of continuous heavy inference:

```text
RFID read event
        |
        v
Capture 1-3 camera frames
        |
        v
Run RKNN class classifier
        |
        v
Estimate color with OpenCV
        |
        v
Compare with EPC
```

Recommended runtime settings:

```text
resolution: 640x360 or 640x480
class model input: 224x224 or 320x320
quantization: INT8
camera mode: fixed ROI if possible
OCR: optional, run only when needed
```

## Security Scope

This software does not prevent exact EPC cloning. It flags cloned or swapped tags only when the observed vehicle class, color, or optional plate-last-4 does not match the EPC metadata.

The EPC MAC prevents unauthorized metadata generation or modification when the secret key is protected.
