# Introduction
This project is from Microcessor Design class at the University of Seoul. Developed with JKit-128-1.

This project is mood light with convenient features such as:
1. Light detection: Light is automatically on at night.
2. Double clap detection: User can toggle the light with a double clap.
3. Sound threshold adjustment: User can adjust the threshold sound value. Every sound bigger than the threshold is checked whether it is a doble clap.

# Demo

https://github.com/user-attachments/assets/2c43986f-2cbc-41b9-9489-d0dff180075a

# Configuration & Build
1. Setup hardware
    
    - Board: JKit-128-1
    - Paripararls

        | Component Name       | Prefix | Manufacturer |
        |---------------------|--------|--------------|
        | Rotary Encoder       | P1     | SunFounder   |
        | RGB LED Module       | P2     | SMG          |
        | Sound Sensor Module (TS0223) | P3     | SMG   



    ![](assets/circuit.png)

2. Install AVR-GCC

    We used `avr-gcc-7.3.0-atmel3.6.1-arduino7-i686-w64-mingw32` compiler which can be downloaded from the link below.

    http://downloads.arduino.cc/tools/avr-gcc-7.3.0-atmel3.6.1-arduino7-i686-w64-mingw32.zip

3. Install make

    We used `make-4.4.1` make program which can be downloaded from the link below.

    https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases/

4. Build project
    ```
    mkdir build
    cd build
    cmake\
        -DAVR_TOOLCHAIN_DIR=${Fill in path to the folder containing AVR-GCC bin folder}\
        -DCMAKE_MAKE_PROGRAM=${Fill in path to make.exe}\
        ..
    ```

5. Upload the executable to your machine using avrdude

    https://github.com/avrdudes/avrdude


# Code
```
┌───────────────────────────────┐
│      System State Machine     │
├───────────────────────────────┤
│        Lamp State Machine     │
├───────────────────────────────┤
│        Clap State Machine     │
├───────────────────────────────┤
│ Peripheral Control Programs   │
│ (FND, LED, Timer, etc.)       │
├──────────────────────┬────────┤
│     Pin Settings     │ System │
├──────────────────────┤ Config │
│    ATmega128 Lib     │        │
└──────────────────────┴────────┘
```

## System State Machine
![](assets/system_state_machine.png)

## Lamp State Machine
![](assets/lamp_state_machine.png)
