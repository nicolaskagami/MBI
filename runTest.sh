
INPUTS=./input
for mbi in bin/*
do
    for file in ${INPUTS}/DEFs/*.def 
    do
        #echo $file
        echo $mbi
        echo $(basename ${file} .def)
        ${mbi} --def $file --sdc ${INPUTS}/example4.sdc --lib ${INPUTS}/simple-cells.lib > Results/$(basename ${file} .def)$(basename $mbi).txt
    done
done
