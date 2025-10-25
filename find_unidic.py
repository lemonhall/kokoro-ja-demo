#!/usr/bin/env python3
"""查找 UniDic 词典位置"""

try:
    import unidic
    print(f"✅ unidic 已安装")
    print(f"📁 词典路径: {unidic.DICDIR}")
    
    import os
    if os.path.exists(unidic.DICDIR):
        print(f"\n📊 词典目录内容:")
        for item in os.listdir(unidic.DICDIR):
            path = os.path.join(unidic.DICDIR, item)
            size = os.path.getsize(path) if os.path.isfile(path) else 0
            size_mb = size / 1024 / 1024
            print(f"  - {item:30s} {size_mb:>10.2f} MB")
    else:
        print("⚠️ 词典目录不存在")
        
except ImportError:
    print("❌ unidic 未安装")

print("\n" + "="*60)

try:
    import unidic_lite
    print(f"✅ unidic-lite 已安装")
    print(f"📁 词典路径: {unidic_lite.DICDIR}")
    
    if os.path.exists(unidic_lite.DICDIR):
        print(f"\n📊 词典目录内容:")
        for item in os.listdir(unidic_lite.DICDIR):
            path = os.path.join(unidic_lite.DICDIR, item)
            size = os.path.getsize(path) if os.path.isfile(path) else 0
            size_mb = size / 1024 / 1024
            print(f"  - {item:30s} {size_mb:>10.2f} MB")
    
except ImportError:
    print("❌ unidic-lite 未安装")
