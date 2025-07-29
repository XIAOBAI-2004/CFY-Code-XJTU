1、音乐部分：包括训练部分train文件夹（包含7百多个音乐片段，空间占用很大，故未放在文件中）和预测部分test文件夹，其中，训练部分不能更改，预测部分可添加或减少音乐文件（建议mp3、wav文件）
2、数据部分：包括static_annotations.csv（后简称annotations）、feature.csv（后简称feature）、Train.csv（后简称Train）三个文件
   annotations与Train中的musicID属性相对应，前者包括V、A值，后者包括V、A值，分类和提取的特征值，而feature只包含提取的特征值
3、代码部分：Music_Emotion_Recognization.ipynb

运行时，要保证 Music_Emotion_Recognization.ipynb 、 Train.csv 、 test文件夹置于同一文件夹目录之下（或者修改程序中的路径，不推荐）

