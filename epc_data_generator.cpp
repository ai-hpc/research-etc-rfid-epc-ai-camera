#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using ByteArray12 = std::array<uint8_t, 12>;
using ByteArray32 = std::array<uint8_t, 32>;

constexpr size_t EPC_BYTES = 12;
constexpr size_t METADATA_BYTES = 7;
constexpr size_t CRC_BYTE_INDEX = 7;
constexpr size_t MAC_START_INDEX = 8;
constexpr size_t MAC_BYTES = 4;

uint32_t rotr(uint32_t value, uint32_t shift) {
    return (value >> shift) | (value << (32U - shift));
}

class Sha256 {
public:
    Sha256() {
        reset();
    }

    void update(const uint8_t* input, size_t length) {
        for (size_t i = 0; i < length; ++i) {
            data_[dataLength_++] = input[i];
            if (dataLength_ == 64) {
                transform();
                bitLength_ += 512;
                dataLength_ = 0;
            }
        }
    }

    void update(const std::vector<uint8_t>& input) {
        update(input.data(), input.size());
    }

    ByteArray32 final() {
        size_t i = dataLength_;

        if (dataLength_ < 56) {
            data_[i++] = 0x80;
            while (i < 56) {
                data_[i++] = 0x00;
            }
        } else {
            data_[i++] = 0x80;
            while (i < 64) {
                data_[i++] = 0x00;
            }
            transform();
            std::fill(data_.begin(), data_.begin() + 56, 0x00);
        }

        bitLength_ += dataLength_ * 8;
        data_[63] = static_cast<uint8_t>(bitLength_);
        data_[62] = static_cast<uint8_t>(bitLength_ >> 8);
        data_[61] = static_cast<uint8_t>(bitLength_ >> 16);
        data_[60] = static_cast<uint8_t>(bitLength_ >> 24);
        data_[59] = static_cast<uint8_t>(bitLength_ >> 32);
        data_[58] = static_cast<uint8_t>(bitLength_ >> 40);
        data_[57] = static_cast<uint8_t>(bitLength_ >> 48);
        data_[56] = static_cast<uint8_t>(bitLength_ >> 56);
        transform();

        ByteArray32 hash{};
        for (size_t word = 0; word < 8; ++word) {
            hash[word * 4] = static_cast<uint8_t>((state_[word] >> 24) & 0xff);
            hash[word * 4 + 1] = static_cast<uint8_t>((state_[word] >> 16) & 0xff);
            hash[word * 4 + 2] = static_cast<uint8_t>((state_[word] >> 8) & 0xff);
            hash[word * 4 + 3] = static_cast<uint8_t>(state_[word] & 0xff);
        }
        return hash;
    }

private:
    void reset() {
        data_.fill(0);
        dataLength_ = 0;
        bitLength_ = 0;
        state_ = {
            0x6a09e667,
            0xbb67ae85,
            0x3c6ef372,
            0xa54ff53a,
            0x510e527f,
            0x9b05688c,
            0x1f83d9ab,
            0x5be0cd19,
        };
    }

    void transform() {
        static constexpr std::array<uint32_t, 64> k = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
        };

        std::array<uint32_t, 64> m{};
        for (size_t i = 0, j = 0; i < 16; ++i, j += 4) {
            m[i] = (static_cast<uint32_t>(data_[j]) << 24) |
                   (static_cast<uint32_t>(data_[j + 1]) << 16) |
                   (static_cast<uint32_t>(data_[j + 2]) << 8) |
                   (static_cast<uint32_t>(data_[j + 3]));
        }

        for (size_t i = 16; i < 64; ++i) {
            const uint32_t s0 = rotr(m[i - 15], 7) ^ rotr(m[i - 15], 18) ^ (m[i - 15] >> 3);
            const uint32_t s1 = rotr(m[i - 2], 17) ^ rotr(m[i - 2], 19) ^ (m[i - 2] >> 10);
            m[i] = m[i - 16] + s0 + m[i - 7] + s1;
        }

        uint32_t a = state_[0];
        uint32_t b = state_[1];
        uint32_t c = state_[2];
        uint32_t d = state_[3];
        uint32_t e = state_[4];
        uint32_t f = state_[5];
        uint32_t g = state_[6];
        uint32_t h = state_[7];

        for (size_t i = 0; i < 64; ++i) {
            const uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const uint32_t ch = (e & f) ^ ((~e) & g);
            const uint32_t temp1 = h + s1 + ch + k[i] + m[i];
            const uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temp2 = s0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }

    std::array<uint8_t, 64> data_{};
    std::array<uint32_t, 8> state_{};
    size_t dataLength_ = 0;
    uint64_t bitLength_ = 0;
};

