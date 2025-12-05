Introduction and description of the project: This project is a proposal for an Autonomous Robot, developed from modifications made to a radio-controlled (RC) car chassis obtained from a content creator known as ""Duke Doks" through his YouTube channel and his official website:

https://dukedoks.com/portfolio/guia-chasis-rc/

The same document contains the download link for the .STL files for 3D printing.
It was decided to use this chassis for 3D printing instead of modifying a model ordered online because the latter option presented greater design and control limitations, which could have led to complications and greater limitations when making modifications to the chassis. In contrast, using a chassis as a base and then selecting the other components and materials needed for the car's proper functioning would have been preferable. Throughout this project, several aspects had to be covered in addition to the selection and modification of the aforementioned chassis, which are:
- Mobility Management
- Power and Sensory Management
- Obstacle Management
The first point refers to how the robot had to perform its most basic functions, namely forward and backward movement. For this, a 21T 540 brushed motor was chosen, as this motor has sufficient torque to rotate a gear system located at the rear of the robot. This system is connected to an axle shared by the two rear wheels, allowing the robot to move forward and backward thanks to the rear-wheel drive provided by the motor. To ensure proper operation of the traction motor, a BTS7960 H-bridge was implemented to control the motor's direction. This was necessary because the brushed motor requires a high current, making it difficult to use with other, more sensitive H-bridges, such as the well-known L298N.
