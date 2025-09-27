//Header file
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <chrono>
//header file for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "VK.h"
//This inclusion of <vulkan/vulkan.h> is needed in all platforms
//to distinguish the platfrom, below macro is needed
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

//glm related macros and header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE   //clipping values between 0 and 1 for depth
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
// Vulkan related library
#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib,"winmm.lib")

//Macros
/* History of Graphics adapter
Aspect ratio od 4:3 is supported by all monitors
Monochrome
CGA -> Color Graphics Adapter
EGA -> Extended Graphics Adapter
VGA -> Video Graphics Adapter
SVGA -> Super Video Graphics Adapter (4:3 Aspect ratio)
*/
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

//Callback function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Update(void);

const char* gpszAppName = "ARTR";

//Global variables
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

int gbActiveWindow = 0;
int gbFullscreen = 0;
int gbWindowMinimized = FALSE;

int giHeight = 0;
int giWidth = 0;

FILE *gpFile = NULL;

typedef struct Scene1_Vulkan
{
//Vulkan related global variables
//Instance Extension releated variables
uint32_t enabledInstanceExtensionCount = 0;
//VK_KHR_SURFACE_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
//VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[3];

//Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

//Vulkan Presentaiton Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//vulkan Physical Device related
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

//vulkan Printing VkInfo related 
VkPhysicalDevice *vkPhysicalDevice_array = NULL; 
uint32_t physicalDeviceCount = 0;

//Device Extension releated variables
uint32_t enabledDeviceExtensionCount = 0;
//VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_array[1];

//Vulkan Device
VkDevice vkDevice = VK_NULL_HANDLE;

//DeviceQueue
VkQueue vkQueue = VK_NULL_HANDLE;

//Color format and color Space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//Presentation Mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

//Swapchain related
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

//for color images
//SwapchainImage and swapchainImagesViews releated
uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;

//for depth image
VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
VkImageView vkImageView_depth = VK_NULL_HANDLE;


//commandPool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

//Command Buffer
VkCommandBuffer* vkCommandBuffer_array = NULL;

//RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//FrameBuffers
VkFramebuffer *vkFrameBuffer_array = NULL;

//semaphore
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;

//Fence
VkFence *vkFence_array = NULL;

//BuildCommandBuffers
//clear color values
VkClearColorValue vkClearColorValue;  // 3 arrays
VkClearDepthStencilValue vkClearDepthStencilValue;  //for depth

//Render
VkBool32 bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;

//Validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayerNames_array[1]; //for VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//VertexBuffer related variables
typedef struct 
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
}VertexData;

//position
VertexData vertexData_position;
VertexData vertexData_texcoord;

//Uniform related declarations
struct MyUniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
	glm::vec4 fade;   // use .x in shaders
};

struct UniformData
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
};

UniformData uniformData;

//Shader related variables
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

//DescriptorSetLayout realted variables
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

//PipelineLayout realted variables
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

//Descriptor pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

//Descriptor set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

//Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor; //mostly viewport and scissor dimensions are same
VkPipeline vkPipeline = VK_NULL_HANDLE;

float angle = 0.0f;

//Texture related global data
VkImage vkImage_texture = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture = VK_NULL_HANDLE;
VkImageView vkImageView_texture = VK_NULL_HANDLE;
VkSampler vkSampler_texture = VK_NULL_HANDLE;

// Offscreen color target for Subpass 0 (Scene A)
VkImage        vkImage_colorA = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_colorA = VK_NULL_HANDLE;
VkImageView    vkImageView_colorA = VK_NULL_HANDLE;

}Scene1_Vulkan_Struct;

Scene1_Vulkan_Struct s1;

typedef struct Scene1_Effects
{
    // Instance data
    float gFade = 0.0f;

    // Static timeline & state (declare only here; define below)
    static const float kBlackSec;
    static const float kFadeInSec;
    static const float kHoldVisibleSec;
    static const float kFadeOutSec;

    static std::chrono::steady_clock::time_point gEffectT0;
    static bool gEffectStarted;
    static bool gEffectDone;
} Scene1_Effects_Struct;

// ---- Scene1_Effects static definitions ----
const float Scene1_Effects::kBlackSec       = 3.0f;
const float Scene1_Effects::kFadeInSec      = 1.0f;
const float Scene1_Effects::kHoldVisibleSec = 4.0f;
const float Scene1_Effects::kFadeOutSec     = 1.0f;

std::chrono::steady_clock::time_point Scene1_Effects::gEffectT0;
bool Scene1_Effects::gEffectStarted = false;
bool Scene1_Effects::gEffectDone    = false;

Scene1_Effects_Struct s1_effect;

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declarations
	VkResult Initialize(void);
	void Uninitialize(void);
	VkResult Display(void);

	//local variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	int bDone = 0;
    VkResult vkResult = VK_SUCCESS;


	//code
    gpFile = fopen("SSA_Log.txt", "w+");
	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("Cannot create/open SSA_Log.txt file"), TEXT("FILE IO ERROR"), MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "WinMain() --> Program started successfully\n");
	}

    wsprintf(szAppName, TEXT("%s"), gpszAppName);

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hInstance = hInstance;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("SSA_VULKAN"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN)/2 - WIN_WIDTH/2,
		GetSystemMetrics(SM_CYSCREEN)/2 - WIN_HEIGHT/2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL, //Parent Window Handle
		NULL, //Menu Handle
		hInstance,
		NULL);

	ghwnd = hwnd;

	vkResult = Initialize();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "WinMain() --> Initialize() is failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "WinMain() --> Initialize() is succedded\n");
    }
    //Error checking of Initialize

	ShowWindow(hwnd, iCmdShow);
    Update();
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Game loop
	while (bDone == 0)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = 1;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == 1)
			{
				//Here you should call update() for OpenGL rendering
                Update();
				//Here you should call display() for OpenGL rendering
                if(gbWindowMinimized == FALSE)
                {
                    vkResult = Display();
                    if((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && (vkResult != VK_SUBOPTIMAL_KHR))
                    {
                        fprintf(gpFile, "WinMain() --> call to Display() failed\n");
                        bDone = TRUE;
                    }
                }
			}
		}
	}

	Uninitialize();

	return((int)(msg.wParam));
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//Local function declaration
	void ToggleFullscreen(void);
	VkResult Resize(int, int);
	void Uninitialize(void);

	int height = 0;
    int width = 0;
    int length = sizeof(WINDOWPLACEMENT);
    
    switch (iMsg)
	{

    case WM_CREATE:
        memset(&wpPrev, 0, length);
        break;

	case WM_SETFOCUS:
		gbActiveWindow = 1;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = 0;
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_SIZE:
        if(wParam == SIZE_MINIMIZED)
        {
            gbWindowMinimized = TRUE;
        }
        else
        {
            gbWindowMinimized = FALSE;
            Resize(LOWORD(lParam), HIWORD(lParam));
        }
        
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
		case 0x66:
			ToggleFullscreen();
			break;

		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		Uninitialize();
		PostQuitMessage(0);
		break;

	default: break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullscreen(void)
{
	//Local variable declaration
	MONITORINFO mi = { sizeof(MONITORINFO) };

	//Code
	if (gbFullscreen == 0) //If current window is normal(Not Fullscreen)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE); //dwStyle will get WS_OVERLAPPEDWINDOW information
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW)); //Removing flags of WS_OVERLAPPEDWINDOW
				SetWindowPos(ghwnd,
					HWND_TOP, //HWND_TOP -> for making it on top order
					mi.rcMonitor.left, // left coordinate of monitor
					mi.rcMonitor.top, // top coordinate of monitor
					mi.rcMonitor.right - mi.rcMonitor.left, // width coordinate of monitor
					mi.rcMonitor.bottom - mi.rcMonitor.top, // height coordinate of monitor
					SWP_NOZORDER | //Window flag --> don't change the Z order
					SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
			}
		}
		ShowCursor(FALSE);  //disappear the cursor in full screen or Game mode
		gbFullscreen = 1;
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
		SetWindowPlacement(ghwnd, &wpPrev); //setting placement of window
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			SWP_NOMOVE | // do not change x, y cordinates for starting position
			SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
		ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
		gbFullscreen = 0;
	}
}


