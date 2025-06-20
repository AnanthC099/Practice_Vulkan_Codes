cls

del Vk.exe Log.txt cube.vert.spv cube.frag.spv

glslc -fshader-stage=vert cube.vert -o cube.vert.spv

glslc -fshader-stage=frag cube.frag -o cube.frag.spv

cl /I"C:\VulkanSDK\Anjaneya\Include" /c Vk.cpp /Fo"Vk.obj"

rc.exe Vk.rc

link Vk.obj Vk.res /LIBPATH:"C:\VulkanSDK\Anjaneya\Lib" vulkan-1.lib user32.lib gdi32.lib kernel32.lib /OUT:Vk.exe 

del Vk.obj Vk.pdb Vk.res

Vk.exe


