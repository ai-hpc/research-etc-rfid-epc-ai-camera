# References

This research track uses references from passive UHF RFID standards, reader control and management, observability, and predictive maintenance.

## Passive RFID and Reader Standards

- GS1 EPC/RFID Generation-2 UHF RFID Standard  
  https://ref.gs1.org/standards/gen2/

- GS1 support note, What is EPC Gen2v2?  
  https://support.gs1.org/support/solutions/articles/43000734270-what-is-epc-gen2v2-

- ISO/IEC 18000-63:2021, RFID air interface at 860 MHz to 960 MHz Type C  
  https://www.iso.org/standard/78309.html

- RAIN Alliance, RAIN terminology and passive UHF RFID overview  
  https://therainalliance.org/faqs/

- RAIN RFID Relevant Standards  
  https://rainrfid.org/wp-content/uploads/2022/05/RAIN-RFID_TWG_RAIN_RFID_Relevant_Standards_FINAL_20220503-v1.3.pdf

## Reader Control and Reader Health Telemetry

- GS1 Low Level Reader Protocol (LLRP) Standard  
  https://ref.gs1.org/standards/llrp/

- GS1 System Architecture, RFID reader management and LLRP context  
  https://ref.gs1.org/architecture/system-architecture/12.0.0/GS1-Architecture-a-i12-2024-10-18.pdf

- Impinj troubleshooting guide for faulty readers  
  https://support.impinj.com/article/202756608

- Impinj troubleshooting guide for antenna hub and antenna status issues  
  https://support.impinj.com/article/202756508

- Impinj IoT Device Interface API inventory configuration examples  
  https://support.impinj.com/hc/en-us/articles/32153110595219-Impinj-IoT-Device-Interface-API-Example-Inventory-Configurations

## Observability and Telemetry Collection

- OpenTelemetry metrics concepts  
  https://opentelemetry.io/docs/concepts/signals/metrics/

- OpenTelemetry metrics specification  
  https://opentelemetry.io/docs/specs/otel/metrics/

- Prometheus exporter writing guide  
  https://prometheus.io/docs/instrumenting/writing_exporters/

- Prometheus metric and label naming guide  
  https://prometheus.io/docs/practices/naming/

## Predictive Maintenance and Anomaly Detection

- Real-Time Predictive Maintenance using Autoencoder Reconstruction and Anomaly Detection  
  https://arxiv.org/abs/2110.01447

- Anomaly Detection in Industrial Machinery using IoT Devices and Machine Learning: a Systematic Mapping  
  https://arxiv.org/abs/2307.15807

- Predictive Maintenance Model Based on Anomaly Detection in Induction Motors: A Machine Learning Approach Using Real-Time IoT Data  
  https://arxiv.org/abs/2310.14949

- A Survey of Predictive Maintenance Methods: An Analysis of Prognostics via Classification and Regression  
  https://arxiv.org/abs/2506.20090

- A Survey of Predictive Maintenance: Systems, Purposes and Approaches  
  https://arxiv.org/abs/1912.07383

## Project-Specific Notes

- The monitored passive RFID tags do not provide infrastructure health by themselves. The maintenance model must observe active reader, antenna-port, RF, network, and environmental telemetry.
- A reader being online is not enough to prove read-zone health. Per-antenna read-rate, RSSI distribution, reference-tag visibility, and event history are needed.
- Predictive maintenance should produce maintenance candidates and diagnostic evidence. Human or operational policy should decide when to replace hardware.
