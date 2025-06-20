#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* ---------- GLM (for perspective matrix) ---------- */
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "vulkan-1.lib")

/* =================================================== */
/*  Ultra‑verbose logging helpers (noob‑friendly)      */
/* =================================================== */
static inline const char* _logTimeStamp(void)
{
    static char buf[32];
    time_t      t  = time(NULL);
    struct tm*  tm = localtime(&t);
    strftime(buf, sizeof(buf), "%H:%M:%S", tm);
    return buf;
}

#define LOG(fmt, ...)  do { if(gFILE) fprintf(gFILE, "[%s] LOG: "   fmt "\n", _logTimeStamp(), ##__VA_ARGS__); } while(0)
#define ERR(fmt, ...)  do { if(gFILE) fprintf(gFILE, "[%s] ERROR: " fmt "\n", _logTimeStamp(), ##__VA_ARGS__); } while(0)

#define VK_CHECK(expr)                                                       \
    do {                                                                     \
        LOG(#expr " …");                                                     \
        VkResult _vk_tmp = (expr);                                           \
        if(_vk_tmp != VK_SUCCESS) {                                          \
            ERR(#expr " failed (VkResult=%d)", _vk_tmp);                     \
            return _vk_tmp;                                                  \
        }                                                                    \
        LOG(#expr " succeeded");                                             \
    } while(0)

/* --------------------------------------------------- */
/*                FORWARD DECLARATIONS                 */
/* --------------------------------------------------- */
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*,
    void*);

static VkResult CreateVulkanInstance(void);
static VkResult CreateSurface(void);
static VkResult PickPhysicalDevice(void);
static VkResult CreateDeviceQueue(void);
static VkResult CreateSwapchain(void);
static VkResult CreateImagesViews(void);
static VkResult CreateCommandPoolBuffers(void);
static VkResult CreateVertexBuffer(void);
static VkResult CreatePipelineLayout(void);
static VkResult CreateGraphicsPipeline(void);
static VkResult buildCommandBuffers(void);
static VkResult CreateSyncObjects(void);
static VkResult recreateSwapchain(void);
static void     cleanupSwapchain(void);
static VkResult drawFrame(void);
static void     resize(int,int);
static void     uninitialize(void);
void ToggleFullscreen(void);

/* ---------- Win32 / global parameters ---------- */
#define MYICON              101
#define WIN_WIDTH           800
#define WIN_HEIGHT          600
#define MAX_FRAMES_IN_FLIGHT 2

HWND   ghwnd            = NULL;
BOOL   gbActive         = FALSE;
DWORD  dwStyle          = 0;
WINDOWPLACEMENT wpPrev;
BOOL   gbFullscreen     = FALSE;
FILE*  gFILE            = NULL;
const char* gpszAppName = "VulkanApp";

bool gEnableValidationLayers = true;

/* ---------- Vulkan globals ---------- */
static VkDebugUtilsMessengerEXT gDebugUtilsMessenger = VK_NULL_HANDLE;
static PFN_vkCmdBeginRenderingKHR pfnCmdBeginRenderingKHR = NULL;
static PFN_vkCmdEndRenderingKHR   pfnCmdEndRenderingKHR   = NULL;

VkInstance        vkInstance                      = VK_NULL_HANDLE;
VkSurfaceKHR      vkSurfaceKHR                    = VK_NULL_HANDLE;
VkPhysicalDevice  vkPhysicalDevice_selected       = VK_NULL_HANDLE;
VkDevice          vkDevice                        = VK_NULL_HANDLE;
VkQueue           vkQueue                         = VK_NULL_HANDLE;
uint32_t          graphicsQuequeFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

VkSwapchainKHR    vkSwapchainKHR                  = VK_NULL_HANDLE;
VkFormat          vkFormat_color                  = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR   vkColorSpaceKHR                 = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
VkPresentModeKHR  vkPresentModeKHR                = VK_PRESENT_MODE_FIFO_KHR;
VkExtent2D        vkExtent2D_SwapChain;
uint32_t          swapchainImageCount             = 0;
VkImage*          swapChainImage_array            = NULL;
VkImageView*      swapChainImageView_array        = NULL;

VkCommandPool     vkCommandPool                   = VK_NULL_HANDLE;
VkCommandBuffer*  vkCommandBuffer_array           = NULL;

VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
VkFence     inFlightFences[MAX_FRAMES_IN_FLIGHT];
uint32_t    currentFrame = 0;

BOOL       bInitialized = FALSE;
VkClearColorValue vkClearColorValue;
int        winWidth = WIN_WIDTH, winHeight = WIN_HEIGHT;

/* ---------- Pipeline / Vertex‑buffer globals ---------- */
static VkPipelineLayout gPipelineLayout = VK_NULL_HANDLE;
static VkPipeline       gPipeline       = VK_NULL_HANDLE;

static const float gTriangleVertices[] =
{
    -0.5f, -0.5f, -3.0f,
     0.5f, -0.5f, -3.0f,
     0.0f,  0.5f, -3.0f
};

static VkBuffer       gVertexBuffer = VK_NULL_HANDLE;
static VkDeviceMemory gVertexMemory = VK_NULL_HANDLE;

/* =================================================== */
/*                   WINDOW CALLBACK                   */
/* =================================================== */
LRESULT CALLBACK WndProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
    switch(iMsg)
    {
        case WM_CREATE:
            memset(&wpPrev,0,sizeof(WINDOWPLACEMENT));
            wpPrev.length=sizeof(WINDOWPLACEMENT);
            break;

        case WM_SETFOCUS:
            gbActive=TRUE;
            LOG("Window gained focus");
            break;

        case WM_KILLFOCUS:
            gbActive=FALSE;
            LOG("Window lost focus");
            break;

        case WM_SIZE:
            if(bInitialized){
                winWidth  = LOWORD(lParam);
                winHeight = HIWORD(lParam);
                LOG("WM_SIZE new client area %d × %d", winWidth, winHeight);

                if(winWidth==0 || winHeight==0){
                    LOG("Window minimised – postponing swap‑chain recreation");
                } else {
                    resize(winWidth, winHeight);
                }
            }
            break;

        case WM_CHAR:
            if(wParam=='f' || wParam=='F'){
                LOG("User toggled full‑screen");
                extern void ToggleFullscreen(void);
                ToggleFullscreen();
                gbFullscreen=!gbFullscreen;
            }
            break;

        case WM_KEYDOWN:
            if(wParam==VK_ESCAPE){
                LOG("ESC pressed – destroying window");
                DestroyWindow(hwnd);
            }
            break;

        case WM_CLOSE:
            LOG("WM_CLOSE received");
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            LOG("WM_DESTROY received – posting quit");
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd,iMsg,wParam,lParam);
}

/* =================================================== */
/*               VALIDATION / DEBUG CALLBACK           */
/* =================================================== */
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT ms,
    VkDebugUtilsMessageTypeFlagsEXT        mt,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    fprintf(gFILE,"[%s] VALIDATION: %s\n", _logTimeStamp(), pCallbackData->pMessage);
    return VK_FALSE;
}

