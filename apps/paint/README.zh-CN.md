# 像素画板

使用 `contracts/app.h` 的、与硬件无关的原生 C 应用。
English: [README.md](README.md)。

- 48×64 网格、15 色；整块屏幕就是画布，没有常驻工具栏。
- 点右下角按钮，从底部滑出两行调色板；选完自动收起，并重绘被它盖住的几行。
- 快速滑动用 Bresenham 补点，笔迹不断线；手指滑出画布时裁剪线段，而不是放弃整笔。
- 位图按 4 bit/格 打包（1536 字节），抬手时写入 storage，启动时读回。
- 需要 RGB565 显示（≥160×200）和指针输入；storage 可选，只影响能否保存。
- 存储长度与当前网格不符时**拒绝加载**，而不是画一半。

```sh
python tools/dada.py build --app paint
python tools/dada.py flash --app paint --port YOUR_PORT
```

主机回归测试见 `tests/host/test_paint.c`。
