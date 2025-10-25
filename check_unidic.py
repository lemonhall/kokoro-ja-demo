import unidic
import os

if hasattr(unidic, 'DICDIR'):
    print(f"✅ UniDic 完整版已安装")
    print(f"📁 词典路径: {unidic.DICDIR}")
    if os.path.exists(unidic.DICDIR):
        print(f"✅ 词典目录存在")
        files = os.listdir(unidic.DICDIR)
        print(f"📊 文件数量: {len(files)}")
        for f in files[:10]:
            print(f"  - {f}")
    else:
        print(f"❌ 词典目录不存在，需要下载")
        print(f"运行: uv run python -m unidic download")
else:
    print("❌ unidic.DICDIR 属性不存在")
