otolith
========

The Otolith, named after the organ in the human ear that is sensitive to acceleration, is an activity monitor worn on the wrist. It was developed in about 2 weeks as the final project for an embedded systems course at the University of Utah. The version here is capable of accurately counting steps as the user walks or runs, and it allows the user to program an alarm which triggers a vibration motor to turn on. Communication between a computer and the device is done using Bluetooth Low Energy.

The application developed to communicate with this device is at https://github.com/kevinavery/otolith-controller

## Hardware
A Nordic NRF51822 Evaluation Kit was used as the hardware platform. This has a 32-bit ARM Cortex M0+ embedded processor and a Bluetooth Low Energy module in a single package. It also has two user-defined buttons, two LEDs, and 31 General Purpose IO pins. Some of these pins were used to interface with a 3-axis accelerometer over SPI, and one was also used to control a vibration motor.

## Project Structure
The `nrf51822` folder is essentially a copy of the Nordic SDK for this device, except that extra example projects were removed from the `Board` directory, and two new projects were added. One project is the `step_counter`, which was used to develop the accelerometer driver and the algorithm used to count steps. The other project was used to develop the bluetooth communications, and eventually became the final project with the step counting code integrated in. Because both projects relied on many of the same source files, code was moved into the the `Source` and `Include` directories to be properly linked to by both projects.

