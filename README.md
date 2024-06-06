# CANopen 
## 项目简介
此项目是一个用于CANopen协议的字典工具，包含了CANopen通信协议及相关的测试和控制示例，遵循CiA301和CiA402规范。

协议栈采用GitHub开源的 CANopenNode：https://github.com/CANopenNode/CANopenNode，感兴趣的朋友可以自己移植一下

## 视频演示链接
https://www.bilibili.com/video/BV1i4421X7mx/?spm_id_from=333.337.search-card.all.click&vd_source=8482d95fe0873c36af69fd64ae582056

## 目录结构
CANopen字典工具/CANopenEditor-build: CANopen字典工具的构建目录

固件: 目前只适配了DM4310的V3版本电机固件，使用上位机升级就行，升级完固件之后，目前还没法使用上位机的CAN进行控制，只能按命令格式去操作。

字典: 存放CANopen协议的字典文件

注意事项：波特率只支持修改位 1M 500k 250k 125k

控制例程: 存放控制相关的示例代码

CANopen协议移植及测试.pdf: CANopen协议的移植及测试文档

DM CANopen通信协议.md: DM CANopen通信协议文档

DM电机字典.md: DM电机相关的字典文档


