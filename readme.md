# 2025 DFR CVC
## Hardware
Runs on the large MCU module connected to the 2025 main GLV board. Large MCU module is based on the STM32F405RGT6 microcontroller and has external 32.768kHz and 12MHz oscillators connected as external oscillators in BYPASS mode.  
Design files for both are in `thayerFS/common/dfr/2025 DFR/Electrical/GLV`.

## Code Structure
This project relies on FreeRTOS for scheduling, synchronization, and queues. All tasks, queues, and mutexes/semaphores are statically allocated, allowing more effective static analysis and accurate compiler checks for program memory use. This project currently has the following tasks:
 

* stateMachineTask  

  Defines vehicle state and manages state transitions. State transitions are determined by a combination of contactor state, shutdown circuit state, throttle & brake input, BSPD state, throttle & brake pressure validity, and driver input. The state machine is designed to match the diagram shown in the Formula Hybrid & Electric 2025 rulebook.  

* throttleTask 

  Handles APPS input validation and converts raw ADC counts to a 0.0-1.0 throttle range. Provides functions for checking throttle percentage and validity.  

* canRxTask

* can1TxTask

* can2TxTask

* torqueTask

* torqueCommandTask

* stateMachineOutputTask

* brakeLightTask

* dashboardBroadcastTask

* wheelSpeedsTask

* analogReadTask


## Project File Structure
This project is designed to be edited in the PlatformIO IDE. PlatformIO expects the following directories:
* boards  
(JSON target MCU definitions)  

* include  
(header files)  

* lib  
(libraries, must be included as build flags)  

* src  
(source files)  

* test  
(unit tests)

It also expects to see a `platformio.ini` file and a `.ld` linker script in the root of the project.

## Code Generation
Code is generated using STM32CubeMX from the `Large_MCU_Module.ioc` CubeMX project located in `/generated`. Project settings should be set to generate code for the STM32CubeIDE, as its structure is the most similar to what PlatformIO expects. CubeMX will create the following file structure:

* Core/Inc
* Core/Src
* Core/Startup
* Drivers
* Middlewares

### STM32CubeIDE to PlatformIO Migration
1. Move all files in `Core/Inc` to `include`.
2. Move all files in `Core/Src` to `src`.
3. Move all files in `Core/Startup` to `src`.
4. Move all folders in `Drivers` to `lib`.
5. Move all folders in `Middlewares` to `lib`. Some folders may be nested (i.e. `Middlewares/ThirdParty/) TODO: finish
6. Move the `.ld` linker scripts to the root of the project, and remove any instances of the `(READONLY)` keyword as it is incompatible with the version of GCC used by PlatformIO.
7. Once all of the files listed above have been moved, clear out the entire `generated` directory, leaving only the `.ioc` project file.

### PlatformIO Setup
Some common STM32 microcontrollers are available as Nucleo development boards and have target definitions built in to PlatformIO. Those that do not require a custom board definition file to be placed in the `boards` directory of the project. Use the `cvc_f405rg.json` file as an example of a custom target.

`platformio.ini` must then be modified to use this custom definition. The most important changes are:
1. Ensure that the target chosen in square brackets and on the `board` line matches the definition.
2. Comment out the `framework` line. This will ensure that PlatformIO uses only the HAL files provided by Stm32CubeMX and not those built into the IDE.
3. Add a line with `lib_archive = no`. This will force PlatformIO to recompile FreeRTOS from the provided source files, rather than using a precompiled library.
4. Add build flags matching those in this repositories `platformio.ini` file, and add all library and driver names to the `lib_deps` line.. These will ensure that the compiler correctly finds all header and library files needed for the project.

