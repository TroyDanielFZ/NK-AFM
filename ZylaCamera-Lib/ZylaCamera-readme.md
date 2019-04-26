
# Andor Zyla Camera

1. 程序需要OpenCV支持。

2.  安装相应的驱动程序`AndorSDK3Setup-3.13.30000.0.exe`，安装之后的目录设为`$(Andor)`，则64位的驱动在`$(Andor)`下，32位的驱动在`$(Andor)/win32/`下；

3. 在32位下使用`$(Andor)/atcore.h`以及`$(Andor)/win32/atcore.lib`（没有验证）；

4. 在64位下使用`$(Andor)/atcore.h`以及`$(Andor)/atcorem.lib`；

5. 然后使用由我（[@Troy\_Daniel](mailto:troy_daniel@163.com))编写的`ZylaCamera`类来完成相关调用。参见`ZylaCamera-example.cpp`。

   1. 在`ZylaCamera-example.cpp`中演示的是「存储到文件」与「从文件中获取图像」两个功能。

   2. 在实际使用时，应直接使用：

```c++
while(mat = camera.accquisitionMat() && !mat.empty()){
	// your processing code
}
```




