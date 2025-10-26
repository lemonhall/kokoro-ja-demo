"""
TTS音频对比审听工具
基于Flask的Web界面，用于批量对比C版本和Python版本的TTS音频质量
"""

from flask import Flask, render_template, request, jsonify, send_from_directory
from pathlib import Path
import json
import os

app = Flask(__name__)

# 配置
OUTPUT_DIR = Path("batch_output")
REVIEW_LOG = OUTPUT_DIR / "review_results.json"

def load_reviews():
    """加载已有的审听记录"""
    if REVIEW_LOG.exists():
        with open(REVIEW_LOG, 'r', encoding='utf-8') as f:
            return json.load(f)
    return {}

def save_reviews(reviews):
    """保存审听记录"""
    with open(REVIEW_LOG, 'w', encoding='utf-8') as f:
        json.dump(reviews, f, ensure_ascii=False, indent=2)

def get_test_cases():
    """获取所有测试用例"""
    cases = []
    phoneme_files = sorted(OUTPUT_DIR.glob("*_phonemes.txt"))
    
    for phoneme_file in phoneme_files:
        idx = phoneme_file.name.split('_')[0]
        
        # 读取音素文件内容
        with open(phoneme_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            input_text = lines[0].replace('输入文本: ', '').strip()
            c_phonemes = lines[1].replace('C版本音素: ', '').strip()
            py_phonemes = lines[2].replace('Python版本音素: ', '').strip()
        
        # 查找对应的音频文件
        c_audio = f"{idx}_c_{input_text[:10]}.wav"
        py_audio = f"{idx}_py_{input_text[:10]}.wav"
        
        cases.append({
            'id': idx,
            'input_text': input_text,
            'c_phonemes': c_phonemes,
            'py_phonemes': py_phonemes,
            'c_audio': c_audio,
            'py_audio': py_audio,
            'phoneme_file': phoneme_file.name
        })
    
    return cases

@app.route('/')
def index():
    """主页面"""
    cases = get_test_cases()
    reviews = load_reviews()
    return render_template('reviewer.html', 
                         total_cases=len(cases),
                         reviewed_count=len(reviews))

@app.route('/api/case/<int:index>')
def get_case(index):
    """获取指定索引的测试用例"""
    cases = get_test_cases()
    if 0 <= index < len(cases):
        case = cases[index]
        reviews = load_reviews()
        case['review'] = reviews.get(case['id'], {})
        return jsonify(case)
    return jsonify({'error': 'Invalid index'}), 404

@app.route('/api/review', methods=['POST', 'GET'])
def handle_review():
    """保存或获取审听评价"""
    if request.method == 'POST':
        # 保存评价
        data = request.json
        case_id = data.get('id')
        status = data.get('status')  # 'c_ok', 'c_fail', 'both_fail'
        note = data.get('note', '')
        
        reviews = load_reviews()
        reviews[case_id] = {
            'status': status,
            'note': note,
            'timestamp': data.get('timestamp', '')
        }
        save_reviews(reviews)
        
        return jsonify({'success': True, 'reviewed_count': len(reviews)})
    else:
        # 获取所有评价
        reviews = load_reviews()
        return jsonify(reviews)

@app.route('/audio/<path:filename>')
def serve_audio(filename):
    """提供音频文件"""
    return send_from_directory(OUTPUT_DIR, filename)

@app.route('/api/stats')
def get_stats():
    """获取统计信息"""
    reviews = load_reviews()
    cases = get_test_cases()
    
    stats = {
        'total': len(cases),
        'reviewed': len(reviews),
        'c_ok': sum(1 for r in reviews.values() if r['status'] == 'c_ok'),
        'c_fail': sum(1 for r in reviews.values() if r['status'] == 'c_fail'),
        'both_fail': sum(1 for r in reviews.values() if r['status'] == 'both_fail'),
    }
    
    return jsonify(stats)

if __name__ == '__main__':
    # 确保输出目录存在
    OUTPUT_DIR.mkdir(exist_ok=True)
    
    # 创建templates目录
    templates_dir = Path(__file__).parent / 'templates'
    templates_dir.mkdir(exist_ok=True)
    
    print("=" * 70)
    print("🎧 TTS音频对比审听工具")
    print("=" * 70)
    print(f"📁 音频目录: {OUTPUT_DIR.absolute()}")
    print(f"📝 评价记录: {REVIEW_LOG}")
    print(f"🌐 访问地址: http://127.0.0.1:5000")
    print("=" * 70)
    
    app.run(debug=True, host='127.0.0.1', port=5000)