VkResult Initialize(void)
{
    //function declaration
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVkInfo(void);
    VkResult createVulkanDevice(void);
    void getDeviceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(void);
    VkResult createVertexBuffer(void);
    VkResult createTexture(const char*);
    VkResult createUniformBuffer(void);
    VkResult createShaders(void);
    VkResult createDescriptorSetLayout(void);
    VkResult createPipelineLayout(void);
    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFrameBuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult buildCommandBuffers(void);
        
    //variable delaration
    VkResult vkResult = VK_SUCCESS;

    //code
    vkResult = createVulkanInstance();
    /*Main points:
        vkEnumerateInstanceExtensionProperties()
        
        struct --> vkApplicationInfo
        struct --> vkInstanceCreateInfo
        vkCreateInstance()
    */
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVulkanInstance() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVulkanInstance() is succedded\n");
    }

    //CreateVulkan Presntation Surface
    vkResult = getSupportedSurface();
    /* Main points:
        struct --> vkWin32SurfaceCreateInfoKHR
        vkCreateWin32SurfaceKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> getSupportedSurface() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> getSupportedSurface() is succedded\n");
    }

    //Get required Physical Device and its Queue family index
    vkResult = getPhysicalDevice();
    /* Main points:
        vkEnumeratePhysicalDevices()
        vkGetPhysicalDeviceQueueFamilyProperties()
        vkGetPhysicalDeviceSurfaceSupportKHR()
        vkGetPhysicalDeviceMemoryProperties()
        vkGetPhysicalDeviceFeatures()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> getPhysicalDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> getPhysicalDevice() is succedded\n");
    }
    
    //Printing Vulkan Infomration
    vkResult = printVkInfo();
    /* Main points:
        vkGetPhysicalDeviceProperties()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> printVkInfo() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> printVkInfo() is succedded\n");
    }
    
    vkResult = createVulkanDevice();
    /* Main Points:
        vkEnumerateDeviceExtensionProperties()
        struct --> vkDeviceQueueCreateInfo
        struct --> vkDeviceCreateInfo
        vkCreateDevice()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVulkanDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVulkanDevice() is succedded\n");
    }
    
    //getDeviceQueue
    getDeviceQueue();
    /* Main Points
        vkGetDeviceQueue()
    */

        
    //Swapchain
    vkResult = createSwapchain(VK_FALSE);
    /*Main Points:
        vkGetPhysicalDeviceSurfaceFormatsKHR()
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
        vkGetPhysicalDeviceSurfacePresentModesKHR()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createSwapchain() is succedded\n");
    }
    
    //Create Vulkan Images and ImageViews
    vkResult = createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createImagesAndImageViews() is succedded\n");
    }
    
    //Create Command Pool
    vkResult = createCommandPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createCommandPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createCommandPool() is succedded\n");
    }
    
    //Command Buffers
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createCommandBuffers() is succedded\n");
    }
    
    //create VertexBUffer
    vkResult = createVertexBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVertexBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVertexBuffer() is succedded\n");
    }
    
    //create texture
    vkResult = createTexture("Vijay_Kundali.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for Stone texture %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for Stone texture\n");
    }
    
    //create Uniform Buffer
    vkResult = createUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createUniformBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createUniformBuffer() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = createShaders();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createShaders() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createShaders() is succedded\n");
    }
    
    
    
    //Create Dewscriptor Set Layout
    vkResult = createDescriptorSetLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSetLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSetLayout() is succedded\n");
    }
    
    //Create Descriptor Set Layout
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createPipelineLayout() is succedded\n");
    }
    
    //create Descriptor pool
    vkResult = createDescriptorPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //create Descriptor set
    vkResult = createDescriptorSet();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSet() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createRenderPass() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createRenderPass() is succedded\n");
    }
    
    //Create Pipeline
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createPipeline() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createPipeline() is succedded\n");
    }
    
    
    //CreateBuffer
    vkResult = createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createFrameBuffers() is succedded\n");
    }
    
    //CreateSemaphores
    vkResult = createSemaphores();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createSemaphores() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createSemaphores() is succedded\n");
    }
    
    //CreateFences
    vkResult = createFences();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createFences() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createFences() is succedded\n");
    }
    
    //Initlaize s1.vkClearColorValue
    memset((void*)&s1.vkClearColorValue, 0, sizeof(s1.vkClearColorValue));
    s1.vkClearColorValue.float32[0] = 0.0f;// R
    s1.vkClearColorValue.float32[1] = 0.0f;// G
    s1.vkClearColorValue.float32[2] = 0.0f;// B
    s1.vkClearColorValue.float32[3] = 1.0f;//Analogous to glClearColor()
    
    memset((void*)&s1.vkClearDepthStencilValue, 0, sizeof(s1.vkClearDepthStencilValue));
    //setDefaultClearDepth
    s1.vkClearDepthStencilValue.depth = 1.0f; //flaot value
    //setDeafaultStencilValue
    s1.vkClearDepthStencilValue.stencil = 0; //uint32_t value
    
    //BuildCommandBuffers
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> buildCommandBuffers() is succedded\n");
    }
    
    //Initialization is completed
    s1.bInitialized = TRUE;
    fprintf(gpFile, "Initialize() --> Initialization is completed successfully\n");
    
    PlaySound(MAKEINTRESOURCE(MYAUDIO), NULL, SND_ASYNC | SND_RESOURCE);
  
    return vkResult;
}


VkResult Resize(int width, int height)
{
    //Function declarations
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandBuffers(void);
    VkResult createPipelineLayout(void);
    VkResult createPipeline(void);
    VkResult createRenderPass(void);
    VkResult createFrameBuffers(void);
    VkResult buildCommandBuffers(void);
    
    
    //variables
    VkResult vkResult = VK_SUCCESS;
	//code
	if (height == 0)
		height = 1;
    
    giHeight = height;
    giWidth = width;
    

    //check the s1.bInitialized variable
    if(s1.bInitialized == FALSE)
    {
        fprintf(gpFile, "Resize() --> Initialization yet not completed or failed\n");  
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    
    //As recreation of swapchain is needed we are goring to repeat many steps of initialized again, hence set s1.bInitialized to \False again
    s1.bInitialized = FALSE;

    //Set gloabl s1.winWidth and s1.winHeight variable
    s1.winWidth = width;
    s1.winHeight = height;
    
    //wait for device to complete inhand tasks
    vkDeviceWaitIdle(s1.vkDevice);
    fprintf(gpFile, "Resize() --> vkDeviceWaitIdle() is done\n");
    
    //check presence of swapchain
    if(s1.vkSwapchainKHR == VK_NULL_HANDLE)
    {
        fprintf(gpFile, "Resize() --> swapchain is already NULL cannot proceed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Destroy vkframebuffer
    for(uint32_t i = 0; i < s1.swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(s1.vkDevice, s1.vkFrameBuffer_array[i], NULL);
    }
    if(s1.vkFrameBuffer_array)
    {
        free(s1.vkFrameBuffer_array);
        s1.vkFrameBuffer_array = NULL;
        fprintf(gpFile, "Resize() --> s1.vkFrameBuffer_array() is done\n");
    }
    
    //Destroy Commandbuffer
    for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(s1.vkDevice, s1.vkCommandPool, 1, &s1.vkCommandBuffer_array[i]);
        fprintf(gpFile, "Resize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(s1.vkCommandBuffer_array)
    {
        free(s1.vkCommandBuffer_array);
        s1.vkCommandBuffer_array = NULL;
        fprintf(gpFile, "Resize() --> s1.vkCommandBuffer_array is freed\n");
    }
    
    
    //Destroy Pipeline
    if(s1.vkPipeline)
    {
        vkDestroyPipeline(s1.vkDevice, s1.vkPipeline, NULL);
        s1.vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkDestroyPipeline() is done\n");
    }
    
    //Destroy s1.vkPipelineLayout
    if(s1.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(s1.vkDevice, s1.vkPipelineLayout, NULL);
        s1.vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> s1.vkPipelineLayout() is done\n");
    }
    
    //Destroy Renderpass
    if(s1.vkRenderPass)
    {
        vkDestroyRenderPass(s1.vkDevice, s1.vkRenderPass, NULL);
        s1.vkRenderPass = VK_NULL_HANDLE;
    }
    
    //destroy depth image view
    if(s1.vkImageView_depth)
    {
        vkDestroyImageView(s1.vkDevice, s1.vkImageView_depth, NULL);
        s1.vkImageView_depth = VK_NULL_HANDLE;
    }
    
    //destroy device memory for depth image
    if(s1.vkDeviceMemory_depth)
    {
        vkFreeMemory(s1.vkDevice, s1.vkDeviceMemory_depth, NULL);
        s1.vkDeviceMemory_depth = VK_NULL_HANDLE;
    }
    
    if(s1.vkImage_depth)
    {
        vkDestroyImage(s1.vkDevice, s1.vkImage_depth, NULL);
        s1.vkImage_depth = VK_NULL_HANDLE;
    }

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // NEW: Destroy Offscreen Color A resources (added for Subpass 0 target)
    if (s1.vkImageView_colorA)
    {
        vkDestroyImageView(s1.vkDevice, s1.vkImageView_colorA, NULL);
        s1.vkImageView_colorA = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkDestroyImageView() for colorA done\n");
    }

    if (s1.vkDeviceMemory_colorA)
    {
        vkFreeMemory(s1.vkDevice, s1.vkDeviceMemory_colorA, NULL);
        s1.vkDeviceMemory_colorA = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkFreeMemory() for colorA done\n");
    }

    if (s1.vkImage_colorA)
    {
        vkDestroyImage(s1.vkDevice, s1.vkImage_colorA, NULL);
        s1.vkImage_colorA = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkDestroyImage() for colorA done\n");
    }
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    //destory image views
    for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkDestroyImageView(s1.vkDevice, s1.swapchainImageView_array[i], NULL);
        fprintf(gpFile, "Resize() --> vkDestoryImageView() is done\n");
    }
    
    if(s1.swapchainImageView_array)
    {
        free(s1.swapchainImageView_array);
        s1.swapchainImageView_array = NULL;
        fprintf(gpFile, "Resize() --> s1.swapchainImageView_array is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    // {
        // vkDestroyImage(s1.vkDevice, s1.swapchainImage_array[i], NULL);
        // fprintf(gpFile, "Resize() --> vkDestroyImage() is done\n");
    // }
    
    
    if(s1.swapchainImage_array)
    {
        free(s1.swapchainImage_array);
        s1.swapchainImage_array = NULL;
        fprintf(gpFile, "Resize() --> s1.swapchainImage_array is freed\n");
    }
    
    //Destory Swapchain
    if(s1.vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(s1.vkDevice, s1.vkSwapchainKHR, NULL);
        s1.vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkSwapchainCreateInfoKHR() is done\n");
    }
    
    //RECREATE FOR RESIZE
    
    //Swapchain
    vkResult = createSwapchain(VK_FALSE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Create Vulkan Images and ImageViews
    vkResult = createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    
    //Create RenderPass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createRenderPass() is failed %d\n", vkResult);
    }
    
    //Create Descriptor Set Layout
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    
    //Create Pipeline
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createPipeline() is failed %d\n", vkResult);
    }
      
    //CreateBuffer
    vkResult = createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    
    //Command Buffers
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    
    //BuildCommandBuffers
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }

    s1.bInitialized = TRUE;

    return vkResult;
}

VkResult Display(void)
{
    //Function Declarations
    VkResult Resize(int, int);
    VkResult updateUniformBuffer(void);
    
    //variable declarations
    VkResult vkResult = VK_SUCCESS; 

    //code
    // if control comes here before initlaization is completed, return FALSE
    if(s1.bInitialized == FALSE)
    {
        fprintf(gpFile, "Display() --> Initialization yet not completed\n");
        return (VkResult)VK_FALSE;
    }
    
    //acquire index of next swapchin image
    //if timour occurs, then function returns VK_NOT_READY
    vkResult = vkAcquireNextImageKHR(s1.vkDevice,
                                     s1.vkSwapchainKHR,
                                     UINT64_MAX, //waiting time in nanaoseconds for swapchain to get the image
                                     s1.vkSemaphore_backBuffer, //semaphore, waiting for another queue to relaease the image held by another queue demanded by swapchain, (InterQueue semaphore)
                                     VK_NULL_HANDLE, //Fence, when you want to halt host also, for device::: (Use Semaphore and fences exclusively, using both is not recommended(Redbook)
                                     &s1.currentImageIndex);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            Resize(s1.winWidth, s1.winHeight);
        }
        else
        {
            fprintf(gpFile, "Display() --> vkAcquireNextImageKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;   
        }
    }
    
    //use fence to allow host to wait for completion of execution of previous commandbuffer
    vkResult = vkWaitForFences(s1.vkDevice,
                               1, //waiting for how many fences
                               &s1.vkFence_array[s1.currentImageIndex], //Which fence
                               VK_TRUE, // wait till all fences get signalled(Blocking and unblocking function)
                               UINT64_MAX); //waiting time in nanaoseconds
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkWaitForFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }

    //Now make Fences execution of next command buffer
    vkResult = vkResetFences(s1.vkDevice,
                             1, //How many fences to reset
                             &s1.vkFence_array[s1.currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkResetFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //One of the mmeber of vkSubmitinfo structure requires array of pipeline stages, we have only one of the completion of color attachment output, still we need 1 member array
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    //Declare memset and initialize VkSubmitInfo structure
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &s1.vkSemaphore_backBuffer;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &s1.vkCommandBuffer_array[s1.currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &s1.vkSemaphore_renderComplete;
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(s1.vkQueue,
                             1,
                             &vkSubmitInfo,
                             s1.vkFence_array[s1.currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //We are going to present rendered image after declaring and initlaizing VkPresentInfoKHR structure
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &s1.vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &s1.currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &s1.vkSemaphore_renderComplete;
    
    //Now present the Queue
    vkResult = vkQueuePresentKHR(s1.vkQueue, &vkPresentInfoKHR);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            Resize(s1.winWidth, s1.winHeight);
        }
        else
        {
            fprintf(gpFile, "Display() --> vkQueuePresentKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;
        }
    }
    
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> updateUniformBuffer() is failed errorcode = %d\n", vkResult);
    }
        
    //validation
    vkDeviceWaitIdle(s1.vkDevice);    
    
    return vkResult;
}
    
void Update(void)
{
    if (Scene1_Effects::gEffectDone) { s1_effect.gFade = 0.0f; return; }

    if (!Scene1_Effects::gEffectStarted) {
        Scene1_Effects::gEffectT0   = std::chrono::steady_clock::now();
        Scene1_Effects::gEffectStarted = true;
    }

    auto  now = std::chrono::steady_clock::now();
    float t   = std::chrono::duration<float>(now - Scene1_Effects::gEffectT0).count(); // seconds

    const float t1 = Scene1_Effects::kBlackSec;
    const float t2 = t1 + Scene1_Effects::kFadeInSec;
    const float t3 = t2 + Scene1_Effects::kHoldVisibleSec;
    const float t4 = t3 + Scene1_Effects::kFadeOutSec;

    if (t < t1) {
        s1_effect.gFade = 0.0f;
    } else if (t < t2) {
        float u = (t - t1) / Scene1_Effects::kFadeInSec;
        s1_effect.gFade = glm::clamp(u, 0.0f, 1.0f);
    } else if (t < t3) {
        s1_effect.gFade = 1.0f;
    } else if (t < t4) {
        float u = (t - t3) / Scene1_Effects::kFadeOutSec;
        s1_effect.gFade = glm::clamp(1.0f - u, 0.0f, 1.0f);
    } else {
        s1_effect.gFade = 0.0f;
        Scene1_Effects::gEffectDone = true;

        // For looping, use:
        // Scene1_Effects::gEffectT0    = now;
        // Scene1_Effects::gEffectStarted = true;
        // Scene1_Effects::gEffectDone    = false;
    }
}

void Uninitialize(void)
{
	//code
	if (gbFullscreen == 1)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
		SetWindowPlacement(ghwnd, &wpPrev); //setting placement of window
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			SWP_NOMOVE | // do not change x, y cordinates for starting position
			SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
		ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
	}

	/**************Shader removal code*******************/

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

    //synchronisation function
    vkDeviceWaitIdle(s1.vkDevice);
    fprintf(gpFile, "Uninitialize() --> vkDeviceWaitIdle() is done\n");
    
    //DestroyFences
    for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkDestroyFence(s1.vkDevice, s1.vkFence_array[i], NULL);
    }
    
    if(s1.vkFence_array)
    {
        free(s1.vkFence_array);
        s1.vkFence_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkDestroyFence() is done\n");
    }
    
    //DestroySemaphore
    if(s1.vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(s1.vkDevice, s1.vkSemaphore_renderComplete, NULL);
        s1.vkSemaphore_renderComplete = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySemaphore() for s1.vkSemaphore_renderComplete is done\n");
    }
    
    if(s1.vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(s1.vkDevice, s1.vkSemaphore_backBuffer, NULL);
        s1.vkSemaphore_backBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySemaphore() for s1.vkSemaphore_backBuffer is done\n");
    }
    
    //vkframebuffer
    for(uint32_t i = 0; i < s1.swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(s1.vkDevice, s1.vkFrameBuffer_array[i], NULL);
    }
    if(s1.vkFrameBuffer_array)
    {
        free(s1.vkFrameBuffer_array);
        s1.vkFrameBuffer_array = NULL;
        fprintf(gpFile, "Uninitialize() --> s1.vkFrameBuffer_array() is done\n");
    }
    
    //Destroy Descriptor Pool
    //When Descriptor pool is destroyed, descriptor set created  by that pool get destroyed implicitly
    if(s1.vkDescriptorPool)
    {
        vkDestroyDescriptorPool(s1.vkDevice, s1.vkDescriptorPool, NULL);        
        s1.vkDescriptorPool = VK_NULL_HANDLE;
        s1.vkDescriptorSet = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDescriptorPool and DescriptorSet() is done\n");
    }
    
    //VkPipelineLayout
    if(s1.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(s1.vkDevice, s1.vkPipelineLayout, NULL);
        s1.vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> s1.vkPipelineLayout() is done\n");
    }
    
    //VkDescriptorSetLayout
    if(s1.vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(s1.vkDevice, s1.vkDescriptorSetLayout, NULL);
        s1.vkDescriptorSetLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDescriptorSetLayout() is done\n");
    }
    
    if(s1.vkPipeline)
    {
        vkDestroyPipeline(s1.vkDevice, s1.vkPipeline, NULL);
        s1.vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyPipeline() is done\n");
    }
    
    //Renderpass
    if(s1.vkRenderPass)
    {
        vkDestroyRenderPass(s1.vkDevice, s1.vkRenderPass, NULL);
        s1.vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyRenderPass() is done\n");
    }
    
    //destroy shader modules
    if(s1.vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(s1.vkDevice, s1.vkShaderModule_fragment_shader, NULL);
        s1.vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    if(s1.vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(s1.vkDevice, s1.vkShaderModule_vertex_shader, NULL);
        s1.vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    //Destroy uniform buffer
    if(s1.uniformData.vkBuffer)
    {
        vkDestroyBuffer(s1.vkDevice, s1.uniformData.vkBuffer, NULL);
        s1.uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer()  for uniformData.vkBuffer is done\n");
    }
    
    if(s1.uniformData.vkDeviceMemory)
    {
        vkFreeMemory(s1.vkDevice, s1.uniformData.vkDeviceMemory, NULL);
        s1.uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory()  for uniformData.vkDeviceMemory is done\n");
        
    }
    
    if(s1.vkSampler_texture)
    {
        vkDestroySampler(s1.vkDevice, s1.vkSampler_texture, NULL);
        s1.vkSampler_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySampler() for vkSampler_texture is done\n");
    }
    
    if(s1.vkImageView_texture)
    {
        vkDestroyImageView(s1.vkDevice, s1.vkImageView_texture, NULL);
        s1.vkImageView_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySampler() for vkImageView_texture is done\n");
    }
    
    if(s1.vkDeviceMemory_texture)
    {
        vkFreeMemory(s1.vkDevice, s1.vkDeviceMemory_texture, NULL);
        s1.vkDeviceMemory_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() for vkDeviceMemory_texture is done\n");
    }
    
    if(s1.vkImage_texture)
    {
        vkDestroyImage(s1.vkDevice, s1.vkImage_texture, NULL);
        s1.vkImage_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyImage() for vkImage_texture is done\n");
    }

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // NEW: Destroy Offscreen Color A resources (added for Subpass 0 target)
    if (s1.vkImageView_colorA)
    {
        vkDestroyImageView(s1.vkDevice, s1.vkImageView_colorA, NULL);
        s1.vkImageView_colorA = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyImageView() for colorA done\n");
    }

    if (s1.vkDeviceMemory_colorA)
    {
        vkFreeMemory(s1.vkDevice, s1.vkDeviceMemory_colorA, NULL);
        s1.vkDeviceMemory_colorA = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() for colorA done\n");
    }

    if (s1.vkImage_colorA)
    {
        vkDestroyImage(s1.vkDevice, s1.vkImage_colorA, NULL);
        s1.vkImage_colorA = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyImage() for colorA done\n");
    }
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    
    //Vertex Tecoord buffer
    if(s1.vertexData_texcoord.vkDeviceMemory)
    {
        vkFreeMemory(s1.vkDevice, s1.vertexData_texcoord.vkDeviceMemory, NULL);
        s1.vertexData_texcoord.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(s1.vertexData_texcoord.vkBuffer)
    {
        vkDestroyBuffer(s1.vkDevice, s1.vertexData_texcoord.vkBuffer, NULL);
        s1.vertexData_texcoord.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    //Vertex position BUffer
    if(s1.vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(s1.vkDevice, s1.vertexData_position.vkDeviceMemory, NULL);
        s1.vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(s1.vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(s1.vkDevice, s1.vertexData_position.vkBuffer, NULL);
        s1.vertexData_position.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    
    for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(s1.vkDevice, s1.vkCommandPool, 1, &s1.vkCommandBuffer_array[i]);
        fprintf(gpFile, "Uninitialize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(s1.vkCommandBuffer_array)
    {
        free(s1.vkCommandBuffer_array);
        s1.vkCommandBuffer_array = NULL;
        fprintf(gpFile, "Uninitialize() --> s1.vkCommandBuffer_array is freed\n");
    }
    
    
    if(s1.vkCommandPool)
    {
        vkDestroyCommandPool(s1.vkDevice, s1.vkCommandPool, NULL);
        s1.vkCommandPool = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyCommandPool() is done\n");
    }
    
    //destroy depth image view
    if(s1.vkImageView_depth)
    {
        vkDestroyImageView(s1.vkDevice, s1.vkImageView_depth, NULL);
        s1.vkImageView_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestoryImageView() is done for depth\n");
    }
    
    //destroy device memory for depth image
    if(s1.vkDeviceMemory_depth)
    {
        vkFreeMemory(s1.vkDevice, s1.vkDeviceMemory_depth, NULL);
        s1.vkDeviceMemory_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() is done for depth\n");
    }
    
    if(s1.vkImage_depth)
    {
        vkDestroyImage(s1.vkDevice, s1.vkImage_depth, NULL);
        s1.vkImage_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestoryImage() is done for depth\n");
    }
    
    //destory image views
    for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkDestroyImageView(s1.vkDevice, s1.swapchainImageView_array[i], NULL);
        fprintf(gpFile, "Uninitialize() --> vkDestoryImageView() is done for color\n");
    }
    
    if(s1.swapchainImageView_array)
    {
        free(s1.swapchainImageView_array);
        s1.swapchainImageView_array = NULL;
        fprintf(gpFile, "Uninitialize() --> s1.swapchainImageView_array is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    // {
    //     vkDestroyImage(s1.vkDevice, s1.swapchainImage_array[i], NULL);
    //     fprintf(gpFile, "Uninitialize() --> vkDestroyImage() is done\n");
    // }
    
    if(s1.swapchainImage_array)
    {
        free(s1.swapchainImage_array);
        s1.swapchainImage_array = NULL;
        fprintf(gpFile, "Uninitialize() --> s1.swapchainImage_array is freed\n");
    }
    
    //Destory Swapchain
    if(s1.vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(s1.vkDevice, s1.vkSwapchainKHR, NULL);
        s1.vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkSwapchainCreateInfoKHR() is done\n");
    }

    //No need to Destroy/Uninitialize the DeviceQueue

    //Destroy vulkan device
    if(s1.vkDevice)
    {
        vkDestroyDevice(s1.vkDevice, NULL);
        s1.vkDevice = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestoryDevice() is done\n");
    }

    //No need to free slected physical device

    //Destroy vkSurfaceKHR:This function is generic and not platform specific
    if(s1.vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(s1.vkInstance, s1.vkSurfaceKHR, NULL);
        s1.vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySurfaceKHR() is done\n");
    }
    
    //Validation destroying
    if(s1.vkDebugReportCallbackEXT && s1.vkDestroyDebugReportCallbackEXT_fnptr)
    {
        s1.vkDestroyDebugReportCallbackEXT_fnptr(s1.vkInstance, 
                                              s1.vkDebugReportCallbackEXT,
                                              NULL);
        s1.vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        s1.vkDestroyDebugReportCallbackEXT_fnptr = NULL;
    }
    
    //Destroy Vulkan Instance
    if(s1.vkInstance)
    {
        vkDestroyInstance(s1.vkInstance, NULL);
        s1.vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyInstance() is done\n");
    }
    
    if (gpFile)
	{
		fprintf(gpFile, "Uninitialize() --> Program terminated successfully\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

/***************Definition of Vulkan functions***********************/
/********************************************************************/

VkResult createVulkanInstance(void)
{
    //Function declarations
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    //variable declarations
    VkResult vkResult = VK_SUCCESS;

    //Step1: Fill and initialize required extension names and count in global variable
    vkResult = fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is failed\n");
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is succedded\n");
    }

    if(s1.bValidation == TRUE)
    {
        //Fill validation Layer Names
        vkResult = fillValidationLayerNames();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance() --> fillValidationLayerNames() is failed\n");
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance() --> fillValidationLayerNames() is succedded\n");
        }
    }

    //Step1: Initialize struct VkApplicationInfo
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //type safety, generic names
    vkApplicationInfo.pNext = NULL; //Linked List
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

    //Step3: Initialize struct VkInstanceCreateInfo by using information in Step1 and Step2
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = s1.enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = s1.enabledInstanceExtensionNames_array;
    
    if(s1.bValidation == TRUE)
    {
        vkInstanceCreateInfo.enabledLayerCount = s1.enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = s1.enabledValidationLayerNames_array;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    //Step4: Call VkCreateInstance() to get vkInstance in a global variable and do error checking
    vkResult = vkCreateInstance(&vkInstanceCreateInfo,
                                 NULL,  //no custom Memory allocater
                                 &s1.vkInstance);
    if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to incompatible driver %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to required extension not present %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to unknown reason %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance succedded\n");
    }
    
    //do for validation callbacks
    if(s1.bValidation == TRUE)
    {
        vkResult = createValidationCallbackFunction();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is failed\n");
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is succedded\n");
        }
    }

    return vkResult;
}

VkResult fillInstanceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Instacne Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t instanceExtensionCount = 0;

    vkResult = vkEnumerateInstanceExtensionProperties(NULL,
                                                      &instanceExtensionCount,
                                                      NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }

    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * instanceExtensionCount);

    vkResult = vkEnumerateInstanceExtensionProperties(NULL,
                                                      &instanceExtensionCount,
                                                      vkExtensionProperties_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }

    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** instanceExtensionNames_array = NULL;

    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillInstanceExtensionNames() --> Vulkan Instance Extension names = %s\n", instanceExtensionNames_array[i]);
    }

   //Step4: As not required henceforth, free the vkExtensionProperties_array;
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;

   // Step5: Find whether below extension names contains our required two extensions
   //VK_KHR_SURFACE_EXTENSION_NAME
   //VK_KHR_WIN32_SURFACE_EXTENSION_NAME
   VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
   VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;
   VkBool32 vulkanDebugReportExtensionFound = VK_FALSE;

    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            s1.enabledInstanceExtensionNames_array[s1.enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanWin32SurfaceExtensionFound = VK_TRUE;
            s1.enabledInstanceExtensionNames_array[s1.enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            vulkanDebugReportExtensionFound = VK_TRUE;
            if(s1.bValidation == TRUE)
            {
                s1.enabledInstanceExtensionNames_array[s1.enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
            else
            {
                //array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
        }
    }

    //Step 6:
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        free(instanceExtensionNames_array[i]);
    }
    free(instanceExtensionNames_array);

    //Step7:Print whether our vulkan driver supports our required extension names or not
    if(vulkanSurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME found\n");
    }

    if(vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found\n");
    }
    
    if(vulkanDebugReportExtensionFound == VK_FALSE)
    {
        if(s1.bValidation == TRUE)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is ON But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is OFF But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
        }
    }
    else
    {
        if(s1.bValidation == TRUE)
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
    }

    //Step8: Print only Enabled Extension Names 
    for(uint32_t i = 0; i < s1.enabledInstanceExtensionCount; i++)
    {
         fprintf(gpFile, "fillInstanceExtensionNames() --> Enabled vulkan Instance extension Names = %s\n", s1.enabledInstanceExtensionNames_array[i]);
    }

    return vkResult;
}

VkResult fillValidationLayerNames(void)
{
    //code
    //variables
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;
    
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    VkLayerProperties* vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*) malloc (sizeof(VkLayerProperties) * validationLayerCount);

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  vkLayerProperties_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    char** validationLayerNames_array = NULL;
    validationLayerNames_array = (char**) malloc(sizeof(char*) * validationLayerCount);
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
        fprintf(gpFile, "fillValidationLayerNames() --> Vulkan Validation Layer names = %s\n", vkLayerProperties_array[i].layerName);
    }
    
    if(vkLayerProperties_array) 
        free(vkLayerProperties_array);
    vkLayerProperties_array = NULL;

    // Step5: Find whether below layer names contains our required two extensions
    VkBool32 vulkanValidationLayerFound = VK_FALSE;
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        if(strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            vulkanValidationLayerFound = VK_TRUE;
            s1.enabledValidationLayerNames_array[s1.enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }
    
     //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);
    
    if(s1.bValidation == TRUE)
    {
        if(vulkanValidationLayerFound == VK_FALSE)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation is supported\n");
        }
    }
    
    //Step8: Print only Enabled validation layer Names
    for(uint32_t i = 0; i < s1.enabledValidationLayerCount; i++)
    {
         fprintf(gpFile, "fillValidationLayerNames() --> Enabled vulkan validation layer Names = %s\n", s1.enabledValidationLayerNames_array[i]);
    }
    
    return (vkResult);    
}

VkResult createValidationCallbackFunction(void)
{
    //code
    //function declarations
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, 
                                                       VkDebugReportObjectTypeEXT,
                                                       uint64_t,
                                                       size_t,
                                                       int32_t,
                                                       const char*,
                                                       const char*,
                                                       void*);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;
    
    //Get the required function pointers
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(s1.vkInstance, "vkCreateDebugReportCallbackEXT");
    if(vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is failed to get function pointer for vkCreateDebugReportCallbackEXT \n");
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is succedded to get function pointer for vkCreateDebugReportCallbackEXT \n");
    }
    
    // Assign to the global function pointer (no local shadowing)
    s1.vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(s1.vkInstance, "vkDestroyDebugReportCallbackEXT");
    if(s1.vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is failed to get function pointer for vkDestroyDebugReportCallbackEXT \n");
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is succedded to get function pointer for vkDestroyDebugReportCallbackEXT \n");
    }
    
    //Get the vulkanDebugReportCallback object
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;
    
    vkResult = vkCreateDebugReportCallbackEXT_fnptr(s1.vkInstance,
                                                    &vkDebugReportCallbackCreateInfoEXT,
                                                    NULL,
                                                    &s1.vkDebugReportCallbackEXT);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT_fnptr() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT_fnptr() is succedded\n");
    }
    
    return (vkResult);
}

VkResult getSupportedSurface(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step2
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    //Step3:
    vkResult = vkCreateWin32SurfaceKHR(s1.vkInstance,
                                       &vkWin32SurfaceCreateInfoKHR,
                                       NULL, //Memory mamnagement function is default
                                       &s1.vkSurfaceKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is succedded\n");
    }

    return vkResult;
}


VkResult getPhysicalDevice(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;

    //code
    vkResult = vkEnumeratePhysicalDevices(s1.vkInstance,
                                          &s1.physicalDeviceCount,
                                          NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(s1.physicalDeviceCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() resulted in zero devices\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    s1.vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * s1.physicalDeviceCount);
    //error checking to be done
    
    vkResult = vkEnumeratePhysicalDevices(s1.vkInstance, 
                                          &s1.physicalDeviceCount,
                                          s1.vkPhysicalDevice_array);
     if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    VkBool32 bFound = VK_FALSE;
    for(uint32_t i = 0; i < s1.physicalDeviceCount; i++)
    {
        uint32_t qCount = UINT32_MAX;
        
        //If physical device is present then it must support at least 1 queue family
        vkGetPhysicalDeviceQueueFamilyProperties(s1.vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * qCount);
        
        vkGetPhysicalDeviceQueueFamilyProperties(s1.vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               vkQueueFamilyProperties_array);
        
        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * qCount);
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(s1.vkPhysicalDevice_array[i], 
                                                 j,
                                                 s1.vkSurfaceKHR,
                                                 &isQueueSurfaceSupported_array[j]);
        }
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if(isQueueSurfaceSupported_array[j] == VK_TRUE)
                {
                    s1.vkPhysicalDevice_selected = s1.vkPhysicalDevice_array[i];
                    s1.graphicsQueueFamilyIndex_selected = j;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }
        
        if(isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> isQueueSurfaceSupported_array succedded to free\n");
        }
        
        if(vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> vkQueueFamilyProperties_array succedded to free\n");
        }
        
        if(bFound == VK_TRUE)
        {
            break;
        }
    }
    
    if(bFound == VK_TRUE)
    {
        fprintf(gpFile, "getPhysicalDevice() -->is succedded to select the required device with graphics enabled\n");
    }
    else
    {
        if(s1.vkPhysicalDevice_array)
        {
            free(s1.vkPhysicalDevice_array);
            s1.vkPhysicalDevice_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> vkPhysicalDevice_array succedded to free\n");
        }
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "getPhysicalDevice() -->is failed to select the required device with graphics enabled\n");
    }
    
    memset((void*)&s1.vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    vkGetPhysicalDeviceMemoryProperties(s1.vkPhysicalDevice_selected, 
                                        &s1.vkPhysicalDeviceMemoryProperties);
                                        
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    
    vkGetPhysicalDeviceFeatures(s1.vkPhysicalDevice_selected, 
                                &vkPhysicalDeviceFeatures);
                                
    if(vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device supports tessellationShader\n");
    }
    else
    {
          fprintf(gpFile, "getPhysicalDevice() -->selected device not supports tessellationShader\n");
    }
    
    if(vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device supports geometryShader\n");
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device not supports geometryShader\n");
    }
    
    return vkResult;
}


