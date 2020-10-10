# leader-election-protocol
Leader Election Protocol implementation for fault tolerance and adaptation in Low power distributed IoT systems.

Contact Information:
Contributed and developed by: MANISH SONI <manishso@mtu.edu>

MICHIGAN TECHNOLOGICAL UNIVERSITY

## Description:
Fault tolerance is an important aspect of any CPS-IoT system for it to be reliable and dependable. It is necessary to mitigate such faults in distributed IoT systems to maintain performance and QoS. 

The objective of this project was to implement a leader election protocol that should be effective to discover the distributed IoT devices in given topology and should be adaptive enough to accommodate a new device at any moment. If at any instance the elected leader goes out of operation the system should re-elect the leader among themselves so that the overall performance of distributed IoT systems should not degrade in terms of performance and QoS.

Outcome:
Convergence time for ring topology made of 5 nodes: 28 seconds
Convergence time for line topology made of 5 nodes: 37.8 seconds 
Convergence time for Mesh topology made of 6 nodes: 35.16 seconds 

Tools and Libraries Used: Arduino, RIOT OS, Desvirt, UDP, Socket Programming,  Ubuntu (Linux)
Programming Language: C

## Conclusion and results

Topology Used:
1. Mesh topology
2. Line topology
3. Ring topology

Conclusion of the project report:
For the leader election protocol implementation, I tried three different topologies to test my application "LEP". The application proved efficient for the objective of leader election and it performed the fault tolerance if any of its leader goes out of the operation. The system reelects another leader given that topology binds all the low powered IoT devices so that another leader should be selected to prevent the system failure. 

Result:
The application took 28 seconds to converge for the leader selection process in ring topology of 5 nodes,
37.8 seconds for line topology of 5 nodes and 35.16 seconds to converge for Mesh topology of 6 nodes.

For the application T1=5 sec, T2=2 sec and K=5 gave me the best results.

The application files for RIOT OS are attached as well as result snapshots for ring, line and mesh topology.

