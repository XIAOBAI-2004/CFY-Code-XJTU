import json
import os
from wordcloud import WordCloud
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
import pandas as pd

def generate_wordclouds(input_file, output_folder):
    """
    根据JSON文件为每个学校生成词云并保存
    
    参数:
        input_file: 输入的JSON文件路径
        output_folder: 输出文件夹路径
    """
    # 确保输出文件夹存在
    os.makedirs(output_folder, exist_ok=True)
    
    # 加载数据
    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 为每个学校生成词云
    for school, interests in data.items():
        # 准备词云数据（权重字典）
        word_weights = {item['interest']: item['normalized_importance'] for item in interests}
        
        # 创建词云对象
        wc = WordCloud(
            width=1200,
            height=800,
            background_color='white',
            colormap='viridis',  # 使用viridis颜色映射
            max_words=200,
            prefer_horizontal=0.8,
            min_font_size=10,
            max_font_size=200,
            relative_scaling=0.5
        )
        
        # 生成词云
        wc.generate_from_frequencies(word_weights)
        
        # 创建图形
        plt.figure(figsize=(12, 12))
        plt.imshow(wc, interpolation='bilinear')
        plt.title(f"Research Interests Distribution of {school}", 
          fontsize=25, 
          pad=15,
          fontdict={
              'fontname': 'Times New Roman',  # 使用Times New Roman字体
              'fontstyle': 'italic',          # 斜体效果
              'fontweight': 'bold',           # 加粗效果
              'color': 'darkblue'             # 可选：设置标题颜色
          })

        plt.axis('off')
        
        # 保存词云
        filename = f"{school.replace(' ', '_')}_wordcloud.png"
        output_path = os.path.join(output_folder, filename)
        plt.savefig(output_path, bbox_inches='tight', dpi=300)
        plt.close()
        
        print(f"已生成 {school} 的词云: {output_path}")

def save_to_excel(input_file, output_excel):
    """
    将分析结果保存为Excel文件（每个学校一个工作表）
    """
    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)

    with pd.ExcelWriter(output_excel, engine='openpyxl') as writer:
        for school, interests in data.items():
            df = pd.DataFrame(interests)
            df['学校'] = school
            df.to_excel(
                writer,
                sheet_name=school[:30],
                index=False,
                columns=['学校', 'interest', 'normalized_importance', 'count', 'centrality']
            )
    print(f"数据已保存到 {output_excel}")

if __name__ == "__main__":
    # 配置参数
    input_json = "university_research_analysis.json"  # 输入JSON文件
    output_dir = "wordclouds"  # 输出文件夹
    
    # 生成词云
    generate_wordclouds(input_json, output_dir)
    # 保存在excel里面
    save_to_excel(input_json, "university_research_analysis.xlsx")
    
    print(f"\n所有词云已保存到 {output_dir} 文件夹")
