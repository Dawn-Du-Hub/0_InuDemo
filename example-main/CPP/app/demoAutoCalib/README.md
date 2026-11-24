# 中文版 (English Vestion at below)

在模组正常使用过程中，通过调用API函数获得当前模组深度质量，返回值会提示当前深度质量是否合格。如果不合格可以调用其他提供的函数，进行重新标定，并写入模组。

# 函数说明
请参考 inuDepth.h，inuDepth.cpp, demoInuAutoCalib.cpp 文件
|函数|参数|说明|
|-|-|-|
TestCalibration| | 获取当前深度质量，如果质量欠佳，返回 eTestCalibrationCompletedCalibrationIsRequired|
|GenerateCalibration| |当质量欠佳时，进行动态标定，生成新的标定文件，成功返回 eGenerateCalibrationCompletedSuccessfully |
|ApplyCalibration||使用新的标定文件来验证效果，sensor需要重新初始化|
|EraseCalibration||擦除模组里的旧标定文件|
|FlashCalibration||把新标定文件写入模组|

# 使用说明

  


# English Version:
