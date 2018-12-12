# wake-up-pc

语音唤醒 2016/08/23

##filterbank.cpp

阅读HTK 3.5中fbank特征提取代码
相同流程原理重写
输入为WAV文件，打印输出特征矩阵并返回

##nNet.cpp

实现神经网络矩阵前向运算
输入为特征矩阵，打印输出关键词权重
##test.cpp

测试打印输出
需要WAV文件和神经网络参数文件