/* =================================================== */
/*                    VULKAN HELPERS                   */
/* =================================================== */
static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT fn =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
    return fn ? fn(instance,pCreateInfo,pAllocator,pMessenger)
              : VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,VkDebugUtilsMessengerEXT messenger,const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT fn =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
    if(fn) fn(instance,messenger,pAllocator);
}

/* ---------- Full‑screen helper ---------- */
void ToggleFullscreen(void)
{
    MONITORINFO mi={sizeof(MONITORINFO)};
    if(!gbFullscreen)
    {
        dwStyle=GetWindowLong(ghwnd,GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if(GetWindowPlacement(ghwnd,&wpPrev) &&
               GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi))
            {
                SetWindowLong(ghwnd,GWL_STYLE,dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd,HWND_TOP,
                             mi.rcMonitor.left,mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left,
                             mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOZORDER|SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowPlacement(ghwnd,&wpPrev);
        SetWindowLong(ghwnd,GWL_STYLE,dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,
                     SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

/* =================================================== */
/*                INSTANCE / LAYER SETUP               */
/* =================================================== */
static uint32_t instExtCount = 0;
static const char* instExts[4];
static uint32_t instLayerCount = 0;
static const char* instLayers[1];

static VkResult FillInstanceExtensions(void)
{
    LOG("FillInstanceExtensions()");
    uint32_t count=0;
    vkEnumerateInstanceExtensionProperties(NULL,&count,NULL);
    if(!count) return VK_ERROR_INITIALIZATION_FAILED;

    VkExtensionProperties* arr=(VkExtensionProperties*)malloc(sizeof(VkExtensionProperties)*count);
    vkEnumerateInstanceExtensionProperties(NULL,&count,arr);

    VkBool32 foundSurface=VK_FALSE,foundWin32=VK_FALSE;
    for(uint32_t i=0;i<count;i++){
        if(!strcmp(arr[i].extensionName,VK_KHR_SURFACE_EXTENSION_NAME))      foundSurface=VK_TRUE;
        if(!strcmp(arr[i].extensionName,VK_KHR_WIN32_SURFACE_EXTENSION_NAME))foundWin32 =VK_TRUE;
    }
    free(arr);

    if(!foundSurface||!foundWin32){
        ERR("Required instance extensions not found");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    instExtCount = 0;
    instExts[instExtCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
    instExts[instExtCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    return VK_SUCCESS;
}

static void TryAddDebugExt(void)
{
    LOG("Searching for VK_EXT_debug_utils");
    uint32_t c=0;
    vkEnumerateInstanceExtensionProperties(NULL,&c,NULL);
    if(!c) return;

    VkExtensionProperties* arr=(VkExtensionProperties*)malloc(sizeof(VkExtensionProperties)*c);
    vkEnumerateInstanceExtensionProperties(NULL,&c,arr);

    for(uint32_t i=0;i<c;i++){
        if(!strcmp(arr[i].extensionName,VK_EXT_DEBUG_UTILS_EXTENSION_NAME)){
            instExts[instExtCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            LOG("Enabled VK_EXT_debug_utils");
            break;
        }
    }
    free(arr);
}

static void TryAddValidationLayer(void)
{
    if (!gEnableValidationLayers) { LOG("Validation layers disabled"); return; }

    uint32_t c=0;
    vkEnumerateInstanceLayerProperties(&c,NULL);
    if(!c) { LOG("No instance layers present"); return; }

    VkLayerProperties* arr=(VkLayerProperties*)malloc(sizeof(VkLayerProperties)*c);
    vkEnumerateInstanceLayerProperties(&c,arr);

    for(uint32_t i=0;i<c;i++){
        if(!strcmp(arr[i].layerName,"VK_LAYER_KHRONOS_validation")){
            instLayers[0]   = "VK_LAYER_KHRONOS_validation";
            instLayerCount  = 1;
            LOG("Enabled VK_LAYER_KHRONOS_validation");
            break;
        }
    }
    free(arr);
}

static VkResult CreateVulkanInstance(void)
{
    LOG("CreateVulkanInstance()");
    VK_CHECK(FillInstanceExtensions());
    TryAddDebugExt();
    TryAddValidationLayer();

    VkApplicationInfo ai = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    ai.pApplicationName   = gpszAppName;
    ai.applicationVersion = 1;
    ai.pEngineName        = gpszAppName;
    ai.engineVersion      = 1;
    ai.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo ici = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    ici.pApplicationInfo        = &ai;
    ici.enabledExtensionCount   = instExtCount;
    ici.ppEnabledExtensionNames = instExts;
    ici.enabledLayerCount       = instLayerCount;
    ici.ppEnabledLayerNames     = instLayers;

    VK_CHECK(vkCreateInstance(&ici,NULL,&vkInstance));

    if(instLayerCount){
        VkDebugUtilsMessengerCreateInfoEXT dbg = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        dbg.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg.messageType     =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg.pfnUserCallback = DebugCallback;
        VK_CHECK(CreateDebugUtilsMessengerEXT(vkInstance,&dbg,NULL,&gDebugUtilsMessenger));
    }
    return VK_SUCCESS;
}

/* =================================================== */
/*                SURFACE / DEVICE SETUP               */
/* =================================================== */
static VkResult CreateSurface(void)
{
    LOG("CreateSurface()");
    VkWin32SurfaceCreateInfoKHR ci = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    ci.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd,GWLP_HINSTANCE);
    ci.hwnd      = ghwnd;
    VK_CHECK(vkCreateWin32SurfaceKHR(vkInstance,&ci,NULL,&vkSurfaceKHR));
    return VK_SUCCESS;
}

static VkResult PickPhysicalDevice(void)
{
    LOG("PickPhysicalDevice()");
    uint32_t c=0;
    vkEnumeratePhysicalDevices(vkInstance,&c,NULL);
    if(!c) return VK_ERROR_INITIALIZATION_FAILED;

    VkPhysicalDevice* devs = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)*c);
    vkEnumeratePhysicalDevices(vkInstance,&c,devs);

    for(uint32_t i=0;i<c;i++){
        uint32_t qc=0;
        vkGetPhysicalDeviceQueueFamilyProperties(devs[i],&qc,NULL);
        if(!qc) continue;

        VkQueueFamilyProperties* qprops=(VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)*qc);
        vkGetPhysicalDeviceQueueFamilyProperties(devs[i],&qc,qprops);

        for(uint32_t f=0; f<qc; f++){
            VkBool32 sup=VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(devs[i],f,vkSurfaceKHR,&sup);
            if((qprops[f].queueFlags & VK_QUEUE_GRAPHICS_BIT) && sup){
                vkPhysicalDevice_selected        = devs[i];
                graphicsQuequeFamilyIndex_selected = f;
                free(qprops);
                free(devs);
                LOG("Selected physical device #%u, queue family %u", i, f);
                vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected,&vkPhysicalDeviceMemoryProperties);
                return VK_SUCCESS;
            }
        }
        free(qprops);
    }
    free(devs);
    return VK_ERROR_INITIALIZATION_FAILED;
}

/* ---------- Device & queue ---------- */
static const char* devExts[2];
static uint32_t    devExtCount = 0;

static VkResult CheckDevExtensions(void)
{
    LOG("CheckDevExtensions()");
    uint32_t c=0;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,NULL,&c,NULL);
    if(!c) return VK_ERROR_INITIALIZATION_FAILED;

    VkExtensionProperties* arr=(VkExtensionProperties*)malloc(sizeof(VkExtensionProperties)*c);
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,NULL,&c,arr);

    VkBool32 haveSwap=VK_FALSE, haveDyn=VK_FALSE;
    for(uint32_t i=0;i<c;i++){
        if(!strcmp(arr[i].extensionName,VK_KHR_SWAPCHAIN_EXTENSION_NAME)) haveSwap=VK_TRUE;
        if(!strcmp(arr[i].extensionName,"VK_KHR_dynamic_rendering"))      haveDyn =VK_TRUE;
    }
    free(arr);

    if(!haveSwap){
        ERR("VK_KHR_swapchain missing");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    devExtCount = 0;
    devExts[devExtCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    if(haveDyn)            devExts[devExtCount++] = "VK_KHR_dynamic_rendering";
    return VK_SUCCESS;
}

static void LoadDynamicRenderingFunctions(void)
{
    pfnCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(vkDevice,"vkCmdBeginRenderingKHR");
    pfnCmdEndRenderingKHR   =
        (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(vkDevice,"vkCmdEndRenderingKHR");
}

static VkResult CreateDeviceQueue(void)
{
    LOG("CreateDeviceQueue()");
    VK_CHECK(CheckDevExtensions());

    float qp = 1.0f;
    VkDeviceQueueCreateInfo dqci = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    dqci.queueFamilyIndex = graphicsQuequeFamilyIndex_selected;
    dqci.queueCount       = 1;
    dqci.pQueuePriorities = &qp;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR drf =
        { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
    drf.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo dci = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    dci.queueCreateInfoCount    = 1;
    dci.pQueueCreateInfos       = &dqci;
    dci.enabledExtensionCount   = devExtCount;
    dci.ppEnabledExtensionNames = devExts;
    dci.pEnabledFeatures        = NULL;
    dci.pNext                   = &drf;

    VK_CHECK(vkCreateDevice(vkPhysicalDevice_selected,&dci,NULL,&vkDevice));
    vkGetDeviceQueue(vkDevice,graphicsQuequeFamilyIndex_selected,0,&vkQueue);
    LoadDynamicRenderingFunctions();
    return VK_SUCCESS;
}

/* =================================================== */
/*              SWAP‑CHAIN  (format, mode)             */
/* =================================================== */
static VkResult ChooseSwapFormat(void)
{
    uint32_t c=0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected,vkSurfaceKHR,&c,NULL);
    if(!c) return VK_ERROR_INITIALIZATION_FAILED;

    VkSurfaceFormatKHR* arr=(VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR)*c);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected,vkSurfaceKHR,&c,arr);

    if(c==1 && arr[0].format==VK_FORMAT_UNDEFINED){
        vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        vkFormat_color = arr[0].format;
    }
    vkColorSpaceKHR = arr[0].colorSpace;
    free(arr);
    return VK_SUCCESS;
}

static VkResult ChoosePresentMode(void)
{
    uint32_t c=0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,vkSurfaceKHR,&c,NULL);
    if(!c) return VK_ERROR_INITIALIZATION_FAILED;

    VkPresentModeKHR* arr=(VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR)*c);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,vkSurfaceKHR,&c,arr);

    vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
    for(uint32_t i=0;i<c;i++){
        if(arr[i]==VK_PRESENT_MODE_MAILBOX_KHR){
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    free(arr);
    return VK_SUCCESS;
}

static VkResult CreateSwapchain(void)
{
    LOG("CreateSwapchain()");
    VK_CHECK(ChooseSwapFormat());
    VK_CHECK(ChoosePresentMode());

    VkSurfaceCapabilitiesKHR cap;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vkPhysicalDevice_selected,vkSurfaceKHR,&cap);

    uint32_t desired = cap.minImageCount + 1;
    if(cap.maxImageCount>0 && desired>cap.maxImageCount)
        desired = cap.maxImageCount;

    if(cap.currentExtent.width != UINT32_MAX){
        vkExtent2D_SwapChain = cap.currentExtent;
    } else {
        vkExtent2D_SwapChain.width  = winWidth;
        vkExtent2D_SwapChain.height = winHeight;
        if(vkExtent2D_SwapChain.width  < cap.minImageExtent.width )
            vkExtent2D_SwapChain.width  = cap.minImageExtent.width;
        if(vkExtent2D_SwapChain.height < cap.minImageExtent.height)
            vkExtent2D_SwapChain.height = cap.minImageExtent.height;
        if(vkExtent2D_SwapChain.width  > cap.maxImageExtent.width )
            vkExtent2D_SwapChain.width  = cap.maxImageExtent.width;
        if(vkExtent2D_SwapChain.height > cap.maxImageExtent.height)
            vkExtent2D_SwapChain.height = cap.maxImageExtent.height;
    }

    VkSwapchainCreateInfoKHR sci = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    sci.surface          = vkSurfaceKHR;
    sci.minImageCount    = desired;
    sci.imageFormat      = vkFormat_color;
    sci.imageColorSpace  = vkColorSpaceKHR;
    sci.imageExtent      = vkExtent2D_SwapChain;
    sci.imageArrayLayers = 1;
    sci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    sci.preTransform     = (cap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)?
                            VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : cap.currentTransform;
    sci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode      = vkPresentModeKHR;
    sci.clipped          = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(vkDevice,&sci,NULL,&vkSwapchainKHR));
    return VK_SUCCESS;
}

static VkResult CreateImagesViews(void)
{
    LOG("CreateImageViews()");
    vkGetSwapchainImagesKHR(vkDevice,vkSwapchainKHR,&swapchainImageCount,NULL);
    if(!swapchainImageCount) return VK_ERROR_INITIALIZATION_FAILED;

    swapChainImage_array = (VkImage*)malloc(sizeof(VkImage)*swapchainImageCount);
    vkGetSwapchainImagesKHR(vkDevice,vkSwapchainKHR,&swapchainImageCount,swapChainImage_array);

    swapChainImageView_array = (VkImageView*)malloc(sizeof(VkImageView)*swapchainImageCount);
    for(uint32_t i=0;i<swapchainImageCount;i++){
        VkImageViewCreateInfo ivci = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        ivci.image            = swapChainImage_array[i];
        ivci.viewType         = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format           = vkFormat_color;
        ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel   = 0;
        ivci.subresourceRange.levelCount     = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount     = 1;
        VK_CHECK(vkCreateImageView(vkDevice,&ivci,NULL,&swapChainImageView_array[i]));
    }
    return VK_SUCCESS;
}

/* =================================================== */
/*           COMMAND POOL / COMMAND BUFFERS            */
/* =================================================== */
static VkResult CreateCommandPoolBuffers(void)
{
    LOG("CreateCommandPoolBuffers()");
    VkCommandPoolCreateInfo cpci = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cpci.queueFamilyIndex = graphicsQuequeFamilyIndex_selected;
    cpci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(vkDevice,&cpci,NULL,&vkCommandPool));

    VkCommandBufferAllocateInfo cbai = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbai.commandPool        = vkCommandPool;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;

    vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer)*swapchainImageCount);
    for(uint32_t i=0;i<swapchainImageCount;i++){
        VK_CHECK(vkAllocateCommandBuffers(vkDevice,&cbai,&vkCommandBuffer_array[i]));
    }
    return VK_SUCCESS;
}

