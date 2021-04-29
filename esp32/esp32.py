import serial
import time
import statistics
import json

INTERVAL = 5
HEIGHT_OFFSET = 1
TEMP_OFFSET = 2
TEMP_FLOAT_VAL_OFFSET = 3

# data = [83, H, T, T.]


def getSensorData():
    conn = serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=1)
    conn.write(b"1")  # write a string
    conn.close()
    start = time.time()
    data = []
    # for j in range(5):
    while time.time() - start < INTERVAL:
        with serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=1) as ser:
            try:
                d = ser.read(64)
                print(d)
                decodeData = d.decode("utf-8")
                # print(decodeData)

                data += decodeData.split("\r\n")
                # print(j)
                ser.close()
            except:
                print("CAN'T GET DEVICE")
                # print(j)
                ser.close()
            # break;
    conn = serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=1)
    conn.write(b"0")  # write a string
    conn.close()
    print(data)
    decData = []
    tempArr = []
    hArr = []
    for x in data:
        temp = []
        print(x)
        if x != "":
            temp = x.split("-")
            print(temp)
            # decData.append(x)
            if (
                (temp[0] == "83" or temp[0] == "083")
                and (float(temp[HEIGHT_OFFSET]) != 0 and float(temp[TEMP_OFFSET]) != 0)
                and (len(temp) == 6)
            ):
                hArr.append(float(temp[HEIGHT_OFFSET]))
                tempArr.append(
                    float(temp[TEMP_OFFSET]) + float(temp[TEMP_FLOAT_VAL_OFFSET]) / 100
                )

    # print(decData)
    result = {"t": max(tempArr), "h": statistics.mean(hArr)}
    return result


print(getSensorData())

# import serial
# open serial port
# print(ser.name)         # check which port was really used
# ser.write(b'0')  # write a string
# ser.close()
