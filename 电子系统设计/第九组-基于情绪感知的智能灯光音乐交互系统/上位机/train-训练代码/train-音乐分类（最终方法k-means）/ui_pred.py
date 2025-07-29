import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import threading
import os
import logging
from predict import predict, simple_emotion_map
from tkinter import font
import tkinter.ttk as ttk

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