#!/usr/bin/env python3
"""
Class-only AI camera verifier for RK3576 edge devices.

The script validates the EPC-96 value, estimates vehicle class/color from a
camera frame or image, and compares the camera result with decoded EPC metadata.
RKNN inference is optional; manual class/color overrides make the comparison
logic testable on non-RK3576 machines.
"""

from __future__ import annotations

import argparse
import hashlib
import hmac
import json
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable

try:
    import numpy as np
except ImportError:  # pragma: no cover - handled at runtime
    np = None  # type: ignore[assignment]


EPC_BYTES = 12
METADATA_BYTES = 7
CRC_BYTE_INDEX = 7
MAC_START_INDEX = 8
MAC_BYTES = 4

CLASS_CODES = {
    0: "Unknown / Unset",
    1: "Car",
    2: "Truck",
    3: "Bus",
    4: "Van",
    5: "Motorcycle",
    6: "Trailer",
    7: "Emergency",
    8: "Heavy Equipment",
    15: "Other / Unrecognizable",
}

CLASS_ALIASES = {
    "unknown": 0,
    "unset": 0,
    "car": 1,
    "sedan": 1,
    "suv": 1,
    "truck": 2,
    "pickup": 2,
    "lorry": 2,
    "bus": 3,
    "van": 4,
    "motorcycle": 5,
    "motorbike": 5,
    "bike": 5,
    "trailer": 6,
    "emergency": 7,
    "ambulance": 7,
    "firetruck": 7,
    "heavyequipment": 8,
    "heavy": 8,
    "other": 15,
    "unrecognizable": 15,
    "notrecognizable": 15,
}

COLOR_CODES = {
    0: "Unknown",
    1: "White",
    2: "Black",
    3: "Silver",
    4: "Red",
    5: "Blue",
    6: "Gray",
    7: "Yellow",
    8: "Green",
    9: "Brown",
}

COLOR_ALIASES = {
    "unknown": 0,
    "white": 1,
    "black": 2,
    "silver": 3,
    "red": 4,
    "blue": 5,
    "gray": 6,
    "grey": 6,
    "yellow": 7,
    "green": 8,
    "brown": 9,
}


@dataclass
class DecodedEpc:
    epc_hex: str
    version: int
    issuer_id: int
    vehicle_class: int
    vehicle_class_name: str
    color_code: int
    color_name: str
    plate_last4: str
    tag_reference: int
    key_id: int
    nonce: int
    stored_crc: str
    expected_crc: str
    stored_mac32: str
    expected_mac32: str
    crc_valid: bool
    mac_valid: bool
    valid: bool


@dataclass
class CameraObservation:
    vehicle_class: int
    vehicle_class_name: str
    class_confidence: float
    color_code: int
    color_name: str
    color_confidence: float
    plate_last4: str | None
    source: str


@dataclass
class VerificationResult:
    status: str
    reasons: list[str]
    epc: DecodedEpc
    camera: CameraObservation


def normalize(value: str) -> str:
    return "".join(ch.lower() for ch in value if ch.isalnum())


def parse_code_or_name(value: str, aliases: dict[str, int], max_value: int, label: str) -> int:
    try:
        parsed = int(value, 0)
    except ValueError:
        key = normalize(value)
        if key in aliases:
            return aliases[key]
        raise ValueError(f"unknown {label}: {value}") from None
    if parsed < 0 or parsed > max_value:
        raise ValueError(f"{label} must be between 0 and {max_value}")
    return parsed


def read_bits(data: bytes, bit_offset: int, width: int) -> int:
    value = 0
    for i in range(width):
        absolute_bit = bit_offset + i
        byte_index = absolute_bit // 8
        bit_in_byte = 7 - (absolute_bit % 8)
        value = (value << 1) | ((data[byte_index] >> bit_in_byte) & 1)
    return value