ByteArray32 sha256(const std::vector<uint8_t>& input) {
    Sha256 sha;
    sha.update(input);
    return sha.final();
}

ByteArray32 hmacSha256(const std::string& secret, const std::vector<uint8_t>& message) {
    std::vector<uint8_t> key(secret.begin(), secret.end());
    if (key.size() > 64) {
        const ByteArray32 hashedKey = sha256(key);
        key.assign(hashedKey.begin(), hashedKey.end());
    }
    key.resize(64, 0x00);

    std::vector<uint8_t> outerPad(64);
    std::vector<uint8_t> innerPad(64);
    for (size_t i = 0; i < 64; ++i) {
        outerPad[i] = static_cast<uint8_t>(key[i] ^ 0x5c);
        innerPad[i] = static_cast<uint8_t>(key[i] ^ 0x36);
    }

    std::vector<uint8_t> innerInput = innerPad;
    innerInput.insert(innerInput.end(), message.begin(), message.end());
    const ByteArray32 innerHash = sha256(innerInput);

    std::vector<uint8_t> outerInput = outerPad;
    outerInput.insert(outerInput.end(), innerHash.begin(), innerHash.end());
    return sha256(outerInput);
}

uint8_t crc8(const uint8_t* input, size_t length) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < length; ++i) {
        crc ^= input[i];
        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x80) != 0) {
                crc = static_cast<uint8_t>((crc << 1) ^ 0x07);
            } else {
                crc = static_cast<uint8_t>(crc << 1);
            }
        }
    }
    return crc;
}

void writeBits(ByteArray12& buffer, size_t bitOffset, size_t width, uint32_t value) {
    for (size_t i = 0; i < width; ++i) {
        const size_t sourceShift = width - 1 - i;
        const bool bitSet = ((value >> sourceShift) & 1U) != 0;
        const size_t absoluteBit = bitOffset + i;
        const size_t byteIndex = absoluteBit / 8;
        const size_t bitInByte = 7 - (absoluteBit % 8);
        if (bitSet) {
            buffer[byteIndex] |= static_cast<uint8_t>(1U << bitInByte);
        }
    }
}

uint32_t readBits(const ByteArray12& buffer, size_t bitOffset, size_t width) {
    uint32_t value = 0;
    for (size_t i = 0; i < width; ++i) {
        const size_t absoluteBit = bitOffset + i;
        const size_t byteIndex = absoluteBit / 8;
        const size_t bitInByte = 7 - (absoluteBit % 8);
        value = (value << 1) | ((buffer[byteIndex] >> bitInByte) & 1U);
    }
    return value;
}

std::vector<uint8_t> firstBytes(const ByteArray12& epc, size_t count) {
    return std::vector<uint8_t>(epc.begin(), epc.begin() + static_cast<std::ptrdiff_t>(count));
}

std::string bytesToHex(const ByteArray12& data) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (uint8_t byte : data) {
        out << std::setw(2) << static_cast<int>(byte);
    }
    return out.str();
}

uint8_t hexValue(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<uint8_t>(10 + c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return static_cast<uint8_t>(10 + c - 'A');
    }
    throw std::invalid_argument("EPC contains a non-hex character");
}

ByteArray12 hexToBytes(std::string hex) {
    if (hex.rfind("0x", 0) == 0 || hex.rfind("0X", 0) == 0) {
        hex = hex.substr(2);
    }
    if (hex.size() != EPC_BYTES * 2) {
        throw std::invalid_argument("EPC-96 must be exactly 24 hex characters");
    }

    ByteArray12 data{};
    for (size_t i = 0; i < EPC_BYTES; ++i) {
        data[i] = static_cast<uint8_t>((hexValue(hex[i * 2]) << 4) | hexValue(hex[i * 2 + 1]));
    }
    return data;
}

struct EpcFields {
    uint8_t version = 3;
    uint8_t issuerId = 0;
    uint8_t vehicleClass = 0;
    uint8_t colorCode = 0;
    uint16_t plateLast4 = 0;
    uint16_t tagRef = 0;
    uint8_t keyId = 0;
    uint8_t nonce = 0;
};

struct DecodedEpc {
    EpcFields fields;
    uint8_t storedCrc = 0;
    uint8_t expectedCrc = 0;
    std::array<uint8_t, MAC_BYTES> storedMac{};
    std::array<uint8_t, MAC_BYTES> expectedMac{};

    bool crcValid() const {
        return storedCrc == expectedCrc;
    }

    bool macValid() const {
        return storedMac == expectedMac;
    }

