<!--
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-09-22 14:37:05
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2023-10-06 18:06:12
 * @License: GPL 3.0
-->
### MJPEG Video

ffmpeg -i input.flv -vf "fps=60,pad=1920:1920:0:420:black,scale=-1:480:flags=lanczos" -q:v 10 -b:v 5000k output.mjpeg
```
