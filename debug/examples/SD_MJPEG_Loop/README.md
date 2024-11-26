<!--
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-09-22 14:37:05
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-11-26 16:16:15
 * @License: GPL 3.0
-->
### MJPEG Video

ffmpeg -i input.flv -vf "fps=60,pad=1920:1920:0:420:black,scale=-1:480:flags=lanczos" -q:v 10 -b:v 5000k output.mjpeg
```
ffmpeg -i input.mp4 -vf "fps=60,scale=450:450" -q:v 10 -b:v 5000k -c:v mjpeg output.mjpeg
```
