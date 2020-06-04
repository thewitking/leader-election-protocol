# leader-election-protocol
Leader Election Protocol implementation for fault tolerance and adaptation in Low power distributed IoT systems.

Contact Information:
Contributed and developed by: MANISH SONI <manishso@mtu.edu>

MICHIGAN TECHNOLOGICAL UNIVERSITY


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