VkResult printVkInfo(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //code
    fprintf(gpFile, "*******************VULKAN INFORMATION*********************\n");
    for(uint32_t i = 0; i < s1.physicalDeviceCount; i++)    
    {
        fprintf(gpFile, "Infomration of Device = %d\n", i);
        
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        
        vkGetPhysicalDeviceProperties(s1.vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);
        
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);;
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);;
        //API Version
        fprintf(gpFile, "apiVersion = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
        
        //Device Name
        fprintf(gpFile, "DeviceName = %s\n", vkPhysicalDeviceProperties.deviceName);
        
        //DeviceType
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
                fprintf(gpFile, "DeviceType = Integrated GPU(iGPU)\n");
                break;
            
            case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
                fprintf(gpFile, "DeviceType = Discrete GPU(dGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
                fprintf(gpFile, "DeviceType = Virtual GPU(vGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_CPU):
                fprintf(gpFile, "DeviceType = CPU\n");
                break;    
                
            case(VK_PHYSICAL_DEVICE_TYPE_OTHER):
                fprintf(gpFile, "DeviceType = Other\n");
                break; 
                
            default: 
                fprintf(gpFile, "DeviceType = UNKNOWN\n");                
        }
        
        //Vendor Id
        fprintf(gpFile, "VendorId = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);
        
        //DeviceId
        fprintf(gpFile, "DeviceId = 0x%04x\n\n", vkPhysicalDeviceProperties.deviceID);
   }
   
   fprintf(gpFile, "****************END OF VULKAN INFORMATION********************\n");
   
    //Freephysical device array
    if(s1.vkPhysicalDevice_array)
    {
        free(s1.vkPhysicalDevice_array);
        s1.vkPhysicalDevice_array = NULL;
        fprintf(gpFile, "printVkInfo() --> vkPhysicalDevice_array succedded to free\n");
    }
    
    return vkResult;
}


VkResult fillDeviceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Device Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t deviceExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(s1.vkPhysicalDevice_selected,
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is succedded\n");
        fprintf(gpFile, "deviceExtensionCount is %u\n", deviceExtensionCount);
    }

    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * deviceExtensionCount);

    vkResult = vkEnumerateDeviceExtensionProperties(s1.vkPhysicalDevice_selected,
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is succedded\n");
    }

    //Step3: Fill and Display a local string array of extension names
    char** deviceExtensionNames_array = NULL;

    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillDeviceExtensionNames() --> Vulkan Device Extension names = %s\n", deviceExtensionNames_array[i]);
    }

   //Step4: free the vkExtensionProperties_array;
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;

   // Step5: Look for VK_KHR_swapchain
   VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;
  
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if(strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapChainExtensionFound = VK_TRUE;
            s1.enabledDeviceExtensionNames_array[s1.enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }

    //Step 6: free local string array
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        free(deviceExtensionNames_array[i]);
    }
    free(deviceExtensionNames_array);

    //Step7: ensure present
    if(vulkanSwapChainExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME found\n");
    }

    //Step8: Print only Enabled Extension Names
    for(uint32_t i = 0; i < s1.enabledDeviceExtensionCount; i++)
    {
         fprintf(gpFile, "fillDeviceExtensionNames() --> Enabled vulkan Device extension Names = %s\n", s1.enabledDeviceExtensionNames_array[i]);
    }

    return vkResult;
}

