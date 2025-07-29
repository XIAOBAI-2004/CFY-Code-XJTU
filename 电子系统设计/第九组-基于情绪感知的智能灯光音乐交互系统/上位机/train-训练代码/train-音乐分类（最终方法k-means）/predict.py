"""
音乐情感聚类分析
使用KMeans算法对音乐进行情感聚类分析

请保证待测试的歌曲长度尽量至少150s(两分半钟)

"""


from train import load_or_create_kmeans_model, parallel_feature_extraction, extract_key_features ,KEY_FEATURE_NAMES
import pandas as pd
from sklearn.cluster import KMeans
from sklearn.preprocessing import MinMaxScaler
from sklearn.decomposition import PCA
from os import listdir
from os.path import isfile, join
import os
import sys

simple_emotion_map = {
        0: "悲伤/忧郁/孤独",
        1: "愉悦/欢快/放松", 
        2: "另类/实验性/失真" # 抽象
    }
def resource_path(relative_path):
    """ 处理打包后的资源路径 """
    if hasattr(sys, '_MEIPASS'):
        base_path = sys._MEIPASS
    else:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

def predict(mp3_paths):
    print(f"正在处理歌曲：{mp3_paths}")
    features, valid_paths = parallel_feature_extraction(mp3_paths)
    
    if not features:
        raise ValueError("没有有效的歌曲特征可分析")
    
    # 转换为DataFrame
    feature_df = pd.DataFrame(features, columns=KEY_FEATURE_NAMES)
    
    # 特征归一化
    scaler = MinMaxScaler()
    scaled_features = scaler.fit_transform(feature_df)
    
    # 加载模型并预测
    kmeans = load_or_create_kmeans_model(
        n_clusters=4,
        model_path=resource_path("music_kmeans_model.pkl")  # 关键修改
    )
    cluster_labels = kmeans.predict(scaled_features)
    
    for i, label in enumerate(cluster_labels):
        print(f"歌曲：{valid_paths[i]}，聚类标签：{label},情感含义为{simple_emotion_map[label]}")

    return valid_paths, cluster_labels

def get_mp3_files(path):
    """获取目录下所有MP3文件"""
    return [f for f in listdir(path) 
            if isfile(join(path, f)) and f.lower().endswith('.mp3')]

if __name__ == "__main__":
    target_dir = "./testset"
    song_files = get_mp3_files(target_dir)
    song_paths = [join(target_dir, f) for f in song_files]
    
    # 自己选取歌曲进行测试
    predict(song_paths)





"""

准确度很高了...

开始并行提取特征 (使用 32 个线程)...
处理进度: 100%|████████████████████████████████████████████████████████████████████████| 17/17 [01:27<00:00,  5.13s/it]
加载现有模型: music_kmeans_model.pkl
歌曲：./testset\Dead Inside - АДЛИН(另类,实验,phonk,金属).mp3，聚类标签：2,情感含义为另类/实验性/失真
歌曲：./testset\Heavenly Key - Glichery(另类,phonk,金属).mp3，聚类标签：2,情感含义为另类/实验性/失真
歌曲：./testset\太聪明-陈绮贞(忧郁).mp3，聚类标签：0,情感含义为悲伤/忧郁/孤独
歌曲：./testset\champagne problems - Taylor Swift(伤感).mp3，聚类标签：0,情感含义为悲伤/忧郁/孤独
歌曲：./testset\Erik Satie - Gymnopedie.no.1(忧郁).mp3，聚类标签：0,情感含义为悲伤/忧郁/孤独
歌曲：./testset\oor-完全感觉dreamer(欢快).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\安静-周杰伦(忧郁).mp3，聚类标签：0,情感含义为悲伤/忧郁/孤独
歌曲：./testset\遥远的她 - 张学友(忧郁).mp3，聚类标签：0,情感含义为悲伤/忧郁/孤独
歌曲：./testset\陈奕迅 - 陪你度过漫长岁月(欢快).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\小镇姑娘 - 陶喆(另类(存疑);欢快).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\就是爱你-陶喆(欢快).mp3，聚类标签：0,情感含义为悲伤/忧郁/孤独
歌曲：./testset\Lemon - 米津玄師(忧郁,但节奏明朗).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\麦恩莉 - 方大同(忧郁).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\Vaundy - 怪獣の花唄(节奏欢快).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\公路之歌 - 痛仰乐队(欢快,摇滚).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\Baby you - 芝麻Mochi(愉悦).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
歌曲：./testset\APT. - ROSÉ,Bruno Mars(愉悦).mp3，聚类标签：1,情感含义为愉悦/欢快/放松
"""