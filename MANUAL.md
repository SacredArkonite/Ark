How to use the current basic version of the precompiler

test.json
  This file describes the dataflow. The operations are done in the order they are in the json file so no parallelism yet.
  
*.arkdata
  These files describe a data structure. For now, I only did a constant integer. I think this would work with a struct but the precompiler doesnt support multi-word declaration so it sould maybe work without whitespaces.
  
*.arkop
  These files describe an operation done in a block. The lines at the beginning of the file MUST BE FOR THE OUTPUTS, IN ONE WORD. Afterward, additionnal operations can be done until the end of the file.
  
LIMITATIONS
The only supported datatype is int.
Necessary includes for an operation must be hardcoded in the precompiler.
The order in witch the operations are defines in the json file is important and must be linear if we want the result to compile.
We are creating a lot of copy operation because there is no options or logic about how to proprely deal with inputs and outputs of blocks.
