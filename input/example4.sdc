create_clock -period 0.048 -name clk

set_input_delay  0.001 -clock clk input_0 
set_input_delay  0.002 -clock clk input_1
set_max_delay  0.046 -to output_0
set_max_delay  0.045 -to output_1
set_max_delay  0.044 -to output_2
set_max_delay  0.043 -to output_3
set_max_delay  0.042 -to output_4
set_max_delay  0.041 -to output_5
set_max_delay  0.040 -to output_6
set_max_delay  0.039 -to output_7
set_max_delay  0.038 -to output_8
set_max_delay  0.037 -to output_9
set_max_delay  0.036 -to output_10
