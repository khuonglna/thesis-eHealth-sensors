#include <Adafruit_MLX90640.h>

Adafruit_MLX90640 mlx;
float frame[32 * 24]; // buffer for full frame of temperatures
bool control = false;
// bool control = true;

//SDA 21
//SCL 22
//TX 17
//RX 16

#include "HX711.h"
const int LOADCELL_DOUT_PIN = 25;
const int LOADCELL_SCK_PIN = 26;
#include "TFMini.h"
#include <HardwareSerial.h>

// Setup software serial port
HardwareSerial mySerial2(2);

HX711 scale;
TFMini tfmini;
// uncomment *one* of the below
// #define PRINT_TEMPERATURES
// #define PRINT_ASCIIART.ino

float maxT;
float calibration_factor = 12025; // this calibration factor is adjusted according to my load cell
float units;
float kg;

void setup()
{
    maxT = 0;
    while (!Serial)
        delay(10);
    Serial.begin(115200);
    // delay(100);
    mySerial2.begin(TFMINI_BAUDRATE);
    tfmini.begin(&mySerial2);
    // Serial.println("Adafruit MLX90640 Simple Test");
    if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire))
    {
        Serial.println("MLX90640 not found!");
        while (1)
            delay(10);
    }
    Serial.println("Found Adafruit MLX90640");

    // Serial.print("Serial number: ");
    // Serial.print(mlx.serialNumber[0], HEX);
    // Serial.print(mlx.serialNumber[1], HEX);
    // Serial.println(mlx.serialNumber[2], HEX);

    //mlx.setMode(MLX90640_INTERLEAVED);
    mlx.setMode(MLX90640_CHESS);
    // Serial.print("Current mode: ");
    // if (mlx.getMode() == MLX90640_CHESS) {
    //   Serial.println("Chess");
    // } else {
    //   Serial.println("Interleave");
    // }

    mlx.setResolution(MLX90640_ADC_18BIT);
    // Serial.print("Current resolution: ");
    // mlx90640_resolution_t res = mlx.getResolution();
    // switch (res) {
    //   case MLX90640_ADC_16BIT: Serial.println("16 bit"); break;
    //   case MLX90640_ADC_17BIT: Serial.println("17 bit"); break;
    //   case MLX90640_ADC_18BIT: Serial.println("18 bit"); break;
    //   case MLX90640_ADC_19BIT: Serial.println("19 bit"); break;
    // }

    mlx.setRefreshRate(MLX90640_2_HZ);
    //   Serial.print("Current frame rate: ");
    //   mlx90640_refreshrate_t rate = mlx.getRefreshRate();
    //   switch (rate) {
    //     case MLX90640_0_5_HZ: Serial.println("0.5 Hz"); break;
    //     case MLX90640_1_HZ: Serial.println("1 Hz"); break;
    //     case MLX90640_2_HZ: Serial.println("2 Hz"); break;
    //     case MLX90640_4_HZ: Serial.println("4 Hz"); break;
    //     case MLX90640_8_HZ: Serial.println("8 Hz"); break;
    //     case MLX90640_16_HZ: Serial.println("16 Hz"); break;
    //     case MLX90640_32_HZ: Serial.println("32 Hz"); break;
    //     case MLX90640_64_HZ: Serial.println("64 Hz"); break;
    //   }
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale();
    scale.tare(); //Reset the scale to 0

    long zero_factor = scale.read_average();
}

void loop()
{
    scale.set_scale(calibration_factor);
    units = scale.get_units();
    if (units < 0)
    {
        units = 0.00;
    }
    kg = units * 0.453592;
    // Serial.println(kg);
    // if (Serial.available())
    // {
    //     char temp = Serial.read();
    //     if (temp == '+' || temp == 'a')
    //         calibration_factor += 1;
    //     else if (temp == '-' || temp == 'z')
    //         calibration_factor -= 1;
    // }
    if (Serial.available() > 0)
    {
        int read = Serial.read();
        if (read == 49)
        {
            control = true;
        }
        else if (read == 48)
        {
            control = false;
            ESP.restart();
        }
    }
    if (control)
    {
        delay(25);
        if (mlx.getFrame(frame) != 0)
        {
            Serial.println("Failed");
            return;
        }

        // Serial.println();
        // Serial.println();
        int data[7] = {0};
        data[1] = (int)83;
        for (uint8_t hT = 0; hT < 24; hT++)
        {
            uint16_t dist = tfmini.getDistance();
            uint16_t strength = tfmini.getRecentSignalStrength();
            for (uint8_t wT = 0; wT < 32; wT++)
            {
                float te = frame[hT * 32 + wT];
                if (te > 35)
                {
                    maxT = max(te, maxT);
                    // Serial.print(dist);
                    // Serial.print(" cm     ");
                    // Serial.println(maxT);
                    data[2] = dist;
                    int tempT = (int)(maxT * 100);
                    int tempW = (int)(kg * 100);
                    data[3] = (int)maxT;
                    data[4] = tempT % 100;
                    data[5] = (int)kg;
                    data[6] = tempW % 100;
                }
                // #ifdef PRINT_TEMPERATURES
                //       Serial.print(t, 1);
                //       Serial.print(", ");
                // #endif
                // #ifdef PRINT_ASCIIART
                //       char c = '&';
                //       if (t < 20) c = ' ';
                //       else if (t < 23) c = '.';
                //       else if (t < 25) c = '-';
                //       else if (t < 27) c = '*';
                //       else if (t < 29) c = '+';
                //       else if (t < 31) c = 'x';
                //       else if (t < 33) c = '%';
                //       else if (t < 35) c = '#';
                //       else if (t < 37) c = 'X';
                //       Serial.print(c);
                // #endif
            }
            // Serial.println();
        }
        for (int i = 0; i < 7; i++)
        {
            Serial.print(data[i]);
            if (i != 6 && i != 0)
            {
                Serial.print('-');
            }
        }
        Serial.println();
    }
    // else {
    //   ESP.restart();
    // }
}

// int incomingByte = 0; // for incoming serial data

// void setup() {
//   Serial.begin(115200); // opens serial port, sets data rate to 9600 bps
// }

// void loop() {
//   // send data only when you receive data:
//   if (Serial.available() > 0) {
//     // read the incoming byte:
//     incomingByte = Serial.read();

//     // say what you got:
//     Serial.print("I received: ");
//     Serial.println(incomingByte, DEC);
//     Serial.println(incomingByte);
//   }
// }