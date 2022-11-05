import subprocess

max_processes = 24
num_of_ex = 100

results = {}

for i in range(1, max_processes + 1):
    local_results = []
    for e in range(num_of_ex):
        p = subprocess.Popen(['./2_4_6.bin', './test_large_file.json', str(i)], stdout=subprocess.PIPE, text=True)
        out, _ = p.communicate()
        res = int(out.split()[1])
        local_results.append(res)
    
    results[i] = sum(local_results) / num_of_ex

print(results)