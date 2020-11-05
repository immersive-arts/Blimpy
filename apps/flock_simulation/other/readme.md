
## Simulation

This Max msp patch is a flocking simulation to auto-control the helium blimps. The simulation sends a position and velocity vector to the manager. By default this patch controls five blimps.
The flocking simulation was inspired by the book 'the nature of code' from Daniel Shiffman https://natureofcode.com/book/chapter-6-autonomous-agents/

### Manager
The manager is documented inside the *manager* folder.

### Blimps
Details on creating a blimp is documented inside the *blimp* folder.

### Motion Capture
At the [IASpace](http://immersive-arts.ch) the [Optitrack](http://optitrack.com) System is used to track blimps with a large volume.

To send Optitracks native NATNET protocoll to the manager and SPARCK, the [NatNetThree2OSC](https://github.com/tecartlab/app_NetNatThree2OSC) application is used.

# Credits

created during a workshop in the [IASpace](http://immersive-arts.ch), Zürich University of the Arts, Switzerland.

* Lilian Lopez - Code for flocking Simulation
* Stucki Cynthia
* Lauper Nils Lou

IASpace Team
* Max Kriegleder - Robotics, motion control, sensor integration
* Joel Gähwiler - Micro-controller, networks, protocols
* Roman Jurt - Materials, rapid prototyping, construction
* Serena Cangiano - Theory
* Martin Fröhlich - Lead, motion tracking, projection mapping
