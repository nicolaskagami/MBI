
INPUTS=./input
for mbi in bin/*
do
    for file in ${INPUTS}/DEFs/*.def 
    do
        #echo $file
        echo $mbi
        echo $(basename ${file} .def)
        ${mbi} --def $file --sdc ${INPUTS}/bench.sdc --lib ${INPUTS}/simple-cells.lib >> Results/$(basename ${file} .def).dat
    done
done