/* =================================================== */
/*                 VERTEX BUFFER SETUP                 */
/* =================================================== */
static VkResult CreateVertexBuffer(void)
{
    LOG("CreateVertexBuffer()");
    VkBufferCreateInfo bci = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bci.size        = sizeof(gTriangleVertices);
    bci.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(vkDevice,&bci,NULL,&gVertexBuffer));

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(vkDevice,gVertexBuffer,&memReq);

    uint32_t memTypeIndex = UINT32_MAX;
    for(uint32_t i=0;i<vkPhysicalDeviceMemoryProperties.memoryTypeCount;i++){
        if((memReq.memoryTypeBits & (1<<i)) &&
           ((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
            (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))){
            memTypeIndex = i; break;
        }
    }
    if(memTypeIndex==UINT32_MAX) return VK_ERROR_INITIALIZATION_FAILED;

    VkMemoryAllocateInfo mai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize  = memReq.size;
    mai.memoryTypeIndex = memTypeIndex;
    VK_CHECK(vkAllocateMemory(vkDevice,&mai,NULL,&gVertexMemory));

    vkBindBufferMemory(vkDevice,gVertexBuffer,gVertexMemory,0);

    void* data=NULL;
    vkMapMemory(vkDevice,gVertexMemory,0,memReq.size,0,&data);
    memcpy(data,gTriangleVertices,sizeof(gTriangleVertices));
    vkUnmapMemory(vkDevice,gVertexMemory);
    return VK_SUCCESS;
}

