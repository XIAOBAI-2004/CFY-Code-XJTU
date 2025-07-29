怎么用？
先把里面的.json文件删了，挂梯子（因为上google），运行build.sh
终端运行的命令全部走VPN的代理：
set HTTP_PROXY=http://127.0.0.1:7890
set HTTPS_PROXY=http://127.0.0.1:7890

各文件作用说明：
build.sh：自动化脚本，控制整个流程
crawl.py：爬取数据，生成 university_research_data.json
process.py：分析数据，生成 university_research_analysis.json
draw.py：生成词云图，保存到 wordclouds 目录

excel表格：
1. centrality（语义中心性）
衡量一个研究方向在语义网络中的“中心地位”。
如果某研究方向（如“深度学习”）与列表中其他方向的语义相似度高，说明它处于研究的核心交叉领域。
2. normalized_importance（归一化重要性）---“综合评分”
核心含义：综合频次和语义中心性的重要性得分，归一化到 0-1 范围。


安装依赖：
pip install scholarly nltk sentence-transformers wordcloud matplotlib numpy Pillow

自定义配置：
1.修改大学列表：在 crawl.py 的 UNIVERSITIES 字典中增减学校。
2.修改关键词：在 crawl.py 的 KEYWORDS 列表中调整搜索关键词。
3.调整词云样式：在 draw.py 中修改 WordCloud 参数（如颜色、字体）。
