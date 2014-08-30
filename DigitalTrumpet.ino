#undef DEBUG 

const int PIEZO_PIN = A3;
const int TRUMPET_VALVE_1_PIN = 10;
const int TRUMPET_VALVE_2_PIN = 11;
const int TRUMPET_VALVE_3_PIN = 12;
const int DEBUG_PIN = A5;

const int sendDataInterval = 20;

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
bool playingState = 0;

const unsigned int noteCount = 7;

/*
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
*/
noteStruct notes[noteCount] = { 
    //{ "C",  60, 0, 0, 0, 1 },
    { "E",  64, 1, 1, 1, 1 },
    { "F",  65, 1, 0, 1, 1 },
    { "G",  67, 0, 1, 1, 1 },
    { "C",  72, 1, 1, 0, 1 },
    { "E",  76, 1, 0, 0, 1 },
    { "F",  77, 0, 1 ,0, 1 },
    { "G",  79, 0, 0 ,1, 1 }

    //{ "G",  79, 0, 1 ,0, 1 },
};

bool trumpetValve1Down = false;
bool trumpetValve2Down = false;
bool trumpetValve3Down = false;

bool lastTrumpetValve1Down = false;
bool lastTrumpetValve2Down = false;
bool lastTrumpetValve3Down = false;

unsigned int quantizedAirVelocity = 0;
unsigned int lastQuantizedAirVelocity = 0;
bool debug = 0;
noteStruct *lastNote;
noteStruct note;
unsigned long last3On = 0;

void setup() {
    Serial.begin ( 115200 );
    pinMode ( PIEZO_PIN, INPUT );
    pinMode ( DEBUG_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_1_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_2_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_3_PIN, INPUT_PULLUP );
    lastSendDataTimestamp = 0;
}

unsigned int quantizeAirVelocity ( unsigned int airVelocityReading ) {
    unsigned int quantizedAirVelocity;

    if ( airVelocityReading < 10 ) {
        lastQuantizedAirVelocity = 0;
        quantizedAirVelocity = 0;
    }  else if ( airVelocityReading <= 100 ) {
        if ( lastQuantizedAirVelocity <= 1 ) {
            quantizedAirVelocity = 1;
            lastQuantizedAirVelocity = 1;
        } else {
            return 0;
        }
    } else {
        if ( lastQuantizedAirVelocity <= 2 ) {
            quantizedAirVelocity = 2;
            lastQuantizedAirVelocity = 2;
        } else {
            return 0;
        }
    }
    return quantizedAirVelocity;
}

void loop() {
    readValves();
    readAirVelocity();
    readDebug();
    iterations++;

    timestamp = millis();
    if ( timestamp >= ( lastSendDataTimestamp + sendDataInterval ) ) {
        quantizedAirVelocity = quantizeAirVelocity ( maxAirVelocityReading );
        lastSendDataTimestamp = timestamp;
        findNote();
        if ( debug ) {
            printDebug();
        } else {
            Serial.write ( 255 );
            if ( lastNote ) {
                Serial.write ( lastNote->midiValue );
            } else {
                Serial.write ( 0 );
            }
            Serial.write ( maxAirVelocityReading );
            Serial.write ( quantizedAirVelocity );
        }
        iterations = 0;
        maxAirVelocityReading = 0;
    }
}

void readValves() {
    trumpetValve1Down = ! digitalRead ( TRUMPET_VALVE_1_PIN );
    trumpetValve2Down = ! digitalRead ( TRUMPET_VALVE_2_PIN );
    trumpetValve3Down = ! digitalRead ( TRUMPET_VALVE_3_PIN );
    /*
    if ( trumpetValve3Down ) {
        unsigned long now = millis();
        if ( now - last3On > 250 ) {
            playingState = ! playingState;
            last3On = now;
        } else {
            last3On = 0;
        }
    }
    */
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
        if ( ( trumpetValve1Down == note.valve1Down ) && ( trumpetValve2Down == note.valve2Down ) && ( trumpetValve3Down == note.valve3Down ) ) {
            lastNote = &note;
            return;
        }
    }
    lastNote = NULL;
}

void printDebug() {
    if ( maxAirVelocityReading == 0 ) {
        return;
    }
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
