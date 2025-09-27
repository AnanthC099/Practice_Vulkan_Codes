cls

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\Include VK_s1.cpp

rc.exe VK.rc

link.exe  VK_s1.obj VK.res /LIBPATH:C:\VulkanSDK\Anjaneya\Lib /SUBSYSTEM:WINDOWS 

VK_s1.exe

del VK_s1.obj VK_s1.exe 

