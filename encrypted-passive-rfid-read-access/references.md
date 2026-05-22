# References

This research track uses references from passive UHF RFID standards, RFID security services, cryptographic tag authentication, and authenticated encryption.

## Passive RFID Standards

- GS1 EPC/RFID Generation-2 UHF RFID Standard  
  https://ref.gs1.org/standards/gen2/

- GS1 support note, What is EPC Gen2v2?  
  https://support.gs1.org/support/solutions/articles/43000734270-what-is-epc-gen2v2-

- GS1 EPC Tag Data Standard  
  https://ref.gs1.org/standards/tds/

- ISO/IEC 18000-63:2021, RFID air interface at 860 MHz to 960 MHz Type C  
  https://www.iso.org/standard/78309.html

- RAIN RFID Relevant Standards  
  https://rainrfid.org/wp-content/uploads/2022/05/RAIN-RFID_TWG_RAIN_RFID_Relevant_Standards_FINAL_20220503-v1.3.pdf

## RFID Security Services and Cryptographic Tags

- ISO/IEC 29167-1:2014, Security services for RFID air interfaces  
  https://www.iso.org/standard/61128.html

- ISO/IEC 29167-10:2026, Crypto suite AES-128 security services for air interface communications  
  https://www.iso.org/standard/88959.html

- ISO/IEC 29167-10:2017, Crypto suite AES-128 security services for air interface communications  
  https://www.iso.org/standard/69410.html

- NXP UCODE DNA UHF tag IC for secure authentication  
  https://www.nxp.com/products/SL3S5002N0FUD

- NXP UCODE DNA brochure  
  https://www.nxp.com/docs/en/brochure/75017646.pdf

- RFID Journal, NXP releases IC for secure encrypted UHF reads  
  https://www.rfidjournal.com/news/nxp-releases-ic-for-secure-encrypted-uhf-reads/73100/

## Cryptography and Key Management

- NIST FIPS 197, Advanced Encryption Standard  
  https://csrc.nist.gov/pubs/fips/197/final

- NIST SP 800-38D, Galois/Counter Mode for authenticated encryption  
  https://csrc.nist.gov/pubs/sp/800/38/d/final

- NIST SP 800-57 Part 1 Revision 5, Recommendation for Key Management  
  https://csrc.nist.gov/pubs/sp/800/57/pt1/r5/final

## RFID Security and Privacy Research

- Survey on Prominent RFID Authentication Protocols for Passive Tags  
  https://pmc.ncbi.nlm.nih.gov/articles/PMC6210508/

- A comprehensive taxonomy of security and privacy issues in RFID  
  https://link.springer.com/article/10.1007/s40747-021-00280-6

- A Survey on Low-cost RFID Authentication Protocols  
  https://arxiv.org/abs/1009.3626

- RFID authentication protocol based on a novel EPC Gen2 PRNG  
  https://arxiv.org/abs/2208.05345

- Practical Attacks on a RFID Authentication Protocol Conforming to EPC C-1 G-2 Standard  
  https://arxiv.org/abs/1102.0763

## Project-Specific Notes

- Encrypting metadata stored on a passive RFID tag is different from encrypting the entire RF air interface.
- Basic passive tags can expose ciphertext and pseudonymous EPC values, but they cannot prove freshness unless they support cryptographic challenge-response.
- EPC access passwords, memory locking, CRCs, and checksums are not substitutes for authenticated encryption or cryptographic tag authentication.
