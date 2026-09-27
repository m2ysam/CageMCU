# CageMCU

CageMCU is a lightweight cybersecurity and anomaly-detection library for Arduino-compatible microcontrollers.

It was developed as the microcontroller-side companion to our **CyberCage antivirus project for TÜBİTAK 2026**.

## Features

- String whitelist validation
- Integer whitelist validation
- Float whitelist validation
- Integer and float range checking
- Message transformation / shifter
- Anti-replay protection
- HMAC-SHA256 authentication
- Failed-attempt counting
- Automatic lockout after repeated invalid inputs
- Serial command validation

## Installation

Download this repository as a ZIP file.

In Arduino IDE:

```text
Sketch
→ Include Library
→ Add .ZIP Library...

Select the downloaded CageMCU ZIP file.

CageMCU also requires the Crypto library by Rhys Weatherley for HMAC-SHA256 support.

Install it using:

Arduino IDE
→ Library Manager
→ Search: Crypto
→ Install
Basic Usage
#include <CageMCU.h>

CageMCU cage;

String validCommands[] = {
    "OPEN",
    "CLOSE",
    "STATUS"
};

int validIntegers[] = {
    10,
    20,
    30
};

float validFloats[] = {
    1.5,
    2.5,
    3.5
};

void setup() {

    Serial.begin(9600);

    cage.begin();

    cage.lockAfter(5);
}

void loop() {

    if (!Serial.available()) {
        return;
    }

    String input =
        Serial.readStringUntil('\n');

    input.trim();

    if (cage.isLocked()) {

        Serial.println("CAGE_LOCKED");

        return;
    }

    bool valid =
        cage.checkInput(
            validCommands,
            3,

            validIntegers,
            3,

            validFloats,
            3,

            input
        );

    if (!valid) {

        Serial.print("INVALID | FAILURES: ");

        Serial.println(
            cage.failedAttempts()
        );

        return;
    }

    if (input == "OPEN") {

        Serial.println(
            "OPEN command accepted"
        );
    }

    else if (input == "CLOSE") {

        Serial.println(
            "CLOSE command accepted"
        );
    }

    else if (input == "STATUS") {

        Serial.println(
            "SYSTEM_OK"
        );
    }
}
Message Transformation

CageMCU includes a lightweight position-based message transformation system.

Example:

String encoded =
    cage.messageEncode(
        "OPEN",
        "235667"
    );

String decoded =
    cage.messageDecode(
        encoded,
        "235667"
    );

The transformation key is interpreted as three rule pairs:

235667

(2,3)
(5,6)
(6,7)

For example, (2,3) means that characters in positions divisible by 2 are shifted by 3 positions.

The transformation system is intended as an obfuscation layer and is not a replacement for cryptographic authentication.

HMAC-SHA256

Set a shared HMAC key:

cage.setHMACKey(
    "YOUR_SECRET_KEY"
);

Create an authentication tag:

String tag =
    cage.hmacCreate(message);

Verify it:

bool valid =
    cage.hmacCheck(
        message,
        tag
    );

The secret HMAC key should not be transmitted together with the message.

Anti-Replay Protection

Create a message:

String packet =
    cage.messageCreate(
        "OPEN"
    );

Check an incoming message:

String result =
    cage.messageCheck(
        packet
    );

Possible results include:

VALID
REPLAY
INVALID_FORMAT
Lockout Protection

Set the number of consecutive invalid inputs allowed:

cage.lockAfter(5);

Check whether the system is locked:

if (cage.isLocked()) {

    Serial.println(
        "CAGE_LOCKED"
    );
}

Check the current failed-attempt count:

cage.failedAttempts();

A valid input resets the consecutive failure counter.

Main API
begin()

validStringCheck()
validIntegerCheck()
validFloatCheck()

validIntegerRange()
validFloatRange()

validTransformKey()
messageEncode()
messageDecode()

messageCreate()
messageCheck()

setHMACKey()
hmacCreate()
hmacCheck()

lockAfter()
checkInput()
registerFailure()
registerSuccess()
isLocked()
failedAttempts()
Project Structure
CageMCU/
├── src/
│   ├── CageMCU.h
│   └── CageMCU.cpp
│
├── examples/
│   └── BasicExample/
│       └── BasicExample.ino
│
├── library.properties
└── README.md
Version

Current version:

CageMCU v1.0.0
Project

CageMCU is part of the CyberCage cybersecurity project developed for TÜBİTAK 2026.
