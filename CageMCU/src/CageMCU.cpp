#include "CageMCU.h"

#include <math.h>
#include <Crypto.h>
#include <SHA256.h>


// ============================================================
// BEGIN
// ============================================================

void CageMCU::begin() {

    _sessionId =
        createSessionId();

    _messageCounter = 0;

    _lastReceivedSequence = 0;

    _lastReceivedSession = "";

    _failedAttempts = 0;

    _locked = false;
}


// ============================================================
// INTEGER INPUT CHECK
// ============================================================

bool CageMCU::isIntegerInput(
    const String& value
) {

    if (
        value.length() == 0
        ) {
        return false;
    }


    unsigned int start = 0;


    if (
        value[0] == '-' ||
        value[0] == '+'
        ) {

        if (
            value.length() == 1
            ) {
            return false;
        }

        start = 1;
    }


    for (
        unsigned int i = start;
        i < value.length();
        i++
        ) {

        if (
            value[i] < '0' ||
            value[i] > '9'
            ) {
            return false;
        }
    }


    return true;
}


// ============================================================
// FLOAT INPUT CHECK
// ============================================================

bool CageMCU::isFloatInput(
    const String& value
) {

    if (
        value.length() == 0
        ) {
        return false;
    }


    unsigned int start = 0;

    bool dotFound = false;

    bool digitFound = false;


    if (
        value[0] == '-' ||
        value[0] == '+'
        ) {

        if (
            value.length() == 1
            ) {
            return false;
        }

        start = 1;
    }


    for (
        unsigned int i = start;
        i < value.length();
        i++
        ) {

        char c =
            value[i];


        if (
            c >= '0' &&
            c <= '9'
            ) {

            digitFound = true;

            continue;
        }


        if (
            c == '.' &&
            !dotFound
            ) {

            dotFound = true;

            continue;
        }


        return false;
    }


    return (
        digitFound &&
        dotFound
        );
}


// ============================================================
// STRING WHITELIST
// ============================================================

bool CageMCU::validStringCheck(
    const String validSignals[],
    const String& checkedSignal,
    int count
) {

    // Integer veya float görünüyorsa
    // String komutu olarak kabul etme.

    if (
        isIntegerInput(
            checkedSignal
        ) ||
        isFloatInput(
            checkedSignal
        )
        ) {

        return false;
    }


    for (
        int i = 0;
        i < count;
        i++
        ) {

        if (
            validSignals[i] ==
            checkedSignal
            ) {

            return true;
        }
    }


    return false;
}


// ============================================================
// INTEGER WHITELIST
// ============================================================

bool CageMCU::validIntegerCheck(
    const int validSignals[],
    const String& checkedSignal,
    int count
) {

    if (
        !isIntegerInput(
            checkedSignal
        )
        ) {

        return false;
    }


    long value =
        checkedSignal.toInt();


    for (
        int i = 0;
        i < count;
        i++
        ) {

        if (
            validSignals[i] ==
            value
            ) {

            return true;
        }
    }


    return false;
}


// ============================================================
// FLOAT WHITELIST
// ============================================================

bool CageMCU::validFloatCheck(
    const float validSignals[],
    const String& checkedSignal,
    int count
) {

    if (
        !isFloatInput(
            checkedSignal
        )
        ) {

        return false;
    }


    float value =
        checkedSignal.toFloat();


    const float tolerance =
        0.001f;


    for (
        int i = 0;
        i < count;
        i++
        ) {

        if (
            fabs(
                validSignals[i] -
                value
            ) < tolerance
            ) {

            return true;
        }
    }


    return false;
}


// ============================================================
// INTEGER RANGE
// ============================================================

bool CageMCU::validIntegerRange(
    long value,
    long minValue,
    long maxValue
) {

    return (
        value >= minValue &&
        value <= maxValue
        );
}


// ============================================================
// FLOAT RANGE
// ============================================================

bool CageMCU::validFloatRange(
    float value,
    float minValue,
    float maxValue
) {

    return (
        value >= minValue &&
        value <= maxValue
        );
}


// ============================================================
// TRANSFORM KEY CHECK
//
// 235667
//
// (2,3)
// (5,6)
// (6,7)
// ============================================================

bool CageMCU::validTransformKey(
    const String& key
) {

    if (
        key.length() != 6
        ) {

        return false;
    }


    for (
        int i = 0;
        i < 6;
        i++
        ) {

        if (
            key[i] < '0' ||
            key[i] > '9'
            ) {

            return false;
        }
    }


    if (
        key[0] == '0' ||
        key[2] == '0' ||
        key[4] == '0'
        ) {

        return false;
    }


    return true;
}


// ============================================================
// MESSAGE ENCODE
// ============================================================

