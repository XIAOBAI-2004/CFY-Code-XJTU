"""
决定采用三个labels:0,1,2
代表
0: 悲伤/忧郁/孤独,
1: 愉悦/欢快/放松,
2: 另类/实验性/失真,
"""


import librosa
import numpy as np
import pandas as pd
import joblib
import os
import threading
from sklearn.cluster import KMeans
from sklearn.preprocessing import MinMaxScaler
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt
from os import listdir
from os.path import isfile, join
from concurrent.futures import ThreadPoolExecutor, as_completed
from tqdm import tqdm
import queue

# 精选的15个关键特征名称
KEY_FEATURE_NAMES = [
    'tempo',
    'chroma_stft_mean',
    'mfcc_mean',
    'zcr_mean',
    'spectral_centroid_mean',
    'rmse_mean',
    'spectral_bandwidth_mean',
    'chroma_cq_mean',
    'mfcc_delta_mean',
    'harmonic_mean',
    'percussive_mean',
    'spectral_rolloff_mean',
    'spectral_contrast_mean',
    'tonnetz_mean',
    'poly_mean'
]

# 全局锁用于线程安全地更新共享变量
print_lock = threading.Lock()

def extract_key_features(song_path):
    """提取15个关键特征"""
    try:
        y, sr = librosa.load(song_path, duration=150)
        S = np.abs(librosa.stft(y))
        
        # 提取基础特征
        tempo, _ = librosa.beat.beat_track(y=y, sr=sr)
        chroma_stft = librosa.feature.chroma_stft(y=y, sr=sr)
        mfcc = librosa.feature.mfcc(y=y, sr=sr)
        zcr = librosa.feature.zero_crossing_rate(y)
        cent = librosa.feature.spectral_centroid(y=y, sr=sr)
        rmse = librosa.feature.rms(y=y)
        spec_bw = librosa.feature.spectral_bandwidth(y=y, sr=sr)
        chroma_cq = librosa.feature.chroma_cqt(y=y, sr=sr)
        mfcc_delta = librosa.feature.delta(mfcc)
        harmonic = librosa.effects.harmonic(y)
        percussive = librosa.effects.percussive(y)
        rolloff = librosa.feature.spectral_rolloff(y=y, sr=sr)
        contrast = librosa.feature.spectral_contrast(S=S, sr=sr)
        tonnetz = librosa.feature.tonnetz(y=y, sr=sr)
        poly = librosa.feature.poly_features(S=S, sr=sr)
        
        # 组织关键特征
        key_features = [
            tempo,
            np.mean(chroma_stft),
            np.mean(mfcc),
            np.mean(zcr),
            np.mean(cent),
            np.mean(rmse),
            np.mean(spec_bw),
            np.mean(chroma_cq),
            np.mean(mfcc_delta),
            np.mean(harmonic),
            np.mean(percussive),
            np.mean(rolloff),
            np.mean(contrast),
            np.mean(tonnetz),
            np.mean(poly)
        ]
        
        return (song_path, key_features, None)
    except Exception as e:
        return (song_path, None, str(e))

def parallel_feature_extraction(song_paths, max_workers=None):
    """使用线程池并行提取特征"""
    if max_workers is None:
        # 默认使用CPU核心数*2的线程数
        max_workers = min(32, (os.cpu_count() or 1) * 2)
    
    features = []
    valid_paths = []
    failed_files = []
    
    # 使用队列线程安全地收集结果
    result_queue = queue.Queue()
    
    def worker(path):
        try:
            result = extract_key_features(path)
            result_queue.put(result)
        except Exception as e:
            result_queue.put((path, None, str(e)))
    
    print(f"开始并行提取特征 (使用 {max_workers} 个线程)...")
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # 提交所有任务
        futures = {executor.submit(worker, path): path for path in song_paths}
        
        # 使用tqdm显示进度
        with tqdm(total=len(song_paths), desc="处理进度") as pbar:
            for future in as_completed(futures):
                path, feat, error = result_queue.get()
                if feat is not None:
                    features.append(feat)
                    valid_paths.append(path)
                else:
                    failed_files.append((os.path.basename(path), error))
                pbar.update(1)
    
    if failed_files:
        with print_lock:
            print("\n处理失败的文件:")
            for filename, error in failed_files:
                print(f"{filename}: {error}")
    
    return features, valid_paths

