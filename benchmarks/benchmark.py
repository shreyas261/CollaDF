import pandas as pd
import time

def bechmrk(path):
    print( "\n---------------------------------------------" )
    print( path)
    print( "---------------------------------------------")

    start_total = time.time()

    # 1. Time the CSV Reader
    start_read = time.time()
    df = pd.read_csv("benchmark_data.csv")
    end_read = time.time()
    print(f"Read CSV Time: {end_read - start_read:.4f} seconds")

    # 2. Time a Filter Operation
    start_filter = time.time()
    rich_df = df[df["salary"] > 80000]
    end_filter = time.time()
    print(f"Filter Time: {end_filter - start_filter:.4f} seconds")

    # 3. Time a GroupBy Operation
    start_groupby = time.time()
    grouped = df.groupby("department")["salary"].mean().reset_index()
    end_groupby = time.time()
    print(f"GroupBy Mean Time: {end_groupby - start_groupby:.4f} seconds")

    end_total = time.time()
    
    print("------------------------------------")
    print("Total Pandas " + path + "Time: " + str(end_total - start_total) + "seconds")
    print("------------------------------------\n\n")

if __name__ == "__main__":
    print("Starting Pandas Benchmark...")
    start_total = time.time()
    bechmrk("10M.csv")
    end_total = time.time()
    print(f"Total Pandas Time: {end_total - start_total:.4f} seconds")
    