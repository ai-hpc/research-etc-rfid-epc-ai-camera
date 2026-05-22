# References

This project uses the following standards, specifications, and implementation references.

## EPC and RFID

- GS1 standards repository  
  https://ref.gs1.org/standards/

- GS1 EPC Tag Data Standard  
  https://ref.gs1.org/standards/tds/

- GS1 EPC/RFID Generation-2 UHF RFID Standard  
  https://ref.gs1.org/standards/gen2/

- GS1 System Architecture, including EPC/RFID workflow  
  https://ref.gs1.org/architecture/system-architecture/

- GS1 guidelines repository  
  https://ref.gs1.org/guidelines/

## Cryptographic Integrity

- NIST FIPS 180-4, Secure Hash Standard  
  https://csrc.nist.gov/pubs/fips/180-4/upd1/final

- NIST FIPS 198-1, The Keyed-Hash Message Authentication Code (HMAC)  
  https://csrc.nist.gov/pubs/fips/198-1/final

- NIST SP 800-107 Revision 1, Recommendation for Applications Using Approved Hash Algorithms  
  https://csrc.nist.gov/pubs/sp/800/107/r1/final

- NIST Message Authentication Codes project  
  https://csrc.nist.gov/Projects/message-authentication-codes

## RK3576 and RKNN

- Rockchip RK3576 product page  
  https://www.rock-chips.com/a/en/products/RK35_Series/2024/1212/2033.html

- Rockchip RK3576 brief datasheet  
  https://www.rock-chips.com/uploads/pdf/2024.3.18/191/RK3576%20Brief%20Datasheet.pdf

- Rockchip RKNN Toolkit2  
  https://github.com/airockchip/rknn-toolkit2

- Rockchip RKNPU2 runtime and examples  
  https://github.com/airockchip/rknpu2

- Ultralytics Rockchip RKNN export documentation  
  https://docs.ultralytics.com/integrations/rockchip-rknn/

## Computer Vision

- OpenCV color conversion reference  
  https://docs.opencv.org/4.x/d8/d01/group__imgproc__color__conversions.html

- OpenCV Python color-space tutorial  
  https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html

## Project-Specific Notes

- The EPC layout in this repository is a research prototype and does not claim to be a GS1-compliant allocation scheme.
- The MAC-32 field is a truncated HMAC output. It is used to demonstrate compact integrity protection inside EPC-96 constraints, not as a replacement for full-length cryptographic authentication in high-security deployments.
- The class-only AI camera design is optimized for RK3576 edge hardware. Vendor/model recognition is intentionally left to registered vehicle records rather than real-time edge inference.
