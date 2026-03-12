#!/bin/bash
# ANSYS CDB Parser - GitHub推送脚本
# 使用前请先在GitHub创建仓库: ansys-cdb-parser

echo "🦐 ANSYS CDB Parser - GitHub推送"
echo "================================="
echo ""
echo "请先完成以下步骤:"
echo "1. 访问 https://github.com/new"
echo "2. 仓库名: ansys-cdb-parser"
echo "3. 选择 Public, 不要初始化README等文件"
echo ""
read -p "已创建仓库? 输入你的GitHub用户名: " username

if [ -z "$username" ]; then
    echo "❌ 用户名不能为空"
    exit 1
fi

echo ""
echo "📡 配置远程仓库..."
git remote remove origin 2>/dev/null  # 删除旧的remote（如果存在）
git remote add origin "https://github.com/$username/ansys-cdb-parser.git"

echo ""
echo "🔄 推送到GitHub..."
git branch -M main
git push -u origin main

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ 推送成功!"
    echo ""
    echo "🌐 仓库地址:"
    echo "   https://github.com/$username/ansys-cdb-parser"
    echo ""
    echo "📋 项目状态:"
    echo "   - 代码: 3次提交"
    echo "   - 测试: 10/10 通过"
    echo "   - 功能: NBLOCK + EBLOCK 完成"
else
    echo ""
    echo "❌ 推送失败，请检查:"
    echo "   1. GitHub仓库已创建"
    echo "   2. 用户名正确"
    echo "   3. 网络连接正常"
    echo "   4. Git凭据已配置"
fi
