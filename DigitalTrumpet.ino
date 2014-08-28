#undef DEBUG 

const int MIC_PIN = A0;
const int TRUMPET_VALVE_1_PIN = 10;
const int TRUMPET_VALVE_2_PIN = 11;
const int TRUMPET_VALVE_3_PIN = 12;

const int sendDataInterval = 50;
const int zeroMicValue = 338;
const float micScalingFactor = 3.48; // max value / 100 ( == 438 / 100 )

unsigned long totalMicValue = 0;
unsigned long iterations = 0;
unsigned long lastSendDataTimestamp;
unsigned long timestamp;

bool trumpetValve1Down = false;
bool trumpetValve2Down = false;
bool trumpetValve3Down = false;

void setup() {
    Serial.begin ( 115200 );
    pinMode ( MIC_PIN, INPUT );
    pinMode ( TRUMPET_VALVE_1_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_2_PIN, INPUT_PULLUP );
    pinMode ( TRUMPET_VALVE_3_PIN, INPUT_PULLUP );
    lastSendDataTimestamp = 0;
}

void loop() {
    readValves();
    processMic();

    timestamp = millis();
    if ( timestamp >= ( lastSendDataTimestamp + sendDataInterval ) ) {
        lastSendDataTimestamp = timestamp;
        sendAllData();
        iterations = 0;
        totalMicValue = 0;
    }
}

void sendAllData() {
#ifdef DEBUG
    Serial.print ( totalMicValue, DEC );
    Serial.print ( " " );
    Serial.println ( iterations, DEC );
#else
    Serial.write ( 255 );
#endif
    sendByte ( ( totalMicValue / iterations ) / micScalingFactor );
    sendByte ( trumpetValve1Down );
    sendByte ( trumpetValve2Down );
    sendByte ( trumpetValve3Down );
}

void sendByte ( int value ) {
#ifdef DEBUG 
    Serial.print ( value, DEC );
    Serial.print ( " " );
#else
    Serial.write ( value );
#endif
}

void readValves() {
    trumpetValve1Down = ! digitalRead ( TRUMPET_VALVE_1_PIN );
    trumpetValve2Down = ! digitalRead ( TRUMPET_VALVE_2_PIN );
    trumpetValve3Down = ! digitalRead ( TRUMPET_VALVE_3_PIN );
}

void processMic() {
    int micValue = analogRead ( MIC_PIN );
    if ( micValue < zeroMicValue ) {
        micValue = zeroMicValue + ( zeroMicValue - micValue );
    }
    micValue -= zeroMicValue;
    //Serial.print ( micValue, DEC );
    //Serial.print ( " " );
    totalMicValue += micValue;
    iterations++;
}