VkResult createVulkanDevice(void)
{  
    //Function declarations
    VkResult fillDeviceExtensionNames(void);
    
    //variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //Fill Device Extensions
    vkResult = fillDeviceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is succedded\n");
    }
    
    /////Newly added code//////
    
    float QueuePriorities[] = {1.0f};
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = s1.graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = QueuePriorities;
    
    //Initialize VkDeviceCreateinfo structure
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = s1.enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = s1.enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0;  // Deprecated
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;  // Deprecated
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
        
    vkResult = vkCreateDevice(s1.vkPhysicalDevice_selected,
                              &vkDeviceCreateInfo,
                              NULL,
                              &s1.vkDevice);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanDevice() --> vkCreateDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanDevice() --> vkCreateDevice() is succedded\n");
    }                     
    
    return vkResult;
}

void getDeviceQueue(void)
{
    //code
    vkGetDeviceQueue(s1.vkDevice, 
                     s1.graphicsQueueFamilyIndex_selected,
                     0, //0th Queue index in that family queue
                     &s1.vkQueue);
    if(s1.vkQueue == VK_NULL_HANDLE) //rarest possibility
    {
        fprintf(gpFile, "getDeviceQueue() --> vkGetDeviceQueue() returned NULL for s1.vkQueue\n");
        return;
    }
    else
    {
        fprintf(gpFile, "getDeviceQueue() --> vkGetDeviceQueue() is succedded\n");
    }
}


VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    uint32_t formatCount = 0;
    
    //get the count of supported SurfaceColorFormats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(s1.vkPhysicalDevice_selected,
                                                    s1.vkSurfaceKHR,
                                                    &formatCount,
                                                    NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(formatCount == 0)
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed as formatCount is zero:: %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
    }
    
    fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> total formatCount are:: %d\n", formatCount);
    
    VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*) malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    
    //Fill the array
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(s1.vkPhysicalDevice_selected,
                                                    s1.vkSurfaceKHR,
                                                    &formatCount,
                                                    vkSurfaceFormatKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        free(vkSurfaceFormatKHR_array);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
    }
    
    if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
    {
        s1.vkFormat_color = VK_FORMAT_R8G8B8A8_UNORM;
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkFormat_color is VK_FORMAT_B8G8R8A8_UNORM\n");
    }
    else
    {
        s1.vkFormat_color = vkSurfaceFormatKHR_array[0].format;
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkFormat_color is %d\n", s1.vkFormat_color);
    }
    
    //Decide the ColorSpace
    s1.vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
    
    free(vkSurfaceFormatKHR_array);
    fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkSurfaceFormatKHR_array is freed\n");
    
    return vkResult;
}


VkResult getPhysicalDevicePresentMode(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    uint32_t presentModeCount = 0;
    
    //query modes
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(s1.vkPhysicalDevice_selected,
                                                         s1.vkSurfaceKHR,
                                                         &presentModeCount,
                                                         NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(presentModeCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> presentModeCount is zero %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }   

    fprintf(gpFile, "getPhysicalDevicePresentMode() --> total presentModeCount are:: %d\n", presentModeCount);

    VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*) malloc(presentModeCount * sizeof(VkPresentModeKHR));
    
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(s1.vkPhysicalDevice_selected,
                                                         s1.vkSurfaceKHR,
                                                         &presentModeCount,
                                                         vkPresentModeKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        free(vkPresentModeKHR_array);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }
    
    //Prefer MAILBOX, else FIFO
    s1.vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
    for(uint32_t i = 0; i < presentModeCount; i++)
    {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            s1.vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR is VK_PRESENT_MODE_MAILBOX_KHR\n");
            break;
        }
    }
    if(s1.vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        s1.vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR is VK_PRESENT_MODE_FIFO_KHR\n");
    }
  
    free(vkPresentModeKHR_array);
    fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR_array is freed\n");
    return vkResult;
}


