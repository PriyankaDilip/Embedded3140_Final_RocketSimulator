# Embedded3140_Final_RocketSimulator
Final Project for ECE 3140 (Embedded Systems): Model Rocket Flight Computer and Launch Simulator
Priyanka Dilip, Alec Wyatt

For our final project, we have used the FRDM K64F board to design a model rocket launch controller and flight computer to accomplish these goals in a small embedded system. Our implementation is built with safety and security foremost in mind. In order to arm the controller, the user must pass a game involving rotation of the board in response to a sequence of LEDs. Once the game is passed, the user creates a password to be used upon recovery in order to view flight data. When the controller is armed, the user mounts the board atop the rocket and connects the controller to the ignition circuit. The user then signals to the board to begin the launch countdown, and the board switches the accelerometer mode to record high-acceleration flight data. When the launch time arrives, the controller sends a signal to close a relay, igniting the engines and launching the rocket. At the same time, the controller begins recording acceleration data for the flight of the rocket. When the board is recovered, flight data is displayable at the serial monitor.

If there are any issues running the project, please contact Alec Wyatt (acw254@cornell.edu) or Priyanka Dilip (ped48@cornell.edu).
