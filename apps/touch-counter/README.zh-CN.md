# 触摸计数器

通过 `contracts/app.h` 使用显示与输入，应用源码不依赖 ESP-IDF、引脚或板卡型号。
English: [README.md](README.md).

- 启动显示 `0000`；点击屏幕上方四分之三区域加一，到 `9999` 停止增加。
- 长按只计一次；按住滑到底部不会触发清零，需要松开再点。
- 底部四分之一区域为 RESET；无持久化，重启归零。
- 需要至少 160×160 的 RGB565 屏幕与指针输入。
- 私有渲染器使用 8 行像素条带，240 像素宽时占 3,840 字节，不分配全屏帧缓冲。
- 无效输入不重新触发点击；显示错误向固件入口返回，由入口停止和清理。

```sh
python tools/dada.py build --app touch-counter
python tools/dada.py flash --app touch-counter --port YOUR_PORT
```

主机回归测试见 `tests/host/test_counter.c`。实机还须验收画面方向、点击区域对应及断电重启。
