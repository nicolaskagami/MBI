
SRC=./src
MBI: $(SRC)/MBI.cpp $(SRC)/Liberty.cpp $(SRC)/Topology.cpp $(SRC)/Geometry.cpp $(SRC)/InverterTree.cpp
	g++ -DNET_ORDER=0 -DCRIT_ALG=0 -DNON_CRIT_PLACE=0 -DNON_CRIT_ALG=1 $(SRC)/MBI.cpp $(SRC)/Liberty.cpp $(SRC)/Topology.cpp $(SRC)/Geometry.cpp $(SRC)/InverterTree.cpp
clean:
	rm -rf MBI