/* =================================================== */
/*                PIPELINE LAYOUT & PIPELINE           */
/* =================================================== */
static VkShaderModule CreateShaderModule(const char* path)
{
    FILE* fp=fopen(path,"rb");
    if(!fp){ ERR("Could not open %s",path); return VK_NULL_HANDLE; }

    fseek(fp,0,SEEK_END); long size=ftell(fp); rewind(fp);
    char* buf=(char*)malloc(size);
    fread(buf,1,size,fp); fclose(fp);

    VkShaderModuleCreateInfo smci = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    smci.codeSize = size;
    smci.pCode    = (const uint32_t*)buf;
    VkShaderModule mod;
    if(vkCreateShaderModule(vkDevice,&smci,NULL,&mod)!=VK_SUCCESS){
        free(buf); ERR("vkCreateShaderModule failed for %s",path); return VK_NULL_HANDLE;
    }
    free(buf); return mod;
}

static VkResult CreatePipelineLayout(void)
{
    LOG("CreatePipelineLayout()");
    //VkPushConstantRange pc;
    //pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //pc.offset     = 0;
    //pc.size       = sizeof(float)*16;
	
	VkPushConstantRange pcRange{};
    pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pcRange.offset     = 0;
    pcRange.size       = sizeof(float) * 20;      // 16 floats mat4 + 4 floats vec4 = 80 B
	
    VkPipelineLayoutCreateInfo plci = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plci.pushConstantRangeCount = 1;
    plci.pPushConstantRanges    = &pcRange;
    VK_CHECK(vkCreatePipelineLayout(vkDevice,&plci,NULL,&gPipelineLayout));
    return VK_SUCCESS;
}

