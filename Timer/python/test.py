import re
from collections import Counter

# 读取日志文件
def read_log_file(filename):
    try:
        with open(filename, 'r') as file:
            return file.readlines()
    except FileNotFoundError:
        print(f"错误：找不到文件 {filename}")
        return []
    except Exception as e:
        print(f"读取文件时发生错误：{e}")
        return []

# 分析线程退出情况
def analyze_threads(log_lines):
    # 正则表达式匹配 "Thread X executed Y operations"
    thread_exit_pattern = re.compile(r"Thread (\d+) executed (\d+) operations")
    
    # 提取所有线程 ID 和操作次数
    thread_ids = []
    for line in log_lines:
        match = thread_exit_pattern.search(line)
        if match:
            thread_id = int(match.group(1))
            ops = int(match.group(2))
            thread_ids.append(thread_id)
    
    # 检查是否有 0-49 的 50 个线程
    expected_threads = set(range(50))  # 期望的线程 ID 集合：0-49
    actual_threads = set(thread_ids)   # 实际出现的线程 ID 集合
    
    # 计算线程数量和缺失的线程
    total_threads = len(actual_threads)
    missing_threads = expected_threads - actual_threads
    extra_threads = actual_threads - expected_threads
    
    # 统计每个线程的操作次数
    ops_counter = Counter(thread_ids)
    
    # 输出分析结果
    print(f"总共检测到 {total_threads} 个线程退出")
    if total_threads == 50 and not missing_threads:
        print("所有 0-49 的 50 个线程均正常退出！")
    else:
        if missing_threads:
            print(f"以下线程未退出：{sorted(missing_threads)}")
        if extra_threads:
            print(f"检测到意外的线程 ID：{sorted(extra_threads)}")
    
    # 打印每个线程的操作次数
    print("\n每个线程的操作次数统计：")
    for tid in sorted(ops_counter.keys()):
        print(f"Thread {tid}: {ops_counter[tid]} 次操作")

    # 检查测试是否完成
    test_completed = any("Test completed" in line for line in log_lines)
    print(f"\n测试是否完成：{'是' if test_completed else '否'}")

# 主函数
def main():
    log_file = "../src/log.txt"
    log_lines = read_log_file(log_file)
    if log_lines:
        analyze_threads(log_lines)

if __name__ == "__main__":
    main()