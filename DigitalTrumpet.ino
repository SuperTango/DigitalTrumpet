#undef DEBUG 

const int MIC_PIN = A0;
const int TRUMPET_VALVE_1_PIN = 10;
const int TRUMPET_VALVE_2_PIN = 11;
const int TRUMPET_VALVE_3_PIN = 12;
const int DEBUG_PIN = A5;

const int sendDataInterval = 200;
const int zeroMicValue = 338;
const int maxMicValue = 348;
const float micScalingFactor = maxMicValue / 4;


typedef struct {
    char *name;
    unsigned int midiValue;
    bool valve1Down;
    bool valve2Down;
    bool valve3Down;
    unsigned int micValue;
} noteStruct;


unsigned long totalMicValue = 0;
unsigned long iterations = 0;
unsigned long lastSendDataTimestamp;
unsigned long timestamp;
unsigned int quantizedMicValue = 0;

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
bool debug = 0;

void setup() {
    Serial.begin ( 115200 );
    pinMode ( MIC_PIN, INPUT );
    pinMode ( DEBUG_PIN, INPUT );
    pinMode ( TRUMPET_VALVE_1_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_2_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_3_PIN, INPUT_PULLUP );
    lastSendDataTimestamp = 0;
}

void loop() {
    readValves();
    readMic();
    readDebug();

    timestamp = millis();
    if ( timestamp >= ( lastSendDataTimestamp + sendDataInterval ) ) {
        quantizedMicValue = ( totalMicValue / iterations ) / micScalingFactor;
        lastSendDataTimestamp = timestamp;
        sendAllData();
        iterations = 0;
        totalMicValue = 0;
        convertNote();
    }
}

void readValves() {
    trumpetValve1Down = ! digitalRead ( TRUMPET_VALVE_1_PIN );
    trumpetValve2Down = ! digitalRead ( TRUMPET_VALVE_2_PIN );
    trumpetValve3Down = ! digitalRead ( TRUMPET_VALVE_3_PIN );
}

void readMic() {
    int micValue = analogRead ( MIC_PIN );
    if ( micValue < zeroMicValue ) {
        micValue = zeroMicValue + ( zeroMicValue - micValue );
    }
    micValue -= zeroMicValue;
    totalMicValue += micValue;
    iterations++;
}

void readDebug() {
    debug = digitalRead ( DEBUG_PIN );
}

unsigned int convertNote() {
    noteStruct note;
    for ( int i = 0; i < noteCount; i++ ) {
        note = notes[i];
        if ( ( trumpetValve1Down == note.valve1Down ) && ( trumpetValve2Down == note.valve2Down ) && ( trumpetValve3Down == note.valve3Down ) && ( quantizedMicValue == note.micValue ) ) {
            Serial.print ( "Note: " );
            Serial.println ( note.name );
            return note.midiValue;
        }
    }
}




void sendAllData() {
    if ( debug ) {
        Serial.print ( totalMicValue, DEC );
        Serial.print ( " " );
        Serial.println ( iterations, DEC );
    } else {
        Serial.write ( 255 );
    }
    sendByte ( quantizedMicValue );
    sendByte ( trumpetValve1Down );
    sendByte ( trumpetValve2Down );
    sendByte ( trumpetValve3Down );
}

void sendByte ( int value ) {
    if ( debug ) {
        Serial.print ( value, DEC );
        Serial.print ( " " );
    } else {
        Serial.write ( value );
    }
}
