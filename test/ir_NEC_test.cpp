// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendNEC().

// Test sending typical data only.
TEST(TestSendNEC, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendNEC(0);
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s108000", irsend.outputStr());
  irsend.sendNEC(0xAA00FF55);
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s1690m560s560"
            "m560s1690m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s1690m560s1690m560s1690m560s1690m560s1690"
            "m560s1690m560s1690m560s1690m560s560m560s1690m560s560m560s1690"
            "m560s560m560s1690m560s560m560s1690m560s108000",
            irsend.outputStr());
}

// Test sending different bit lengths.
TEST(TestSendNEC, SendSmallData) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendNEC(0xA, 4);  // Send only 4 data bits.
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s108000",
            irsend.outputStr());
  irsend.sendNEC(0, 8);  // Send only 8 data bits.
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s108000", irsend.outputStr());
  irsend.sendNEC(0x1234567890ABCDEF, 64);  // Send 64 data bits.
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s1690m560s560m560s560"
            "m560s1690m560s560m560s560m560s560m560s1690m560s1690m560s560"
            "m560s1690m560s560m560s560m560s560m560s1690m560s560m560s1690"
            "m560s560m560s1690m560s1690m560s560m560s560m560s1690m560s1690"
            "m560s1690m560s1690m560s560m560s560m560s560m560s1690m560s560"
            "m560s560m560s1690m560s560m560s560m560s560m560s560m560s1690m560s560"
            "m560s1690m560s560m560s1690m560s560m560s1690m560s1690m560s1690"
            "m560s1690m560s560m560s560m560s1690m560s1690m560s560m560s1690"
            "m560s1690m560s1690m560s1690m560s560m560s1690m560s1690m560s1690"
            "m560s1690m560s108000", irsend.outputStr());
}

// Test sending with repeats.
TEST(TestSendNEC, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendNEC(0, 8, 0);  // Send a command with 0 repeats.
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s108000", irsend.outputStr());
  irsend.sendNEC(0xAA, 8, 1);  // Send a command with 1 repeat.
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s1690m560s560"
            "m560s1690m560s560m560s108000"
            "m9000s2250m560s108000",
            irsend.outputStr());
  irsend.sendNEC(0xAA, 8, 3);  // Send a command with 3 repeats.
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s1690m560s560"
            "m560s1690m560s560m560s108000"
            "m9000s2250m560s108000"
            "m9000s2250m560s108000"
            "m9000s2250m560s108000",
            irsend.outputStr());
}

// Tests for encodeNEC().

TEST(TestEncodeNEC, NormalNECEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x807F40BF, irsend.encodeNEC(1, 2));
  EXPECT_EQ(0x9A656897, irsend.encodeNEC(0x59, 0x16));
}

TEST(TestEncodeNEC, ExtendedNECEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x9A806897, irsend.encodeNEC(0x159, 0x16));
}

TEST(TestEncodeNEC, CommandTrimmedTo8Bits) {
  IRsendTest irsend(4);
  EXPECT_EQ(irsend.encodeNEC(0x1, 0x2), irsend.encodeNEC(0x1, 0xF02));
  EXPECT_EQ(irsend.encodeNEC(0xFFF0, 0x2), irsend.encodeNEC(0xFFF0, 0xF02));
}

// Tests for decodeNEC().
// Decode normal NEC messages.
TEST(TestDecodeNEC, NormalNECDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Synthesised Normal NEC message.
  irsend.reset();
  irsend.sendNEC(irsend.encodeNEC(0x1, 0x2));
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x807F40BF, irsend.capture.value);
  EXPECT_EQ(0x1, irsend.capture.address);
  EXPECT_EQ(0x2, irsend.capture.command);

  // Real-life Extended NEC code from an actual capture/decode.
  irsend.reset();
  irsend.sendNEC(0xC1A28877);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0xC1A28877, irsend.capture.value);
  EXPECT_EQ(0x4583, irsend.capture.address);
  EXPECT_EQ(0x11, irsend.capture.command);

  // Test strict decoding rejects a NEC-like message.
  irsend.reset();
  irsend.sendNEC(0x0);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeNEC(&irsend.capture));

  // Synthesised Normal NEC message with a repeat.
  irsend.reset();
  irsend.sendNEC(irsend.encodeNEC(0x1, 0x2), 32, 1);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x807F40BF, irsend.capture.value);
  EXPECT_EQ(0x1, irsend.capture.address);
  EXPECT_EQ(0x2, irsend.capture.command);
}

// NEC-like messages without strict mode.
TEST(TestDecodeNEC, NormalNECDecodeWithoutStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendNEC(0x0);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture, 32, false));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  irsend.reset();
  irsend.sendNEC(0x12345678);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture, 32, false));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x12345678, irsend.capture.value);
  EXPECT_EQ(0x2C48, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);
}

// Short NEC-like messages (without strict naturally)
TEST(TestDecodeNEC, ShortNECDecodeWithoutStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendNEC(0x0, 16);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture, 16, false));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(16, irsend.capture.bits);
  EXPECT_EQ(0, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  // Expecting less than what was sent is not valid.
  irsend.reset();
  irsend.sendNEC(0x0, 32);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeNEC(&irsend.capture, 16, false));

  // Send 16 bits of data, but fail because we are expecting 17.
  irsend.reset();
  irsend.sendNEC(0x0, 16);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeNEC(&irsend.capture, 17, false));
}

// Longer NEC-like messages (without strict naturally)
TEST(TestDecodeNEC, LongerNECDecodeWithoutStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendNEC(0x1234567890ABCDEF, 64);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture, 64, false));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0x1234567890ABCDEF, irsend.capture.value);
  EXPECT_EQ(0xD509, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  // Send 63 bits of data, but fail because we are expecting 64.
  irsend.reset();
  irsend.sendNEC(0x0, 63);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeNEC(&irsend.capture, 64, false));
}

// Incorrect decoding reported in Issue #243
// Incorrect handling of decodes when there is no gap recorded at
// the end of a command when using the interrupt code. sendRaw() best emulates
// this for unit testing purposes. sendGC() and sendXXX() will add the trailing
// gap. Users won't see this in normal use.
TEST(TestDecodeNEC, NoTrailingGap_Issue243) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[67] = {9000, 4500, 650, 550, 650, 1650, 600, 550, 650, 550,
                          600, 1650, 650, 550, 600, 1650, 650, 1650, 650, 1650,
                          600, 550, 650, 1650, 650, 1650, 650, 550, 600, 1650,
                          650, 1650, 650, 550, 650, 550, 650, 1650, 650, 550,
                          650, 550, 650, 550, 600, 550, 650, 550, 650, 550,
                          650, 1650, 600, 550, 650, 1650, 650, 1650, 650, 1650,
                          650, 1650, 650, 1650, 650, 1650, 600};
  irsend.sendRaw(rawData, 67, 38);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x4BB640BF, irsend.capture.value);
  EXPECT_EQ(0x6DD2, irsend.capture.address);
  EXPECT_EQ(0x2, irsend.capture.command);

  irsend.reset();
  irsend.sendRaw(rawData, 67, 38);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x4BB640BF, irsend.capture.value);
}
