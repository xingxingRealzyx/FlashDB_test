# FlashDB 测试代码
文档：https://w6pkhwnako.feishu.cn/docx/PpO6dQPwyo5e43xLw3Lc0JiynJb

测试发现kvdb在命中cache时insert时间为3ms，没命中时为80ms，不知道是否正常，官方git仓库没有kvdb的性能测试数据，已经给作者提交了issue。

nvs在写i32类型数据时，仅用时24us！！！

