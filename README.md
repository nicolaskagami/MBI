# Monotonic Buffer Insertion

The main objective of this process is to compose an inverter tree capable of
supplying all of a node’s positive and negative consumers without violating the fanout.
This imposes a decision about how to construct this inverter tree. In an effort to minimize
the critical path delay of the circuit we will apply heuristics to determine the connections
and positions of the inverters and how they supply the targets.

The targets are sorted by post-delay and divided into critical and non-critical targets. 
Critical targets with higher delay will be allotted preferably closer to the root, in detriment to the delay of the non-critical targets. 
The tree is then expanded to be able to supply all targets and the non-critical targets are distributed along the leaves. 
The targets are appointed to inverters according to a locality heuristic.
