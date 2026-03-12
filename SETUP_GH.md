# GitHub CLI 认证与推送

## 当前状态
✅ GitHub CLI已安装 (v2.88.0)
✅ SSH密钥已配置 (blog-guang)
✅ Git协议设置为SSH
⏳ 需要GitHub CLI认证

---

## 方案A：使用GitHub Token（推荐，快速）

### 1. 获取Token
访问: https://github.com/settings/tokens/new

配置:
- **Note**: `ansys-cdb-parser-cli`
- **Expiration**: `30 days`
- **Scopes**: 勾选 ✅ `repo` (全部权限)

生成后复制token（形如 `ghp_xxxxxxxxxxxx`）

### 2. 认证并创建仓库
```bash
cd /home/admin/.openclaw/workspace/ansys-cdb-parser

# 方式1：直接认证
echo "YOUR_TOKEN" | gh auth login --with-token

# 方式2：一条命令完成
export GH_TOKEN="YOUR_TOKEN"
gh repo create blog-guang/ansys-cdb-parser \
  --public \
  --source=. \
  --description="A modern C++17 library for parsing ANSYS MAPDL archive (CDB) files" \
  --push
```

---

## 方案B：Web浏览器认证（交互式）

```bash
gh auth login
```

选择:
1. GitHub.com
2. SSH
3. Login with a web browser
4. 复制一次性code
5. 浏览器中粘贴code完成认证

然后运行:
```bash
cd /home/admin/.openclaw/workspace/ansys-cdb-parser
gh repo create blog-guang/ansys-cdb-parser \
  --public \
  --source=. \
  --description="A modern C++17 library for parsing ANSYS MAPDL archive (CDB) files" \
  --push
```

---

## 推送后验证

```bash
# 查看仓库信息
gh repo view blog-guang/ansys-cdb-parser

# 或访问浏览器
# https://github.com/blog-guang/ansys-cdb-parser
```

---

吽总，选择一个方案运行即可！🦐