VkResult createSwapchain(VkBool32 vsync)  // vertical sync
{
    VkResult vkResult = VK_SUCCESS;

    // 1) Choose surface format + color space (prefer SRGB)
    uint32_t formatCount = 0;
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        s1.vkPhysicalDevice_selected, s1.vkSurfaceKHR, &formatCount, NULL);
    if (vkResult != VK_SUCCESS || formatCount == 0)
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceFormatsKHR() failed or returned 0 formats (%d)\n", vkResult);
        return (formatCount == 0) ? VK_ERROR_INITIALIZATION_FAILED : vkResult;
    }

    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        s1.vkPhysicalDevice_selected, s1.vkSurfaceKHR, &formatCount, formats);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> 2nd vkGetPhysicalDeviceSurfaceFormatsKHR() failed (%d)\n", vkResult);
        free(formats);
        return vkResult;
    }

    VkSurfaceFormatKHR chosenFmt = formats[0];
    if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        chosenFmt.format     = VK_FORMAT_B8G8R8A8_SRGB;
        chosenFmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        for (uint32_t i = 0; i < formatCount; ++i)
        {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                chosenFmt = formats[i];
                break;
            }
        }
        if (chosenFmt.format != VK_FORMAT_B8G8R8A8_SRGB ||
            chosenFmt.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            for (uint32_t i = 0; i < formatCount; ++i)
            {
                if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
                    formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    chosenFmt = formats[i];
                    break;
                }
            }
        }
    }

    s1.vkFormat_color   = chosenFmt.format;
    s1.vkColorSpaceKHR  = chosenFmt.colorSpace;

    fprintf(gpFile, "createSwapchain() --> Chosen surface format=%d, colorspace=%d\n",
            (int)s1.vkFormat_color, (int)s1.vkColorSpaceKHR);

    free(formats);
    formats = NULL;

    // 2) Query surface capabilities
    VkSurfaceCapabilitiesKHR caps;
    memset(&caps, 0, sizeof(caps));
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        s1.vkPhysicalDevice_selected, s1.vkSurfaceKHR, &caps);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed (%d)\n", vkResult);
        return vkResult;
    }

    // 3) Decide image count
    uint32_t desiredImageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && desiredImageCount > caps.maxImageCount)
        desiredImageCount = caps.maxImageCount;

    // 4) Decide extent
    if (caps.currentExtent.width != UINT32_MAX)
    {
        s1.vkExtent2D_swapchain = caps.currentExtent;
    }
    else
    {
        VkExtent2D wanted;
        wanted.width  = (uint32_t)s1.winWidth;
        wanted.height = (uint32_t)s1.winHeight;

        s1.vkExtent2D_swapchain.width  =
            (wanted.width  < caps.minImageExtent.width ) ? caps.minImageExtent.width  :
            (wanted.width  > caps.maxImageExtent.width ) ? caps.maxImageExtent.width  : wanted.width;

        s1.vkExtent2D_swapchain.height =
            (wanted.height < caps.minImageExtent.height) ? caps.minImageExtent.height :
            (wanted.height > caps.maxImageExtent.height) ? caps.maxImageExtent.height : wanted.height;
    }
    fprintf(gpFile, "createSwapchain() --> Extent: %ux%u\n",
            s1.vkExtent2D_swapchain.width, s1.vkExtent2D_swapchain.height);

    // 5) Usage
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // 6) Pre-transform
    VkSurfaceTransformFlagBitsKHR preTransform =
        (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        : caps.currentTransform;

    // 7) Present mode (honor vsync)
    uint32_t presentModeCount = 0;
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
        s1.vkPhysicalDevice_selected, s1.vkSurfaceKHR, &presentModeCount, NULL);
    if (vkResult != VK_SUCCESS || presentModeCount == 0)
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfacePresentModesKHR() failed or 0 modes (%d)\n", vkResult);
        return (presentModeCount == 0) ? VK_ERROR_INITIALIZATION_FAILED : vkResult;
    }

    VkPresentModeKHR* modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
        s1.vkPhysicalDevice_selected, s1.vkSurfaceKHR, &presentModeCount, modes);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> 2nd vkGetPhysicalDeviceSurfacePresentModesKHR() failed (%d)\n", vkResult);
        free(modes);
        return vkResult;
    }

    // default vsync on: FIFO
    VkPresentModeKHR chosenPresent = VK_PRESENT_MODE_FIFO_KHR;

    if (!vsync)
    {
        VkBool32 hasMailbox = VK_FALSE, hasImmediate = VK_FALSE;
        for (uint32_t i = 0; i < presentModeCount; ++i)
        {
            if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)   hasMailbox   = VK_TRUE;
            if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) hasImmediate = VK_TRUE;
        }
        if (hasMailbox)   chosenPresent = VK_PRESENT_MODE_MAILBOX_KHR;
        else if (hasImmediate) chosenPresent = VK_PRESENT_MODE_IMMEDIATE_KHR;
        else chosenPresent = VK_PRESENT_MODE_FIFO_KHR;
    }
    else
    {
        VkBool32 hasFifo = VK_FALSE;
        for (uint32_t i = 0; i < presentModeCount; ++i)
            if (modes[i] == VK_PRESENT_MODE_FIFO_KHR) { hasFifo = VK_TRUE; break; }
        chosenPresent = hasFifo ? VK_PRESENT_MODE_FIFO_KHR : modes[0];
    }

    s1.vkPresentModeKHR = chosenPresent;

    const char* pmName = "UNKNOWN";
    switch (s1.vkPresentModeKHR)
    {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:      pmName = "IMMEDIATE"; break;
        case VK_PRESENT_MODE_MAILBOX_KHR:        pmName = "MAILBOX";   break;
        case VK_PRESENT_MODE_FIFO_KHR:           pmName = "FIFO";      break;
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:   pmName = "FIFO_RELAXED"; break;
        default: break;
    }
    fprintf(gpFile, "createSwapchain() --> Present mode: %s (vsync=%s)\n",
            pmName, vsync ? "ON" : "OFF");

    free(modes);
    modes = NULL;

    // 8) Composite alpha (prefer OPAQUE)
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(caps.supportedCompositeAlpha & compositeAlpha))
    {
        const VkCompositeAlphaFlagBitsKHR candidates[] = {
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };
        for (uint32_t i = 0; i < (uint32_t)(sizeof(candidates)/sizeof(candidates[0])); ++i)
        {
            if (caps.supportedCompositeAlpha & candidates[i])
            {
                compositeAlpha = candidates[i];
                break;
            }
        }
    }

    // 9) Create the swapchain
    VkSwapchainCreateInfoKHR ci;
    memset(&ci, 0, sizeof(ci));
    ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface          = s1.vkSurfaceKHR;
    ci.minImageCount    = desiredImageCount;
    ci.imageFormat      = s1.vkFormat_color;
    ci.imageColorSpace  = s1.vkColorSpaceKHR;
    ci.imageExtent      = s1.vkExtent2D_swapchain;
    ci.imageArrayLayers = 1;
    ci.imageUsage       = usage;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform     = preTransform;
    ci.compositeAlpha   = compositeAlpha;
    ci.presentMode      = s1.vkPresentModeKHR;
    ci.clipped          = VK_TRUE;
    ci.oldSwapchain     = VK_NULL_HANDLE;

    vkResult = vkCreateSwapchainKHR(s1.vkDevice, &ci, NULL, &s1.vkSwapchainKHR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> vkCreateSwapchainKHR() failed (%d)\n", vkResult);
        return vkResult;
    }

    fprintf(gpFile, "createSwapchain() --> Swapchain created: images=%u, format=%d, %ux%u, present=%s\n",
            ci.minImageCount, (int)ci.imageFormat, ci.imageExtent.width, ci.imageExtent.height, pmName);

    return VK_SUCCESS;
}

static inline VkBool32 HasStencilComponent(VkFormat fmt) {
    return (fmt == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
           (fmt == VK_FORMAT_D24_UNORM_S8_UINT)  ||
           (fmt == VK_FORMAT_D16_UNORM_S8_UINT);
}

VkResult createImagesAndImageViews(void)
{
    //fuction  declarations
    VkResult GetSupportedDepthFormat(void);

    //variables
    VkResult vkResult = VK_SUCCESS;

    // ---- Swapchain images count ----
    vkResult = vkGetSwapchainImagesKHR(s1.vkDevice, s1.vkSwapchainKHR, &s1.swapchainImageCount, NULL);
    if (vkResult != VK_SUCCESS || s1.swapchainImageCount == 0) 
	{
        fprintf(gpFile, "createImagesAndImageViews() --> vkGetSwapchainImagesKHR() failed or zero (%d)\n", vkResult);
        return (vkResult == VK_SUCCESS) ? VK_ERROR_INITIALIZATION_FAILED : vkResult;
    }
	else
	{
		fprintf(gpFile, "createImagesAndImageViews() --> swapchainImageCount = %u\n", s1.swapchainImageCount);
	}

    // ---- Swapchain images ----
    s1.swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * s1.swapchainImageCount);
    vkResult = vkGetSwapchainImagesKHR(s1.vkDevice, s1.vkSwapchainKHR, &s1.swapchainImageCount, s1.swapchainImage_array);
    if (vkResult != VK_SUCCESS)
	{
        fprintf(gpFile, "createImagesAndImageViews() --> 2nd vkGetSwapchainImagesKHR() failed %d\n", vkResult);
        return vkResult;
    }

    // ---- Swapchain image views ----
    s1.swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * s1.swapchainImageCount);

    VkImageViewCreateInfo ivci;
    memset(&ivci, 0, sizeof(ivci));
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format  = s1.vkFormat_color;
    ivci.components.r = VK_COMPONENT_SWIZZLE_R;
    ivci.components.g = VK_COMPONENT_SWIZZLE_G;
    ivci.components.b = VK_COMPONENT_SWIZZLE_B;
    ivci.components.a = VK_COMPONENT_SWIZZLE_A;
    ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel   = 0;
    ivci.subresourceRange.levelCount     = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.layerCount     = 1;

    for (uint32_t i = 0; i < s1.swapchainImageCount; ++i)
	{
        ivci.image = s1.swapchainImage_array[i];
        vkResult = vkCreateImageView(s1.vkDevice, &ivci, NULL, &s1.swapchainImageView_array[i]);
        if (vkResult != VK_SUCCESS) {
            fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() failed for swap %u (%d)\n", i, vkResult);
            return vkResult;
        }
    }

    // ---- Depth A (reuse your single depth image) ----
    vkResult = GetSupportedDepthFormat();
    if (vkResult != VK_SUCCESS) 
	{
        fprintf(gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() failed %d\n", vkResult);
        return vkResult;
    }

    VkImageCreateInfo depthCI;
    memset(&depthCI, 0, sizeof(depthCI));
    depthCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthCI.imageType = VK_IMAGE_TYPE_2D;
    depthCI.format = s1.vkFormat_depth;
    depthCI.extent.width  = s1.winWidth;
    depthCI.extent.height = s1.winHeight;
    depthCI.extent.depth  = 1;
    depthCI.mipLevels = 1;
    depthCI.arrayLayers = 1;
    depthCI.samples = VK_SAMPLE_COUNT_1_BIT;
    depthCI.tiling  = VK_IMAGE_TILING_OPTIMAL;
    depthCI.usage   = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(s1.vkDevice, &depthCI, NULL, &s1.vkImage_depth);
    if (vkResult != VK_SUCCESS) 
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImage(depth) failed %d\n", vkResult); return vkResult;
	}

    VkMemoryRequirements dreq;
    vkGetImageMemoryRequirements(s1.vkDevice, s1.vkImage_depth, &dreq);

    VkMemoryAllocateInfo dai;
    memset(&dai, 0, sizeof(dai));
    dai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    dai.allocationSize  = dreq.size;
    dai.memoryTypeIndex = 0;

    uint32_t bits = dreq.memoryTypeBits;
    for (uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i) 
	{
        if ((bits & 1) && (s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) 
		{ 
			dai.memoryTypeIndex = i; break;
		}
        bits >>= 1;
    }

    vkResult = vkAllocateMemory(s1.vkDevice, &dai, NULL, &s1.vkDeviceMemory_depth);
    if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createImagesAndImageViews() --> vkAllocateMemory(depth) failed %d\n", vkResult); return vkResult; 
	}
	
    vkResult = vkBindImageMemory(s1.vkDevice, s1.vkImage_depth, s1.vkDeviceMemory_depth, 0);
    if (vkResult != VK_SUCCESS) 
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkBindImageMemory(depth) failed %d\n", vkResult); return vkResult; 
	}

    memset(&ivci, 0, sizeof(ivci));
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format  = s1.vkFormat_depth;
    ivci.subresourceRange.aspectMask  = VK_IMAGE_ASPECT_DEPTH_BIT | (HasStencilComponent(s1.vkFormat_depth) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    ivci.subresourceRange.baseMipLevel  = 0;
    ivci.subresourceRange.levelCount  = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.layerCount  = 1;
    ivci.image = s1.vkImage_depth;

    vkResult = vkCreateImageView(s1.vkDevice, &ivci, NULL, &s1.vkImageView_depth);
    if (vkResult != VK_SUCCESS)
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView(depth) failed %d\n", vkResult);
		return vkResult;
	}

    // ---- NEW: Offscreen Color A (for Subpass 0) ----
    VkImageCreateInfo colorACI;
    memset(&colorACI, 0, sizeof(colorACI));
    colorACI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    colorACI.imageType = VK_IMAGE_TYPE_2D;
    colorACI.format = s1.vkFormat_color; // keep same pixel format as swapchain for simplicity
    colorACI.extent.width  = s1.winWidth;
    colorACI.extent.height = s1.winHeight;
    colorACI.extent.depth  = 1;
    colorACI.mipLevels = 1;
    colorACI.arrayLayers = 1;
    colorACI.samples = VK_SAMPLE_COUNT_1_BIT;
    colorACI.tiling  = VK_IMAGE_TILING_OPTIMAL;
    colorACI.usage   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    colorACI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    colorACI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(s1.vkDevice, &colorACI, NULL, &s1.vkImage_colorA);
    if (vkResult != VK_SUCCESS)
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImage(colorA) failed %d\n", vkResult); 
		return vkResult;
	}

    VkMemoryRequirements creq;
    vkGetImageMemoryRequirements(s1.vkDevice, s1.vkImage_colorA, &creq);

    VkMemoryAllocateInfo cai;
    memset(&cai, 0, sizeof(cai));
    cai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    cai.allocationSize  = creq.size;
    cai.memoryTypeIndex = 0;

    uint32_t cbits = creq.memoryTypeBits;
    for (uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
	{
        if ((cbits & 1) && (s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		{ 
			cai.memoryTypeIndex = i; 
			break;
		}
        cbits >>= 1;
    }

    vkResult = vkAllocateMemory(s1.vkDevice, &cai, NULL, &s1.vkDeviceMemory_colorA);
    if (vkResult != VK_SUCCESS)
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkAllocateMemory(colorA) failed %d\n", vkResult);
		return vkResult;
	}

    vkResult = vkBindImageMemory(s1.vkDevice, s1.vkImage_colorA, s1.vkDeviceMemory_colorA, 0);
    if (vkResult != VK_SUCCESS)
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkBindImageMemory(colorA) failed %d\n", vkResult);
		return vkResult;
	}

    VkImageViewCreateInfo colorAViewCI;
    memset(&colorAViewCI, 0, sizeof(colorAViewCI));
    colorAViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorAViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorAViewCI.format   = s1.vkFormat_color;
    colorAViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
    colorAViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
    colorAViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
    colorAViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
    colorAViewCI.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    colorAViewCI.subresourceRange.baseMipLevel   = 0;
    colorAViewCI.subresourceRange.levelCount     = 1;
    colorAViewCI.subresourceRange.baseArrayLayer = 0;
    colorAViewCI.subresourceRange.layerCount     = 1;
    colorAViewCI.image = s1.vkImage_colorA;

    vkResult = vkCreateImageView(s1.vkDevice, &colorAViewCI, NULL, &s1.vkImageView_colorA);
    if (vkResult != VK_SUCCESS)
	{ 
		fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView(colorA) failed %d\n", vkResult);
		return vkResult;
	}

    return VK_SUCCESS;
}