    bool valid() const {
        return crcValid() && macValid();
    }
};

struct VehicleClassEntry {
    uint8_t code;
    const char* name;
};

const std::vector<VehicleClassEntry>& vehicleClassTable() {
    static const std::vector<VehicleClassEntry> classes = {
        {0, "Unknown / Unset"},
        {1, "Car"},
        {2, "Truck"},
        {3, "Bus"},
        {4, "Van"},
        {5, "Motorcycle"},
        {6, "Trailer"},
        {7, "Emergency"},
        {8, "Heavy Equipment"},
        {15, "Other / Unrecognizable"},
    };
    return classes;
}

std::string normalizeName(const std::string& value) {
    std::string normalized;
    for (unsigned char c : value) {
        if (std::isalnum(c) != 0) {
            normalized.push_back(static_cast<char>(std::tolower(c)));
        }
    }
    return normalized;
}

std::string vehicleClassName(uint8_t code) {
    for (const VehicleClassEntry& entry : vehicleClassTable()) {
        if (entry.code == code) {
            return entry.name;
        }
    }
    return "Unassigned class code";
}

bool tryParseUint(const std::string& value, uint32_t& output) {
    size_t processed = 0;
    unsigned long parsed = 0;
    try {
        parsed = std::stoul(value, &processed, 0);
    } catch (const std::exception&) {
        return false;
    }
    if (processed != value.size() || parsed > 0xffffffffUL) {
        return false;
    }
    output = static_cast<uint32_t>(parsed);
    return true;
}

void requireRange(const std::string& name, uint32_t value, uint32_t maxValue) {
    if (value > maxValue) {
        std::ostringstream message;
        message << name << " must be between 0 and " << maxValue;
        throw std::invalid_argument(message.str());
    }
}

void validateFields(const EpcFields& fields) {
    requireRange("version", fields.version, 15);
    requireRange("issuer", fields.issuerId, 255);
    requireRange("class", fields.vehicleClass, 15);
    requireRange("color", fields.colorCode, 15);
    requireRange("plate", fields.plateLast4, 9999);
    requireRange("tag", fields.tagRef, 65535);
    requireRange("key-id", fields.keyId, 15);
    requireRange("nonce", fields.nonce, 3);
}

ByteArray12 generateEpc(const EpcFields& fields, const std::string& secret) {
    if (secret.empty()) {
        throw std::invalid_argument("secret must not be empty");
    }
    validateFields(fields);

    ByteArray12 epc{};
    size_t bit = 0;
    writeBits(epc, bit, 4, fields.version);
    bit += 4;
    writeBits(epc, bit, 8, fields.issuerId);
    bit += 8;
    writeBits(epc, bit, 4, fields.vehicleClass);
    bit += 4;
    writeBits(epc, bit, 4, fields.colorCode);
    bit += 4;
    writeBits(epc, bit, 14, fields.plateLast4);
    bit += 14;
    writeBits(epc, bit, 16, fields.tagRef);
    bit += 16;
    writeBits(epc, bit, 4, fields.keyId);
    bit += 4;
    writeBits(epc, bit, 2, fields.nonce);

    epc[CRC_BYTE_INDEX] = crc8(epc.data(), METADATA_BYTES);

    const std::vector<uint8_t> macInput = firstBytes(epc, MAC_START_INDEX);
    const ByteArray32 mac = hmacSha256(secret, macInput);
    epc[MAC_START_INDEX] = mac[0];
    epc[MAC_START_INDEX + 1] = mac[1];
    epc[MAC_START_INDEX + 2] = mac[2];
    epc[MAC_START_INDEX + 3] = mac[3];

    return epc;
}

DecodedEpc decodeEpc(const ByteArray12& epc, const std::string& secret) {
    if (secret.empty()) {
        throw std::invalid_argument("secret must not be empty");
    }

    DecodedEpc decoded;
    size_t bit = 0;
    decoded.fields.version = static_cast<uint8_t>(readBits(epc, bit, 4));
    bit += 4;
    decoded.fields.issuerId = static_cast<uint8_t>(readBits(epc, bit, 8));
    bit += 8;
    decoded.fields.vehicleClass = static_cast<uint8_t>(readBits(epc, bit, 4));
    bit += 4;
    decoded.fields.colorCode = static_cast<uint8_t>(readBits(epc, bit, 4));
    bit += 4;
    decoded.fields.plateLast4 = static_cast<uint16_t>(readBits(epc, bit, 14));
    bit += 14;
    decoded.fields.tagRef = static_cast<uint16_t>(readBits(epc, bit, 16));
    bit += 16;
    decoded.fields.keyId = static_cast<uint8_t>(readBits(epc, bit, 4));
    bit += 4;
    decoded.fields.nonce = static_cast<uint8_t>(readBits(epc, bit, 2));

    decoded.storedCrc = epc[CRC_BYTE_INDEX];
    decoded.expectedCrc = crc8(epc.data(), METADATA_BYTES);

    decoded.storedMac = {epc[MAC_START_INDEX], epc[MAC_START_INDEX + 1], epc[MAC_START_INDEX + 2], epc[MAC_START_INDEX + 3]};
    const std::vector<uint8_t> macInput = firstBytes(epc, MAC_START_INDEX);
    const ByteArray32 mac = hmacSha256(secret, macInput);
    decoded.expectedMac = {mac[0], mac[1], mac[2], mac[3]};

    return decoded;
}

