
glslangValidator -V shader.vert shader.frag
xcopy "*.spv" "spv/*.spv" /Y
del *.spv