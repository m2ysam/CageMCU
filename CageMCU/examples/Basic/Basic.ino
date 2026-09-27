#include <CageMCU.h>

CageMCU cage;


// Geçerli String komutları
String validCommands[] = {
    "OPEN",
    "CLOSE",
    "STATUS"
};


// Geçerli integer değerler
int validIntegers[] = {
    10,
    20,
    30
};


// Geçerli float değerler
float validFloats[] = {
    1.5,
    2.5,
    3.5
};


void setup() {

    Serial.begin(9600);

    cage.begin();

    // 5 ardışık hatalı girişten sonra kilitle
    cage.lockAfter(5);

    Serial.println("CageMCU Basic Example");
    Serial.println("Enter a command:");
}


void loop() {

    if (!Serial.available()) {
        return;
    }


    String input =
        Serial.readStringUntil('\n');

    input.trim();


    // Sistem kilitli mi?
    if (cage.isLocked()) {

        Serial.println("CAGE_LOCKED");

        return;
    }


    // String, Integer ve Float listelerini kontrol et
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


    // Geçersiz giriş
    if (!valid) {

        Serial.print("INVALID INPUT | FAILURES: ");

        Serial.println(
            cage.failedAttempts()
        );

        return;
    }


    // ------------------------------------------
    // STRING COMMANDS
    // ------------------------------------------

    if (input == "OPEN") {

        Serial.println("OPEN command accepted");
    }

    else if (input == "CLOSE") {

        Serial.println("CLOSE command accepted");
    }

    else if (input == "STATUS") {

        Serial.println("SYSTEM_OK");
    }

    else {

        // Integer veya Float whitelist'ten geçti.
        Serial.print("VALID VALUE: ");
        Serial.println(input);
    }
}