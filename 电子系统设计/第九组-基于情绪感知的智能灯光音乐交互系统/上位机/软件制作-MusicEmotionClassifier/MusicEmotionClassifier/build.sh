#!/bin/bash

# 定义环境名称
ENV_NAME="music_emotion_classifier"

# 创建 Conda 环境
conda create -n $ENV_NAME python=3.12 -y

# 激活环境
source activate $ENV_NAME

# 安装依赖项
pip install librosa numpy pandas joblib scikit-learn matplotlib tqdm pyinstaller

# 打包为 EXE 文件，关闭控制台
pyinstaller --onefile --add-data "music_kmeans_model.pkl;." -i "ico.ico" ui_pred.py

# 提示完成
echo "打包完成！生成的 EXE 文件在 dist 目录中。"

# 删除 Conda 环境
conda deactivate
conda remove -n $ENV_NAME --all -y

# 提示环境已删除
echo "已删除 Conda 环境：$ENV_NAME"
echo "打包完成！生成的 EXE 文件在 dist 目录中。"