uint32_t parseUint(const std::string& value, const std::string& name) {
    uint32_t parsed = 0;
    if (!tryParseUint(value, parsed)) {
        throw std::invalid_argument("invalid numeric value for " + name + ": " + value);
    }
    return parsed;
}

uint8_t parseClassCode(const std::string& value) {
    uint32_t numericValue = 0;
    if (tryParseUint(value, numericValue)) {
        requireRange("class", numericValue, 15);
        return static_cast<uint8_t>(numericValue);
    }

    const std::string normalized = normalizeName(value);
    if (normalized == "other" || normalized == "unrecognizable" || normalized == "notrecognizable") {
        return 15;
    }
    if (normalized == "unknown" || normalized == "unset") {
        return 0;
    }

    for (const VehicleClassEntry& entry : vehicleClassTable()) {
        if (normalizeName(entry.name) == normalized) {
            return entry.code;
        }
    }

    throw std::invalid_argument("unknown vehicle class: " + value + ". Use --list-classes or code 15 for Other / Unrecognizable");
}

uint8_t randomNonce() {
    std::random_device random;
    std::uniform_int_distribution<int> distribution(0, 3);
    return static_cast<uint8_t>(distribution(random));
}

std::string nextArgument(int& index, int argc, char* argv[], const std::string& optionName) {
    if (index + 1 >= argc) {
        throw std::invalid_argument(optionName + " requires a value");
    }
    ++index;
    return argv[index];
}

void printClassTable() {
    std::cout << "Vehicle class codes:\n";
    for (const VehicleClassEntry& entry : vehicleClassTable()) {
        std::cout << "  " << std::setw(3) << static_cast<int>(entry.code)
                  << "  " << entry.name << '\n';
    }
}

void printUsage(const char* executable) {
    std::cout
        << "Usage:\n"
        << "  " << executable << " --issuer N --class CODE_OR_NAME --color N --plate N --tag N --key-id N --secret TEXT [--version N] [--nonce N]\n"
        << "  " << executable << " --decode EPC_HEX --secret TEXT\n\n"
        << "  " << executable << " --list-classes\n\n"
        << "Field limits for EPC-96 layout:\n"
        << "  version: 0-15\n"
        << "  issuer:  0-255\n"
        << "  class:   0-15, vehicle class code or name; 15 means Other / Unrecognizable\n"
        << "  color:   0-15\n"
        << "  plate:   0-9999, last four digits only\n"
        << "  tag:     0-65535, compact backend reference\n"
        << "  key-id:  0-15\n"
        << "  nonce:   0-3, generated automatically if omitted\n\n"
        << "Example:\n"
        << "  " << executable << " --issuer 12 --class truck --color 3 --plate 6789 --tag 1024 --key-id 2 --secret toll-secret --nonce 1\n";
}

void printFields(const EpcFields& fields) {
    std::cout
        << "Version:        " << static_cast<int>(fields.version) << '\n'
        << "Issuer ID:      " << static_cast<int>(fields.issuerId) << '\n'
        << "Vehicle class:  " << static_cast<int>(fields.vehicleClass) << '\n'
        << "Class name:     " << vehicleClassName(fields.vehicleClass) << '\n'
        << "Color code:     " << static_cast<int>(fields.colorCode) << '\n'
        << "Plate last 4:   " << std::setw(4) << std::setfill('0') << fields.plateLast4 << std::setfill(' ') << '\n'
        << "Tag reference:  " << fields.tagRef << '\n'
        << "Key ID:         " << static_cast<int>(fields.keyId) << '\n'
        << "Nonce:          " << static_cast<int>(fields.nonce) << '\n';
}

