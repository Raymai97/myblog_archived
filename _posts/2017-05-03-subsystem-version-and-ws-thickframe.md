---
layout: post
title:  "Subsystem version and WS_THICKFRAME are related somehow"
date:   2017-05-03 23:00:00 +0800
categories: windows undocd
permalink:  subsystem-version-and-ws-thickframe
---

> I always thought it was just an intended change made by the WinForm development team. I was wrong.

Since Visual Studio 2012, the WinForm designer uses thick window frame even if the form border style is `FixedSingle`.

![img]({% asset_path vs2015-designer-fixedsingle.png %})

This is how thing was looked like back in the older version of Visual Studio.

![img]({% asset_path vs2008-designer-fixedsingle.png %})

Do you ever wonder why? Here's the answer:

> Because since 2012, Visual Studio requires NT6.0 (Windows Vista) to run.

What?

> To be more precise, it is because the subsystem version of `devenv.exe` is set to 6.0.

### What is subsystem version?

Inside every EXE file, there is a **PE header** which provides various information about the file. For example, it will tell if the EXE file is designed for x86, IA64 or AMD64 processor. There is a field that tells if the EXE file should be started with `Console` or `Windows` subsystem, and it also contains the **minimum Windows version required to run the EXE.**

This explains that why sometimes you don't even have a chance to run a certain program, if your OS version is quite old.

![img]({% asset_path win2k-cant-run-sdelete.png %})

### And WS_THICKFRAME would be...?

In simple word, it is a window style that determines if the window can be resized or not. In the old days where DWM is not enabled, resizable window has thicker window border than unresizable window, hence the name.

### Aren't they totally unrelated?

I had the same thought too, until this happened to me.

I was developing a Win32 GUI framework. During debugging, I found an inconsistent behavior between the build that compiled using MSVC2005 and MSVC2015. The test subject was toggling the `Resizable` (which toggle the `WS_THICKFRAME`). In MSVC2015 build, the client size remained the same. In MSVC2005 build, **the client size changed**.

So I tried to pinpoint the root cause of problem. With debugger, I found the problem was occured at a line of code. The code would call `SetWindowLong` the `GWL_STYLE` to enable or disable the `WS_THICKFRAME` window style. So I coded [a minimal Win32 C++ GUI app]({% asset_path test.cpp %}) just to see what would happen when I toggled the style.

<video controls><source src="{% asset_path test.mp4 %}" type="video/mp4"></video>

WTF? Fine, I changed my mind, I wanted to run the EXE in my Windows XP guest and see what would happen. **I used the `editbin` to modify the subsystem version number.** (For the record, I never install Windows XP targeting toolset because I don't want to.)

![img]({% asset_path sendto-editbin.png %})

I ran the program again, and after I toggled the style, **the client size changed.**

> Wait. Visual Studio 2010 is the last version that runs on Windows XP, while Visual Studio 2012 is the first version that starts to use thick window frame for `FixedSingle`. They were connected, weren't they?!

I got it now! I always thought it was just an intended change made by the WinForm development team. I was wrong.

### Let's prove with proof

First, a WinForm application that toggles `FixedSingle` and `Sizable` form border style. Test the program **before and after changing subsystem version to 6.0** with a batch file that calls `editbin`. [set-vista-as-min-winver.cmd]({% asset_path set-vista-as-min-winver.cmd %})

<video controls><source src="{% asset_path proof-winform.mp4 %}" type="video/mp4"></video>

<a name="winform-workaround"></a>
Well, the client size doesn't change, but you can see that something is off. I guess WinForm has workaround like
```c++
void ToggleResizable(bool val) {
	auto size = GetClientSize();
	RealToggleResizable(val);
	SetClientSize(size);
}
```

Next, let's try this trick on Visual Studio 2015. I modify the subsystem version of `devenv.exe`, and see if anything changes.

<video controls><source src="{% asset_path proof-vs2015.mp4 %}" type="video/mp4"></video>

See? With just a change of a byte, we can get back the good-old thin window border in Form Designer again.

### Your window theme does matter too

#### For Win32 C++
```c
if ( usingClassicTheme() || subsystem_ver < 6.0 ) {
	ASSERT( client_size_will_change );
}
```

v5.0:
<video controls><source src="{% asset_path test-v5.mp4 %}" type="video/mp4"></video>
v6.0:
<video controls><source src="{% asset_path test-v6.mp4 %}" type="video/mp4"></video>
[test-v5-v6.7z]({% asset_path test-v5-v6.7z %})

#### For WinForm
```c
ASSERT( client_size_never_change ); // but
if ( subsystem_ver < 6.0 ) {
	// you can see something is off
	// because it is WinForm's workaround after all
}
```
v5.0:
<video controls><source src="{% asset_path winform-v5.mp4 %}" type="video/mp4"></video><br>
v6.0:
<video controls><source src="{% asset_path winform-v6.mp4 %}" type="video/mp4"></video>
[winform-v5-v6.7z]({% asset_path winform-v5-v6.7z %})

### Conclusion

If you're developing with WinForm, then good for you, you don't actually have to do anything, because you can assume the client size never change by itself. That little oddity is not really a big matter. Who would toggle the window style every two seconds?

If you're developing Win32 application, then you have to make up your mind. Set subsystem version to 6.0 is probably the most easy thing to do, but it is not consistent enough IMO because the size still changes if user is using Windows Classic theme. You might want to use the [WinForm workaround I proposed](#winform-workaround) up there. If you really want the ultimate consistency (like me), then you should never toggle the `WS_THICKFRAME`. There are other ways to prevent the window from resizing, I will leave that to you.
