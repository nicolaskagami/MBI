create_clock -period 48 -name clk

set_input_delay  1 -clock clk input_0 
set_input_delay  2 -clock clk input_1
set_max_delay  46 -to output_0
set_max_delay  45 -to output_1
set_max_delay  44 -to output_2
set_max_delay  43 -to output_3
set_max_delay  42 -to output_4
set_max_delay  41 -to output_5
set_max_delay  40 -to output_6
set_max_delay  39 -to output_7
set_max_delay  38 -to output_8
set_max_delay  37 -to output_9
set_max_delay  36 -to output_10
