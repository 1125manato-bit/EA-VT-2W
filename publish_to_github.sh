#!/bin/bash

# EA VT-2W GitHub Publish Script

# 1. リポジトリの準備
REPO_NAME="EA-VT-2W"
echo "GitHubにリポジトリ '$REPO_NAME' を作成してください (https://github.com/new)"
read -p "作成したリポジトリのURLを入力してください (例: https://github.com/ユーザー名/EA-VT-2W.git): " REPO_URL

# 2. リモートの設定
cd /Users/manato/.gemini/antigravity/scratch/vt2w-white
git remote remove origin 2>/dev/null
git remote add origin "$REPO_URL"
git branch -M main

# 3. プッシュの実行
echo "GitHubにプッシュしています..."
git push -u origin main

echo ""
echo "完了しました！"
echo "GitHubのリポジトリページで 'Actions' タブを確認してください。"
