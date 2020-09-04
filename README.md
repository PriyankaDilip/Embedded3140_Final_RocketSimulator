# Embedded3140_Final_RocketSimulator
Final Project for ECE 3140 (Embedded Systems): Model Rocket Flight Computer and Launch Simulator

Priyanka Dilip, Alec Wyatt

For our final project, we have used the FRDM K64F board to design a model rocket launch controller and flight computer to accomplish these goals in a small embedded system. Our implementation is built with safety and security foremost in mind. In order to arm the controller, the user must pass a game involving rotation of the board in response to a sequence of LEDs. Once the game is passed, the user creates a password to be used upon recovery in order to view flight data. When the controller is armed, the user mounts the board atop the rocket and connects the controller to the ignition circuit. The user then signals to the board to begin the launch countdown, and the board switches the accelerometer mode to record high-acceleration flight data. When the launch time arrives, the controller sends a signal to close a relay, igniting the engines and launching the rocket. At the same time, the controller begins recording acceleration data for the flight of the rocket. When the board is recovered, flight data is displayable at the serial monitor.

If there are any issues running the project, please contact Alec Wyatt (acw254@cornell.edu) or Priyanka Dilip (ped48@cornell.edu).

Priyankas-MBP:acw254-ped48-master priyankadilip_admin$ tree
.
├── README.md
├── accel_64
│   └── readme.txt
├── feedback.md
├── final
│   ├── EventRecorderStub.scvd
│   ├── Listings
│   │   ├── 3140.lst
│   │   ├── 3140_old.lst
│   │   ├── final.map
│   │   └── startup_mk64f12.lst
│   ├── Objects
│   │   ├── 3140.d
│   │   ├── 3140.o
│   │   ├── 3140_accel.crf
│   │   ├── 3140_accel.d
│   │   ├── 3140_accel.o
│   │   ├── 3140_concur.crf
│   │   ├── 3140_concur.d
│   │   ├── 3140_concur.o
│   │   ├── 3140_i2c.crf
│   │   ├── 3140_i2c.d
│   │   ├── 3140_i2c.o
│   │   ├── 3140_old.d
│   │   ├── 3140_old.o
│   │   ├── 3140_serial.crf
│   │   ├── 3140_serial.d
│   │   ├── 3140_serial.o
│   │   ├── ExtDll.iex
│   │   ├── accel_game.crf
│   │   ├── accel_game.d
│   │   ├── accel_game.o
│   │   ├── controller.crf
│   │   ├── controller.d
│   │   ├── controller.o
│   │   ├── dsp.crf
│   │   ├── dsp.d
│   │   ├── dsp.o
│   │   ├── final.axf
│   │   ├── final.build_log.htm
│   │   ├── final.htm
│   │   ├── final.lnp
│   │   ├── final_Target\ 1.dep
│   │   ├── fsl_clock.crf
│   │   ├── fsl_clock.d
│   │   ├── fsl_clock.o
│   │   ├── fsl_common.crf
│   │   ├── fsl_common.d
│   │   ├── fsl_common.o
│   │   ├── fsl_gpio.crf
│   │   ├── fsl_gpio.d
│   │   ├── fsl_gpio.o
│   │   ├── fsl_i2c.crf
│   │   ├── fsl_i2c.d
│   │   ├── fsl_i2c.o
│   │   ├── led_random.crf
│   │   ├── led_random.d
│   │   ├── led_random.o
│   │   ├── math_helper.crf
│   │   ├── math_helper.d
│   │   ├── math_helper.o
│   │   ├── process.crf
│   │   ├── process.d
│   │   ├── process.o
│   │   ├── rocket.crf
│   │   ├── rocket.d
│   │   ├── rocket.o
│   │   ├── startup_mk64f12.d
│   │   ├── startup_mk64f12.o
│   │   ├── system_mk64f12.crf
│   │   ├── system_mk64f12.d
│   │   ├── system_mk64f12.o
│   │   ├── utils.crf
│   │   ├── utils.d
│   │   └── utils.o
│   ├── RTE
│   │   ├── Device
│   │   │   └── MK64FN1M0xxx12
│   │   │       ├── startup_MK64F12.s
│   │   │       └── system_MK64F12.c
│   │   └── _Target_1
│   │       └── RTE_Components.h
│   ├── final.uvguix.alecw
│   ├── final.uvoptx
│   ├── final.uvprojx
│   ├── finalzip.zip
│   └── src
│       ├── 3140.s
│       ├── 3140_accel.c
│       ├── 3140_accel.h
│       ├── 3140_concur.c
│       ├── 3140_concur.h
│       ├── 3140_i2c.c
│       ├── 3140_i2c.h
│       ├── 3140_serial.c
│       ├── 3140_serial.h
│       ├── controller.c
│       ├── process.c
│       ├── realtime.h
│       ├── utils.c
│       └── utils.h
└── lab5
    ├── 3140.s
    ├── 3140_concur.c
    ├── 3140_concur.h
    ├── EventRecorderStub.scvd
    ├── Lab\ 5\ Report.docx
    ├── Lab\ 5\ Report.pdf
    ├── Listings
    │   ├── 3140.lst
    │   ├── lab5.map
    │   └── startup_mk64f12.lst
    ├── Objects
    │   ├── 3140.d
    │   ├── 3140.o
    │   ├── 3140_concur.crf
    │   ├── 3140_concur.d
    │   ├── 3140_concur.o
    │   ├── ExtDll.iex
    │   ├── amaya_process.crf
    │   ├── amaya_process.d
    │   ├── amaya_process.o
    │   ├── amaya_process_v2.crf
    │   ├── amaya_process_v2.d
    │   ├── amaya_process_v2.o
    │   ├── lab5.axf
    │   ├── lab5.build_log.htm
    │   ├── lab5.htm
    │   ├── lab5.lnp
    │   ├── lab5_Target\ 1.dep
    │   ├── lab5_p0.crf
    │   ├── lab5_p0.d
    │   ├── lab5_p0.o
    │   ├── lab5_p1.crf
    │   ├── lab5_p1.d
    │   ├── lab5_p1.o
    │   ├── lab5_t0.crf
    │   ├── lab5_t0.d
    │   ├── lab5_t0.o
    │   ├── lab5_test3.crf
    │   ├── lab5_test3.d
    │   ├── lab5_test3.o
    │   ├── process.crf
    │   ├── process.d
    │   ├── process.o
    │   ├── startup_mk64f12.d
    │   ├── startup_mk64f12.o
    │   ├── system_mk64f12.crf
    │   ├── system_mk64f12.d
    │   ├── system_mk64f12.o
    │   ├── test_r1.crf
    │   ├── test_r1.d
    │   ├── test_r1.o
    │   ├── test_r2.crf
    │   ├── test_r2.d
    │   ├── test_r2.o
    │   ├── test_r3.crf
    │   ├── test_r3.d
    │   ├── test_r3.o
    │   ├── testcaseryan1.crf
    │   ├── testcaseryan1.d
    │   ├── testcaseryan1.o
    │   ├── testcaseryan2.crf
    │   ├── testcaseryan2.d
    │   ├── testcaseryan2.o
    │   ├── utils.crf
    │   ├── utils.d
    │   └── utils.o
    ├── RTE
    │   ├── Device
    │   │   └── MK64FN1M0xxx12
    │   │       ├── startup_MK64F12.s
    │   │       └── system_MK64F12.c
    │   └── _Target_1
    │       └── RTE_Components.h
    ├── amaya_process_v2.c
    ├── lab5.uvguix.alecw
    ├── lab5.uvoptx
    ├── lab5.uvprojx
    ├── lab5_p0.c
    ├── lab5_p1.c
    ├── lab5_t0.c
    ├── lab5_test3.c
    ├── process.c
    ├── realtime.h
    ├── test_cases.docx
    ├── test_r1.c
    ├── test_r2.c
    ├── test_r3.c
    ├── testcaseryan1.c
    ├── testcaseryan2.c
    ├── utils.c
    └── utils.h

16 directories, 177 files
