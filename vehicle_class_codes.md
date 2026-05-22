# Vehicle Class Code Table

## Purpose

The class-only EPC layout uses one 4-bit field for broad vehicle class.

This gives 16 possible values:

- `0` is reserved for unknown or unset class data.
- `1-8` are configured common toll-lane vehicle classes.
- `9-14` are available for future local or research-specific classes.
- `15` is reserved for `Other / Unrecognizable`.

Using vehicle class instead of vendor or exact model reduces the AI workload at the edge. The camera model only needs to classify broad vehicle type, which is faster and more reliable on low-power toll-lane hardware.

## Configured Class Codes

| Code | Vehicle Class |
| ---: | --- |
| 0 | Unknown / Unset |
| 1 | Car |
| 2 | Truck |
| 3 | Bus |
| 4 | Van |
| 5 | Motorcycle |
| 6 | Trailer |
| 7 | Emergency |
| 8 | Heavy Equipment |
| 15 | Other / Unrecognizable |

## Notes

Codes `9-14` are intentionally left available for local toll categories, special permits, fleet-specific labels, or future research experiments.

Exact vehicle vendor and model data, such as Toyota Hilux or Honda Civic, should be stored in the backend vehicle profile and linked through the EPC tag reference. The EPC stores only the compact class code to keep edge verification simple and fast.