VkResult GetSupportedDepthFormat(void)
{
    //code
    VkResult vkResult = VK_SUCCESS;
    VkFormat vkFormat_depth_array[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, 
                                       VK_FORMAT_D32_SFLOAT,
                                       VK_FORMAT_D24_UNORM_S8_UINT,
                                       VK_FORMAT_D16_UNORM_S8_UINT,
                                       VK_FORMAT_D16_UNORM };
    
    for(uint32_t i = 0; i < (sizeof(vkFormat_depth_array)/sizeof(vkFormat_depth_array[0])); i++)
    {
        VkFormatProperties vkFormatProperties;
        memset((void*)&vkFormatProperties, 0, sizeof(VkFormatProperties));
        
        vkGetPhysicalDeviceFormatProperties(s1.vkPhysicalDevice_selected, vkFormat_depth_array[i], &vkFormatProperties);
        
        if(vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
           s1.vkFormat_depth = vkFormat_depth_array[i];
           vkResult = VK_SUCCESS;
           break;
        }
    }
    
   return vkResult;
}


VkResult createCommandPool(void)
{
    //variables
   VkResult vkResult = VK_SUCCESS;
    
   //code
   VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
   memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
   
   vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   vkCommandPoolCreateInfo.pNext = NULL;
   vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCommandPoolCreateInfo.queueFamilyIndex = s1.graphicsQueueFamilyIndex_selected;
   
   vkResult = vkCreateCommandPool(s1.vkDevice, 
                                  &vkCommandPoolCreateInfo,
                                  NULL,
                                  &s1.vkCommandPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createCommandPool() --> vkCreateCommandPool() is failed and error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createCommandPool() --> vkCreateCommandPool() is succedded \n");
    }

   return vkResult;   
}


VkResult createCommandBuffers(void)
{
   //variables
   VkResult vkResult = VK_SUCCESS;
   
   //code
   //vkCommandBuffer allocate info structure initliazation
   VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo; 
   memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
   
   vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   vkCommandBufferAllocateInfo.pNext = NULL;
   vkCommandBufferAllocateInfo.commandPool = s1.vkCommandPool;
   vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkCommandBufferAllocateInfo.commandBufferCount = 1;
   
   s1.vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * s1.swapchainImageCount);
   //malloc check to be done
   
   for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(s1.vkDevice, &vkCommandBufferAllocateInfo, &s1.vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is succedded for iteration %d\n", i);
        }
    }
   
   return vkResult;   
}