static VkResult CreateGraphicsPipeline(void)
{
    LOG("CreateGraphicsPipeline()");
    VkShaderModule vert = CreateShaderModule("./triangle.vert.spirv");
    VkShaderModule frag = CreateShaderModule("./triangle.frag.spirv");
    if(!vert||!frag) return VK_ERROR_INITIALIZATION_FAILED;

    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vert;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = frag;
    stages[1].pName  = "main";

    VkVertexInputBindingDescription   bind = {0, sizeof(float)*3, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attr = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    VkPipelineVertexInputStateCreateInfo visci = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    visci.vertexBindingDescriptionCount   = 1;
    visci.pVertexBindingDescriptions      = &bind;
    visci.vertexAttributeDescriptionCount = 1;
    visci.pVertexAttributeDescriptions    = &attr;

    VkPipelineInputAssemblyStateCreateInfo ias = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    ias.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vps = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    vps.viewportCount = 1; vps.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode    = VK_CULL_MODE_NONE;
    rs.frontFace   = VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState cbAtt = {0};
    cbAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|
                           VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo cb = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    cb.attachmentCount = 1; cb.pAttachments = &cbAtt;

    VkDynamicState dyns[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo ds = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    ds.dynamicStateCount = 2; ds.pDynamicStates = dyns;

    VkPipelineRenderingCreateInfoKHR pr = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
    pr.colorAttachmentCount    = 1;
    pr.pColorAttachmentFormats = &vkFormat_color;

    VkGraphicsPipelineCreateInfo pci = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pci.pNext               = &pr;
    pci.stageCount          = 2;
    pci.pStages             = stages;
    pci.pVertexInputState   = &visci;
    pci.pInputAssemblyState = &ias;
    pci.pViewportState      = &vps;
    pci.pRasterizationState = &rs;
    pci.pMultisampleState   = &ms;
    pci.pColorBlendState    = &cb;
    pci.pDynamicState       = &ds;
    pci.layout              = gPipelineLayout;

    VK_CHECK(vkCreateGraphicsPipelines(vkDevice,VK_NULL_HANDLE,1,&pci,NULL,&gPipeline));
    vkDestroyShaderModule(vkDevice,vert,NULL);
    vkDestroyShaderModule(vkDevice,frag,NULL);
    return VK_SUCCESS;
}

/* =================================================== */
/*              COMMAND BUFFER RECORDING               */
/* =================================================== */
static VkResult buildCommandBuffers(void)
{
    LOG("buildCommandBuffers()");
    for(uint32_t i=0;i<swapchainImageCount;i++){
        VK_CHECK(vkResetCommandBuffer(vkCommandBuffer_array[i],0));

        VkCommandBufferBeginInfo bi = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        VK_CHECK(vkBeginCommandBuffer(vkCommandBuffer_array[i],&bi));
        LOG("Recording command buffer for image %u", i);

        /* Barrier UNDEFINED → COLOR_ATTACHMENT_OPTIMAL */
        VkImageMemoryBarrier toColor={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        toColor.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
        toColor.newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        toColor.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toColor.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toColor.image            = swapChainImage_array[i];
        toColor.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        toColor.subresourceRange.baseMipLevel   = 0;
        toColor.subresourceRange.levelCount     = 1;
        toColor.subresourceRange.baseArrayLayer = 0;
        toColor.subresourceRange.layerCount     = 1;
        toColor.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(
            vkCommandBuffer_array[i],
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,0,NULL,0,NULL,1,&toColor);

        /* ----- Dynamic rendering begin ----- */
        VkRenderingAttachmentInfoKHR colAtt={VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
        colAtt.imageView   = swapChainImageView_array[i];
        colAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colAtt.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colAtt.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        colAtt.clearValue.color = vkClearColorValue;

        VkRenderingInfoKHR ri={VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        ri.renderArea.extent = vkExtent2D_SwapChain;
        ri.layerCount        = 1;
        ri.colorAttachmentCount = 1;
        ri.pColorAttachments = &colAtt;

        pfnCmdBeginRenderingKHR(vkCommandBuffer_array[i], &ri);

        /* Dynamic viewport/scissor */
        VkViewport vp={0, (float)vkExtent2D_SwapChain.height,
                       (float)vkExtent2D_SwapChain.width,
                      -(float)vkExtent2D_SwapChain.height,0,1};
        vkCmdSetViewport(vkCommandBuffer_array[i],0,1,&vp);
        VkRect2D sci={{0,0},vkExtent2D_SwapChain};
        vkCmdSetScissor(vkCommandBuffer_array[i],0,1,&sci);

        /* Bind pipeline / vertex buffer & push constants */
        vkCmdBindPipeline(vkCommandBuffer_array[i],VK_PIPELINE_BIND_POINT_GRAPHICS,gPipeline);

        float aspect=(float)vkExtent2D_SwapChain.width/(float)vkExtent2D_SwapChain.height;
        glm::mat4 proj=glm::perspective(glm::radians(45.0f),aspect,0.1f,100.0f);
        proj[1][1]*=-1.0f;
		
		glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);   // <‑‑ solid red

		typedef struct 
		{ 
			glm::mat4 m; 
			glm::vec4 c; 
		} pc;
		pc pc_var = {0};
		pc_var = { proj, color };
		
        //vkCmdPushConstants(vkCommandBuffer_array[i], gPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &proj[0][0]);			   
		vkCmdPushConstants(vkCommandBuffer_array[i], gPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc_var), &pc_var);

        VkDeviceSize off=0;
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i],0,1,&gVertexBuffer,&off);
        vkCmdDraw(vkCommandBuffer_array[i],3,1,0,0);

        pfnCmdEndRenderingKHR(vkCommandBuffer_array[i]);

        /* Barrier COLOR_ATTACHMENT_OPTIMAL → PRESENT_SRC_KHR */
        VkImageMemoryBarrier toPresent={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toPresent.image  = swapChainImage_array[i];
        toPresent.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        toPresent.subresourceRange.levelCount=1;
        toPresent.subresourceRange.layerCount=1;
        toPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(
            vkCommandBuffer_array[i],
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,0,NULL,0,NULL,1,&toPresent);

        VK_CHECK(vkEndCommandBuffer(vkCommandBuffer_array[i]));
    }
    return VK_SUCCESS;
}

/* =================================================== */
/*                   SYNC OBJECTS                      */
/* =================================================== */
static VkResult CreateSyncObjects(void)
{
    LOG("CreateSyncObjects()");
    VkSemaphoreCreateInfo sci={VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo     fci={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
        VK_CHECK(vkCreateSemaphore(vkDevice,&sci,NULL,&imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(vkDevice,&sci,NULL,&renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence    (vkDevice,&fci,NULL,&inFlightFences[i]));
    }
    return VK_SUCCESS;
}

/* =================================================== */
/*                SWAP‑CHAIN RECREATION                */
/* =================================================== */
static void cleanupSwapchain(void)
{
    LOG("cleanupSwapchain()");
    vkDeviceWaitIdle(vkDevice);

    if(vkCommandBuffer_array){
        for(uint32_t i=0;i<swapchainImageCount;i++){
            if(vkCommandBuffer_array[i])
                vkFreeCommandBuffers(vkDevice,vkCommandPool,1,&vkCommandBuffer_array[i]);
        }
        free(vkCommandBuffer_array);
        vkCommandBuffer_array=NULL;
    }
    if(vkCommandPool){
        vkDestroyCommandPool(vkDevice,vkCommandPool,NULL);
        vkCommandPool=VK_NULL_HANDLE;
    }
    if(swapChainImageView_array){
        for(uint32_t i=0;i<swapchainImageCount;i++){
            if(swapChainImageView_array[i])
                vkDestroyImageView(vkDevice,swapChainImageView_array[i],NULL);
        }
        free(swapChainImageView_array);
        swapChainImageView_array=NULL;
    }
    if(swapChainImage_array){
        free(swapChainImage_array);
        swapChainImage_array=NULL;
    }
    if(vkSwapchainKHR){
        vkDestroySwapchainKHR(vkDevice,vkSwapchainKHR,NULL);
        vkSwapchainKHR=VK_NULL_HANDLE;
    }
}

static VkResult recreateSwapchain(void)
{
    LOG("recreateSwapchain() – new extent %u × %u", winWidth, winHeight);
    if(winWidth==0||winHeight==0) return VK_SUCCESS;
    cleanupSwapchain();
    VK_CHECK(CreateSwapchain());
    VK_CHECK(CreateImagesViews());
    VK_CHECK(CreateCommandPoolBuffers());
    return buildCommandBuffers();
}

void resize(int w,int h)
{
    winWidth=w; winHeight=h;
    recreateSwapchain();
}

/* =================================================== */
/*                     DRAW FRAME                      */
/* =================================================== */
static VkResult drawFrame(void)
{
    LOG("---- Frame %u begin ----", currentFrame);

    VK_CHECK(vkWaitForFences(vkDevice,1,&inFlightFences[currentFrame],VK_TRUE,UINT64_MAX));
    VK_CHECK(vkResetFences(vkDevice,1,&inFlightFences[currentFrame]));

    uint32_t imgIndex;
    VkResult r = vkAcquireNextImageKHR(
        vkDevice,vkSwapchainKHR,UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,&imgIndex);

    if(r==VK_ERROR_OUT_OF_DATE_KHR){ recreateSwapchain(); return VK_SUCCESS; }
    if(r!=VK_SUCCESS && r!=VK_SUBOPTIMAL_KHR) return r;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.waitSemaphoreCount   = 1;
    si.pWaitSemaphores      = &imageAvailableSemaphores[currentFrame];
    si.pWaitDstStageMask    = &waitStage;
    si.commandBufferCount   = 1;
    si.pCommandBuffers      = &vkCommandBuffer_array[imgIndex];
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores    = &renderFinishedSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(vkQueue,1,&si,inFlightFences[currentFrame]));

    VkPresentInfoKHR pi={VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores    = &renderFinishedSemaphores[currentFrame];
    pi.swapchainCount     = 1;
    pi.pSwapchains        = &vkSwapchainKHR;
    pi.pImageIndices      = &imgIndex;

    r=vkQueuePresentKHR(vkQueue,&pi);
    if(r==VK_ERROR_OUT_OF_DATE_KHR||r==VK_SUBOPTIMAL_KHR){ recreateSwapchain(); }

    currentFrame=(currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    LOG("---- Frame end ----");
    return VK_SUCCESS;
}

/* =================================================== */
/*                    INITIALISE                       */
/* =================================================== */
static VkResult initialize(void)
{
    LOG("initialize()");
    VK_CHECK(CreateVulkanInstance());
    VK_CHECK(CreateSurface());
    VK_CHECK(PickPhysicalDevice());
    VK_CHECK(CreateDeviceQueue());
    VK_CHECK(CreateSwapchain());
    VK_CHECK(CreateImagesViews());
    VK_CHECK(CreateCommandPoolBuffers());
    VK_CHECK(CreateVertexBuffer());
    VK_CHECK(CreatePipelineLayout());
    VK_CHECK(CreateGraphicsPipeline());
    VK_CHECK(CreateSyncObjects());

    memset(&vkClearColorValue,0,sizeof(vkClearColorValue));
    vkClearColorValue.float32[2]=1.0f;
    vkClearColorValue.float32[3]=1.0f;

    VK_CHECK(buildCommandBuffers());
    bInitialized=TRUE;
    return VK_SUCCESS;
}

/* =================================================== */
/*                    UNINITIALISE                     */
/* =================================================== */
static void uninitialize(void)
{
    LOG("uninitialize()");
    if(gbFullscreen){ ToggleFullscreen(); gbFullscreen=FALSE; }
    if(vkDevice){
        vkDeviceWaitIdle(vkDevice);

        for(int i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
            if(inFlightFences[i])        vkDestroyFence    (vkDevice,inFlightFences[i],NULL);
            if(renderFinishedSemaphores[i])vkDestroySemaphore(vkDevice,renderFinishedSemaphores[i],NULL);
            if(imageAvailableSemaphores[i]) vkDestroySemaphore(vkDevice,imageAvailableSemaphores[i],NULL);
        }
        if(gPipeline)       vkDestroyPipeline      (vkDevice,gPipeline,NULL);
        if(gPipelineLayout) vkDestroyPipelineLayout(vkDevice,gPipelineLayout,NULL);
        if(gVertexBuffer)   vkDestroyBuffer        (vkDevice,gVertexBuffer,NULL);
        if(gVertexMemory)   vkFreeMemory           (vkDevice,gVertexMemory,NULL);

        cleanupSwapchain();
        vkDestroyDevice(vkDevice,NULL);
    }
    if(gDebugUtilsMessenger) DestroyDebugUtilsMessengerEXT(vkInstance,gDebugUtilsMessenger,NULL);
    if(vkSurfaceKHR)         vkDestroySurfaceKHR(vkInstance,vkSurfaceKHR,NULL);
    if(vkInstance)           vkDestroyInstance(vkInstance,NULL);
    if(ghwnd){ DestroyWindow(ghwnd); ghwnd=NULL; }
    if(gFILE){ LOG("Program ended"); fclose(gFILE); gFILE=NULL; }
}

/* =================================================== */
/*                       MAIN                          */
/* =================================================== */
int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR lpszCmdLine,int iCmdShow)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst,MAKEINTRESOURCE(MYICON));
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = gpszAppName;
    wc.hIconSm       = wc.hIcon;
    RegisterClassEx(&wc);

    int sw=GetSystemMetrics(SM_CXSCREEN);
    int sh=GetSystemMetrics(SM_CYSCREEN);
    int x=(sw-WIN_WIDTH)/2, y=(sh-WIN_HEIGHT)/2;

    gFILE=fopen("Log.txt","w");
    if(!gFILE){ MessageBox(NULL,TEXT("Cannot open Log.txt"),TEXT("Error"),MB_OK); return 0; }
    LOG("Creating main window");

    ghwnd=CreateWindowEx(
        WS_EX_APPWINDOW,gpszAppName,
        TEXT("VulkanApp – Perspective Triangle (Dynamic Rendering)"),
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        x,y,WIN_WIDTH,WIN_HEIGHT,
        NULL,NULL,hInst,NULL);

    if(initialize()!=VK_SUCCESS){
        ERR("initialize() failed");
        DestroyWindow(ghwnd);
        ghwnd=NULL;
    }

    ShowWindow(ghwnd,iCmdShow);
    UpdateWindow(ghwnd);
    SetFocus(ghwnd);
    SetForegroundWindow(ghwnd);

    MSG msg; BOOL done=FALSE;
    while(!done){
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT) done=TRUE;
            else{ TranslateMessage(&msg); DispatchMessage(&msg); }
        } else if(gbActive){
            drawFrame();
        }
    }
    uninitialize();
    return (int)msg.wParam;
}
