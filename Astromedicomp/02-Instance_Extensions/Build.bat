cls

del Vk.exe Log.txt

cl /EHsc /I"C:\VulkanSDK\Vulkan\Include" /c Vk.c /Fo"Vk.obj"

rc.exe Vk.rc

link Vk.obj Vk.res /LIBPATH:"C:\VulkanSDK\Vulkan\Lib" vulkan-1.lib user32.lib gdi32.lib kernel32.lib /OUT:Vk.exe 

del Vk.res Vk.obj

Vk.exe
