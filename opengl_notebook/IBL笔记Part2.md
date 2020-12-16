# IBL笔记Part2

part1说了下为什么要用IBL，以及如何使用IBL技术计算漫反射部分的值，主要是计算一个辐照度图（cubemap）。

## 3. IBL的镜面反射部分

镜面部分的公式为：

![[公式]](https://www.zhihu.com/equation?tex=L_s%28%5Comega_o%29%3D%5Cint_%5COmega+f_s%28%5Comega_i%2C%5Comega_o%29+L_i+%5Cleft%28+%5Comega+_+%7B+i+%7D+%5Cright%29+n+%5Ccdot+%5Comega+_+%7B+i+%7D+d+%5Comega+_+%7B+i+%7D)

如果直接将渲染方程使用蒙特卡洛积分法进行积分，公式为：

![[公式]](https://www.zhihu.com/equation?tex=%5Cint_%5COmega+f_s%28%5Comega_i%2C%5Comega_o%29+L_i+%5Cleft%28+%5Comega+_+%7B+i+%7D+%5Cright%29+n+%5Ccdot+%5Comega+_+%7B+i+%7D+%5Cmathrm%7Bd%7D%5Comega_i+%5Capprox+%5Cfrac%7B1%7D%7BN%7D%5Csum_j%5EN%5Cfrac%7Bf_s%28%5Comega_i%5E%7B%28j%29%7D%2C%5Comega_o%29L_i%28%5Comega_i%29%28n+%5Ccdot+%5Comega_i%5E%7B%28j%29%7D%29%7D%7Bp%28%5Comega_i%5E%7B%28j%29%7D%29%7D)

计算值的结果由$w_i,n,\alpha,F0$决定，如果直接预计算的话，需要存储这四个变量上的结果，可以预想，预计算的数据量很大。因此，虚幻提出了一种简化的预计算-Split Sum Approximation，其公式如下：



![[公式]](https://www.zhihu.com/equation?tex=%5Cfrac%7B1%7D%7BN%7D%5Csum_j%5EN%5Cfrac%7Bf_s%28%5Comega_i%5E%7B%28j%29%7D%2C%5Comega_o%29L_i%28%5Comega_i%5E%7B%28j%29%7D%29%28n+%5Ccdot+%5Comega_i%5E%7B%28j%29%7D%29%7D%7Bp%28%5Comega_i%5E%7B%28j%29%7D%29%7D++%5Capprox++%5Cleft%28%5Cfrac+%7B%5Csum_k%5EN+L_i%28%5Comega_i%5E%7B%28k%29%7D%29W%28%5Comega_i%5E%7B%28k%29%7D%29%7D+%7B%5Csum_k%5EN+W%28%5Comega_i%5E%7B%28k%29%7D%29%7D+%5Cright%29++%5Cleft%28%5Cfrac%7B1%7D%7BN%7D%5Csum_j%5EN%5Cfrac%7Bf_s%28%5Comega_i%5E%7B%28j%29%7D%2C%5Comega_o%29%28n+%5Ccdot+%5Comega+_+%7B+i+%7D%5E%7B%28j%29%7D%29%7D%7Bp%28%5Comega_i%5E%7B%28j%29%7D%29%7D%5Cright%29)

其中权重函数$W(w_i)=n\cdot w_i$。

### 3.1 pre-filter environment map

Split Sum Approximation 公式左半部分：

![[公式]](https://www.zhihu.com/equation?tex=L_c%5E%2A%28%5Cmathbf%7BR%7D%29+%5Capprox%5Cfrac+%7B%5Csum_k%5EN+L_i%28%5Comega_i%5E%7B%28k%29%7D%29%28n+%5Ccdot+%5Comega_i%5E%7B%28k%29%7D%29%7D+%7B%5Csum_k%5EN+%28n+%5Ccdot+%5Comega_i%5E%7B%28k%29%7D%29%7D+)

这个公式可以看作将Image Light与根据GGX法线采样结果卷积，卷积（积分）的时候还使用一个权重因子$W(w_i^k)=(n\cdot w_i^k)$。卷积结果存储在一张cubemap中，名为pre-filter environment map。ggx法线分布与粗糙度有关系，对于不同的粗糙度，对应不同的pre-filter map, 如果粗糙度没有预计算，便使用已有数据进行插值。

![图：不同粗糙度对应的pre-filter map](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203140951411.png)

卷积计算中使用的法线分布函数是基于微平面模型，与half向量有关，即与view向量相关。在这里使用了假设$n=v=r$。使用该假设目-减少预计算结果数量级；使用该假设的影响-它是一个各向同性假设，因此在view视角与法线的夹角很大（grazing angle），没有反射拉长的现象。

![image-20201203144447913](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203144447913.png)

### 3.2  BRDF LUT（双向反射函数积分贴图）

LUT存储的是![[公式]](https://www.zhihu.com/equation?tex=%5Cint_%5COmega+f_s%28%5Comega_i%2C%5Comega_o%29+n+%5Ccdot+%5Comega+_+%7B+i+%7D+d+%5Comega+_+%7B+i+%7D)的预计算结果，对于这个公式进行一些数学变化：

![[公式]](https://www.zhihu.com/equation?tex=%5Cbegin%7Baligned%7D+%5Cint+_+%7B+%5COmega+%7D+f+_+%7B+s+%7D+%5Cleft%28%5Comega+_+%7B+i+%7D+%2C+%5Comega+_+%7B+o+%7D+%5Cright%29+n+%5Ccdot+%5Comega+_+%7B+i+%7D+d+%5Comega+_+%7B+i+%7D+++%26%3D+%5Cint+_+%7B+%5COmega+%7D+f+_+%7B+s+%7D+%5Cleft%28%5Comega+_+%7B+i+%7D+%2C+%5Comega+_+%7B+o+%7D+%5Cright%29+%5Cfrac+%7B+F+%5Cleft%28+%5Comega+_+%7B+o+%7D+%2C+h+%5Cright%29+%7D+%7B+F+%5Cleft%28+%5Comega+_+%7B+o+%7D+%2C+h+%5Cright%29+%7D+n+%5Ccdot+%5Comega+_+%7B+i+%7D+d+%5Comega+_+%7B+i+%7D%5C%5C++%26%3D+%5Cint+_+%7B+%5COmega+%7D+%5Cfrac+%7B+f+_+%7B+s+%7D+%5Cleft%28%5Comega+_+%7B+i+%7D+%2C+%5Comega+_+%7B+o+%7D+%5Cright%29+%7D+%7B+F+%5Cleft%28+%5Comega+_+%7B+o+%7D+%2C+h+%5Cright%29+%7D+%5Cleft%28+F+_+%7B+0+%7D+%2B+%5Cleft%28+1+-+F+_+%7B+0+%7D+%5Cright%29+%5Cleft%28+1+-+%5Comega+_+%7B+o+%7D+%5Ccdot+h+%5Cright%29+%5E+%7B+5+%7D+%5Cright%29+n+%5Ccdot+%5Comega+_+%7B+i+%7D+d+%5Comega+_+%7B+i+%7D%5C%5C++%26%3DF_0+%5Cint+_+%7B+%5COmega+%7D+%5Cfrac+%7B+f+_+%7B+s+%7D+%5Cleft%28%5Comega+_+%7B+i+%7D+%2C+%5Comega+_+%7B+o+%7D+%5Cright%29+%7D+%7B+F+%5Cleft%28+%5Comega+_+%7B+o+%7D+%2C+h+%5Cright%29+%7D%281-%281-%5Comega_o%5Ccdot+h%29%5E5%29+n+%5Ccdot+%5Comega_i+d%5Comega_i%5C%5C+%26%5Cquad+%2B+%5Cint+_+%7B+%5COmega+%7D+%5Cfrac+%7B+f+_+%7B+s+%7D+%5Cleft%28%5Comega+_+%7B+i+%7D+%2C+%5Comega+_+%7B+o+%7D+%5Cright%29+%7D+%7B+F+%5Cleft%28+%5Comega+_+%7B+o+%7D+%2C+h+%5Cright%29+%7D%281-%5Comega_o%5Ccdot+h%29%5E5+n+%5Ccdot+%5Comega_i+d%5Comega_i%5C%5C++%26%3DF_0%2A%5Ctext%7Bscale%7D%2B%5Ctext%7Bbias%7D+%5Cend%7Baligned%7D)

积分结果scale和bias不再包含菲涅尔部分了 。公式虽然出现了$h,w_0$，但是只用到了他俩的点乘，因此只将$cos\theta=h\cdot w_0$ 作为预计算结果的存储维度。另一个存储维度是粗糙度，因为基于微平面理论的DFG还与粗糙度有关，因此预计算的另一个存储维度就是粗糙度。通常情况，将预计算结果存储在一张二维贴图中。如下：

![image-20201203151231056](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203151231056.png)

  

scale和bias的计算根据法线分布进行重要性采样的蒙特卡洛积分来解决。其计算过程公式如下：

![[公式]](https://www.zhihu.com/equation?tex=%5Cbegin%7Baligned%7D+%5Ctext%7Bscale%7D++%26%5Capprox+%5Cfrac%7B1%7D%7BN%7D%5Csum_k%5EN%5Cfrac+%7B%5Cfrac%7BD%28%5Comega_h%5E%7B%28k%29%7D%29G%28%5Comega_o%2C%5Comega_i%5E%7B%28k%29%7D%29%7D%7B4%28%5Comega_o%5Ccdot+n%29%28%5Comega_i%5E%7B%28k%29%7D%5Ccdot+n%29%7D%281-%281-%5Comega_o%5Ccdot%5Comega_h%5E%7B%28k%29%7D%29%5E5%29%28%5Comega_i%5E%7B%28k%29%7D+%5Ccdot+n%29%7D+%7Bp%28%5Comega_i%5E%7B%28k%29%7D%29%7D%5C%5C++%26%3D+%5Cfrac%7B1%7D%7BN%7D%5Csum_k%5EN%5Cfrac+%7B%5Cfrac%7BD%28%5Comega_h%5E%7B%28k%29%7D%29G%28%5Comega_o%2C%5Comega_i%5E%7B%28k%29%7D%29%7D%7B4%28%5Comega_o%5Ccdot+n%29%28%5Comega_i%5E%7B%28k%29%7D%5Ccdot+n%29%7D%281-%281-%5Comega_o%5Ccdot%5Comega_h%5E%7B%28k%29%7D%29%5E5%29%28%5Comega_i%5E%7B%28k%29%7D+%5Ccdot+n%29%7D+%7B%5Cfrac%7BD%28%5Comega_h%5E%7B%28k%29%7D%29%28%5Comega_i%5E%7B%28k%29%7D%5Ccdot+n%29%7D%7B4%28%5Comega_o%5Ccdot%5Comega_h%5E%7B%28k%29%7D%29%7D%7D%5C%5C++%26%3D+%5Cfrac%7B1%7D%7BN%7D%5Csum_k%5EN%5Cfrac+%7BG%28%5Comega_o%2C%5Comega_i%5E%7B%28k%29%7D%29%28%5Comega_o%5Ccdot+%5Comega_h%5E%7B%28k%29%7D%29%281-%281-%5Comega_o%5Ccdot%5Comega_h%5E%7B%28k%29%7D%29%5E5%29%7D+%7B%28%5Comega_o%5Ccdot+n%29%28%5Comega_i%5E%7B%28k%29%7D%5Ccdot+n%29%7D%5C%5C+%5Cend%7Baligned%7D)4.

### 3.3 综合

最终的计算公式为![[公式]](https://www.zhihu.com/equation?tex=L%28%5Comega_o%29%3Dk_d%5E%2A%5Crho%5C+%5Ctext%7BIrradianceMap%7D%28%5Cmathbf%7Bn%7D%29+%2BL_c%5E%2A%28%5Cmathbf%7BR%7D%29%28F_0%2A%5Ctext%7Bscale%7D%2B%5Ctext%7Bbias%7D%29)

通过预计算我们通过一张envmap得到三张贴图，用来实时计算$L(w_0)$。核心代码框架如下：

```glsl
vec3 F0 = mix(vec3(0.04), albedo, metalness);
vec3 F = F_roughness(wo, normal, F0, roughness);
vec3 kS = F;
vec3 kD = (1 - metalness) * (vec3(1) - kS);

vec3 irradiance = texture(irradianceMap, norm).rgb;

vec3 diffuse = irradiance * albedo;

vec3 R = reflect(-wo, norm);
const float MAX_REFLECTION_LOD = 4.0;
// 用 R 来采样
vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
vec2 scale_bias = texture(brdfLUT, vec2(max(dot(norm, wo), 0.0), roughness)).rg;
vec3 specular = prefilteredColor * (F0 * scale_bias.x + scale_bias.y);

vec3 ambient = kD * diffuse + specular;
```

## 4.补充说明：

### 4.1重要性采样

重要性采样的采样空间分布再一个我们构造出来概率空间中，它大致与$f(x)$形状一致。相比于使用均匀分布空间，重要性采样有着更快的收敛速度---相同采样次数，方差更小。在ue4的实现中，重要性采样的关键代码是：

![image-20201203160232510](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203160232510.png)

这里将均匀分布分低差异序列中的xi.y重新映射到服从法线分布的$cos\theta$上。这里使用了反演法（一种将随机分布映射到期望分布的方法）：

![image-20201203161129562](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203161129562.png)

不过我将代码里的公式提取出来，通过反演法的步骤逆向得到，$cos\theta$的分布服从的是：$\frac{-2(n\cdot h)\alpha^2}{[(n\cdot h)^2(\alpha^2-1)+1]^2}$，与ggx法线分布有点出入，不知道是自己错了，还是一个trick。我的推到如下（高中数学）：

![image-20201203163910958](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203163910958.png)

重要性采样还有一个问题，就是概率密度公式的转换。对于服从法线分布的重要性采样，是围绕半角向量分法线分布，但是积分公式是对入射向量$w_i$的积分，因此需要将半角分布的pdf转换为入射角的pdf。

转换公式为：$p_y(y)(\frac{dy}{dx})=p_x(x)$ 。那么将半角向量的概率分布转为入射方向的概率分布为：

![image-20201203171100844](C:\Users\scent\AppData\Roaming\Typora\typora-user-images\image-20201203171100844.png)

转换后的pdf为$p(\theta)=\frac{p_h(\theta)}{4(w_o\cdot w_h)}$

但是，ue4是根据$cos\theta$来生成半程向量，而不是直接使用$\theta$。