def load_or_create_kmeans_model(n_clusters, model_path=None):
    """加载或创建K-Means模型"""
    if model_path and os.path.exists(model_path):
        print(f"加载现有模型: {model_path}")
        return joblib.load(model_path)
    else:
        print(f"创建新的K-Means模型，聚类数={n_clusters}")
        return KMeans(n_clusters=n_clusters, random_state=42)

def visualize_clusters(feature_df, labels):
    """Visualize clustering results"""
    try:
        # Reduce dimensions to 2D using PCA
        pca = PCA(n_components=2)
        reduced_features = pca.fit_transform(feature_df)
        
        plt.figure(figsize=(12, 8))
        scatter = plt.scatter(reduced_features[:, 0], reduced_features[:, 1], 
                            c=labels, cmap='viridis', alpha=0.6, s=80)
        plt.colorbar(scatter, label='Cluster Label')
        plt.title("Music Emotion Clustering Visualization (PCA Reduced)", fontsize=14)
        plt.xlabel("Principal Component 1", fontsize=12)
        plt.ylabel("Principal Component 2", fontsize=12)
        plt.grid(True, linestyle='--', alpha=0.6)
        
        # Save visualization
        plt.savefig('music_clusters_visualization.png', dpi=300, bbox_inches='tight')
        print("Cluster visualization saved as music_clusters_visualization.png")
        plt.show()
        
    except Exception as e:
        print(f"Visualization failed: {str(e)}")

