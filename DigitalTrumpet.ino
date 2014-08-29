#undef DEBUG 

const int PIEZO_PIN = A3;
const int TRUMPET_VALVE_1_PIN = 10;
const int TRUMPET_VALVE_2_PIN = 11;
const int TRUMPET_VALVE_3_PIN = 12;
const int DEBUG_PIN = A5;

const int sendDataInterval = 200;
const int maxPossibleAirVelocityReading = 50;

float airVelocityReadingDivisor = (float)maxPossibleAirVelocityReading / 4.0;

typedef struct {
    char *name;
    unsigned int midiValue;
    bool valve1Down;
    bool valve2Down;
    bool valve3Down;
    unsigned int quantizedAirVelocity;
} noteStruct;


unsigned long iterations = 0;
unsigned long lastSendDataTimestamp;
unsigned long timestamp;
unsigned int maxAirVelocityReading = 0;

const unsigned int noteCount = 13;

noteStruct notes[noteCount] = { 
    { "C",  60, 0, 0, 0, 1 },
    { "C#", 61, 1, 1, 1, 1 },
    { "D",  62, 1, 0, 1, 1 },
    { "D#", 63, 0, 1, 1, 1 },
    { "E",  64, 1, 1, 0, 1 },
    { "F",  65, 1, 0, 0, 1 },
    { "F#", 66, 0, 2, 0, 1 },
    { "G",  67, 0, 0, 0, 2 },
    { "G#", 68, 0, 1, 1, 2 },
    { "A",  69, 1, 1, 0, 2 },
    { "A#", 70, 1, 0, 0, 2 },
    { "B",  71, 0, 2, 0, 2 },
    { "C",  72, 0, 0, 0, 3 },
};

bool trumpetValve1Down = false;
bool trumpetValve2Down = false;
bool trumpetValve3Down = false;
unsigned int quantizedAirVelocity = 0;
bool debug = 0;
noteStruct *lastNote;
noteStruct note;

void setup() {
    Serial.begin ( 115200 );
    pinMode ( PIEZO_PIN, INPUT );
    pinMode ( DEBUG_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_1_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_2_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_3_PIN, INPUT_PULLUP );
    lastSendDataTimestamp = 0;
}

void loop() {
    readValves();
    readAirVelocity();
    readDebug();
    iterations++;

    timestamp = millis();
    if ( timestamp >= ( lastSendDataTimestamp + sendDataInterval ) ) {
        quantizedAirVelocity = maxAirVelocityReading / airVelocityReadingDivisor;
        lastSendDataTimestamp = timestamp;
        findNote();
        if ( debug ) {
            printDebug();
        } else {
            if ( lastNote ) {
                Serial.write ( lastNote->midiValue );
            }
        }
        iterations = 0;
        maxAirVelocityReading = 0;
    }
}

void readValves() {
    trumpetValve1Down = ! digitalRead ( TRUMPET_VALVE_1_PIN );
    trumpetValve2Down = ! digitalRead ( TRUMPET_VALVE_2_PIN );
    trumpetValve3Down = ! digitalRead ( TRUMPET_VALVE_3_PIN );
}

void readAirVelocity() {
    unsigned int airVelocityReading = analogRead ( PIEZO_PIN );
    if ( airVelocityReading > maxAirVelocityReading ) {
        maxAirVelocityReading = airVelocityReading;
    }
}

void readDebug() {
    debug = ! digitalRead ( DEBUG_PIN );
}

void findNote() {
    for ( int i = 0; i < noteCount; i++ ) {
        note = notes[i];
        if ( ( trumpetValve1Down == note.valve1Down ) && ( trumpetValve2Down == note.valve2Down ) && ( trumpetValve3Down == note.valve3Down ) && ( quantizedAirVelocity == note.quantizedAirVelocity ) ) {
            lastNote = &note;
            return;
        }
    }
    lastNote = NULL;
}

void printDebug() {
    Serial.print ( "air velocity: " );
    Serial.print ( maxAirVelocityReading, DEC );
    Serial.print ( "/" );
    Serial.print ( quantizedAirVelocity, DEC );
    Serial.print ( ", buttons: " );
    Serial.print ( trumpetValve1Down );
    Serial.print ( " " );
    Serial.print ( trumpetValve2Down );
    Serial.print ( " " );
    Serial.print ( trumpetValve3Down );
    Serial.print ( ", Note: " );
    if ( lastNote ) {
        Serial.print ( lastNote->name );
        Serial.print ( " midi value: " );
        Serial.print ( lastNote->midiValue );
    } else {
        Serial.print ( "none" );
    }
    Serial.print ( ", iterations: " );
    Serial.println ( iterations, DEC );
}
