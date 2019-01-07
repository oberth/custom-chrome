## At least read this before contacting me | 2019 update

I guess some people can't be bothered to read the manual. This is **not** a basis for your frame-modifying GUI application. This is a minimalistic sample that began as a solution to a problem a friend of mine proposed, a demo on how to modify the window frame / chrome area in order to have your content protrude into it.

**This is not how you build an actual GUI application.** The renderer, resource management and compositor are all fused into a single simplified system, along with a very coarse application and window construct. All of the GUI "APIs" are limited only to what is needed to facilitate the mockup with **minimal effort** and lazily mix with platform API elements. All of this minimizes the amount of code and maximizes the willingness of a user to read through it. To this end, I spent a few hours simplifying the code further.

The sample was built on the 15063 Windows 10 SDK using the latest Visual Studio 2017 and MSVC version. I always make a strong attempt to use the latest language features, but that effort is often hampered by MSVC's ineptitude.

# custom-chrome

A sample demonstrating how to make your GUI protrude into the non-client area on Windows (down to Vista where DWM was introduced), while preserving user Windows theme settings, doing the bare minimum when handling window behavior.

![](https://i.imgur.com/OojkRRQ.png)

You can find a short recap on how it works [here](https://github.com/oberth/custom-chrome/wiki). This was developed for demonstration purposes at the behest of a friend who brought it up as a curiosity, with minimal time invested being the driving principle meaning the **code is quick and dirty**. It is licensed under MIT, as Unlicense has issues in certain countries. Enjoy.

## What this sample *isn't* about

1. Code that you're supposed to drop into your code **(it is for demonstration purposes only)**.
2. Building a framework you can use for your applications.
3. Implementing any logic behind the pretty face of it, mockup is just to show the potential.

This approach is heavily undocumented, especially as it pertains to preserving the style and behavior of an ordinary window. There is also a lot of misunderstanding in the community on how to go about doing these feats, especially as they were made popular by the distinct visual style of browsers like Mozilla Firefox (which redraws it personally) and Google Chrome which place their tabs into the window frame, along with some less obvious examples like Ribbon UI based interfaces (Windows/File Explorer).
