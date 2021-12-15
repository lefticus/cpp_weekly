Here in this version, I wanted to increase compilation speed by splitting the compiled file in multiple objects
CMakeList compile the smallpt_parallel file one time per point of the image, setting all coordinates/sizes as defines
Finally gathering them in the main.

The third part of this exercise that I imposed myself was the hardest, and allowed me to generate a bigger image (than with the 'one file constexpr' that is only compilable for really small images...) (I did not made the auto exercise because i'm from the 'no auto club' (except for really specific use, like in templates))

Even with 10 cores compiling at 2.6Ghz it lasted for more than a day... But finally got an executable and an image.

Remarks:
- The image is far from original:
  * but recognisable if we know original
  * some white lines at the top, maybe I did something wrong with cam vector
  * vertical 'black' lines probably from interferences due to random being reused when sampling the points? (I had to do it like that or the random calculation time would just explode and the compilation fails.
- The compilation result from constexpr smallpt was quite small (as expected), but here with parallel version the executable is 3Mo and is really slow. I didn't expect it, but I do not see the problem. If someone has some ideas, I could for sure test them (I will keep the compilation file for some time before deleting them... In order to avoid someone else loosing a day ;)

PS: I did not plan touching this anymore, the learning/exercise purpose is achieved, and looking for the mistakes won't bring a lot more.

