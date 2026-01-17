/**
 * TLE5012 Incremental Interface (Final Calibrated Version)
 * Platform: ESP32
 * Sensor: Infineon TLE5012 (Configured as 12-bit / 4096 PPR)
 * Driver: ESP32 Hardware PCNT Unit via ESP32Encoder Library
 */

#include <Arduino.h>
#include <ESP32Encoder.h>

// --- Pin Definitions ---
const int PIN_ENC_A = 18; // TLE5012 IFA -> ESP32 GPIO 18
const int PIN_ENC_B = 19; // TLE5012 IFB -> ESP32 GPIO 19

// --- Calibrated Constants ---
// Berdasarkan analisis data empiris: 
// Sensor Resolution = 12-bit (4096 Pulse Per Rev)
// Decoding Mode     = Full Quadrature (x4)
// Total CPR         = 4096 * 4 = 16384
const float ENCODER_CPR = 16384.0; 

// --- Objects ---
ESP32Encoder encoder;

// --- Timing Variables ---
unsigned long previousMillis = 0;
long previousEncoderCount = 0;
const int INTERVAL_MS = 50; // Update rate 20Hz (Cukup cepat untuk monitoring)

void setup() {
    Serial.begin(115200);
    while(!Serial && millis() < 2000); // Safety delay

    Serial.println("--- TLE5012 Calibrated Interface Started ---");
    Serial.printf("Target CPR: %.0f counts/rev\n", ENCODER_CPR);

    // 1. Enable Internal Pull-up (Fallback safety)
    // Note: Pastikan external pull-up (4k7/10k) ada jika kabel panjang/noisy
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    // 2. Attach to Hardware PCNT Unit (Full Resolution x4)
    encoder.attachFullQuad(PIN_ENC_A, PIN_ENC_B);
    
    // 3. Reset Origin
    encoder.setCount(0);
    
    Serial.println("System Ready.");
}

void loop() {
    unsigned long currentMillis = millis();

    // Non-blocking Timer
    if (currentMillis - previousMillis >= INTERVAL_MS) {
        
        // --- A. Data Acquisition ---
        // Atomic read dari register hardware ESP32
        long currentCount = encoder.getCount();
        
        // --- B. Angle Calculation (0 - 360 degrees) ---
        // Rumus: (Count % CPR) / CPR * 360
        // fmod digunakan karena operator % tidak bekerja pada float
        float angleRaw = (float)currentCount / ENCODER_CPR * 360.0;
        
        // Normalisasi agar selalu 0 - 360 (termasuk saat berputar mundur/negatif)
        float angleNormalized = fmod(angleRaw, 360.0);
        if (angleNormalized < 0) angleNormalized += 360.0;

        // --- C. RPM Calculation ---
        // Rumus: (Delta Count / CPR) * (60000ms / Delta Time)
        long deltaCount = currentCount - previousEncoderCount;
        float rpm = ((float)deltaCount / ENCODER_CPR) * (60000.0 / INTERVAL_MS);

        // --- D. Output ---
        // Format: CNT, Angle, RPM
        Serial.printf("CNT: %ld \t Angle: %.2f° \t RPM: %.1f\n", 
                      currentCount, angleNormalized, rpm);

        // --- E. Update State ---
        previousMillis = currentMillis;
        previousEncoderCount = currentCount;
    }
}