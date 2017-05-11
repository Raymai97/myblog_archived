---
layout: post
title:  "Visual C++ supports UTF-8 string literal since VC6.0 (Updated)"
date:   2017-05-04 21:00:00 +0800
categories: msvc
permalink:  msvc-support-utf8-string-literal-since-vc6
---

If you're here for Visual C++ 6.0, [click here](#vc6).

I'm not sure about Visual C++ 7.0 and 7.1 (.NET 2002 and 2003), but since 8.0 (2005), Visual C++ compiler already supports  source file with UTF-8 no BOM encoding. It is just that, for it to work as expected, you need to follow a rule that no one in Microsoft will ever tell you. Now we are in 2017 and I have failed to find any thread or article that emphasizes it, that's why I think I need to write this.

![img]({% asset_path screenshot.png %})

Here is the test source file we will use today: [utf8-string-literal-test.cpp]({% asset_path utf8-string-literal-test.cpp %})

### For Visual C++ 8.0 and above

To get your UTF-8 source files compiled correctly, you must save in **UTF-8 without BOM** encoding, and the **system locale (non-Unicode language) must be English.**

![img]({% asset_path ansi-must-be-english.png %})

I know this looks like bullshit and a bit racist, but that is what my test result tells me. 

Here is what I get if I compile this file when my system locale is set to Japanese:

![img]({% asset_path japanese-locale-vc8.png %})

See? If the source file doesn't contain BOM (byte-order mark), and your system locale is not English, VC will assume that your source file is not in Unicode. *What a **bold** assumption.* I think this is yet another compatibility trick by Microsoft. If you are sure your source file always use UTF-8 without BOM, you may mod the compiler so it stops making such assumption.

### Mod the compiler (for Visual C++ 8.0 and above only)

I don't like the idea of changing system locale, so I mod the compiler instead. This is how it looks like after I have modded my compiler:

![img]({% asset_path japanese-locale-vc8-modded.png %})

Before you proceed, you should know that it is always a good idea to backup your compiler files. Better safe than sorry, right?

#### Step 1: Determine what to mod

Since this is related with character set, I `grep` for related API such as `WideCharToMultiByte`, then I choose a few files I think I needed to mod. For this example, I choose `cl.exe`, `c1.dll`, `c1xx.dll` and `c2.dll`.

![img]({% asset_path grep-widechartomultibyte.png %})

#### Step 2: Mod the files to load `kernel31` instead

Open the file in hex editor. Search for `kernel32.dll` string and replace it with `kernel31.dll` string. If there are multiple occurences, choose the one that is surrounded by many API names.

#### Step 3: Assert codepage 1252 in your `kernel31`

If you don't know `kernel31`, please look forward to a future post I will write someday.

In your `kernel31` project, you need to redirect two functions to your own implementation, `MultiByteToWideChar` and `WideCharToMultiByte`. The idea is, if the incoming codepage is not `CP_UTF8`, change it to `1252` (English codepage), and then return `MultiByteToWideChar` or `WideCharToMultiByte`. This is how I coded it:

```c++
KERNEL31_API int __stdcall
My_MultiByteToWideChar(
    UINT codepage, DWORD dwFlags, LPCSTR szMulti, int cbMulti,
    LPWSTR szWide, int cchWide)
{
    if (codepage != CP_UTF8) {
        codepage = 1252;
    }
    return MultiByteToWideChar(codepage, dwFlags, szMulti, cbMulti,
        szWide, cchWide);
}

KERNEL31_API int __stdcall
My_WideCharToMultiByte(
    UINT codepage, DWORD dwFlags, LPCWSTR szWide, int cchWide,
    LPSTR szMulti, int cbMulti, LPCSTR lpDefChar, LPBOOL lpUsedDefChar)
{
    if (codepage != CP_UTF8) {
        codepage = 1252;
    }
    return WideCharToMultiByte(codepage, dwFlags, szWide, cchWide,
        szMulti, cbMulti, lpDefChar, lpUsedDefChar);
}
```

