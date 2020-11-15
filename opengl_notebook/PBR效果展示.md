## PBR效果展示

### 1. 效果展示

![image-20201115144654560](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115144654560.png)

这个效果图用了四个点光源，用加法来计算着色方程中的积分。点光源的颜色值都为白色。

从上到下边是粗糙度从0.05到1.0的排列，从左边到右边是金属度从1.0到0.0的效果排列。为gui框架添加了一个控制组件，可以在控制组件中调节金属度等pbr的输入参数。可以看出当粗糙度越大的时候，高光点越来越粗，反射的强度也越来越弱，这是能量守恒的一种体现。

### 2. 一些思考

![](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115151634566.png)

当金属度介于0和1之间的时候，球体上的高光点是红中带白色，随着金属度的越来越大，红色越来越多，最后完全变成红色。这个效果的原因是，当我们用金属度插值出反射率。shader代码为：

```glsl
F0 = mix(F0, albedo, metallic); //插值之后介于金属与非金属的反射率
...
vec3 kS = F; //镜面反射系数
vec3 kD = vec3(1.0) - kS; //能量守恒计算漫反射
kD *= 1.0 - metallic; //金属没有漫反射
```

### 2.1 深入解释下

刚开始我想，虽然这可以大概解释为什么红色和白色混在一起，但是为什么是红色慢慢包围白色，而不是两者之间的的插值呢。比如金属度0.5的时候

```glsl
mix_color = 0.5 * (white + red);
mix_color的值(1.0, 0.5, 0.5);
```

而(1.0, 0.5, 0.5)色块如下图：

![image-20201115151159853](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115151159853.png)

（在思考到这里的时候，我发现我的思路是不合适的，其实这个我提出的这个问题本身就不好，因为我根本无法看到红色和白色混合下的效果。）

然后，我就更改方向解释这个效果，首先从一个金属度为1的效果看

![image-20201115151932061](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115151932061.png)

可以看出是完全没有白色光效果的时候，这是由于金属没有漫反射，只有镜面反射，而且镜面反射对于各个波长的光反射率不一样，在这个demo中，只反射红光。

当金属度渐渐从1减少的时候，插值出来的反射率也在减少，但是此时有了漫反射，漫反射使得从左向右对比，发现左边的偏向黑，右边更加红，这是因为漫反射（折射后的效果）出红光。

![image-20201115192352089](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115192352089.png)

但是思考到这里我最初提出的问题还是没有解决，我决定在shader代码中用一下debug手段，首先我把漫反射和几何遮挡以及法线分布函数都去掉，然后只渲染一个点光照，得到如下的效果图。

![image-20201115190120021](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115190120021.png)

当金属度小于1的时候，渐渐偏向粉色，其实这种效果就是红色和白色mix的效果。在shader层面，金属度为1的时候，$F0=vec3(0.5, 0.0, 0.0)$，但是当金属度小于1的时候，$F0=vecx3(0.5, x, x)$，其中x的值大于0且小于1。但是粉色怎么最后怎么会白色呢？

![img](file://C:/Users/scent/AppData/Roaming/Typora/typora-user-images/image-20201115151634566.png?lastModify=1605438079)

这是我们去除了双向反射函数中的法线分布函数的作用。加上法线分布函数之后，就可以得到如下的效果，

![image-20201115190958577](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115190958577.png)

对比加上法线分布函数和没有法线分布函数的图像，可以把法线分布函数理解成为讲反射的高光点聚集或者发散，并且满足能量守恒。

之前绘制过法线分布函数的图像，如下图：

![image-20201115191323032](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201115191323032.png)

当粗糙度很小的时候，法线和H向量越一致的情况下，概率值越大。之前由于mix的原因，菲涅尔函数结果rgb都大于0，乘以分布函数结果的话，可能会导致rbg的值都大于1，截断以后便会出现白色光效果。