def crc8(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


def decode_epc(epc_hex: str, secret: str) -> DecodedEpc:
    cleaned = epc_hex.strip()
    if cleaned.lower().startswith("0x"):
        cleaned = cleaned[2:]
    if len(cleaned) != EPC_BYTES * 2:
        raise ValueError("EPC-96 must be exactly 24 hex characters")

    epc = bytes.fromhex(cleaned)
    bit = 0
    version = read_bits(epc, bit, 4)
    bit += 4
    issuer_id = read_bits(epc, bit, 8)
    bit += 8
    vehicle_class = read_bits(epc, bit, 4)
    bit += 4
    color_code = read_bits(epc, bit, 4)
    bit += 4
    plate_last4 = read_bits(epc, bit, 14)
    bit += 14
    tag_reference = read_bits(epc, bit, 16)
    bit += 16
    key_id = read_bits(epc, bit, 4)
    bit += 4
    nonce = read_bits(epc, bit, 2)

    stored_crc = epc[CRC_BYTE_INDEX]
    expected_crc = crc8(epc[:METADATA_BYTES])

    stored_mac = epc[MAC_START_INDEX : MAC_START_INDEX + MAC_BYTES]
    expected_mac = hmac.new(
        secret.encode("utf-8"),
        epc[:MAC_START_INDEX],
        hashlib.sha256,
    ).digest()[:MAC_BYTES]

    crc_valid = stored_crc == expected_crc
    mac_valid = hmac.compare_digest(stored_mac, expected_mac)

    return DecodedEpc(
        epc_hex=cleaned.upper(),
        version=version,
        issuer_id=issuer_id,
        vehicle_class=vehicle_class,
        vehicle_class_name=CLASS_CODES.get(vehicle_class, "Unassigned class code"),
        color_code=color_code,
        color_name=COLOR_CODES.get(color_code, "Unassigned color code"),
        plate_last4=f"{plate_last4:04d}",
        tag_reference=tag_reference,
        key_id=key_id,
        nonce=nonce,
        stored_crc=f"{stored_crc:02X}",
        expected_crc=f"{expected_crc:02X}",
        stored_mac32=stored_mac.hex().upper(),
        expected_mac32=expected_mac.hex().upper(),
        crc_valid=crc_valid,
        mac_valid=mac_valid,
        valid=crc_valid and mac_valid,
    )


def require_numpy() -> Any:
    if np is None:
        raise RuntimeError("numpy is required for camera/image processing")
    return np


def require_cv2() -> Any:
    try:
        import cv2
    except ImportError as exc:
        raise RuntimeError("opencv-python is required for camera/image processing") from exc
    return cv2


def parse_size(value: str) -> tuple[int, int]:
    parts = value.lower().split("x", 1)
    if len(parts) != 2:
        raise ValueError("size must be WIDTHxHEIGHT, for example 224x224")
    return int(parts[0]), int(parts[1])


def parse_roi(value: str | None, frame_width: int, frame_height: int) -> tuple[int, int, int, int]:
    if not value:
        return 0, 0, frame_width, frame_height
    parts = [float(part.strip()) for part in value.split(",")]
    if len(parts) != 4:
        raise ValueError("ROI must be x,y,w,h")
    x, y, width, height = parts
    if all(0.0 <= part <= 1.0 for part in parts):
        x *= frame_width
        width *= frame_width
        y *= frame_height
        height *= frame_height
    x_i = max(0, min(frame_width - 1, int(round(x))))
    y_i = max(0, min(frame_height - 1, int(round(y))))
    w_i = max(1, min(frame_width - x_i, int(round(width))))
    h_i = max(1, min(frame_height - y_i, int(round(height))))
    return x_i, y_i, w_i, h_i


def load_frame(args: argparse.Namespace) -> tuple[Any | None, str]:
    if args.image:
        cv2 = require_cv2()
        frame = cv2.imread(str(args.image))
        if frame is None:
            raise RuntimeError(f"could not read image: {args.image}")
        return frame, str(args.image)

    if args.camera is not None:
        cv2 = require_cv2()
        capture = cv2.VideoCapture(args.camera)
        if not capture.isOpened():
            raise RuntimeError(f"could not open camera index {args.camera}")
        frame = None
        for _ in range(max(1, args.warmup_frames)):
            ok, candidate = capture.read()
            if ok:
                frame = candidate
        capture.release()
        if frame is None:
            raise RuntimeError("camera did not return a frame")
        return frame, f"camera:{args.camera}"

    return None, "manual"


def crop_frame(frame: Any, roi: str | None) -> Any:
    height, width = frame.shape[:2]
    x, y, w, h = parse_roi(roi, width, height)
    return frame[y : y + h, x : x + w]


class RknnClassClassifier:
    def __init__(self, model_path: Path, input_size: tuple[int, int], class_labels: list[int]) -> None:
        try:
            from rknnlite.api import RKNNLite
        except ImportError as exc:
            raise RuntimeError("rknnlite is not installed on this system") from exc

        self._np = require_numpy()
        self._cv2 = require_cv2()
        self._input_size = input_size
        self._class_labels = class_labels
        self._rknn = RKNNLite()
        ret = self._rknn.load_rknn(str(model_path))
        if ret != 0:
            raise RuntimeError(f"failed to load RKNN model: {model_path}")
        ret = self._rknn.init_runtime()
        if ret != 0:
            raise RuntimeError("failed to initialize RKNN runtime")

    def predict(self, crop: Any) -> tuple[int, float]:
        resized = self._cv2.resize(crop, self._input_size)
        rgb = self._cv2.cvtColor(resized, self._cv2.COLOR_BGR2RGB)
        tensor = self._np.expand_dims(rgb, axis=0)
        outputs = self._rknn.inference(inputs=[tensor])
        if not outputs:
            raise RuntimeError("RKNN inference returned no outputs")
        scores = self._np.asarray(outputs[0]).reshape(-1).astype("float32")
        if scores.size == 0:
            raise RuntimeError("RKNN output is empty")
        if scores.size != len(self._class_labels):
            usable = min(scores.size, len(self._class_labels))
            scores = scores[:usable]
            labels = self._class_labels[:usable]
        else:
            labels = self._class_labels
        probs = softmax(scores)
        index = int(self._np.argmax(probs))
        return labels[index], float(probs[index])


def softmax(scores: Any) -> Any:
    np_mod = require_numpy()
    shifted = scores - np_mod.max(scores)
    exp = np_mod.exp(shifted)
    return exp / np_mod.sum(exp)


def estimate_color(crop: Any) -> tuple[int, float]:
    np_mod = require_numpy()
    cv2 = require_cv2()

    if crop.size == 0:
        return 0, 0.0

    height, width = crop.shape[:2]
    x0 = int(width * 0.15)
    x1 = int(width * 0.85)
    y0 = int(height * 0.15)
    y1 = int(height * 0.85)
    body = crop[y0:y1, x0:x1] if x1 > x0 and y1 > y0 else crop

    hsv = cv2.cvtColor(body, cv2.COLOR_BGR2HSV)
    pixels = hsv.reshape(-1, 3)
    if pixels.size == 0:
        return 0, 0.0

    h = pixels[:, 0].astype("float32")
    s = pixels[:, 1].astype("float32")
    v = pixels[:, 2].astype("float32")

    mean_s = float(np_mod.mean(s))
    mean_v = float(np_mod.mean(v))

    if mean_v < 55:
        return 2, confidence_from_margin(55 - mean_v, 55)
    if mean_s < 35 and mean_v > 190:
        return 1, min(0.95, 0.55 + (mean_v - 190) / 120)
    if mean_s < 45:
        if mean_v >= 135:
            return 3, min(0.9, 0.55 + (mean_v - 135) / 150)
        return 6, min(0.9, 0.55 + (135 - mean_v) / 150)

    hue_votes = {
        4: hue_fraction(h, [(0, 10), (170, 180)]),
        7: hue_fraction(h, [(18, 38)]),
        8: hue_fraction(h, [(40, 85)]),
        5: hue_fraction(h, [(90, 135)]),
        9: hue_fraction(h, [(10, 24)]),
    }
    color_code, vote = max(hue_votes.items(), key=lambda item: item[1])
    if vote < 0.15:
        return 0, 0.25
    return color_code, min(0.95, 0.45 + vote)


def hue_fraction(hue_values: Any, ranges: Iterable[tuple[int, int]]) -> float:
    np_mod = require_numpy()
    mask = np_mod.zeros_like(hue_values, dtype=bool)
    for low, high in ranges:
        mask |= (hue_values >= low) & (hue_values <= high)
    return float(np_mod.mean(mask))


def confidence_from_margin(margin: float, scale: float) -> float:
    return max(0.5, min(0.95, 0.5 + margin / max(scale, 1.0)))


def get_camera_observation(args: argparse.Namespace) -> CameraObservation:
    frame, source = load_frame(args)
    crop = crop_frame(frame, args.roi) if frame is not None else None

    if args.ai_class:
        class_code = parse_code_or_name(args.ai_class, CLASS_ALIASES, 15, "vehicle class")
        class_confidence = args.manual_confidence
    elif args.rknn_model:
        if crop is None:
            raise RuntimeError("--rknn-model requires --image or --camera")
        labels = [int(item.strip()) for item in args.class_labels.split(",") if item.strip()]
        classifier = RknnClassClassifier(args.rknn_model, parse_size(args.input_size), labels)
        class_code, class_confidence = classifier.predict(crop)
    else:
        class_code = 0
        class_confidence = 0.0

    if args.ai_color:
        color_code = parse_code_or_name(args.ai_color, COLOR_ALIASES, 15, "color")
        color_confidence = args.manual_confidence
    elif crop is not None:
        color_code, color_confidence = estimate_color(crop)
    else:
        color_code = 0
        color_confidence = 0.0

    plate_last4 = args.ai_plate_last4
    if plate_last4 is not None:
        if not plate_last4.isdigit() or len(plate_last4) > 4:
            raise ValueError("--ai-plate-last4 must be one to four digits")
        plate_last4 = f"{int(plate_last4):04d}"

    return CameraObservation(
        vehicle_class=class_code,
        vehicle_class_name=CLASS_CODES.get(class_code, "Unassigned class code"),
        class_confidence=round(class_confidence, 4),
        color_code=color_code,
        color_name=COLOR_CODES.get(color_code, "Unassigned color code"),
        color_confidence=round(color_confidence, 4),
        plate_last4=plate_last4,
        source=source,
    )


def verify(epc: DecodedEpc, camera: CameraObservation, args: argparse.Namespace) -> VerificationResult:
    reasons: list[str] = []

    if not epc.valid:
        reasons.append("epc_integrity_failed")

    if camera.class_confidence < args.min_class_confidence:
        reasons.append("class_low_confidence")
    elif epc.vehicle_class != camera.vehicle_class:
        reasons.append("class_mismatch")

    if camera.color_confidence < args.min_color_confidence:
        reasons.append("color_low_confidence")
    elif epc.color_code != camera.color_code:
        reasons.append("color_mismatch")

    if camera.plate_last4 is not None and epc.plate_last4 != camera.plate_last4:
        reasons.append("plate_last4_mismatch")

    if "epc_integrity_failed" in reasons or any(reason.endswith("_mismatch") for reason in reasons):
        status = "suspicious"
    elif any(reason.endswith("_low_confidence") for reason in reasons):
        status = "review"
    else:
        status = "valid"

    return VerificationResult(status=status, reasons=reasons, epc=epc, camera=camera)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="RK3576 class-only AI camera EPC verifier")
    parser.add_argument("--epc", required=True, help="24-character EPC-96 hex string")
    parser.add_argument("--secret", required=True, help="HMAC secret used for EPC validation")
    parser.add_argument("--image", type=Path, help="image path to process")
    parser.add_argument("--camera", type=int, help="camera index to capture from")
    parser.add_argument("--warmup-frames", type=int, default=3, help="camera warmup frames before inference")
    parser.add_argument("--roi", help="vehicle ROI as x,y,w,h in pixels or normalized 0-1 values")
    parser.add_argument("--rknn-model", type=Path, help="RKNN class classifier model path")
    parser.add_argument("--input-size", default="224x224", help="RKNN input size, for example 224x224")
    parser.add_argument(
        "--class-labels",
        default="0,1,2,3,4,5,6,7,8,15",
        help="comma-separated class codes in classifier output order",
    )
    parser.add_argument("--ai-class", help="manual class result for testing, for example truck or 2")
    parser.add_argument("--ai-color", help="manual color result for testing, for example silver or 3")
    parser.add_argument("--ai-plate-last4", help="optional OCR plate tail result")
    parser.add_argument("--manual-confidence", type=float, default=0.99, help="confidence for manual values")
    parser.add_argument("--min-class-confidence", type=float, default=0.60)
    parser.add_argument("--min-color-confidence", type=float, default=0.50)
    parser.add_argument("--pretty", action="store_true", help="pretty-print JSON")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    try:
        epc = decode_epc(args.epc, args.secret)
        camera = get_camera_observation(args)
        result = verify(epc, camera, args)
    except Exception as exc:  # noqa: BLE001 - command-line tool should return structured errors
        print(json.dumps({"status": "error", "error": str(exc)}), file=sys.stderr)
        return 1

    payload = asdict(result)
    print(json.dumps(payload, indent=2 if args.pretty else None))
    return 0 if result.status == "valid" else 2


if __name__ == "__main__":
    raise SystemExit(main())
