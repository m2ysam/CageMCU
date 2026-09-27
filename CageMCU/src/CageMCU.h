#ifndef CAGEMCU_H
#define CAGEMCU_H

#include <Arduino.h>


class CageMCU {

public:

    // ========================================================
    // BEGIN
    // ========================================================

    void begin();


    // ========================================================
    // WHITELIST CHECKS
    // ========================================================

    bool validStringCheck(
        const String validSignals[],
        const String& checkedSignal,
        int count
    );


    bool validIntegerCheck(
        const int validSignals[],
        const String& checkedSignal,
        int count
    );


    bool validFloatCheck(
        const float validSignals[],
        const String& checkedSignal,
        int count
    );


    // ========================================================
    // RANGE CHECKS
    // ========================================================

    bool validIntegerRange(
        long value,
        long minValue,
        long maxValue
    );


    bool validFloatRange(
        float value,
        float minValue,
        float maxValue
    );


    // ========================================================
    // TRANSFORM / SHIFTER
    // ========================================================

    bool validTransformKey(
        const String& key
    );


    String messageEncode(
        const String& message,
        const String& key
    );


    String messageDecode(
        const String& message,
        const String& key
    );


    // ========================================================
    // ANTI-REPLAY
    // ========================================================

    String messageCreate(
        const String& payload
    );


    String messageCheck(
        const String& message
    );


    // ========================================================
    // HMAC-SHA256
    // ========================================================

    void setHMACKey(
        const String& key
    );


    String hmacCreate(
        const String& message
    );


    bool hmacCheck(
        const String& message,
        const String& receivedHMAC
    );


    // ========================================================
    // LOCK SYSTEM
    //
    // cage.lockAfter(5);
    // ========================================================

    void lockAfter(
        int limit
    );


    bool checkInput(
        const String validStrings[],
        int stringCount,

        const int validIntegers[],
        int integerCount,

        const float validFloats[],
        int floatCount,

        const String& input
    );


    void registerFailure();


    void registerSuccess();


    bool isLocked();


    int failedAttempts();


private:

    // ========================================================
    // INPUT TYPE CHECKS
    // ========================================================

    bool isIntegerInput(
        const String& value
    );


    bool isFloatInput(
        const String& value
    );


    bool isUnsignedNumber(
        const String& value
    );


    // ========================================================
    // ANTI-REPLAY INTERNAL
    // ========================================================

    String createSessionId();


    unsigned long _messageCounter = 0;

    unsigned long _lastReceivedSequence = 0;


    String _sessionId = "";

    String _lastReceivedSession = "";


    // ========================================================
    // HMAC INTERNAL
    // ========================================================

    String _hmacKey = "";


    // ========================================================
    // LOCK INTERNAL
    // ========================================================

    int _failedAttempts = 0;

    int _lockLimit = 0;

    bool _locked = false;
};


#endif