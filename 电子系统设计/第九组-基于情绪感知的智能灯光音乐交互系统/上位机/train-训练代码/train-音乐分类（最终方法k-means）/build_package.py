# -*- mode: python ; coding: utf-8 -*-
import os
import sys
block_cipher = None

# 需添加的关键配置部分
def get_resource_path():

    return ['.', sys._MEIPASS] if hasattr(sys, '_MEIPASS') else ['.']

# 主分析配置
a = Analysis(
    ['ui_pred.py'],  # 主入口文件
    pathex=get_resource_path(),  # 自定义资源路径
    binaries=[],
    datas=[
        # 模型文件必须明确包含
        ('music_kmeans_model.pkl', '.'),
        # 包含训练模块（因为predict.py依赖train.py）
        ('train.py', '.'),
        # 包含Librosa的示例音频文件（重要！）
        (os.path.join(os.path.dirname(__file__), 'librosa', 'util', 'example_data'), 'example_data')
    ],
    hiddenimports=[
        'sklearn.utils._weight_vector',
        'sklearn.neighbors._partition_nodes',
        'sklearn.metrics._pairwise_distances_reduction',
        'librosa.util.exceptions',  # librosa的隐式依赖
        'threading',  # 确保多线程支持
        'joblib'  # sklearn的并行处理依赖
    ],
    hookspath=[],
    hooksconfig={
        'PySide2': {
            'cmake': 'auto'
        }
    },
    runtime_hooks=[],
    excludes=['matplotlib'],  # 如果没有绘图需求可排除
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False
)

# 处理Tkinter依赖
if sys.platform == 'darwin':
    a.binaries += [('Tcl', '/usr/local/opt/tcl-tk/lib/libtcl8.6.dylib', 'TCL'),
                  ('Tk', '/usr/local/opt/tcl-tk/lib/libtk8.6.dylib', 'TK')]
elif sys.platform == 'win32':
    a.binaries += [('tcl86t.dll', 'C:/Python3*/DLLs/tcl86t.dll', 'BINARY'),
                  ('tk86t.dll', 'C:/Python3*/DLLs/tk86t.dll', 'BINARY')]

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='MusicEmotionClassifier',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,  # 使用UPX压缩（需安装upx）
    runtime_tmpdir=None,
    console=True,  # 控制台格式
    # icon='app_icon.ico',  # 需提前准备的图标文件
    version='version_info.txt',  # 版本信息文件（可选）
)
