from kokoro import KPipeline
import torch

p = KPipeline('j')
voice = p.load_voice('jf_nezumi')
print(f'Voice shape: {voice.shape}')

input_ids = torch.tensor([[0, 53, 57, 0]])

# 测试 [1, 256]
with torch.no_grad():
    wav1, dur1 = p.model.forward_with_tokens(input_ids, voice[0, 0, :].unsqueeze(0), 1.0)
    print(f'✅ Works with [1, 256]: wav shape {wav1.shape}')

# 测试 [510, 256]
try:
    with torch.no_grad():
        wav2, dur2 = p.model.forward_with_tokens(input_ids, voice[:, 0, :], 1.0)
        print(f'✅ Works with [510, 256]: wav shape {wav2.shape}')
except Exception as e:
    print(f'❌ Failed with [510, 256]: {e}')
