# References

This research track uses references from electronic fee collection, RFID/EPC, license plate recognition, vehicle detection, and multi-sensor fusion.

## Electronic Fee Collection and Tolling

- FHWA, All-Electronic Tolling Implementation background  
  https://www.fhwa.dot.gov/federalaid/170112.pdf

- ISO 12855:2025, Electronic fee collection - Information exchange between service provision and toll charging  
  https://www.iso.org/standard/88211.html

- ISO 17575-1:2016, Electronic fee collection - Application interface definition for autonomous systems - Charging  
  https://www.iso.org/standard/64161.html

- ISO 19299:2020, Electronic fee collection - Security framework  
  https://www.iso.org/standard/78357.html

- ISO 13141:2024, Electronic fee collection - Localization augmentation communication for autonomous systems  
  https://www.iso.org/standard/82930.html

- ISO 14816:2005, Road transport and traffic telematics - Automatic vehicle and equipment identification - Numbering and data structure  
  https://www.iso.org/standard/38667.html

## RFID and EPC

- GS1 EPC Tag Data Standard  
  https://ref.gs1.org/standards/tds/

- GS1 EPC/RFID Generation-2 UHF RFID Standard  
  https://ref.gs1.org/standards/gen2/

- ISO/IEC 18000-63:2021, RFID air interface at 860 MHz to 960 MHz Type C  
  https://www.iso.org/standard/78309.html

## License Plate Recognition and Computer Vision

- Shashirangana et al., Automated License Plate Recognition: A Survey on Methods and Techniques, IEEE Access, 2021  
  https://cir.nii.ac.jp/crid/1360861711809191424

- Henry et al., Automatic Number Plate Recognition: A Detailed Survey of Relevant Algorithms, Sensors, 2021  
  https://pmc.ncbi.nlm.nih.gov/articles/PMC8123416/

- Hendry and Chen, Automatic License Plate Recognition via Sliding-Window Darknet-YOLO Deep Learning, Image and Vision Computing, 2019  
  https://doi.org/10.1016/j.imavis.2019.04.007

- OpenCV color conversion reference  
  https://docs.opencv.org/4.x/d8/d01/group__imgproc__color__conversions.html

- OpenCV Python color-space tutorial  
  https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html

- Tesseract OCR  
  https://github.com/tesseract-ocr/tesseract

- PaddleOCR  
  https://github.com/PaddlePaddle/PaddleOCR

## Vehicle Detection and Multi-Sensor Fusion

- Sivaraman and Trivedi, Looking at Vehicles on the Road: A Survey of Vision-Based Vehicle Detection, Tracking, and Behavior Analysis, IEEE Transactions on Intelligent Transportation Systems, 2013  
  https://trid.trb.org/View/1308304

- Data Fusion for ITS: Techniques and Research Needs, Transportation Research Procedia, 2016  
  https://doi.org/10.1016/j.trpro.2016.06.042

- Vehicle Detection in Intelligent Transportation Systems and its Applications Under Varying Environments: A Review, Image and Vision Computing, 2018  
  https://doi.org/10.1016/j.imavis.2017.09.008

- Multi-Sensor Traffic Data Fusion, UC Berkeley Institute of Transportation Studies  
  https://its.berkeley.edu/publications/multi-sensor-traffic-data-fusion-0

## Multi-Object Tracking

- SORT, Simple Online and Realtime Tracking  
  https://arxiv.org/abs/1602.00763

- Deep SORT, Simple Online and Realtime Tracking with a Deep Association Metric  
  https://arxiv.org/abs/1703.07402

- ByteTrack, Multi-Object Tracking by Associating Every Detection Box  
  https://arxiv.org/abs/2110.06864

- ByteTrack implementation  
  https://github.com/FoundationVision/ByteTrack

- OC-SORT, Observation-Centric SORT for Robust Multi-Object Tracking  
  https://arxiv.org/abs/2203.14360

## Edge AI Deployment

- Rockchip RK3576 product page  
  https://www.rock-chips.com/a/en/products/RK35_Series/2024/1212/2033.html

- Rockchip RK3576 brief datasheet  
  https://www.rock-chips.com/uploads/pdf/2024.3.18/191/RK3576%20Brief%20Datasheet.pdf

- Rockchip RKNN Toolkit2  
  https://github.com/airockchip/rknn-toolkit2

- Rockchip RKNPU2 runtime and examples  
  https://github.com/airockchip/rknpu2

## Project-Specific Notes

- No-tag detection should not rely only on RFID absence. It should combine independent vehicle-presence sensing, camera evidence, and lane/time association.
- Unpaid detection should distinguish no tag, invalid tag, valid tag with failed payment, and ambiguous evidence.
- Multi-modal evidence should be treated as a violation candidate unless local enforcement rules allow fully automatic issuance.