def plot_feature_importance(cluster_centers, feature_names):
    """Plot feature importance for each cluster center"""
    plt.figure(figsize=(15, 8))
    for i, center in enumerate(cluster_centers):
        plt.subplot(2, 2, i+1)
        pd.Series(center, index=feature_names).plot(kind='bar', color='skyblue')
        plt.title(f'Cluster {i} Center Feature Values', fontsize=12)
        plt.xticks(rotation=45, ha='right')
        plt.grid(axis='y', linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig('cluster_features_importance.png', dpi=300, bbox_inches='tight')
    plt.show()


# def visualize_clusters(feature_df, labels):
#     """聚类结果可视化"""
#     try:
#         # 使用PCA降维到2D
#         pca = PCA(n_components=2)
#         reduced_features = pca.fit_transform(feature_df)
        
#         plt.figure(figsize=(12, 8))
#         scatter = plt.scatter(reduced_features[:, 0], reduced_features[:, 1], 
#                             c=labels, cmap='viridis', alpha=0.6, s=80)
#         plt.colorbar(scatter, label='聚类标签')
#         plt.title("音乐情感聚类可视化 (基于PCA降维)", fontsize=14)
#         plt.xlabel("主成分1", fontsize=12)
#         plt.ylabel("主成分2", fontsize=12)
#         plt.grid(True, linestyle='--', alpha=0.6)
        
#         # 保存可视化图像
#         plt.savefig('music_clusters_visualization.png', dpi=300, bbox_inches='tight')
#         print("聚类可视化图已保存为 music_clusters_visualization.png")
#         plt.show()
        
#     except Exception as e:
#         print(f"可视化失败: {str(e)}")

# def plot_feature_importance(cluster_centers, feature_names):
#     """绘制各聚类中心的特征重要性"""
#     plt.figure(figsize=(15, 8))
#     for i, center in enumerate(cluster_centers):
#         plt.subplot(2, 2, i+1)
#         pd.Series(center, index=feature_names).plot(kind='bar', color='skyblue')
#         plt.title(f'聚类 {i} 中心特征值', fontsize=12)
#         plt.xticks(rotation=45, ha='right')
#         plt.grid(axis='y', linestyle='--', alpha=0.6)
#     plt.tight_layout()
#     plt.savefig('cluster_features_importance.png', dpi=300, bbox_inches='tight')
#     plt.show()

def cluster_songs(song_paths, n_clusters=3, model_path=None, max_workers=None):
    """
    使用K-Means对歌曲进行聚类分析
    
    参数:
        song_paths (list): 歌曲文件路径列表
        n_clusters (int): 聚类数量，默认为4
        model_path (str, optional): 模型保存路径
        max_workers (int, optional): 最大线程数
        
    返回:
        dict: 包含聚类结果和特征的字典
    """
    # 1. 并行提取所有歌曲的关键特征
    features, valid_paths = parallel_feature_extraction(song_paths, max_workers)
    
    if not features:
        raise ValueError("没有有效的歌曲特征可分析")
    
    # 2. 转换为DataFrame
    feature_df = pd.DataFrame(features, columns=KEY_FEATURE_NAMES)
    
    # 3. 特征归一化
    print("正在进行特征归一化...")
    scaler = MinMaxScaler()
    scaled_features = scaler.fit_transform(feature_df)
    
    # 4. 加载或创建K-Means模型
    kmeans = load_or_create_kmeans_model(n_clusters, model_path)
    
    # 5. 训练模型(如果没有预训练模型)或预测聚类
    if not model_path or not os.path.exists(model_path):
        print("训练K-Means模型中...")
        kmeans.fit(scaled_features)
        if model_path:  # 如果提供了保存路径
            joblib.dump(kmeans, model_path)
            print(f"模型已保存到: {model_path}")
    else:
        print("使用预训练模型进行聚类")
    
    # 6. 预测聚类标签
    print("进行聚类预测...")
    cluster_labels = kmeans.predict(scaled_features)
    
    # 7. 可视化
    print("生成可视化图表...")
    visualize_clusters(feature_df, cluster_labels)
    
    # 8. 分析聚类中心特征
    cluster_centers = kmeans.cluster_centers_
    cluster_features = pd.DataFrame(scaler.inverse_transform(cluster_centers), 
                                  columns=KEY_FEATURE_NAMES)
    
    # 9. 绘制特征重要性
    plot_feature_importance(cluster_centers, KEY_FEATURE_NAMES)
    
    return {
        'song_paths': valid_paths,
        'features': feature_df,
        'cluster_labels': cluster_labels,
        'model': kmeans,
        'cluster_features': cluster_features,
        'scaler': scaler
    }

def interpret_clusters(cluster_features):
    """解释聚类结果的情感含义"""
    interpretations = []
    for i, row in cluster_features.iterrows():
        tempo = row['tempo']
        centroid = row['spectral_centroid_mean']
        zcr = row['zcr_mean']
        
        # if tempo > 120 and centroid > 2000 and zcr > 0.1:
        #     emotion = "快乐/欢快"
        # elif tempo < 90 and centroid < 1500 and zcr < 0.05:
        #     emotion = "悲伤/忧郁"
        # elif tempo > 140 and zcr > 0.15:
        #     emotion = "愤怒/激烈"
        # else:
        #     emotion = "平静/放松"
            
        interpretations.append({
            'cluster': i,
            # 'emotion': emotion,
            'tempo': tempo,
            'spectral_centroid': centroid,
            'zcr': zcr
        })
    
    return pd.DataFrame(interpretations)

def get_mp3_files(path):
    """获取目录下所有MP3文件"""
    return [f for f in listdir(path) 
            if isfile(join(path, f)) and f.lower().endswith('.mp3')]

def main():
    # 1. 获取测试集中的所有MP3文件
    test_dir = "./Dataset"  # 替换为你的音乐目录
    if not os.path.exists(test_dir):
        os.makedirs(test_dir)
        print(f"请将音乐文件放入 {test_dir} 目录中")
        return
    
    song_files = get_mp3_files(test_dir)
    if not song_files:
        print(f"{test_dir} 目录中没有找到MP3文件")
        return
    
    song_paths = [join(test_dir, f) for f in song_files]
    
    # 2. 进行聚类分析
    print("\n开始音乐情感聚类分析...")
    results = cluster_songs(song_paths, n_clusters=3, model_path="music_kmeans_model.pkl")
    
    # 3. 解释聚类结果
    interpretation = interpret_clusters(results['cluster_features'])
    print("\n聚类情感解释:")
    print(interpretation[['cluster', 'tempo']])
    
    # 4. 保存结果
    result_df = pd.DataFrame({
        'song': [os.path.basename(p) for p in results['song_paths']],
        'path': results['song_paths'],
        'cluster': results['cluster_labels'],
        # 'predicted_emotion': [interpretation.loc[label, 'emotion'] for label in results['cluster_labels']]
    })
    
    # 合并特征数据
    feature_df = results['features'].copy()
    feature_df['cluster'] = results['cluster_labels']
    feature_df['song'] = [os.path.basename(p) for p in results['song_paths']]
    
    # 保存到CSV
    result_df.to_csv("song_clusters_results.csv", index=False)
    feature_df.to_csv("song_features_with_clusters.csv", index=False)
    results['cluster_features'].to_csv("cluster_centers_features.csv", index=False)
    
    print("\n分析结果已保存:")
    print("- song_clusters_results.csv: 歌曲聚类结果")
    print("- song_features_with_clusters.csv: 歌曲特征与聚类标签")
    print("- cluster_centers_features.csv: 聚类中心特征值")

if __name__ == "__main__":
    main()
