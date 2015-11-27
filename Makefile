
SRC=./src
MBI: $(SRC)/MBI.cpp $(SRC)/Liberty.cpp $(SRC)/Topology.cpp $(SRC)/Geometry.cpp $(SRC)/InverterTree.cpp
	g++ -DNET_ORDER=1 -DCRIT_ALG=2 -DNON_CRIT_PLACE=1 $(SRC)/MBI.cpp $(SRC)/Liberty.cpp $(SRC)/Topology.cpp $(SRC)/Geometry.cpp $(SRC)/InverterTree.cpp
clean:
	rm -rf MBI
