cls

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\Include VK.cpp

rc.exe VK.rc

link.exe  VK.obj VK.res /LIBPATH:C:\VulkanSDK\Anjaneya\Lib /SUBSYSTEM:WINDOWS 

VK.exe

del VK.obj VK.exe 