If you prefer to download the one I created instead:  
[Kernel31 for VC8.0 (VC2005)](Kernel31-vc8.7z)  
[Kernel31 for VC14.0 (VC2015)](Kernel31-vc14.7z)

#### Step 4: Showtime

Just put the `kernel31.dll` into the compiler directory, and you're done.

![img]({% asset_path vc8-dir-after-mod.png %})

Congraz! Now your VC can compile UTF-8 source file without BOM! No more crappy BOM or UTF-16!

<a name="vc6"></a>
### For Visual C++ 6.0

Let me make one thing clear, **the IDE of Visual C++ 6.0 itself doesn't know anything about Unicode.** The moment you open your UTF-8 source file in the IDE, you're doomed. You **must use command-line to compile such thing with VC6.0**. In fact, the compiler doesn't know Unicode too, but because **UTF-8 is a multi-byte character set**, it happens to support such encoding too, with a little bit issue of course.

Let's see what would happen if we compile the source file with VC6.0 compiler.

![img]({% asset_path vc6.png %})

This is the issue I was talking about. To fix it, just append a space to the problematic string literal.

**UPDATE May 11: [A better way has been discovered](#vc6-localeName).**

![img]({% asset_path vc6-fixed.png %})

信じられない？ See for yourself:  
[utf8-string-literal-test-vc6-exe.7z]({% asset_path utf8-string-literal-test-vc6-exe.7z %})

### UPDATE: File encoding plays role too

You might think that as long as your system locale is English, which enable MSVC to compile UTF-8 source file, the string literal in your source file will always be UTF-8. This is NOT true. Here is the pseudocode that describes the MSVC behavior:

```c++
if (ansi_codepage == 1252) {
    open_source_file_with_encoding(utf_8);
}
else {
    open_source_file_with_encoding(ansi_codepage);
}
copy_string_literal_into_exe_with_encoding(file_encoding);
```

| ANSI CP 	| Source file CP 	| String literal CP in EXE 	| Conversion flow        	|
|---------	|----------------	|--------------------------	|------------------------	|
| 1252    	| UTF-8          	| UTF-8                    	| UTF-8 -> wide -> UTF-8 	|
| 1252    	| 932            	| 932                      	| UTF-8 -> wide -> 932   	|
| 1252    	| 936            	| 936                      	| UTF-8 -> wide-> 936    	|
| 936     	| UTF-8          	| UTF-8                    	| 936 -> wide -> UTF8    	|
| 936     	| 932            	| 932                      	| 936 -> wide -> 932     	|
| 936     	| 936            	| 936                      	| 936 -> wide -> 936     	|
| 932     	| UTF-8          	| UTF-8                    	| 932 -> wide -> UTF-8   	|
| 932     	| 932            	| 932                      	| 932 -> wide -> 932     	|
| 932     	| 936            	| 936                      	| 932 -> wide -> 936     	|

<a name="vc6-localeName"></a>
### UPDATE: For VC6.0, the key is LocaleName.

Say no more, I am totally speechless now. As long as your `LocaleName` is valid and start with `en-`, it will compile without issue. I'm not sure why (racist?) but it works. See for yourself.

![img]({% asset_path vc6-japan-locale-name.png %})

![img]({% asset_path vc6-us-locale-name.png %})

Good thing is, this change **does not require a reboot**, and you can automate it with batch file. I have wrapped as a function, so you can integrate it into your batch file easily. Note that you need to `setlocal enableDelayedExpansion`. Don't know where to put? Put it after your `@echo off`.

```
:localeName
set _path_="HKCU\Control Panel\International"
set _name_=LocaleName
if "%~1"=="patch" (
	call :localeName get _localeName_
	call :localeName set en-US
)
if "%~1"=="unpatch" (
	call :localeName set !_localeName_!
)
if "%~1"=="get" (
	for /f "tokens=3 skip=2" %%i in ('reg query !_path_! /v !_name_!') do (
		set _localeName_=%%i
	)
)
if "%~1"=="set" (
	reg add !_path_! /v !_name_! /d "%~2" /f >nul || exit/b 1
)
exit/b
```

Before compile script: `call :localeName patch`

After compile script: `call :localeName unpatch`