void printMac(const std::array<uint8_t, MAC_BYTES>& mac) {
    std::cout << std::uppercase << std::hex << std::setfill('0')
              << std::setw(2) << static_cast<int>(mac[0])
              << std::setw(2) << static_cast<int>(mac[1])
              << std::setw(2) << static_cast<int>(mac[2])
              << std::setw(2) << static_cast<int>(mac[3])
              << std::dec << std::setfill(' ');
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc == 1) {
            printUsage(argv[0]);
            return 0;
        }

        EpcFields fields;
        std::string secret;
        std::string decodeHex;
        bool decodeMode = false;
        bool nonceProvided = false;

        bool hasIssuer = false;
        bool hasClass = false;
        bool hasColor = false;
        bool hasPlate = false;
        bool hasTag = false;
        bool hasKeyId = false;

        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];

            if (arg == "--help" || arg == "-h") {
                printUsage(argv[0]);
                return 0;
            }
            if (arg == "--list-classes") {
                printClassTable();
                return 0;
            }
            if (arg == "--decode") {
                decodeMode = true;
                decodeHex = nextArgument(i, argc, argv, arg);
            } else if (arg == "--secret") {
                secret = nextArgument(i, argc, argv, arg);
            } else if (arg == "--version") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("version", value, 15);
                fields.version = static_cast<uint8_t>(value);
            } else if (arg == "--issuer") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("issuer", value, 255);
                fields.issuerId = static_cast<uint8_t>(value);
                hasIssuer = true;
            } else if (arg == "--class" || arg == "--vehicle-class") {
                fields.vehicleClass = parseClassCode(nextArgument(i, argc, argv, arg));
                hasClass = true;
            } else if (arg == "--color") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("color", value, 15);
                fields.colorCode = static_cast<uint8_t>(value);
                hasColor = true;
            } else if (arg == "--plate") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("plate", value, 9999);
                fields.plateLast4 = static_cast<uint16_t>(value);
                hasPlate = true;
            } else if (arg == "--tag") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("tag", value, 65535);
                fields.tagRef = static_cast<uint16_t>(value);
                hasTag = true;
            } else if (arg == "--key-id") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("key-id", value, 15);
                fields.keyId = static_cast<uint8_t>(value);
                hasKeyId = true;
            } else if (arg == "--nonce") {
                const uint32_t value = parseUint(nextArgument(i, argc, argv, arg), arg);
                requireRange("nonce", value, 3);
                fields.nonce = static_cast<uint8_t>(value);
                nonceProvided = true;
            } else {
                throw std::invalid_argument("unknown option: " + arg);
            }
        }

        if (decodeMode) {
            const ByteArray12 epc = hexToBytes(decodeHex);
            const DecodedEpc decoded = decodeEpc(epc, secret);

            std::cout << "EPC-96:         " << bytesToHex(epc) << '\n';
            printFields(decoded.fields);
            std::cout << "Stored CRC-8:   0x" << std::uppercase << std::hex << std::setw(2)
                      << std::setfill('0') << static_cast<int>(decoded.storedCrc)
                      << std::dec << std::setfill(' ') << '\n';
            std::cout << "Expected CRC-8: 0x" << std::uppercase << std::hex << std::setw(2)
                      << std::setfill('0') << static_cast<int>(decoded.expectedCrc)
                      << std::dec << std::setfill(' ') << '\n';
            std::cout << "Stored MAC-32:  ";
            printMac(decoded.storedMac);
            std::cout << '\n';
            std::cout << "Expected MAC-32:";
            std::cout << ' ';
            printMac(decoded.expectedMac);
            std::cout << '\n';
            std::cout << "CRC valid:      " << (decoded.crcValid() ? "yes" : "no") << '\n';
            std::cout << "MAC valid:      " << (decoded.macValid() ? "yes" : "no") << '\n';
            std::cout << "Overall valid:  " << (decoded.valid() ? "yes" : "no") << '\n';
            return decoded.valid() ? 0 : 2;
        }

        if (!hasIssuer || !hasClass || !hasColor || !hasPlate || !hasTag || !hasKeyId) {
            throw std::invalid_argument("generation requires --issuer, --class, --color, --plate, --tag, --key-id, and --secret");
        }
        if (!nonceProvided) {
            fields.nonce = randomNonce();
        }

        const ByteArray12 epc = generateEpc(fields, secret);
        const DecodedEpc decoded = decodeEpc(epc, secret);

        std::cout << "Generated EPC-96: " << bytesToHex(epc) << "\n\n";
        printFields(decoded.fields);
        std::cout << "CRC valid:       " << (decoded.crcValid() ? "yes" : "no") << '\n';
        std::cout << "MAC valid:       " << (decoded.macValid() ? "yes" : "no") << '\n';

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n\n";
        printUsage(argv[0]);
        return 1;
    }
}
