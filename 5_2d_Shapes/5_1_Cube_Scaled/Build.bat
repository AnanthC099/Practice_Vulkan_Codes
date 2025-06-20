cls

del Vk.exe Log.txt pyramid.vert.spv pyramid.frag.spv

glslc -fshader-stage=vert pyramid.vert -o pyramid.vert.spv

glslc -fshader-stage=frag pyramid.frag -o pyramid.frag.spv

cl /I"C:\VulkanSDK\Anjaneya\Include" /c Vk.cpp /Fo"Vk.obj"

rc.exe Vk.rc

link Vk.obj Vk.res /LIBPATH:"C:\VulkanSDK\Anjaneya\Lib" vulkan-1.lib user32.lib gdi32.lib kernel32.lib /OUT:Vk.exe 

del Vk.obj Vk.pdb Vk.res

Vk.exe


