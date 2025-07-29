import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import threading
import os
import logging
from tkinter import font
import tkinter.ttk as ttk
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
        max_workers = min(16, (os.cpu_count() or 1) * 1)
    
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

# def get_mp3_files(path):
#     """获取目录下所有MP3文件"""
#     return [f for f in listdir(path) 
#             if isfile(join(path, f)) and f.lower().endswith('.mp3')]




class MusicEmotionClassifierUI:
    def __init__(self, master):
        self.master = master
        master.title("音乐情感分析系统")
        master.geometry("1000x900")
        master.configure(bg='#F0F3F5')

        self.style = ttk.Style()
        self.style.theme_use('clam')

        # 自定义颜色方案
        self.style.configure('TFrame', background='#F0F3F5')
        self.style.configure('TLabelFrame', background='#F0F3F5', foreground='#2D3436',
                             font=('微软雅黑', 10, 'bold'), relief='flat')
        self.style.configure('TButton', font=('微软雅黑', 10), padding=6)
        self.style.map('TButton',
                       foreground=[('active', '#FFFFFF'), ('!active', '#2D3436')],
                       background=[('active', '#0984E3'), ('!active', '#74B9FF')])

        # Treeview样式
        self.style.configure('Treeview',
                             font=('微软雅黑', 10),
                             rowheight=25,
                             background='#FFFFFF',
                             fieldbackground='#FFFFFF')
        self.style.map('Treeview', background=[('selected', '#74B9FF')])

        # 进度条样式
        self.style.configure("custom.Horizontal.TProgressbar",
                             troughcolor='#DFE6E9',
                             background='#00B894',
                             thickness=20)

        self.results = {emotion: [] for emotion in simple_emotion_map.values()}
        self.create_widgets()
        #logging.basicConfig(filename='app.log', level=logging.ERROR)
        self.logger = logging.getLogger('MusicEmotion')
        self.logger.setLevel(logging.DEBUG)

        # 创建文件处理器
        try:
            fh = logging.FileHandler(resource_path('app.log'), mode='w', encoding='utf-8')
        except Exception as e:
            # 若无法创建日志文件则启用备用控制台日志
            fh = logging.StreamHandler()
            self.logger.warning(f"无法创建日志文件: {str(e)}")

        # 格式设置
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        fh.setFormatter(formatter)
        self.logger.addHandler(fh)

    def create_widgets(self):
        # 文件操作区域 - 使用新的布局结构
        header_frame = ttk.Frame(self.master)
        header_frame.pack(pady=20, padx=20, fill='x')

        control_panel = ttk.Frame(header_frame)
        control_panel.pack(side='left', fill='y')

        # 使用现代图标风格按钮
        self.btn_add = ttk.Button(control_panel, text="➕ 添加文件", command=self.add_files, style='TButton')
        self.btn_add.pack(side='left', padx=5)

        self.btn_clear = ttk.Button(control_panel, text="🗑️ 清空列表", command=self.clear_list)
        self.btn_clear.pack(side='left', padx=5)

        # 文件列表容器
        list_container = ttk.LabelFrame(self.master, text="待分析文件列表")
        list_container.pack(pady=10, padx=20, fill='both', expand=True)

        # 文件列表样式优化
        self.listbox = tk.Listbox(list_container,
                                  selectmode=tk.EXTENDED,
                                  bg='#FFFFFF',
                                  font=('微软雅黑', 10),
                                  relief='flat',
                                  highlightthickness=0)
        self.listbox.pack(fill='both', expand=True, padx=5, pady=5)

        # 分析控制面板
        analysis_control = ttk.Frame(self.master)
        analysis_control.pack(pady=15, padx=20, fill='x')

        self.btn_analyze = ttk.Button(analysis_control, text="🔍 开始分析", command=self.start_analysis)
        self.btn_analyze.pack(side='left')

        self.progress = ttk.Progressbar(analysis_control,
                                        style="custom.Horizontal.TProgressbar",
                                        mode="determinate")
        self.progress.pack(side='left', padx=10, fill='x', expand=True)

        self.btn_export = ttk.Button(analysis_control, text="📤 导出结果",
                                     command=self.export_results, state="disabled")
        self.btn_export.pack(side='left')

        # 结果展示优化
        result_frame = ttk.LabelFrame(self.master, text="分析结果")
        result_frame.pack(pady=10, padx=20, fill='both', expand=True)

        # 优化Treeview列配置
        self.tree = ttk.Treeview(result_frame,
                                 columns=("emotion", "count", "files"),
                                 show="headings",
                                 style='Treeview')

        # 设置列样式
        tree_font = font.Font(family='微软雅黑', size=10)
        self.tree.tag_configure('evenrow', background='#F8F9FA')
        self.tree.tag_configure('oddrow', background='#FFFFFF')

        self.tree.heading("emotion", text="情感分类", anchor="w")
        self.tree.heading("count", text="文件数量", anchor="center")
        self.tree.heading("files", text="文件列表", anchor="w")

        self.tree.column("emotion", width=200, minwidth=150, anchor="w")
        self.tree.column("count", width=120, minwidth=100, anchor="center")
        self.tree.column("files", width=700, minwidth=500, anchor="w")

        # 滚动条样式
        vsb = ttk.Scrollbar(result_frame, orient="vertical", command=self.tree.yview)
        hsb = ttk.Scrollbar(result_frame, orient="horizontal", command=self.tree.xview)
        self.tree.configure(yscrollcommand=vsb.set, xscrollcommand=hsb.set)

        # 网格布局
        self.tree.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")
        hsb.grid(row=1, column=0, sticky="ew")

        result_frame.grid_rowconfigure(0, weight=1)
        result_frame.grid_columnconfigure(0, weight=1)

        # 添加状态栏
        self.status_bar = ttk.Label(self.master,
                                    text="就绪",
                                    anchor='w',
                                    font=('微软雅黑', 9),
                                    background='#DFE6E9',
                                    foreground='#2D3436')
        self.status_bar.pack(side='bottom', fill='x')

    def add_files(self):
        files = filedialog.askopenfilenames(
            title="选择音乐文件",
            filetypes=[("MP3文件", "*.mp3"), ("所有文件", "*.*")]
        )
        for f in files:
            if f not in self.listbox.get(0, tk.END):
                self.listbox.insert(tk.END, f)

    def clear_list(self):
        self.listbox.delete(0, tk.END)
        self.tree.delete(*self.tree.get_children())
        for emotion in self.results:
            self.results[emotion].clear()
        self.btn_export.config(state="disabled")

    def start_analysis(self):
        files = self.listbox.get(0, tk.END)
        if not files:
            messagebox.showwarning("警告", "请先添加要分析的MP3文件！")
            return

        self.btn_analyze.config(state="disabled")
        self.btn_export.config(state="disabled")
        self.progress["value"] = 0
        self.progress["maximum"] = len(files)

        threading.Thread(
            target=self.analyze_files,
            args=(files,),
            daemon=True
        ).start()

    def analyze_files(self, files):
        try:
            valid_paths, predictions = predict(files)

            # 清空结果
            for emotion in self.results:
                self.results[emotion].clear()

            # 处理有效文件
            for idx, (file_path, label) in enumerate(zip(valid_paths, predictions)):
                try:
                    emotion = simple_emotion_map[label]
                    filename = os.path.basename(file_path)
                    self.results[emotion].append(filename)
                except Exception as e:
                    logging.error(f"文件处理失败：{file_path}\n{str(e)}")
                self.master.after(10, self.update_progress, idx + 1)

            # 处理无效文件
            invalid_files = set(files) - set(valid_paths)
            if invalid_files:
                self.master.after(10, lambda: messagebox.showwarning(
                    "部分文件无效",
                    f"以下文件无法分析（请确保时长≥150秒）：\n" + "\n".join(os.path.basename(f) for f in invalid_files)
                ))

            self.master.after(10, self.display_results)

        except ValueError as ve:
            self.master.after(10, lambda: messagebox.showerror("错误", str(ve)))
        except Exception as e:
            #logging.error(f"分析过程出错：{str(e)}")
            if hasattr(self, 'logger') and self.logger:
                self.logger.error(f"分析过程出错：{str(e)}")
            else:
                print(f"分析过程出错（日志不可用）：{str(e)}")  # 备用输出
            self.master.after(10, lambda: messagebox.showerror("错误", f"分析失败：{str(e)}"))
        finally:
            self.master.after(10, lambda: self.btn_analyze.config(state="normal"))

    def update_progress(self, value):
        self.progress["value"] = value
        self.status_bar.config(text=f"处理进度：{value}/{len(self.listbox.get(0, tk.END))}")
        self.master.update_idletasks()

    def display_results(self):
        self.tree.delete(*self.tree.get_children())
        for emotion, files in self.results.items():
            if files:
                # 在 UI 中显示情感分类、文件数量和文件列表
                self.tree.insert("", "end", values=(
                    emotion,
                    len(files),
                    "\n".join(files)
                ))
        self.btn_export.config(state="normal")

    def export_results(self):
        output_path = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("文本文件", "*.txt")]
        )
        if not output_path:
            return

        try:
            with open(output_path, "w", encoding="utf-8") as f:
                for emotion, files in self.results.items():
                    if files:
                        f.write(f"【{emotion}】共{len(files)}个文件：\n")
                        f.write("\n".join(f" - {name}" for name in files))
                        f.write("\n\n")

            messagebox.showinfo("导出成功", f"结果已保存到：\n{output_path}")
            os.startfile(output_path)
        except Exception as e:
            # 增加日志器检查
            if hasattr(self, 'logger') and self.logger:
                self.logger.exception("导出结果失败")  # 会自动记录堆栈
            else:
                traceback.print_exc()  # 当日志不可用时打印到控制台
            messagebox.showerror("导出失败", str(e))


if __name__ == "__main__":
    root = tk.Tk()
    app = MusicEmotionClassifierUI(root)
    root.mainloop()