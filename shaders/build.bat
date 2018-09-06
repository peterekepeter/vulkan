@echo off
glslangValidator -V shader.vert shader.frag
if errorlevel 1 exit /b
spirv-opt frag.spv -o spv/frag.spv -O
spirv-opt vert.spv -o spv/vert.spv -O
del *.spv