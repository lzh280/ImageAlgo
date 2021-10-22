# ImageAlgo
基于Qt的图像矢量化工具，主要包括以下步骤：
（1）预处理：OSTU最佳阈值、中值/高斯滤波、锐化、边缘检测（Sobel和Canny算子）;
（2）边缘追踪：径向扫描法（http://www.imageprocessingplace.com/downloads_V3/root_downloads/tutorials/contour_tracing_Abeer_George_Ghuneim/ray.html）；
（3）边缘简化：根据DDA/Bresenham画线原理，逆向判断像素点共线。亦可采用Douglas-Peucker简化；
（4）边缘顺滑：相邻折线段采用二阶Bezier曲线替代轮廓。
