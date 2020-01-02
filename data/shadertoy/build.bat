@echo off
if not exist spv mkdir spv
glslangValidator -V shader.vert 
if %errorlevel% neq 0 exit /b 1
glslangValidator -V shader.frag
if %errorlevel% neq 0 exit /b 1
spirv-opt frag.spv -o spv/frag.spv -O
spirv-opt vert.spv -o spv/vert.spv -O
del *.spv
exit /b 0