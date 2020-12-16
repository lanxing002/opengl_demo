# IBL-Part3 技术细节

## 1.反射向量

在ibl有这样一段代码，之前不是很理解，且匆匆划过。

![image-20201215195129369](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201215195129369.png)

主要原因是先入为主的将R看作光线在fragment的反射方向。但是其真正的解释如下图：

![image-20201215201658270](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201215201658270.png)

也就是以上代码是求view在normal的反射方向。

## 2.重要性采样

重要性采样的公式就不再重复了，但是有一个细节：brdf中的$D(h)$从公式可以看出是围绕半角向量（$dw_h$）的法线分布；pre-filtered的值是在入射方向（$dw_i$）上使用ggx进行重要性采样来进行卷积计算的结果。构造两者之间的几何联系如下图：

![image-20201215204019975](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201215204019975.png)

由于$w_i$是由$w_o$围绕$w_h$反射进行计算，因而有$\theta_i=2\times\theta_h$，且$\phi_i=\phi_h$。那么在球坐标系下：
$$
\frac{dw_h}{dw_i}=\frac{sin\theta_h\cdot d\theta_h\cdot d\phi_d}{sin2\theta_h\cdot2d\theta_h\cdot d\phi_h}
\\=\frac{1}{4\cdot cos\theta_h}=\frac{1}{4(w_i\cdot w_h)}=\frac{1}{4(w_o\cdot w_h)}
$$
那么其概率密度函数之间的转换为：
$$
p(w_i)=\frac{p(w_h)}{4(w_o\cdot w_h)}
$$

## 3.Split Sum Approximation

该公式之前没有进行推导，现在根据网上一些资料，尝试推导一下：
$$
L_o=\frac{\int_\Omega{L_i(l)f_s(w_i, w_o)(n\cdot w_i))dw_i}}{\int_\Omega{f_s(w_i, w_o)(n\cdot w_i))dw_i}}\int_\Omega{f_s(w_i,w_o)(n\cdot w_i))dw_i}
$$
在实时渲染中，由于内存限制，不允许存放所有出射方向的积分结果，因此假设出射方向即观察向量和法向量一样。这会造成掠角上与实际不符（实际为各向异性，现在为各向同性）。现在$v=n$。然后根据ggx法线分布在$w_i$上进行重要性采样。公式推导如下（借鉴知乎一位同学的推导）

![image-20201216142342261](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201216142342261.png)

过程①由蒙特卡洛积分公式直接得到。由①到②有几个点需要注意：



- 前一节已经推导出：$p(w_i)=\frac{p(w_h)}{4(w_o\cdot w_h)}$，而公式①到②可以看出$p(w_i^{(k)})=\frac{D(w_h^{(k)})(w_h \cdot n)}{4(w_o \cdot w_h^{(k)})}$。可以看出$p(w_h)=D(w_h)(w_h\cdot n)$。

- $w_h$的概率密度公式应该满足微分为1的条件,将其转换为球坐标系下进行积分，有$w_h\cdot n = cos\theta$
  $$
  \int p(w_h)dw_h=\int D(w_h)\cdot(w_h\cdot n) dw_h 
  \\=\int_0^{2\pi}{d\phi}\int_0^{0.5\pi}\frac{\alpha^2}{\pi(cos\theta^2(\alpha^2-1)+1)^2}(cos\theta)(sin\theta)d\theta
  $$
  求一下原函数为：
  $$
  F(\theta)=\frac{\alpha^2}{2(\alpha^2-1)(cos\theta^2(\alpha^2-1)+1)}
  $$
  则原积分公式可以化简为：


$$
  \int p(w_h)dw_h=\frac{1}{\pi}\int_0^{2\pi}(F(0.5\pi)-F(0))d\phi=1
$$

公式②到③是没有什么说的。现在处理一下公式③。

- 当$\theta$小于45°的时候，Fresnel-Schlick的返回值变化很小，可以看作一个常数。当$\theta$的值大于45°的时候，在光滑平面上，h与l一致的概率很小；在非光滑平面，镜面反射效果很不明显，因此将F当作常数约去之后，对于最终效果来说，可以接受吧！

- 由于假设$n=v=r$，即$w_o=n=R, 其中R=reflect(-w_o, n)$,可以约去$\frac{(w_o\cdot w_h)}{w_h\cdot n}$。对于$G(w_o, w_i^{h})$，把$w_o=n$带入进去之后，变成$G(w_i^{k})$。原公式化简为：
  $$
  L_c(w_o)=\frac{\sum_k^K{G(w_i^{(k)})L_i(w_i^{(k)})}}{\sum_k^K{G(w_i^{(k)})}}
  $$
  如果使用ggx几何函数，其中$G(w_i^{(k)})=\frac{n\cdot w_i^{(k)}}{n\cdot w_i^{(k)}(1-k)+k}$。而虚幻实现中直接用$n\cdot w_i$，即：

  
  $$
  L_c(w_o)=\frac{\sum_k^K{(n\cdot w_i^{(k)})L_i(w_i^{(k)})}}{(n\cdot w_i^{(k)})}
  $$

- 以上解释的步骤一中将F项看作const约去。若$F(w_o,w_h^{(k)})\cdot G(w_i^{(k)})=n\cdot w_i^{(k)}$。也可以解释通。

# 4.效果

![image-20201216153838478](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201216153838478.png)

在opengl复现了一下效果，打算之后使用复杂些的模型和材质贴图试试效果。