VkResult createVertexBuffer(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS; 
    
    // CUBE positions/texcoords (unchanged) …
    float cubePosition[] = {
        1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
         1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,
    };

    float cubeTexcoords[] = {
        1.0f,1.0f, 0.0f,1.0f, 1.0f,0.0f,
        1.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f,
        1.0f,1.0f, 0.0f,1.0f, 1.0f,0.0f,
        1.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f,
        1.0f,1.0f, 0.0f,1.0f, 1.0f,0.0f,
        1.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f,
        1.0f,1.0f, 0.0f,1.0f, 1.0f,0.0f,
        1.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f,
        1.0f,1.0f, 0.0f,1.0f, 1.0f,0.0f,
        1.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f,
        1.0f,1.0f, 0.0f,1.0f, 1.0f,0.0f,
        1.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f,
    };
    
    // VERTEX POSITION BUFFER 
    memset((void*)&s1.vertexData_position, 0, sizeof(s1.vertexData_position));
    
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(cubePosition);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    vkResult = vkCreateBuffer(s1.vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &s1.vertexData_position.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(s1.vkDevice, s1.vertexData_position.vkBuffer, &vkMemoryRequirements);
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(s1.vkDevice, &vkMemoryAllocateInfo, NULL, &s1.vertexData_position.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded\n");
    }
    
    vkResult = vkBindBufferMemory(s1.vkDevice, s1.vertexData_position.vkBuffer, s1.vertexData_position.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded\n");
    }
    
    void* data = NULL;
    vkResult = vkMapMemory(s1.vkDevice, s1.vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded\n");
    }
   
    memcpy(data, cubePosition, sizeof(cubePosition));
    vkUnmapMemory(s1.vkDevice, s1.vertexData_position.vkDeviceMemory);
   
    // VERTEX TEXCOORD BUFFER 
    memset((void*)&s1.vertexData_texcoord, 0, sizeof(s1.vertexData_texcoord));
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(cubeTexcoords);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    vkResult = vkCreateBuffer(s1.vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &s1.vertexData_texcoord.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord buffer \n");
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(s1.vkDevice, s1.vertexData_texcoord.vkBuffer, &vkMemoryRequirements);
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(s1.vkDevice, &vkMemoryAllocateInfo, NULL, &s1.vertexData_texcoord.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord buffer\n");
    }
    
    vkResult = vkBindBufferMemory(s1.vkDevice, s1.vertexData_texcoord.vkBuffer, s1.vertexData_texcoord.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord buffer\n");
    }
    
    data = NULL;
    vkResult = vkMapMemory(s1.vkDevice, s1.vertexData_texcoord.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord buffer\n");
    }
   
    memcpy(data, cubeTexcoords, sizeof(cubeTexcoords));
    vkUnmapMemory(s1.vkDevice, s1.vertexData_texcoord.vkDeviceMemory);
   
    return (vkResult);
}

VkResult createTexture(const char* textureFileName)
{
    VkResult vkResult = VK_SUCCESS;
    
    //step 1: load
    FILE* fp = fopen(textureFileName, "rb");
    if(fp == NULL)
    {
        fprintf(gpFile, "createTexture() --> fOpen() failed to open Stone.png texture file\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    
    uint8_t* imageData = NULL;
    int texture_width, texture_height, texture_channels;

    imageData = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if(imageData == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0)
    {
        fprintf(gpFile, "createTexture() --> stbi_load_from_file() failed to read Stone.png texture file\n");
        fclose(fp);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    fclose(fp); fp = NULL;
    
    VkDeviceSize image_size = (VkDeviceSize)texture_width * (VkDeviceSize)texture_height * 4;

    //step 2: staging buffer
    VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    
    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));    
    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = image_size;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    vkResult = vkCreateBuffer(s1.vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vkBuffer_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(s1.vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL; 
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;
    
    for(uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
        {
            if(s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(s1.vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
    }
    
    vkResult = vkBindBufferMemory(s1.vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBindBufferMemory() is succedded\n");
    }
    
    void* data = NULL;
    vkResult = vkMapMemory(s1.vkDevice, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkMapMemory() is succedded\n");
    }
    
    memcpy(data, imageData, (size_t)image_size);
    vkUnmapMemory(s1.vkDevice, vkDeviceMemory_stagingBuffer);
    
    stbi_image_free(imageData);
    imageData = NULL;
    fprintf(gpFile, "createTexture() --> stbi_image_free() Freeing of image data is succedded\n");
    
    // Step#3: create device image
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageCreateInfo.extent.width = texture_width;
    vkImageCreateInfo.extent.height = texture_height;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(s1.vkDevice, &vkImageCreateInfo, NULL, &s1.vkImage_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateImage() is succedded\n");
        fflush(gpFile);
    }
    
    //Memory requirement for texture image
    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(s1.vkDevice, s1.vkImage_texture, &vkMemoryRequirements_image);
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL; 
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1)
        {
            if(s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(s1.vkDevice, &vkMemoryAllocateInfo_image, NULL, &s1.vkDeviceMemory_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    vkResult = vkBindImageMemory(s1.vkDevice, s1.vkImage_texture, s1.vkDeviceMemory_texture, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBindImageMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBindImageMemory() is succedded\n");
        fflush(gpFile);
    }
    
    // Step#4: transition UNDEFINED -> TRANSFER_DST_OPTIMAL
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_transition_image_layout; 
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = s1.vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(s1.vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_transition_image_layout;
    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
    }

    VkPipelineStageFlags vkPipelineStageFlags_source = 0;
    VkPipelineStageFlags vkPipelineStageFlags_destination = 0;
    VkImageMemoryBarrier vkImageMemoryBarrier;
    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = s1.vkImage_texture;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    
    if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> Unsupported texture layout transition()\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout, vkPipelineStageFlags_source, vkPipelineStageFlags_destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    }
    
    VkSubmitInfo  vkSubmitInfo_transition_image_layout;
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    
    vkResult = vkQueueSubmit(s1.vkQueue, 1, &vkSubmitInfo_transition_image_layout, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    vkResult = vkQueueWaitIdle(s1.vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    vkFreeCommandBuffers(s1.vkDevice, s1.vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    // Step #5 : copy buffer -> image
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_buffer_to_image_copy; 
    memset((void*)&vkCommandBufferAllocateInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandPool = s1.vkCommandPool;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(s1.vkDevice, &vkCommandBufferAllocateInfo_buffer_to_image_copy, &vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed for buffer_to_image_copy and error code is %d \n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded for buffer_to_image_copy\n");
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_buffer_to_image_copy;
    memset((void*)&vkCommandBufferBeginInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferBeginInfo_buffer_to_image_copy.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_buffer_to_image_copy, &vkCommandBufferBeginInfo_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded for buffer_to_image_copy \n");
    }
    
    VkBufferImageCopy vkBufferImageCopy;
    memset((void*)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));
    vkBufferImageCopy.bufferOffset = 0; 
    vkBufferImageCopy.bufferRowLength = 0;
    vkBufferImageCopy.bufferImageHeight = 0;
    vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopy.imageSubresource.mipLevel = 0;
    vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
    vkBufferImageCopy.imageSubresource.layerCount = 1;
    vkBufferImageCopy.imageOffset.x = 0;
    vkBufferImageCopy.imageOffset.y = 0;
    vkBufferImageCopy.imageOffset.z = 0;
    vkBufferImageCopy.imageExtent.width = texture_width;
    vkBufferImageCopy.imageExtent.height = texture_height;
    vkBufferImageCopy.imageExtent.depth = 1;
    
    vkCmdCopyBufferToImage(vkCommandBuffer_buffer_to_image_copy, 
                           vkBuffer_stagingBuffer, 
                           s1.vkImage_texture, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                           1, 
                           &vkBufferImageCopy);
    
    vkResult = vkEndCommandBuffer(vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded for buffer_to_image_copy\n");
    }
    
    VkSubmitInfo  vkSubmitInfo_buffer_to_image_copy;
    memset((void*)&vkSubmitInfo_buffer_to_image_copy, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_buffer_to_image_copy.pNext = NULL;
    vkSubmitInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkSubmitInfo_buffer_to_image_copy.pCommandBuffers = &vkCommandBuffer_buffer_to_image_copy;
    
    vkResult = vkQueueSubmit(s1.vkQueue, 1, &vkSubmitInfo_buffer_to_image_copy, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is succedded for buffer_to_image_copy");
    }

    vkResult = vkQueueWaitIdle(s1.vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded for buffer_to_image_copy\n");
    }
    
    vkFreeCommandBuffers(s1.vkDevice, s1.vkCommandPool, 1, &vkCommandBuffer_buffer_to_image_copy);
    vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    
    // Step 6: TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = s1.vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(s1.vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
    }

    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
    }

    vkPipelineStageFlags_source = 0;
    vkPipelineStageFlags_destination = 0;

    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = s1.vkImage_texture;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    
    if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> Unsupported texture layout transition for 2nd time in step 6\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout, vkPipelineStageFlags_source, vkPipelineStageFlags_destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is failed for 2nd time in step 6 and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    }
    
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    
    vkResult = vkQueueSubmit(s1.vkQueue, 1, &vkSubmitInfo_transition_image_layout, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is failed  for 2nd time in step 6 errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() succeded  for 2nd time in step 6 \n", vkResult);
    }

    vkResult = vkQueueWaitIdle(s1.vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() is failed  for 2nd time in step 6 errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded for 2nd time in step 6 \n", vkResult);
    }
    
    vkFreeCommandBuffers(s1.vkDevice, s1.vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    // Step #7: destroy staging
    if(vkBuffer_stagingBuffer)
    {
        vkDestroyBuffer(s1.vkDevice, vkBuffer_stagingBuffer, NULL);
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "createTexture() --> vkDestroyBuffer() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    if(vkDeviceMemory_stagingBuffer)
    {
       vkFreeMemory(s1.vkDevice, vkDeviceMemory_stagingBuffer, NULL);
       vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
       fprintf(gpFile, "createTexture() --> vkFreeMemory() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    
    // Step8: image view for texture
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = s1.vkImage_texture;
    
    vkResult = vkCreateImageView(s1.vkDevice, &vkImageViewCreateInfo, NULL, &s1.vkImageView_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateImageView() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateImageView() is succedded\n");
    }
    
    //Step 9: sampler
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void*)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
    vkSamplerCreateInfo.maxAnisotropy = 16;
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    vkSamplerCreateInfo.compareEnable = VK_FALSE;
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    
    vkResult = vkCreateSampler(s1.vkDevice, &vkSamplerCreateInfo, NULL, &s1.vkSampler_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateSampler() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateSampler() is succedded\n");
    }
     
    return vkResult; 
}

VkResult createUniformBuffer(void)
{
    // Function declarations
    VkResult updateUniformBuffer(void);

    VkResult vkResult = VK_SUCCESS;

    memset((void*)&s1.uniformData, 0, sizeof(s1.uniformData));

    VkBufferCreateInfo vkBufferCreateInfo{};
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.size  = sizeof(Scene1_Vulkan::MyUniformData); // <-- scope fix
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    vkResult = vkCreateBuffer(s1.vkDevice, &vkBufferCreateInfo, NULL, &s1.uniformData.vkBuffer);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() failed %d\n", vkResult); return vkResult; }

    VkMemoryRequirements vkMemoryRequirements{};
    vkGetBufferMemoryRequirements(s1.vkDevice, s1.uniformData.vkBuffer, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo{};
    vkMemoryAllocateInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    uint32_t bits = vkMemoryRequirements.memoryTypeBits;
    for(uint32_t i = 0; i < s1.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((bits & 1) &&
           (s1.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            vkMemoryAllocateInfo.memoryTypeIndex = i;
            break;
        }
        bits >>= 1;
    }

    vkResult = vkAllocateMemory(s1.vkDevice, &vkMemoryAllocateInfo, NULL, &s1.uniformData.vkDeviceMemory);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() failed %d\n", vkResult); return vkResult; }

    vkResult = vkBindBufferMemory(s1.vkDevice, s1.uniformData.vkBuffer, s1.uniformData.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() failed %d\n", vkResult); return vkResult; }

    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createUniformBuffer() --> updateUniformBuffer() failed %d\n", vkResult); return vkResult; }

    return vkResult;
}

VkResult updateUniformBuffer(void)
{
    VkResult vkResult = VK_SUCCESS;

    Scene1_Vulkan::MyUniformData myUniformData{}; // <-- scope fix

    myUniformData.modelMatrix = glm::mat4(1.0f);

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));

    glm::mat4 rotationMatrix_X = glm::rotate(glm::mat4(1.0f), glm::radians(s1.angle), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationMatrix_Y = glm::rotate(glm::mat4(1.0f), glm::radians(s1.angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationMatrix_Z = glm::rotate(glm::mat4(1.0f), glm::radians(s1.angle), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotationMatrix   = rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z;

    glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.05f, 1.25f, 1.25f));
    myUniformData.modelMatrix = translationMatrix * ScaleMatrix * rotationMatrix;

    myUniformData.viewMatrix = glm::mat4(1.0f);

    glm::mat4 perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f),
                                        (float)s1.winWidth/(float)s1.winHeight, 0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] *= -1.0f; // Vulkan Y fix
    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    myUniformData.fade = glm::vec4(s1_effect.gFade, 0.0f, 0.0f, 0.0f);

    void* data = NULL;
    vkResult = vkMapMemory(s1.vkDevice, s1.uniformData.vkDeviceMemory,
                           0, sizeof(Scene1_Vulkan::MyUniformData), 0, &data); // <-- scope fix
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "updateUniformBuffer() --> vkMapMemory() failed %d\n", vkResult);
        return vkResult;
    }

    memcpy(data, &myUniformData, sizeof(Scene1_Vulkan::MyUniformData)); // <-- scope fix
    vkUnmapMemory(s1.vkDevice, s1.uniformData.vkDeviceMemory);

    return vkResult;
}

VkResult createShaders(void)
{
    VkResult vkResult = VK_SUCCESS;

    // Vertex shader
    const char* szFileName = "shader_s1.vert.spv";
    FILE* fp = fopen(szFileName, "rb");
    if(!fp){ fprintf(gpFile, "createShaders() --> fopen() failed for %s\n", szFileName); return VK_ERROR_INITIALIZATION_FAILED; }
    fprintf(gpFile, "createShaders() --> fopen() succeeded for %s\n", szFileName);

    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    if(size == 0){ fprintf(gpFile, "createShaders() --> ftell() size 0 for %s\n", szFileName); fclose(fp); return VK_ERROR_INITIALIZATION_FAILED; }
    fseek(fp, 0L, SEEK_SET);

    char* shaderData = (char*)malloc(size);
    size_t retVal = fread(shaderData, size, 1, fp);
    fclose(fp);
    if(retVal != 1){ fprintf(gpFile, "createShaders() --> fread() failed for %s\n", szFileName); free(shaderData); return VK_ERROR_INITIALIZATION_FAILED; }

    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = size;
    ci.pCode = (const uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(s1.vkDevice, &ci, NULL, &s1.vkShaderModule_vertex_shader); // <-- FIX
    free(shaderData);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createShaders() --> vkCreateShaderModule() failed (vert) %d\n", vkResult); return vkResult; }
    fprintf(gpFile, "createShaders() --> vertex Shader module created\n");

    // Fragment shader
    szFileName = "shader_s1.frag.spv";
    fp = fopen(szFileName, "rb");
    if(!fp){ fprintf(gpFile, "createShaders() --> fopen() failed for %s\n", szFileName); return VK_ERROR_INITIALIZATION_FAILED; }
    fprintf(gpFile, "createShaders() --> fopen() succeeded for %s\n", szFileName);

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    if(size == 0){ fprintf(gpFile, "createShaders() --> ftell() size 0 for %s\n", szFileName); fclose(fp); return VK_ERROR_INITIALIZATION_FAILED; }
    fseek(fp, 0L, SEEK_SET);

    shaderData = (char*)malloc(size);
    retVal = fread(shaderData, size, 1, fp);
    fclose(fp);
    if(retVal != 1){ fprintf(gpFile, "createShaders() --> fread() failed for %s\n", szFileName); free(shaderData); return VK_ERROR_INITIALIZATION_FAILED; }

    memset(&ci, 0, sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = size;
    ci.pCode = (const uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(s1.vkDevice, &ci, NULL, &s1.vkShaderModule_fragment_shader); // <-- FIX
    free(shaderData);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createShaders() --> vkCreateShaderModule() failed (frag) %d\n", vkResult); return vkResult; }
    fprintf(gpFile, "createShaders() --> fragment Shader module created\n");

    return vkResult;
}

VkResult createDescriptorSetLayout(void)
{
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorSetLayoutBinding bindings[2]{};
    // binding 0: uniform buffer
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].binding         = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // binding 1: combined image sampler
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].binding         = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo ci{};
    ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount = (uint32_t)_ARRAYSIZE(bindings);
    ci.pBindings    = bindings;

    vkResult = vkCreateDescriptorSetLayout(s1.vkDevice, &ci, NULL, &s1.vkDescriptorSetLayout); // <-- FIX
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayout failed %d\n", vkResult); return vkResult; }
    fprintf(gpFile, "createDescriptorSetLayout() --> success\n");
    return vkResult;
}

VkResult createPipelineLayout(void)
{
    VkResult vkResult = VK_SUCCESS;

    VkPipelineLayoutCreateInfo ci{};
    ci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    ci.setLayoutCount         = 1;
    ci.pSetLayouts            = &s1.vkDescriptorSetLayout; // <-- FIX
    ci.pushConstantRangeCount = 0;
    ci.pPushConstantRanges    = NULL;

    vkResult = vkCreatePipelineLayout(s1.vkDevice, &ci, NULL, &s1.vkPipelineLayout);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createPipelineLayout() --> vkCreatePipelineLayout failed %d\n", vkResult); return vkResult; }
    fprintf(gpFile, "createPipelineLayout() --> success\n");
    return vkResult;
}

VkResult createDescriptorPool(void)
{
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorPoolSize sizes[2]{};
    sizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sizes[0].descriptorCount = 1;
    sizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo ci{};
    ci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.poolSizeCount = (uint32_t)_ARRAYSIZE(sizes);
    ci.pPoolSizes    = sizes;
    ci.maxSets       = 2;

    vkResult = vkCreateDescriptorPool(s1.vkDevice, &ci, NULL, &s1.vkDescriptorPool); // <-- FIX
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createDescriptorPool() --> vkCreateDescriptorPool failed %d\n", vkResult); return vkResult; }
    fprintf(gpFile, "createDescriptorPool() --> success\n");
    return vkResult;
}

VkResult createDescriptorSet(void)
{
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorSetAllocateInfo ai{};
    ai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool     = s1.vkDescriptorPool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts        = &s1.vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(s1.vkDevice, &ai, &s1.vkDescriptorSet);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets failed %d\n", vkResult); return vkResult; }

    VkDescriptorBufferInfo dbi{};
    dbi.buffer = s1.uniformData.vkBuffer;
    dbi.offset = 0;
    dbi.range  = sizeof(Scene1_Vulkan::MyUniformData); // <-- scope fix

    VkDescriptorImageInfo dii{};
    dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    dii.imageView   = s1.vkImageView_texture;
    dii.sampler     = s1.vkSampler_texture;

    VkWriteDescriptorSet writes[2]{};

    writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet          = s1.vkDescriptorSet;
    writes[0].dstBinding      = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo     = &dbi;

    writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet          = s1.vkDescriptorSet;
    writes[1].dstBinding      = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo      = &dii;

    vkUpdateDescriptorSets(s1.vkDevice, (uint32_t)_ARRAYSIZE(writes), writes, 0, NULL);
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() succeeded\n");
    return vkResult;
}

VkResult createRenderPass(void)
{
    VkResult vkResult = VK_SUCCESS;

    VkAttachmentDescription atts[3];
    memset(atts, 0, sizeof(atts));

    // [0] Offscreen Color A (Scene A target)
    atts[0].format         = s1.vkFormat_color;
    atts[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    atts[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    atts[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // will be read in a later subpass/phase

    // [1] Depth A for Scene A
    atts[1].format         = s1.vkFormat_depth;
    atts[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    atts[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // [2] Swapchain color (final present)
    atts[2].format         = s1.vkFormat_color;
    atts[2].samples        = VK_SAMPLE_COUNT_1_BIT;
    atts[2].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear to black for now
    atts[2].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    atts[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[2].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // ---- Subpass 0: write Scene A to Color A + Depth A ----
    VkAttachmentReference refColorA{};
    refColorA.attachment = 0;
    refColorA.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference refDepthA{};
    refDepthA.attachment = 1;
    refDepthA.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass0{};
    subpass0.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass0.colorAttachmentCount    = 1;
    subpass0.pColorAttachments       = &refColorA;
    subpass0.pDepthStencilAttachment = &refDepthA;

    // ---- Subpass 1: (placeholder) target swapchain ----
    // We'll composite later; for now we just clear and present.
    VkAttachmentReference refSwap{};
    refSwap.attachment = 2;
    refSwap.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.colorAttachmentCount    = 1;
    subpass1.pColorAttachments       = &refSwap;
    subpass1.pDepthStencilAttachment = NULL;

    VkSubpassDescription subpasses[2] = { subpass0, subpass1 };

    // ---- Dependencies (EXT->0, 0->1, 1->EXT) ----
    VkSubpassDependency deps[3];
    memset(deps, 0, sizeof(deps));

    // External -> Subpass 0
    deps[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass    = 0;
    deps[0].srcStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    deps[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].srcAccessMask = 0;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[0].dependencyFlags = 0;

    // Subpass 0 -> Subpass 1
    deps[1].srcSubpass    = 0;
    deps[1].dstSubpass    = 1;
    deps[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dependencyFlags = 0;

    // Subpass 1 -> External
    deps[2].srcSubpass    = 1;
    deps[2].dstSubpass    = VK_SUBPASS_EXTERNAL;
    deps[2].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[2].dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    deps[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[2].dstAccessMask = 0;
    deps[2].dependencyFlags = 0;

    VkRenderPassCreateInfo rpci{};
    rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = (uint32_t)_ARRAYSIZE(atts);
    rpci.pAttachments    = atts;
    rpci.subpassCount    = (uint32_t)_ARRAYSIZE(subpasses);
    rpci.pSubpasses      = subpasses;
    rpci.dependencyCount = (uint32_t)_ARRAYSIZE(deps);
    rpci.pDependencies   = deps;

    vkResult = vkCreateRenderPass(s1.vkDevice, &rpci, NULL, &s1.vkRenderPass);
    if (vkResult != VK_SUCCESS) { fprintf(gpFile, "createRenderPass() --> vkCreateRenderPass failed %d\n", vkResult); return vkResult; }
    fprintf(gpFile, "createRenderPass() --> success (Subpass0: ColorA+DepthA, Subpass1: Swapchain)\n");
    return vkResult;
}


VkResult createPipeline(void)
{
    VkResult vkResult = VK_SUCCESS;

    // vertex input
    VkVertexInputBindingDescription binds[2]{};
    binds[0].binding   = 0; binds[0].stride = sizeof(float)*3; binds[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binds[1].binding   = 1; binds[1].stride = sizeof(float)*2; binds[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[2]{};
    attrs[0].binding  = 0; attrs[0].location = 0; attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[0].offset = 0;
    attrs[1].binding  = 1; attrs[1].location = 1; attrs[1].format = VK_FORMAT_R32G32_SFLOAT;    attrs[1].offset = 0;

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount   = (uint32_t)_ARRAYSIZE(binds);
    vi.pVertexBindingDescriptions      = binds;
    vi.vertexAttributeDescriptionCount = (uint32_t)_ARRAYSIZE(attrs);
    vi.pVertexAttributeDescriptions    = attrs;

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode             = VK_POLYGON_MODE_FILL;
    rs.cullMode                = VK_CULL_MODE_NONE;
    rs.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth               = 1.0f;

    VkPipelineColorBlendAttachmentState cba{};
    cba.blendEnable           = VK_TRUE;
    cba.srcColorBlendFactor   = VK_BLEND_FACTOR_ONE;
    cba.dstColorBlendFactor   = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cba.colorBlendOp          = VK_BLEND_OP_ADD;
    cba.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE;
    cba.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cba.alphaBlendOp          = VK_BLEND_OP_ADD;
    cba.colorWriteMask        = 0xF;

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments    = &cba;

    // viewport/scissor
    memset((void*)&s1.vkViewport, 0, sizeof(VkViewport)); // <-- FIX
    s1.vkViewport.x        = 0.0f;
    s1.vkViewport.y        = 0.0f;
    s1.vkViewport.width    = (float)s1.vkExtent2D_swapchain.width;  // <-- FIX
    s1.vkViewport.height   = (float)s1.vkExtent2D_swapchain.height; // <-- FIX
    s1.vkViewport.minDepth = 0.0f;
    s1.vkViewport.maxDepth = 1.0f;

    memset((void*)&s1.vkRect2D_scissor, 0, sizeof(VkRect2D)); // <-- FIX
    s1.vkRect2D_scissor.offset.x = 0;
    s1.vkRect2D_scissor.offset.y = 0;
    s1.vkRect2D_scissor.extent.width  = s1.vkExtent2D_swapchain.width;  // <-- FIX
    s1.vkRect2D_scissor.extent.height = s1.vkExtent2D_swapchain.height; // <-- FIX

    VkPipelineViewportStateCreateInfo vp{};
    vp.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.pViewports    = &s1.vkViewport;           // <-- FIX
    vp.scissorCount  = 1;
    vp.pScissors     = &s1.vkRect2D_scissor;     // <-- FIX

    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable       = VK_TRUE;
    ds.depthWriteEnable      = VK_TRUE;
    ds.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.stencilTestEnable     = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = s1.vkShaderModule_vertex_shader;   // <-- FIX
    stages[0].pName  = "main";

    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = s1.vkShaderModule_fragment_shader; // <-- FIX
    stages[1].pName  = "main";

    VkPipelineCacheCreateInfo pcci{};
    pcci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    vkResult = vkCreatePipelineCache(s1.vkDevice, &pcci, NULL, &pipelineCache);
    if(vkResult != VK_SUCCESS){ fprintf(gpFile, "createPipeline() --> vkCreatePipelineCache failed %d\n", vkResult); return vkResult; }

    VkGraphicsPipelineCreateInfo pci{};
    pci.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pci.pVertexInputState   = &vi;
    pci.pInputAssemblyState = &ia;
    pci.pRasterizationState = &rs;
    pci.pColorBlendState    = &cb;
    pci.pViewportState      = &vp;
    pci.pDepthStencilState  = &ds;
    pci.pMultisampleState   = &ms;
    pci.stageCount          = (uint32_t)_ARRAYSIZE(stages);
    pci.pStages             = stages;
    pci.layout              = s1.vkPipelineLayout;
    pci.renderPass          = s1.vkRenderPass;
    pci.subpass             = 0;

    vkResult = vkCreateGraphicsPipelines(s1.vkDevice, pipelineCache, 1, &pci, NULL, &s1.vkPipeline);
    if(vkResult != VK_SUCCESS){
        fprintf(gpFile, "createPipeline() --> vkCreateGraphicsPipelines failed %d\n", vkResult);
        vkDestroyPipelineCache(s1.vkDevice, pipelineCache, NULL);
        return vkResult;
    }
    fprintf(gpFile, "createPipeline() --> vkCreateGraphicsPipelines succeeded\n");

    vkDestroyPipelineCache(s1.vkDevice, pipelineCache, NULL);
    return vkResult;
}

VkResult createFrameBuffers(void)
{
    VkResult vkResult = VK_SUCCESS;

    s1.vkFrameBuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * s1.swapchainImageCount);
    if(!s1.vkFrameBuffer_array){ fprintf(gpFile, "createFrameBuffers() --> malloc failed\n"); return VK_ERROR_OUT_OF_HOST_MEMORY; }

    for (uint32_t i = 0; i < s1.swapchainImageCount; ++i)
    {
        VkImageView attachments[3];
        attachments[0] = s1.vkImageView_colorA;             // attachment 0 (offscreen color A)
        attachments[1] = s1.vkImageView_depth;              // attachment 1 (depth A)
        attachments[2] = s1.swapchainImageView_array[i];    // attachment 2 (swapchain)

        VkFramebufferCreateInfo fci{};
        fci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fci.renderPass      = s1.vkRenderPass;
        fci.attachmentCount = (uint32_t)_ARRAYSIZE(attachments);
        fci.pAttachments    = attachments;
        fci.width           = s1.vkExtent2D_swapchain.width;
        fci.height          = s1.vkExtent2D_swapchain.height;
        fci.layers          = 1;

        vkResult = vkCreateFramebuffer(s1.vkDevice, &fci, NULL, &s1.vkFrameBuffer_array[i]);
        if (vkResult != VK_SUCCESS) {
            fprintf(gpFile, "createFrameBuffers() --> vkCreateFramebuffer failed at %u (%d)\n", i, vkResult);
            return vkResult;
        }
    }

    fprintf(gpFile, "createFrameBuffers() --> created %u framebuffers (ColorA,DepthA,Swap)\n", s1.swapchainImageCount);
    return vkResult;
}

VkResult createSemaphores(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
    
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;  //Binary and Timeline Semaphore info, bydefault it is Binary
    vkSemaphoreCreateInfo.flags = 0; //RESERVED: must be zero
    
    //backBuffer Semaphore
    vkResult = vkCreateSemaphore(s1.vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &s1.vkSemaphore_backBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for s1.vkSemaphore_backBuffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for s1.vkSemaphore_backBuffer\n");
    }
    
    
    //renderComplete Semaphore
    vkResult = vkCreateSemaphore(s1.vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &s1.vkSemaphore_renderComplete);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for s1.vkSemaphore_renderComplete and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for s1.vkSemaphore_renderComplete\n");
    }
    
    return vkResult;
    
}


VkResult createFences(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
    
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    s1.vkFence_array = (VkFence*)malloc(sizeof(VkFence) * s1.swapchainImageCount);
    //malloc error checking to be done
    
    for(uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(s1.vkDevice, 
                                 &vkFenceCreateInfo,
                                 NULL,
                                 &s1.vkFence_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createFences() --> vkCreateFence() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createFences() --> vkCreateFence() is succedded for %d iteration\n", i);
        }
    }

    return vkResult;
}


VkResult buildCommandBuffers(void)
{
    VkResult vkResult = VK_SUCCESS;

    for (uint32_t i = 0; i < s1.swapchainImageCount; i++)
    {
        vkResult = vkResetCommandBuffer(s1.vkCommandBuffer_array[i], 0);
        if (vkResult != VK_SUCCESS) { fprintf(gpFile, "buildCommandBuffers() --> vkResetCommandBuffer failed %u (%d)\n", i, vkResult); return vkResult; }

        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkResult = vkBeginCommandBuffer(s1.vkCommandBuffer_array[i], &bi);
        if (vkResult != VK_SUCCESS) { fprintf(gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer failed %u (%d)\n", i, vkResult); return vkResult; }

        // Clear values: [0]=ColorA, [1]=DepthA, [2]=Swapchain
        VkClearValue clears[3];
        memset(clears, 0, sizeof(clears));
        clears[0].color        = s1.vkClearColorValue;        // Color A clear (black)
        clears[1].depthStencil = s1.vkClearDepthStencilValue; // Depth A clear
        clears[2].color        = s1.vkClearColorValue;        // Swapchain clear (black)

        VkRenderPassBeginInfo rpbi{};
        rpbi.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpbi.renderPass        = s1.vkRenderPass;
        rpbi.renderArea.offset = {0, 0};
        rpbi.renderArea.extent = { s1.vkExtent2D_swapchain.width, s1.vkExtent2D_swapchain.height };
        rpbi.clearValueCount   = (uint32_t)_ARRAYSIZE(clears);
        rpbi.pClearValues      = clears;
        rpbi.framebuffer       = s1.vkFrameBuffer_array[i];

        vkCmdBeginRenderPass(s1.vkCommandBuffer_array[i], &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        // ---- SUBPASS 0: render Scene A into Color A + Depth A ----
        vkCmdBindPipeline(s1.vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, s1.vkPipeline);

        vkCmdBindDescriptorSets(s1.vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                s1.vkPipelineLayout,
                                0, 1, &s1.vkDescriptorSet, 0, NULL);

        // Bind position buffer at binding 0
        VkBuffer posBufs[1] = { s1.vertexData_position.vkBuffer };
        VkDeviceSize posOffs[1] = { 0 };
        vkCmdBindVertexBuffers(s1.vkCommandBuffer_array[i], 0, 1, posBufs, posOffs);

        // Bind texcoord buffer at binding 1
        VkBuffer texBufs[1] = { s1.vertexData_texcoord.vkBuffer };
        VkDeviceSize texOffs[1] = { 0 };
        vkCmdBindVertexBuffers(s1.vkCommandBuffer_array[i], 1, 1, texBufs, texOffs);

        vkCmdDraw(s1.vkCommandBuffer_array[i], 36, 1, 0, 0);

        // ---- Move to SUBPASS 1 (currently a placeholder that just clears/presents the swapchain) ----
        vkCmdNextSubpass(s1.vkCommandBuffer_array[i], VK_SUBPASS_CONTENTS_INLINE);

        // (No draws yet; we’ll composite A -> swapchain in the next step)

        vkCmdEndRenderPass(s1.vkCommandBuffer_array[i]);

        vkResult = vkEndCommandBuffer(s1.vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS) { fprintf(gpFile, "buildCommandBuffers() --> vkEndCommandBuffer failed %u (%d)\n", i, vkResult); return vkResult; }
    }

    return vkResult;
}

 VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, 
                                                   VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
                                                   uint64_t object,
                                                   size_t location,
                                                   int32_t messageCode,
                                                   const char* pLayerPrefix,
                                                   const char* pMessage,
                                                   void* pUserData)
{
    //code
    fprintf(gpFile, "SSA_Validation: debugReportCallback() --> %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);
    
    return(VK_FALSE);
}
