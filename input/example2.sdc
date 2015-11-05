create_clock -period 0.048 -name clk

set_input_delay  0.001 -clock clk input_1 
set_input_delay  0.002 -clock clk input_2
set_max_delay  0.042 -to output_1
