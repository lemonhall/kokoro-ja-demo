import unidic
import shutil
import os

src = unidic.DICDIR
dst = 'extracted_data/ja/unidic_dicdir'

print(f"📁 源路径: {src}")
print(f"📁 目标路径: {dst}")

# 删除旧的
if os.path.exists(dst):
    print("🗑️  删除旧目录...")
    shutil.rmtree(dst)

# 复制
print("📦 开始复制...")
shutil.copytree(src, dst)

# 统计
files = os.listdir(dst)
total_size = sum(
    os.path.getsize(os.path.join(dst, f)) 
    for f in files 
    if os.path.isfile(os.path.join(dst, f))
)

print(f"\n✅ 复制完成！")
print(f"📊 文件数量: {len(files)}")
print(f"📊 总大小: {total_size/1024/1024:.1f} MB")
print(f"\n📋 文件列表:")
for f in sorted(files):
    path = os.path.join(dst, f)
    if os.path.isfile(path):
        size_mb = os.path.getsize(path) / 1024 / 1024
        print(f"  {f:20s} {size_mb:>10.2f} MB")
