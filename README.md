This is a new version of chromium open source project, its called voyager, 
its main feture is light-weight and speed, with a beautifull UI.

It also servers as a good example to understand how chrome's content module works

Build Instructions (currentely only supported for windows)
pls follow all instructions on -> http://www.chromium.org/developers/how-tos/build-instructions-windows
ounce u manage to build chromium, do ->
 
cd chromium/src
git clone https://github.com/KeshavZbhide/Voyager
gclient runhooks

This should build a visual studio project if u have configured gclient to use visual studio.
Thats about it, go ahead and compile. 

release and debug version of the software loads resources and images from ../KaminoResource from the current working directory of the executable.
U can obtain these resourse by downloading the built version of voyjor(msi installation) at http://voyjor.com/
msi installation unpacks these resources in the installation directory.
copy them to the right places relative to the executable you bilt.

 



