import json
from collections import defaultdict
import numpy as np
from sentence_transformers import SentenceTransformer, util

# 初始化模型（全局加载一次）
model = SentenceTransformer('all-MiniLM-L6-v2')

# 常见AI缩写到全称的映射表
ABBREVIATION_MAP = {
    "AI": "Artificial Intelligence",
    "XAI": "Explainable Artificial Intelligence",
    "NLP": "Natural Language Processing",
    "CV": "Computer Vision",
    "ML": "Machine Learning",
    "DL": "Deep Learning",
    "RL": "Reinforcement Learning",
    "LLM": "Large Language Model",
    "GAN": "Generative Adversarial Network",
    "CNN": "Convolutional Neural Network",
    "RNN": "Recurrent Neural Network",
    "LSTM": "Long Short-Term Memory",
    "IoT": "Internet of Things",
    "AIoT": "Artificial Intelligence of Things",
    "VR": "Virtual Reality",
    "AR": "Augmented Reality",
    "MR": "Mixed Reality"
}

def expand_abbreviations(interest):
    """将兴趣中的缩写转换为全称"""
    normalized = interest.strip().title()
    for abbrev, full in ABBREVIATION_MAP.items():
        if abbrev.lower() == normalized.lower():
            return full
        if abbrev.lower() in normalized.lower().split():
            normalized = normalized.replace(abbrev, full)
    return normalized

def calculate_semantic_importance(interests):
    """
    计算研究兴趣的语义重要性得分（保留所有中间分数）
    
    参数:
        interests: 兴趣列表 [{"interest": str, "count": int}, ...]
        
    返回:
        [
            {
                "interest": str,
                "count": int,
                "centrality": float,  # 语义中心性
                "normalized_count": float,  # 标准化频次
                "raw_importance": float,  # 原始重要性得分
                "normalized_importance": float  # 归一化后重要性(0-1)
            },
            ...
        ]
    """
    if not interests or len(interests) == 1:
        for item in interests:
            item.update({
                "centrality": 1.0,
                "normalized_count": 1.0,
                "raw_importance": 1.0,
                "normalized_importance": 1.0
            })
        return interests
    
    # 准备数据和权重
    interest_texts = [item["interest"] for item in interests]
    counts = np.array([item["count"] for item in interests])
    
    # 获取嵌入向量
    embeddings = model.encode(interest_texts, convert_to_tensor=True)
    
    # 计算相似度矩阵
    sim_matrix = util.cos_sim(embeddings, embeddings).cpu().numpy()
    
    # 计算各项指标
    centrality_scores = np.sum(sim_matrix, axis=1)  # 语义中心性
    normalized_counts = counts / np.sum(counts)     # 标准化频次
    raw_importance = 0.6 * centrality_scores + 0.4 * normalized_counts * len(interests)
    
    # 归一化到0-1范围
    norm_importance = (raw_importance - np.min(raw_importance)) / (
        np.max(raw_importance) - np.min(raw_importance))
    
    # 构建完整结果
    results = []
    for i, item in enumerate(interests):
        results.append({
            "interest": item["interest"],
            "count": int(item["count"]),
            "centrality": float(centrality_scores[i]),
            "normalized_count": float(normalized_counts[i]),
            "raw_importance": float(raw_importance[i]),
            "normalized_importance": float(norm_importance[i])
        })
    
    return sorted(results, key=lambda x: -x["normalized_importance"])

def analyze_school_interests(json_data):
    """
    完整的学校研究兴趣分析流程
    返回包含所有评分细节的结果
    """
    school_interests = defaultdict(lambda: defaultdict(int))
    
    # 收集和标准化兴趣
    for school, researchers in json_data.items():
        for researcher in researchers:
            if 'interests' in researcher and researcher['interests']:
                for interest in researcher['interests']:
                    full_interest = expand_abbreviations(interest)
                    school_interests[school][full_interest] += 1
    
    # 计算语义重要性
    result = {}
    for school, interests in school_interests.items():
        interest_list = [{"interest": k, "count": v} for k, v in interests.items()]
        result[school] = calculate_semantic_importance(interest_list)
    
    return result

def print_detailed_results(results):
    """详细打印分析结果"""
    print("各学校研究兴趣详细分析报告：")
    for school, interests in results.items():
        print(f"\n▍ {school} (共{len(interests)}个研究方向)")
        print(f"{'排名':<5}{'研究方向':<40}{'频次':<8}{'中心性':<10}{'标准化频次':<12}{'重要性得分':<12}")
        print("-"*85)
        
        for i, item in enumerate(interests[:], 1):  # 只显示前10个重要方向
            print(f"{i:<5}{item['interest'][:38]:<40}"
                  f"{item['count']:<8}"
                  f"{item['centrality']:.3f}{'':<7}"
                  f"{item['normalized_count']:.3f}{'':<8}"
                  f"{item['normalized_importance']:.3f}")
        
        if len(interests) > 10:
            print(f"... 和{len(interests)-10}个其他研究方向")

# 使用示例
if __name__ == "__main__":
    def save_results_to_json(results, output_file):
        """将分析结果保存为JSON文件"""
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(results, f, indent=2, ensure_ascii=False)
        print(f"分析结果已保存到 {output_file}")
    
    with open('university_research_data.json', 'r', encoding='utf-8') as f:
        data = json.load(f)
#     interests_summary = analyze_research_interests(data)
    # 完整分析流程
    analyzed_results = analyze_school_interests(data)
    save_results_to_json(analyzed_results, 'university_research_analysis.json')
    
    # 打印详细结果
    print_detailed_results(analyzed_results)
    
    # 也可以直接访问完整数据（包含所有评分细节）
    print("\n完整数据结构示例：")
    first_school = list(analyzed_results.keys())[0]
    print(f"{first_school}的第一个研究方向详情：")
    print(json.dumps(analyzed_results[first_school][0], indent=2))