String CageMCU::messageEncode(
    const String& message,
    const String& key
) {

    if (
        !validTransformKey(
            key
        )
        ) {

        return "INVALID_KEY";
    }


    String result =
        message;


    for (
        unsigned int i = 0;
        i < result.length();
        i++
        ) {

        char currentChar =
            result[i];


        bool upperCase =
            currentChar >= 'A' &&
            currentChar <= 'Z';


        bool lowerCase =
            currentChar >= 'a' &&
            currentChar <= 'z';


        if (
            !upperCase &&
            !lowerCase
            ) {

            continue;
        }


        int position =
            i + 1;


        int totalShift =
            0;


        for (
            int k = 0;
            k < 6;
            k += 2
            ) {

            int divisor =
                key[k] - '0';


            int shift =
                key[k + 1] - '0';


            if (
                position %
                divisor == 0
                ) {

                totalShift +=
                    shift;
            }
        }


        totalShift %=
            26;


        if (upperCase) {

            result[i] =
                'A' +
                (
                    (
                        currentChar -
                        'A' +
                        totalShift
                        ) % 26
                    );
        }

        else {

            result[i] =
                'a' +
                (
                    (
                        currentChar -
                        'a' +
                        totalShift
                        ) % 26
                    );
        }
    }


    return result;
}


// ============================================================
// MESSAGE DECODE
// ============================================================

String CageMCU::messageDecode(
    const String& message,
    const String& key
) {

    if (
        !validTransformKey(
            key
        )
        ) {

        return "INVALID_KEY";
    }


    String result =
        message;


    for (
        unsigned int i = 0;
        i < result.length();
        i++
        ) {

        char currentChar =
            result[i];


        bool upperCase =
            currentChar >= 'A' &&
            currentChar <= 'Z';


        bool lowerCase =
            currentChar >= 'a' &&
            currentChar <= 'z';


        if (
            !upperCase &&
            !lowerCase
            ) {

            continue;
        }


        int position =
            i + 1;


        int totalShift =
            0;


        for (
            int k = 0;
            k < 6;
            k += 2
            ) {

            int divisor =
                key[k] - '0';


            int shift =
                key[k + 1] - '0';


            if (
                position %
                divisor == 0
                ) {

                totalShift +=
                    shift;
            }
        }


        totalShift %=
            26;


        if (upperCase) {

            result[i] =
                'A' +
                (
                    (
                        currentChar -
                        'A' -
                        totalShift +
                        26
                        ) % 26
                    );
        }

        else {

            result[i] =
                'a' +
                (
                    (
                        currentChar -
                        'a' -
                        totalShift +
                        26
                        ) % 26
                    );
        }
    }


    return result;
}


// ============================================================
// CREATE SESSION ID
// ============================================================

String CageMCU::createSessionId() {

    unsigned long value =
        micros() ^
        millis();


    String session =
        String(
            value,
            HEX
        );


    session.toUpperCase();


    return session;
}


// ============================================================
// MESSAGE CREATE
//
// SESSION|SEQUENCE|PAYLOAD
// ============================================================

String CageMCU::messageCreate(
    const String& payload
) {

    if (
        _sessionId.length() ==
        0
        ) {

        _sessionId =
            createSessionId();
    }


    _messageCounter++;


    return (
        _sessionId +
        "|" +
        String(
            _messageCounter
        ) +
        "|" +
        payload
        );
}


// ============================================================
// UNSIGNED NUMBER CHECK
// ============================================================

bool CageMCU::isUnsignedNumber(
    const String& value
) {

    if (
        value.length() == 0
        ) {

        return false;
    }


    for (
        unsigned int i = 0;
        i < value.length();
        i++
        ) {

        if (
            value[i] < '0' ||
            value[i] > '9'
            ) {

            return false;
        }
    }


    return true;
}


// ============================================================
// MESSAGE CHECK
//
// VALID
// REPLAY
// INVALID_FORMAT
// ============================================================

String CageMCU::messageCheck(
    const String& message
) {

    int firstSeparator =
        message.indexOf('|');


    if (
        firstSeparator ==
        -1
        ) {

        return "INVALID_FORMAT";
    }


    int secondSeparator =
        message.indexOf(
            '|',
            firstSeparator + 1
        );


    if (
        secondSeparator ==
        -1
        ) {

        return "INVALID_FORMAT";
    }


    String session =
        message.substring(
            0,
            firstSeparator
        );


    String sequenceText =
        message.substring(
            firstSeparator + 1,
            secondSeparator
        );


    String payload =
        message.substring(
            secondSeparator + 1
        );


    if (
        session.length() == 0 ||
        sequenceText.length() == 0 ||
        payload.length() == 0
        ) {

        return "INVALID_FORMAT";
    }


    if (
        !isUnsignedNumber(
            sequenceText
        )
        ) {

        return "INVALID_FORMAT";
    }


    unsigned long sequence =
        strtoul(
            sequenceText.c_str(),
            nullptr,
            10
        );


    // Yeni session.

    if (
        session !=
        _lastReceivedSession
        ) {

        _lastReceivedSession =
            session;

        _lastReceivedSequence =
            sequence;

        return "VALID";
    }


    // Aynı session içinde aynı veya
    // eski sequence = REPLAY.

    if (
        sequence <=
        _lastReceivedSequence
        ) {

        return "REPLAY";
    }


    _lastReceivedSequence =
        sequence;


    return "VALID";
}


