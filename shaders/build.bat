@echo off
glslangValidator -V shader.vert shader.frag
spirv-opt frag.spv -o spv/frag.spv -O
spirv-opt vert.spv -o spv/vert.spv -O
del *.spv