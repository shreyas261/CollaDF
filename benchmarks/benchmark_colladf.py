import time
import os
import colladf_py as cdf
import pandas as pd

def format_time(t):
    return f"{t:.4f} seconds"

def run_colladf_benchmark(filepath):
    print(f"\n--- CollaDF Benchmark: {filepath} ---")
    
    # 1. IO Read
    t0 = time.time()
    df = cdf.read_csv(filepath)
    t_read = time.time() - t0
    print(f"Read CSV:        {format_time(t_read)} | Shape: {df.shape()}")
    
    # 2. Filter / Masking
    t0 = time.time()
    # Note: df["salary"] returns a single-col DataFrame. 
    # > 80000 returns a fast C++ BoolMask. 
    # df[mask] filters the DataFrame.
    rich_df = df[df["salary"] > 80000]
    t_filter = time.time() - t0
    print(f"Filter (>80k):   {format_time(t_filter)} | Shape: {rich_df.shape()}")
    
    # 3. GroupBy Aggregation
    t0 = time.time()
    grouped = df.groupby("department").mean("salary")
    t_group = time.time() - t0
    print(f"GroupBy (Mean):  {format_time(t_group)}")
    
    print(f"Total CollaDF Time: {format_time(t_read + t_filter + t_group)}")

def run_pandas_benchmark(filepath):
    print(f"\n--- Pandas Benchmark: {filepath} ---")
    
    # 1. IO Read
    t0 = time.time()
    df = pd.read_csv(filepath)
    t_read = time.time() - t0
    print(f"Read CSV:        {format_time(t_read)} | Shape: {df.shape}")
    
    # 2. Filter / Masking
    t0 = time.time()
    rich_df = df[df["salary"] > 80000]
    t_filter = time.time() - t0
    print(f"Filter (>80k):   {format_time(t_filter)} | Shape: {rich_df.shape}")
    
    # 3. GroupBy Aggregation
    t0 = time.time()
    grouped = df.groupby("department")["salary"].mean().reset_index()
    t_group = time.time() - t0
    print(f"GroupBy (Mean):  {format_time(t_group)}")
    
    print(f"Total Pandas Time:  {format_time(t_read + t_filter + t_group)}")


if __name__ == "__main__":
    files_to_test = ["10M.csv", "50M.csv", "100M.csv"]
    
    for f in files_to_test:
        if os.path.exists(f):
            print("="*50)
            run_colladf_benchmark(f)
            run_pandas_benchmark(f)
        else:
            print(f"\nFile {f} not found. Skipping...")
            # If you want to quickly test with a tiny generated file, uncomment below:
            '''
            if f == "10M.csv": 
                print("Generating dummy test file instead...")
                with open("test_dummy.csv", "w") as out:
                    out.write("id,name,department,salary,rating\n")
                    for i in range(1000000):
                        out.write(f"{i},Person,Engineering,{60000 + (i%5)*10000},4.5\n")
                run_colladf_benchmark("test_dummy.csv")
                run_pandas_benchmark("test_dummy.csv")
                os.remove("test_dummy.csv")
            '''