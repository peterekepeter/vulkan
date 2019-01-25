@echo off
if not exist "./spv" mkdir "./spv"
glslangValidator -V shader.vert shader.frag
if errorlevel neq 0 exit /b
spirv-opt frag.spv -o spv/frag.spv -O
spirv-opt vert.spv -o spv/vert.spv -O
del *.spv