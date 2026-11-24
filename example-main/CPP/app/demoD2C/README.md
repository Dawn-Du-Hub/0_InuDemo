# 中文版 (English Vestion at below)



# 示例说明
D2C 是指深度图对齐RGB图，深度图的分辨率和RGB图的分辨率一致，RGB图中的点和深度图中的点位置一一对应
想知道RGB图中某点（X，Y）的深度值，只要取得深度图对应的点（X，Y）的数据即可

# 参数配置
修改 Params_D2C.yaml 中  inu_Depth_setting 的 RegistrationChannel 为 4 （R232 或 R258 设置 8）

mm 级精度，使用:
inu_Depth_setting:Channel: 3
如果想得到亚毫米级精度，使用
inu_Depth_setting:Channel: 5
inu_depth_setting:DepthScale:10
输出的深度数据被 X 10，需要除以10，得到带小数的 mm 数据

# English Version:
