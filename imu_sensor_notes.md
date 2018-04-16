# 6-Axis sensor information

## Packet data information

If 6-axis sensor is enabled, the IMU data in an 0x30, 0x31, 0x32 and 0x33 input report is packaged like this (assuming the packet ID is located at byte 0):

| Byte       | Remarks                                                       |
|:----------:| ------------------------------------------------------------- |
|   13-14    | accel_x (Int16LE)                                             |
|   15-16    | accel_y (Int16LE)                                             |
|   17-18    | accel_z (Int16LE)                                             |
|   19-20    | gyro_1 (Int16LE)                                              |
|   21-22    | gyro_2 (Int16LE)                                              |
|   23-24    | gyro_3 (Int16LE)                                              |
|   25-48    | The data is repeated 2 more times. Each with 5ms Δt sampling. |

The 6-Axis data is repeated 3 times. On Joy-con with a 15ms packet push, this is translated to 5ms difference sampling. E.g. 1st sample 0ms, 2nd 5ms, 3rd 10ms. Using all 3 samples let you have a 5ms precision instead of 15ms.

## Axes definition

The accelerator axes are defined by the output in packet. Because of the placement of the IMU chip, the 2 Joy-Con have an axis reversed each. 

The following images show exactly the 6 axes:

![alt text](http://ctcaer.com/wii/switch/joycon_acc-gyro_left2.png)![alt text](http://ctcaer.com/wii/switch/joycon_acc-gyro_right2.png)

## Convert to basic useful data using raw values

### Accelerometer - Acceleration (in G)

The following equation should scale an int16 IMU value into an acceleration vector component (measured in Gs):

`acc_vector_component = acc_raw_component * G_RANGE / SENSOR_RES / 1000` [Value in G]

where `G_RANGE` is the sensitivity range setting of the accelerometer, as explained [here](http://ozzmaker.com/accelerometer-to-g/).

The Joy-Con are ranged to ±8000 MilliGs (G_RANGE = 16000 MilliGs), the sensitivity calibration is always ±8192 MilliGs and the SENSOR_RES is 16bit, so the above equation can be simplified to:

##### Normal ±8000 mG:

`acc_vector_component = acc_raw_component * 0.000244f`. (=16000/65535/1000)

### Gyroscope - Rotation (in Degrees/s - dps)

For Gyro the equation to convert the values into angular velocity (measured in degrees per second):

`gyro_vector_component = gyro_raw_component * G_GAIN / SENSOR_RES` [Value in dps]

where G_GAIN is the degrees per second sensitivity range.

The Joy-Con, based on their configuration, have a gyro sensitivity of ±2000dps (G_GAIN = 4000dps), so the above equation can be simplified to:

##### Normal ±2000 dps or 61 mdps/digit:

`gyro_vector_component = gyro_raw_component * 0.06103f` (=4000/65535)

To express the full-scale rates in both directions without saturating you can add a 15% (STMicroelectronics LSM6DS3 datasheet) and it's the recommended way to do it.

The new conversion will be:  

##### LSM6DS3 ±2000 dps or 70 mdps/digit:

`gyro_vector_component = gyro_raw_component * 0.070f` (=4588/65535)

### Gyroscope - Rotation (in revolutions/s)

Internally Switch uses revolutions per second instead of degrees. That's why the gyro calibration is always 13371.

Normally to get revolutions/s you need to follow the below equation:

`gyro_revolutions = gyro_raw_component * G_GAIN / SENSOR_RES / 360.0f` [Value in revolutions/s]

So the equation for the above 3 sensitivities will become:

##### Normal:

`gyro_revolutions = gyro_raw_component * 0.0001694f` (=4813/65535/360)

##### LSM6DS3:

`gyro_revolutions = gyro_raw_component * 0.0001944f` (=4813/65535/360)

The [SparkFun library for the LSM6DS3](https://github.com/sparkfun/SparkFun_LSM6DS3_Arduino_Library) will likely be a valuable resource for future IMU hacking.

## Convert to basic useful data using SPI Calibration

### Accelerometer (Calibrated) - Acceleration (in G)

The SPI `accelerometer calibration`, includes 3 important values for each axis:

The `cal_acc_origin` is the origin scale value when the Joy-Con is held completely horizontally. It's not an origin position but how the raw values are affected by noise and by the unleveled body of the controller (triggers protruding).

The `cal_acc_horizontal_offset` is the offest (difference) that the Joy-Con/Pro-con has when it's on a flat surface than being completely horizontal. (The Trigger bumps change its position and you can use this offset to calibrate the position when it is on a flat surface)

The `cal_acc_horizontal_offset` is always the same. Advise [here](spi_flash_dump_notes.md#6-axis-and-stick-device-parameters) for the values each model (JC Left, JC Right, Pro).

The `cal_acc_coeff` is used for the equations and it's always `x4000` (`16384`).

Based on these we can conclude on the following equation to find the final coefficient:

##### Origin posititon is horizontal and stick is upside:

`acc_coeff = (float)(1.0 / (float)(16384 - uint16_to_int16(cal_acc_origin))) * 4.0f;`

Then we use the coefficient to convert the value into G (SI: 9.8m/s²):

`acc_vector_component = acc_raw_component * acc_coeff`

### Completely level* Accelerometer when horizontal on flat surface:

*This will level the accelerometer to 0 values when resting on flat surface. It's useful when we perform a manual calibration.

We subtract `cal_acc_horizontal_offset` and then we use the coefficient to convert the value into G (SI: 9.8m/s²):

`acc_vector_component = (acc_raw_component - uint16_to_int16(cal_acc_horizontal_offset)) * acc_coeff`

### Gyroscope (Calibrated) - Rotation (in Degrees/s - dps)

The SPI `gyro calibration`, includes 2 important values for each axis:

The `cal_gyro_offset` is the offset when the Joy-Con is stable (held still).

The `cal_gyro_coeff` is the coeff that is used in the equation and it's always `x343B` (`13371`).

Based on these we can conclude on the final equation:

##### Default (saturation free) LSM6DS3 ±2000 dps : 70 mdps/digit:

`gyro_cal_coeff = (float)(936.0 / (float)(13371 - uint16_to_int16(cal_gyro_offset)));`

##### Accurate ±2000 dps : 61 mdps/digit:

`gyro_cal_coeff = (float)(816.0 / (float)(13371 - uint16_to_int16(cal_gyro_offset)));`

Then we use the coefficient to convert the value into degrees°/s (SI: 0.01745 rad/s):

`acc_vector_component = (acc_raw_component - uint16_to_int16(cal_gyro_offset)) * acc_coeff`

Here, unlike acceleration, the origin position is indeed the still position and not a scaling value. So it must be also used in the unit calculation, in addition to coeffition.

### Gyroscope - Rotation (in revolutions/s)

The equation will become:

`acc_vector_component = (acc_raw_component - uint16_to_int16(cal_gyro_offset)) * acc_coeff * 0.0027777778`

## Noise level range for each sensitivity configuration

This is useful to set a noise cancellation deadzone for acc and gyro.

Based on official values. Also it's a range value (not a ± value).

Accelerometer:

| Sensitivity   | Noise level range |
|:-------------:|:-----------------:|
| ±2G           | 328 * 2.5 = 2050  |
| ±4G           | 164 * 2.5 = 410   |
| ±8G (default) | 82 * 2.5 = 205    |
| ±16G          | No official value |

Gyroscope:

| Sensitivity        | Noise level range |
|:------------------:|:-----------------:|
| ±250dps            | 236 * 2.5 = 590   |
| ±500dps            | 118 * 2.5 = 295   |
| ±1000dps           | 59 * 2.5 = 147    |
| ±2000dps (default) | 30 * 2.5 = 75     |