// ============================================================
// HMAC KEY
// ============================================================

void CageMCU::setHMACKey(
    const String& key
) {

    _hmacKey =
        key;
}


// ============================================================
// HMAC CREATE
// ============================================================

String CageMCU::hmacCreate(
    const String& message
) {

    if (
        _hmacKey.length() ==
        0
        ) {

        return "NO_HMAC_KEY";
    }


    SHA256 sha256;


    uint8_t result[32];


    sha256.resetHMAC(
        _hmacKey.c_str(),
        _hmacKey.length()
    );


    sha256.update(
        message.c_str(),
        message.length()
    );


    sha256.finalizeHMAC(
        _hmacKey.c_str(),
        _hmacKey.length(),
        result,
        sizeof(result)
    );


    const char hexChars[] =
        "0123456789ABCDEF";


    String output;

    output.reserve(64);


    for (
        int i = 0;
        i < 32;
        i++
        ) {

        output +=
            hexChars[
                (
                    result[i] >>
                    4
                    ) &
                    0x0F
            ];


        output +=
            hexChars[
                result[i] &
                    0x0F
            ];
    }


    return output;
}


// ============================================================
// HMAC CHECK
// ============================================================

bool CageMCU::hmacCheck(
    const String& message,
    const String& receivedHMAC
) {

    String calculated =
        hmacCreate(
            message
        );


    if (
        calculated ==
        "NO_HMAC_KEY"
        ) {

        return false;
    }


    if (
        receivedHMAC.length() !=
        calculated.length()
        ) {

        return false;
    }


    uint8_t difference =
        0;


    for (
        unsigned int i = 0;
        i < calculated.length();
        i++
        ) {

        difference |=
            (
                calculated[i] ^
                receivedHMAC[i]
                );
    }


    return (
        difference == 0
        );
}


// ============================================================
// LOCK AFTER
//
// Örnek:
//
// cage.lockAfter(5);
//
// 5 ardışık geçersiz girişten sonra kilit.
// ============================================================

void CageMCU::lockAfter(
    int limit
) {

    if (
        limit <= 0
        ) {

        _lockLimit = 0;

        return;
    }


    _lockLimit =
        limit;
}


// ============================================================
// REGISTER FAILURE
// ============================================================

void CageMCU::registerFailure() {

    // Kilit sistemi etkin değilse
    // hiçbir şey yapma.

    if (
        _lockLimit <= 0
        ) {

        return;
    }


    // Zaten kilitliyse
    // tekrar sayaç artırmaya gerek yok.

    if (_locked) {

        return;
    }


    _failedAttempts++;


    if (
        _failedAttempts >=
        _lockLimit
        ) {

        _locked = true;
    }
}


// ============================================================
// REGISTER SUCCESS
//
// Doğru giriş gelirse ardışık hata sayacı sıfırlanır.
// ============================================================

void CageMCU::registerSuccess() {

    if (_locked) {

        return;
    }


    _failedAttempts =
        0;
}


// ============================================================
// CHECK INPUT
//
// String / Integer / Float whitelist'lerinin
// tamamını kontrol eder.
//
// Herhangi biri TRUE -> geçerli.
//
// Üçü de FALSE -> hata.
//
// Kilitliyse direkt FALSE.
// ============================================================

bool CageMCU::checkInput(
    const String validStrings[],
    int stringCount,

    const int validIntegers[],
    int integerCount,

    const float validFloats[],
    int floatCount,

    const String& input
) {

    if (_locked) {

        return false;
    }


    bool stringValid = false;

    bool integerValid = false;

    bool floatValid = false;


    if (
        validStrings != nullptr &&
        stringCount > 0
        ) {

        stringValid =
            validStringCheck(
                validStrings,
                input,
                stringCount
            );
    }


    if (
        validIntegers != nullptr &&
        integerCount > 0
        ) {

        integerValid =
            validIntegerCheck(
                validIntegers,
                input,
                integerCount
            );
    }


    if (
        validFloats != nullptr &&
        floatCount > 0
        ) {

        floatValid =
            validFloatCheck(
                validFloats,
                input,
                floatCount
            );
    }


    bool valid =
        stringValid ||
        integerValid ||
        floatValid;


    if (valid) {

        registerSuccess();

        return true;
    }


    registerFailure();


    return false;
}


// ============================================================
// IS LOCKED
// ============================================================

bool CageMCU::isLocked() {

    return _locked;
}


// ============================================================
// FAILED ATTEMPTS
// ============================================================

int CageMCU::failedAttempts() {

    return _failedAttempts;
}
