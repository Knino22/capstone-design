#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MPU6050.h>

// Replace with your network credentials
const char* ssid = "SILK_8AFC92";
const char* password = "35712357";

// Define new GPIO pins for the motors
#define MOTOR1_PIN 4   // GPIO 4 (D2)
#define MOTOR2_PIN 5   // GPIO 5 (D1)
#define MOTOR3_PIN 16  // GPIO 16 (D0)

// Create a web server on port 80
ESP8266WebServer server(80);

// Create an instance of the MPU6050 class
MPU6050 gyro;

// Track motor states
bool motor1State = false;
bool motor2State = false;
bool motor3State = false;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(10);

    Serial.println("Initializing...");

    // Initialize GPIO pins as outputs for the motors
    pinMode(MOTOR1_PIN, OUTPUT);
    pinMode(MOTOR2_PIN, OUTPUT);
    pinMode(MOTOR3_PIN, OUTPUT);

    // Set all motors to off initially
    digitalWrite(MOTOR1_PIN, LOW);
    digitalWrite(MOTOR2_PIN, LOW);
    digitalWrite(MOTOR3_PIN, LOW);

    // Print initial states for debugging
    Serial.println("Initial Motor States:");
    Serial.print("Motor 1: ");
    Serial.println(digitalRead(MOTOR1_PIN) ? "ON" : "OFF");
    Serial.print("Motor 2: ");
    Serial.println(digitalRead(MOTOR2_PIN) ? "ON" : "OFF");
    Serial.print("Motor 3: ");
    Serial.println(digitalRead(MOTOR3_PIN) ? "ON" : "OFF");

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    // Wait until the connection is established
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize the MPU6050
    Wire.begin();
    gyro.initialize();
    if (gyro.testConnection()) {
        Serial.println("Gyro connected successfully.");
    } else {
        Serial.println("Failed to connect to gyro.");
    }

    // Set up web server routes
    server.on("/", handleRoot);
    server.on("/toggleMotor1", handleToggleMotor1);
    server.on("/toggleMotor2", handleToggleMotor2);
    server.on("/toggleMotor3", handleToggleMotor3);
    server.on("/gyro", handleGyroReadings);

    // Start the web server
    server.begin();
    Serial.println("HTTP server started on IP: " + WiFi.localIP().toString());
}

void loop() {
    server.handleClient();
}

// Handler to serve the main web page
void handleRoot() {
    String html = "<html>\
                    <head>\
                      <title>Motor and Gyro Control</title>\
                      <style>\
                        body { font-family: Arial, sans-serif; text-align: center; }\
                        button { padding: 15px 25px; font-size: 16px; margin: 10px; cursor: pointer; }\
                        .active { background-color: green; color: white; }\
                      </style>\
                      <script>\
                        function toggleMotor(motor) {\
                          var xhr = new XMLHttpRequest();\
                          xhr.open('GET', '/toggle' + motor, true);\
                          xhr.onload = function () {\
                            if (xhr.status == 200) {\
                              var button = document.getElementById('button' + motor);\
                              if (button.classList.contains('active')) {\
                                button.classList.remove('active');\
                                button.innerText = 'Turn ' + motor + ' ON';\
                              } else {\
                                button.classList.add('active');\
                                button.innerText = 'Turn ' + motor + ' OFF';\
                              }\
                            } else {\
                              console.error('Failed to toggle motor ' + motor);\
                            }\
                          };\
                          xhr.send();\
                        }\
                        function fetchGyroData() {\
                          var xhr = new XMLHttpRequest();\
                          xhr.open('GET', '/gyro', true);\
                          xhr.onload = function () {\
                            if (xhr.status == 200) {\
                              var data = JSON.parse(xhr.responseText);\
                              document.getElementById('gyroX').innerText = 'Gyro X: ' + data.gyroX;\
                              document.getElementById('gyroY').innerText = 'Gyro Y: ' + data.gyroY;\
                              document.getElementById('gyroZ').innerText = 'Gyro Z: ' + data.gyroZ;\
                            } else {\
                              console.error('Failed to fetch gyro data.');\
                            }\
                          };\
                          xhr.send();\
                        }\
                        setInterval(fetchGyroData, 1000); // Fetch data every second\
                      </script>\
                    </head>\
                    <body>\
                      <h1>Control Vibration Motors and View Gyroscope Readings</h1>\
                      <button id=\"buttonMotor1\" onclick=\"toggleMotor('Motor1')\">Turn Motor1 ON</button>\
                      <br>\
                      <button id=\"buttonMotor2\" onclick=\"toggleMotor('Motor2')\">Turn Motor2 ON</button>\
                      <br>\
                      <button id=\"buttonMotor3\" onclick=\"toggleMotor('Motor3')\">Turn Motor3 ON</button>\
                      <h2>Gyroscope Readings</h2>\
                      <p id=\"gyroX\">Gyro X: -</p>\
                      <p id=\"gyroY\">Gyro Y: -</p>\
                      <p id=\"gyroZ\">Gyro Z: -</p>\
                    </body>\
                   </html>";

    server.send(200, "text/html", html);
}

// Handler to provide gyro readings in JSON format
void handleGyroReadings() {
    int16_t gyroX = 0, gyroY = 0, gyroZ = 0;

    if (gyro.testConnection()) {
        gyroX = gyro.getRotationX();
        gyroY = gyro.getRotationY();
        gyroZ = gyro.getRotationZ();
    } else {
        Serial.println("Gyro test connection failed in handleGyroReadings.");
    }

    String json = "{\"gyroX\":" + String(gyroX) + ",\"gyroY\":" + String(gyroY) + ",\"gyroZ\":" + String(gyroZ) + "}";
    server.send(200, "application/json", json);
}

// Toggle handlers for each motor
void handleToggleMotor1() {
    motor1State = !motor1State;
    digitalWrite(MOTOR1_PIN, motor1State ? HIGH : LOW);
    Serial.print("Motor 1 is now ");
    Serial.println(motor1State ? "ON" : "OFF");
    server.send(200, "text/plain", motor1State ? "ON" : "OFF");
}

void handleToggleMotor2() {
    motor2State = !motor2State;
    digitalWrite(MOTOR2_PIN, motor2State ? HIGH : LOW);
    Serial.print("Motor 2 is now ");
    Serial.println(motor2State ? "ON" : "OFF");
    server.send(200, "text/plain", motor2State ? "ON" : "OFF");
}

void handleToggleMotor3() {
    motor3State = !motor3State;
    digitalWrite(MOTOR3_PIN, motor3State ? HIGH : LOW);
    Serial.print("Motor 3 is now ");
    Serial.println(motor3State ? "ON" : "OFF");
    server.send(200, "text/plain", motor3State ? "ON" : "OFF");
}
