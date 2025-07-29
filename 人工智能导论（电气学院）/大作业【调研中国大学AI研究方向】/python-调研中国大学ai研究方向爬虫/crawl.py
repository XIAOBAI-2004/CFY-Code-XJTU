import json
from scholarly import scholarly
import time
import re
from collections import defaultdict, Counter

from nltk.corpus import stopwords
from nltk.tokenize import word_tokenize
import nltk

nltk.download('punkt_tab')
nltk.download('punkt')
nltk.download('stopwords')

# 初始化NLTK
nltk.download('punkt')
nltk.download('stopwords')

# 中国主要大学列表
UNIVERSITIES = {
    # 原有条目
    "Tsinghua University": ["清华", "Tsinghua"],
    "Peking University": ["北大", "Peking", "PKU"],
    "Fudan University": ["复旦", "Fudan"],
    "Zhejiang University": ["浙大", "Zhejiang", "ZJU"],
    "Shanghai Jiao Tong University": ["上海交通", "Shanghai Jiao Tong", "SJTU"],

    # 新增条目
    "Nanjing University": ["南大", "NJU"],
    "University of Science and Technology of China": ["中科大", "USTC"],
    "Xi'an University of Electronic Science and Technology": ["西电", "XDU"],
    "Harbin Institute of Technology": ["哈工大", "HIT"],
    "Huazhong University of Science and Technology": ["华科", "HUST"],
    "Southeast University": ["东大", "SEU"],
    "Xi'an Jiaotong University": ["西交", "XJTU"],
    "Beihang University": ["北航", "BUAA"],
    "Beijing University of Posts and Telecommunications": ["北邮", "BUPT"],
    "Renmin University of China": ["人大", "RUC"],
    "Central South University": ["中南", "CSU"],
    "Northwestern Polytechnical University": ["西工大", "NPU"],
    "Wuhan University": ["武大", "WHU"],
    "Sun Yat-sen University": ["中大", "SYSU"],
    "Sichuan University": ["川大", "SCU"],
    "Tongji University": ["同济", "TJU"],
    "Shandong University": ["山大", "SDU"],
    "Jilin University": ["吉大", "JLU"],
    "South China University of Technology": ["华工", "SCUT"],
    "Beijing Jiaotong University": ["北交", "BJTU"],
    "Beijing Institute of Technology": ["北理工", "BIT"],
    "Nanjing University of Aeronautics and Astronautics": ["南航", "NUAA"],
    "Tianjin University": ["天大", "TJU"],
    "Nankai University": ["南开", "NKU"],

    # 补充英文全称不同的院校
    "University of Electronic Science and Technology of China": ["电子科大", "UESTC"],
    "Beijing University of Aeronautics and Astronautics": ["北航", "Beihang"]  # 北航的另一种英文写法
}

# 搜索关键词列表
KEYWORDS = ["AI", "machine learning", "deep learning","large language models", "generative AI", "multimodal learning",
    "explainable AI", "AI alignment", "AI safety", "quantum machine learning","neural networks", "computer vision", "natural language processing", 
    "reinforcement learning", "transfer learning", "federated learning",
    "self-supervised learning", "contrastive learning", "meta learning",
    "强化学习", "自然语言处理", "计算机视觉",]

def extract_keywords(text, top_n=5):
    """从文本提取关键词"""
    custom_stopwords = set(stopwords.words('english')).union({
        'using', 'based', 'approach', 'method', 'learning',
        'model', 'network', 'via', 'new', 'results', 'show'
    })
    text = re.sub(r'[^a-zA-Z]', ' ', text.lower())
    words = [word for word in word_tokenize(text) 
             if word not in custom_stopwords and len(word) > 2]
    return [word for word, _ in Counter(words).most_common(top_n)]




def fetch_researcher_data(researcher, max_papers=1):
    """获取单个研究者的完整数据"""
    try:
        researcher = scholarly.fill(researcher)
        time.sleep(1)
        
        papers = []
        for pub in researcher.get('publications', [])[:max_papers]:
            try:
                pub = scholarly.fill(pub)
                time.sleep(1)
                
                papers.append({
                    'title': pub['bib'].get('title'),
                    'year': pub['bib'].get('year'),
                    'venue': pub['bib'].get('venue'),
                    'keywords': extract_keywords(pub['bib'].get('title', ''))
                })
                print("论文标题")
                print(pub['bib'].get('title'))
                print("关键词")
                print(papers[-1]['keywords'])    
                print("研究兴趣")
                print(researcher.get('interests', []))
            except Exception as e:
                print(f"论文处理错误: {str(e)}")
        
        return {
            'name': researcher['name'],
            'scholar_id': researcher['scholar_id'],
            'affiliation': researcher.get('affiliation'),
            'citedby': researcher.get('citedby', 0),
            'interests': researcher.get('interests', []),
            'papers': papers
        }
    except Exception as e:
        print(f"研究者数据处理错误: {str(e)}")
        return None

def build_university_research_map(max_researchers=5, max_papers=1):
    """构建按大学分类的研究数据"""
    research_map = defaultdict(list)
    
    for uni, aliases in UNIVERSITIES.items():
        print(f"\n正在处理 {uni}...")
        
        for keyword in KEYWORDS:
            print(f"正在处理大学: {uni}")
            print("正在搜索关键词:", keyword)
            try:
                query = f"{keyword} {uni}"
                search_query = scholarly.search_author(query)
                
                for _ in range(max_researchers):
                    try:
                        researcher = next(search_query)
                        data = fetch_researcher_data(researcher, max_papers)
                        if data:
                            research_map[uni].append(data)
                    except StopIteration:
                        break
                    except Exception as e:
                        print(f"研究者获取错误: {str(e)}")
                
                time.sleep(1)  # 每个关键词间隔1分钟
            except Exception as e:
                print(f"搜索错误 {uni}-{keyword}: {str(e)}")
    
    return dict(research_map)

def save_to_json(data, filename="university_research_data.json"):
    """保存为JSON文件"""
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    print(f"数据已保存到 {filename}")

# 主程序
if __name__ == "__main__":
    
    # 获取并保存数据
    research_data = build_university_research_map()
    save_to_json(research_data)
