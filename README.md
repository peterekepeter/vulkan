
# vulkan

This project was mainly started for learning vulkan. 
I went with a RAII method of handling resources. 
Because of this the swapchain is easily recreated when resizing the window or if otherwise lost as well
as application cleanup is pretty simple as well.

I also tried to encapsulate what I can in a OO fashion however 
at one point this all falls apart because all the objects are coupled with other object.
This is when I fell back to more basic procedural style of coding. 
However, even like this I'm still using the block scopes for easy pipeline cleanup and re-creation when context is lost.

## Requirements

 - Visual Studio 2019, community, free to download, I usually update to latest
 - Vulkan SDK, you should have a VULKAN_SDK env var defined pointing to the vulkan 
 - switch platform to x86, even though the x64 is configured, I'm currently 
   relying on a 32-bit bass.dll for audio playback, this forces the whole
   project to be 32-bit only
 - when running, set the working directory to one of the data folders, 
   for examlple `.\data\archeoptical` or `.\data\search`