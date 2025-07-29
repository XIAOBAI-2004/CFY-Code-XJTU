#!/bin/bash

DIR="wordclouds"
if [ -d "$DIR" ]; then
    rm -rf "$DIR"
    echo "目录 $DIR 已删除"
else
    echo "目录 $DIR 不存在"
fi

if [ ! -f "university_research_data.json" ]; then
    python crawl.py
else
    echo "爬取数据已存在，跳过执行crawl.py"
fi

if [ ! -f "university_research_analysis.json" ]; then
    python process.py
else
    echo "数据分析已完成，跳过执行process.py"
fi

python draw.py
if [ -d "wordclouds" ]; then
    echo "结果成功保存wordclouds目录"
fi

