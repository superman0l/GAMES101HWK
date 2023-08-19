作业8-绳索模拟

很简单 算一下力算一下速度算一下位置就完事了

其中力的公式有用到课程中提到的弹簧阻力，另外还增加了一定程度的空气阻力，不然绳子都不带停的

各个位置计算方法的效果：

- 显式欧拉法用不了一点，绳子直接起飞
- 半隐式欧拉法效果明显好不少，还是会有点抖
- 显示Verlet效果相当不错，虽然不物理但是呈现的效果确实稳定

放一下力的计算代码

```
for (auto &s : springs)
{
    Vector2D dir = (s->m2->position-s->m1->position).unit();
    float f = s->k*((s->m2->position-s->m1->position).norm()-s->rest_length);
    s->m1->forces+=f*dir;
    s->m2->forces+=-f*dir;//胡克定律的弹簧力

    Vector2D dampforcetom2 = -0.05*dot(dir, s->m2->velocity-s->m1->velocity)*dir;
    Vector2D dampforcetom1 = -0.05*dot(-dir, s->m1->velocity-s->m2->velocity)*-dir;
    s->m1->forces+=dampforcetom1;s->m2->forces+=dampforcetom2;//弹簧阻力

    s->m1->forces+=s->m1->velocity*-0.005;
    s->m2->forces+=s->m2->velocity*-0.005;//空气阻力
}
```

位置计算的代码就不放了 懒