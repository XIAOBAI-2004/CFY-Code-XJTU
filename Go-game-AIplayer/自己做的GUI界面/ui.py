import os
import tkinter as tk

class GoBoardUI:
    def __init__(self, root):
        self.root = root
        self.root.title("围棋棋盘实时展示")
        self.board_size = 5  # 5x5 棋盘
        self.cell_size = 60   # 每个格子的大小
        self.canvas = tk.Canvas(root, width=self.board_size * self.cell_size, 
                                height=self.board_size * self.cell_size, bg="burlywood")
        self.canvas.pack()
        self.current_player = 1  # 当前玩家编号（1 或 2）
        self.board = [[0 for _ in range(self.board_size)] for _ in range(self.board_size)]  # 初始化棋盘
        self.last_modified_time = 0  # 文件最后修改时间
        self.file_path = "input.txt"  # 监控的文件路径
        self.draw_board()
        self.update_ui()

    def draw_board(self):
        """绘制棋盘"""
        for i in range(self.board_size):
            for j in range(self.board_size):
                x1 = j * self.cell_size
                y1 = i * self.cell_size
                x2 = x1 + self.cell_size
                y2 = y1 + self.cell_size
                self.canvas.create_rectangle(x1, y1, x2, y2, outline="black")
                if self.board[i][j] == 1:
                    self.draw_stone(i, j, "black")
                elif self.board[i][j] == 2:
                    self.draw_stone(i, j, "white")

    def draw_stone(self, row, col, color):
        """在指定位置绘制棋子"""
        x = col * self.cell_size + self.cell_size // 2
        y = row * self.cell_size + self.cell_size // 2
        radius = self.cell_size // 2 - 5
        self.canvas.create_oval(x - radius, y - radius, x + radius, y + radius, fill=color)

    def update_ui(self):
        """更新UI界面"""
        self.canvas.delete("all")  # 清空画布
        self.draw_board()

    def load_board_state(self):
        """从 input.txt 文件中加载棋盘状态"""
        if os.path.exists(self.file_path):
            with open(self.file_path, "r") as f:
                lines = f.readlines()
                if lines:
                    self.current_player = int(lines[0].strip())  # 读取当前玩家编号
                    for i in range(self.board_size):
                        row = lines[i + 1].strip()
                        for j in range(self.board_size):
                            self.board[i][j] = int(row[j])
                    self.update_ui()

    def check_file_updates(self):
        """检查文件是否被修改，如果是则更新UI"""
        if os.path.exists(self.file_path):
            modified_time = os.path.getmtime(self.file_path)
            if modified_time != self.last_modified_time:
                self.last_modified_time = modified_time
                self.load_board_state()
        self.root.after(1000, self.check_file_updates)  # 每隔1秒检查一次

if __name__ == "__main__":
    root = tk.Tk()
    ui = GoBoardUI(root)
    ui.check_file_updates()  # 启动文件监控
    root.mainloop()
