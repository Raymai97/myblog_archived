---
layout:	post
title:	"Enable Direct3D acceleration in Windows 2000 in VirtualBox"
date:	2017-05-03 15:34:00 +0800
categories: windows undocd
permalink:  enable-direct3d-in-win2k-vbox
---

VBox version: v5.0.10.r104061

## Part 1: Obtaining required files

First, you must need to obtain these files:
- d3d8.dll
- d3d9.dll
- VBoxD3D8.dll
- VBoxD3D9.dll
- VBoxOGL.dll
- VBoxOGLarrayspu.dll
- VBoxOGLcrutil.dll
- VBoxOGLerrorspu.dll
- VBoxOGLfeedbackspu.dll
- VBoxOGLpackspu.dll
- VBoxOGLpassthroughspu.dll
- wined3d.dll

I get these files from my Windows XP guest OS, with Direct3D acceleration enabled.

## Part 2: Mod the files to load kernel31.dll instead

> Win2K doesn't support new APIs introduced in WinXP. In this case, all of these APIs are in kernel32.dll. We need a DLL, say, kernel31.dll, which will forward most of the calls to the real kernel32.dll. For "new" APIs, we will provide implementation by ourselves. Creating such DLL requires some programming knowledge. If you know how to create DLL that exports function, then you may refer Part X. Otherwise, you might just want to [download the kernel31.dll here]({% asset_path Kernel31.7z %}).

To do this, you need a hex editor. I recommended [HxD](https://mh-nexus.de/en/hxd/) as it is free, small and serves the purpose well.

You need to mod the following files:
- VBoxD3D8.dll
- VBoxD3D9.dll
- VBoxOGL.dll
- VBoxOGLarrayspu.dll
- VBoxOGLcrutil.dll
- VBoxOGLerrorspu.dll
- VBoxOGLfeedbackspu.dll
- VBoxOGLpackspu.dll
- VBoxOGLpassthroughspu.dll
- wined3d.dll

### To mod a file:
1. Open the file with hex editor.
2. Search (Ctrl+F) for text-string `kernel32.dll`.  
Be careful. Sometimes there are more than one occurence in a file.
  - This is the `kernel32.dll` we are looking for:
  ![img]({% asset_path true-kernel32-string.png %})  
  - This is **NOT** the one we are looking for:  
  ![img]({% asset_path false-kernel32-string.png %})

3. Overwrite so it become `kernel31.dll` and save the file.  
![img]({% asset_path overwrite-like-this.png %})

## Part 3: Showtime

1. If you haven't install Win2K as guest OS yet, do it now.
- Install VirtualBox Guest Addition as usual.
- No need to tick Direct3D support.

2. Go to Machine > Settings > Display
  - Tick "Enable 3D Acceleration"
  - Tick "Enable 2D Video Acceleration"
  - Video Memory 128MB  

3. Copy and paste the following files into `C:\WINNT\System32` in Win2K.  
Overwrite the file if prompted.
- d3d8.dll
- d3d9.dll
- VBoxD3D8.dll
- VBoxD3D9.dll
- VBoxOGL.dll
- VBoxOGLarrayspu.dll
- VBoxOGLcrutil.dll
- VBoxOGLerrorspu.dll
- VBoxOGLfeedbackspu.dll
- VBoxOGLpackspu.dll
- VBoxOGLpassthroughspu.dll
- wined3d.dll
- [Kernel31.dll](({% asset_path Kernel31.7z %}))

4) Finally, run `dxdiag` to see if the Direct3D acceleration is working.  
It is expected to fail in Direct3D 7, but succeed in Direct3D 8 and 9 test.  
Here is a screenshot running Touhou games in Win2k VBox guest:  
![img]({% asset_path proof-running-touhou-rss.png %})

## Part X: Creating the Kernel31.dll

When you read this, I assume you already know how to create a DLL and export C function, so I'm just going to key out some tips on creating the DLL.

- First, determine a list of `kernel32` APIs to be handled.
- You can use Dumpbin to find out what APIs are used by a DLL.
- You should handle all DLLs mentioned in Part 2.
- To forward API calls, write corresponding pragma to instruct the linker about it. For example, if the API name is `AddVectoredExceptionHandler`, you write the pragma like this:  
```c
#pragma comment(linker, "/export:AddVectoredExceptionHandler=kernel32.AddVectoredExceptionHandler")
```
  to forward the API call to the real `kernel32.dll`.
- If you want to provide your own implementation, you forward the API call to your own exported function, with pragma like this:
```c
#pragma comment(linker, "/export:AddVectoredExceptionHandler=kernel31.My_AddVectoredExceptionHandler")
```
  And remember to export the function you specified. In this example, it is `My_AddVectoredExceptionHandler`.
- Make sure you use .def file, or the exported symbol will be decorated and your DLL wouldn't work.

![img]({% asset_path bad-example.png %})
![img]({% asset_path good-example.png %})
